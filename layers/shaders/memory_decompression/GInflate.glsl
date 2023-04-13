/* Copyright (c) 2023 The Khronos Group Inc.
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Ilya Terentiev <iterentiev@nvidia.com>
 * Author: Vikram Kushwaha <vkushwaha@nvidia.com>
 */


#ifdef GDEFLATE_HAVE_INT16
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#define SymT uint16_t
#else
#define SymT uint
#endif

#ifdef GDEFLATE_HAVE_INT64
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#define BuffT uint64_t
#else
#define BuffT uvec2
#endif

// Number of bits to fetch each refill
#define GDEFLATE_READ_SIZE 32

#define GDEFLATE_ENABLE_DEFLATE64

// Cooperative bit reader
struct BitReader {
	uint base;
	BuffT buf;
	uint cnt;
} g_BitReader;


void BitReader_refillImpl(uint offset, bool init) {
	uint data = Src_ReadAlignedDword(offset);

#ifdef GDEFLATE_HAVE_INT64
	g_BitReader.buf |= uint64_t(data) << g_BitReader.cnt;
#else
	g_BitReader.buf.x |= data << g_BitReader.cnt;
	if (!init) g_BitReader.buf.y |= data >> (32 - g_BitReader.cnt);
#endif

	g_BitReader.cnt += GDEFLATE_READ_SIZE;
}

// Reset bit reader - assume base pointer is word-aligned
void BitReader_init(uint base) {
	g_BitReader.cnt = 0;
	g_BitReader.buf = BuffT(0);

	BitReader_refillImpl(base/4 + tid(), true);

	g_BitReader.base = base/4 + NUM_THREADS;
}

// Refill bit buffer if needed and advance shared base pointer
void BitReader_refill(bool _p) {
	bool p = _p && (g_BitReader.cnt < GDEFLATE_READ_SIZE);
	uint ballot = vote(p);
	uint offset = bitCount(ballot&ltMask());
	if (p) {
		BitReader_refillImpl(g_BitReader.base + offset, false);
	}
	g_BitReader.base += bitCount(ballot);
}

// Remove n bits from the bit buffer
void BitReader_eat(uint n, bool p) {
	if (p) {
#ifdef GDEFLATE_HAVE_INT64
		g_BitReader.buf >>= n;
#else
		g_BitReader.buf.x >>= n;
		g_BitReader.buf.x |= g_BitReader.buf.y << (32 - n);
		g_BitReader.buf.y >>= n;
#endif
		g_BitReader.cnt -= n;
	}
	BitReader_refill(p);
}

uint BitReader_peek() {
#ifdef GDEFLATE_HAVE_INT64
	return uint(g_BitReader.buf);
#else
	return g_BitReader.buf.x;
#endif
}

// Return n bits from the bit buffer without changing reader state (up to 32 bits at a time)
uint BitReader_peek(uint n) { return BitReader_peek() & mask(n); }

// Return n bits from the bit buffer and remove them
uint BitReader_read(uint n, bool p) {
	uint bits = p ? BitReader_peek(n) : 0;
	BitReader_eat(n, p);
	return bits;
}

// Return n bits from the next byte-aligned location and remove them
uint BitReader_readAligned(uint n, bool p) {
	uint bits = uint(g_BitReader.buf >> (g_BitReader.cnt & 7)) & mask(n);
	BitReader_eat(n + (g_BitReader.cnt & 7), p);
	return bits;
}

#if SIMD_WIDTH >= NUM_THREADS
#define IN_REGISTER_DECODER
#endif

// Maintains state of a pair of decoders (in higher and lower numbered threads)
const uint kMaxCodeLen = 15;
struct DecoderPair {
	// Aligned so that both can be indexed with (len-1)
	uint baseCodes;		// Base codes for each code length + sentinel code
	uint offsets;		// Offsets into the symbol table
};

#ifdef IN_REGISTER_DECODER
DecoderPair g_DecoderPair;
#else
shared DecoderPair g_DecoderPair[NUM_THREADS];
#endif

uint DecoderPair_getBaseCodes(uint idx) {
#ifdef IN_REGISTER_DECODER
	return shuffle(g_DecoderPair.baseCodes, idx);
#else
	return g_DecoderPair[idx].baseCodes;
#endif
}

uint DecoderPair_getOffsets(uint idx) {
#ifdef IN_REGISTER_DECODER
	return shuffle(g_DecoderPair.offsets, idx);
#else
	return g_DecoderPair[idx].offsets;
#endif
}

void DecoderPair_setBaseCodes(uint value, uint idx) {
#ifdef IN_REGISTER_DECODER
	g_DecoderPair.baseCodes = value;
#else
	g_DecoderPair[idx].baseCodes = value;
#endif
}

void DecoderPair_setOffsets(uint value, uint idx) {
#ifdef IN_REGISTER_DECODER
	g_DecoderPair.offsets = value;
#else
	g_DecoderPair[idx].offsets = value;
#endif
}

// Build two decoders in parallel
// counts contain a histogram of code lengths in registers
void DecoderPair_init(uint counts, uint maxlen) {
	// Calculate offsets into the symbol table
	DecoderPair_setOffsets(scan16(counts), tid());

#ifndef IN_REGISTER_DECODER
	g_tmp1[tid()] = counts;
	groupMemoryBarrier();
	barrier();
#endif

	// Calculate base codes
	uint baseCode = 0;
	for (uint i = 1; i < maxlen; i++) {
		uint lane = tid() & 15;

#ifndef IN_REGISTER_DECODER
		uint cnt = g_tmp1[(tid() & 16) + i];
#else
		uint cnt = shuffle(counts, (tid() & 16) + i);
#endif

		if (lane >= i) baseCode += cnt << (lane - i);
	}

	// Left-align and fill in sentinel values
	uint lane = tid() & 15;
	uint tmp = baseCode << (32 - lane);
	DecoderPair_setBaseCodes(tmp < baseCode || (lane >= maxlen) ? 0xffffffff : tmp, tid());
}

// Maps a code to its length (base selects decoder)
uint DecoderPair_len4code(uint code, uint base) {
	uint len = 1;
	if (code >= DecoderPair_getBaseCodes(7 + base)) len = 8;
	if (code >= DecoderPair_getBaseCodes(len + 3 + base)) len += 4;
	if (code >= DecoderPair_getBaseCodes(len + 1 + base)) len += 2;
	if (code >= DecoderPair_getBaseCodes(len + base)) len += 1;
	return len;
}

// Maps a code and its length to a symbol id (base selects decoder)
uint DecoderPair_id4code(uint code, uint len, uint base) {
	uint i = len + base - 1;
	return DecoderPair_getOffsets(i) +
		((code - DecoderPair_getBaseCodes(i)) >> (32 - len));
}

shared struct Scratch {
	uint data[NUM_THREADS*2];			// Storage for code length array.
} g_buf;

void Scratch_clear() {
	// Clear first 64 words
	g_buf.data[tid()] = g_buf.data[tid() + NUM_THREADS] = 0;
}

// Returns a nibble of data
uint Scratch_get4b(uint i) {
	return (g_buf.data[i / 8] >> (4 * (i % 8))) & 15;
}

void Scratch_set4b(uint nibbles, uint n, uint i) {
	// Expand nibbles
	nibbles |= (nibbles << 4);
	nibbles |= (nibbles << 8);
	nibbles |= (nibbles << 16);
	nibbles &= ~(int(0xf0000000) >> (28 - n*4));

	uint base = i / 8;
	uint shift = i % 8;

	atomicOr(g_buf.data[base], nibbles << (shift * 4));
	if (shift + n > 8) atomicOr(g_buf.data[base + 1], nibbles >> ((8 - shift) * 4));
}

// Symbol table
const uint kMaxSymbols = 288 + 32;
const uint kDistanceCodesBase = 288;

shared struct SymbolTable {
	SymT symbols[kMaxSymbols];
} g_SymbolTable;

SymT SymbolTable_getSymbol(uint id) {
	return g_SymbolTable.symbols[id];
}

void SymbolTable_setSymbol(uint idx, uint id) {
	g_SymbolTable.symbols[idx] = SymT(id);
}

// Scatter symbols according to in-register lengths and their corresponding offsets
uint SymbolTable_scatter(uint sym, uint len, uint offset) {
	uint mask = match(len);
	if (len != 0)
		SymbolTable_setSymbol(offset + bitCount(mask & ltMask()), sym);
	return mask;
}

// Init symbol table from an array of code lengths in shared memory
// hlit is at least 257
// Assumes offsets contain literal/length offsets in lower numbered threads and distance code offsets in higher-numbered threads
void SymbolTable_init(uint hlit, uint offsets) {
	if (tid() != 15 && tid() != 31) g_tmp[tid() + 1] = offsets;

	sync();

	// 8 unconditional iterations, fully unroll
	for (uint i = 0; i<256/NUM_THREADS; i++) {
		uint sym = i * NUM_THREADS + tid();
		uint len = Scratch_get4b(sym);
		uint match = SymbolTable_scatter(sym, len, g_tmp[len]);
		if (tid() == findLSB(match)) g_tmp[len] += bitCount(match);

		sync();
	}

	// Bounds check on the last iteration for literals
	uint sym = 8 * NUM_THREADS + tid();
	uint len = sym < hlit ? Scratch_get4b(sym) : 0;
	SymbolTable_scatter(sym, len, g_tmp[len]);

	// Scatter distance codes (assumes source array is padded with 0)
	len = Scratch_get4b(tid() + hlit);
	SymbolTable_scatter(tid(), len, kDistanceCodesBase + g_tmp[16 + len]);
}

// Decode a huffman-coded symbol
int DecoderPair_decode(uint bits, out uint len, bool isdist) {
	uint code = bitfieldReverse(bits);

	len = DecoderPair_len4code(code, isdist ? 16 : 0);
	uint idx = DecoderPair_id4code(code, len, isdist ? 16 : 0) + (isdist ? 288 : 0);

	return int(SymbolTable_getSymbol(idx));
}

// Calculate a histogram from in-register code lengths (each thread maps to a symbol)
uint GetHistogram(uint cnt, uint len, uint maxlen) {
	g_tmp[tid()] = 0;

	sync();

	if (len != 0 && tid() < cnt) atomicAdd(g_tmp[len], 1);

	sync();

	return g_tmp[tid() & 15];
}

// Read and sort code length code lengths
uint ReadLenCodes(uint hclen) {
	const uint lane4id[32] = {
		3, 17, 15, 13, 11, 9, 7, 5, 4, 6, 8, 10, 12, 14, 16, 18, 0, 1, 2,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	uint len = BitReader_read(3, tid() < hclen);	// Read reordered code length code lengths in the first hclen threads
	len = shuffle(len, lane4id[tid()]);	// Restore original order
	len &= tid() < 19 ? 0xf : 0;			// Zero-out the garbage

	return len;
}

// Update histograms
// (distance codes are histogrammed in the higher numbered threads, literal/length codes - in lower numbered threads)
void UpdateHistograms(uint len, int i, int n, int hlit) {
	uint cnt = max(min(hlit - i, n), 0);
	if (cnt != 0) atomicAdd(g_tmp[len], cnt);

	cnt = max(min(i + n - hlit, n), 0);
	if (cnt != 0) atomicAdd(g_tmp[16 + len], cnt);
}

// Unpack code lengths and create a histogram of lengths
// Returns a histogram of literal/length code lengths in lower numbered threads,
// and a histogram of distance code lenghts in higher numbered threads
uint UnpackCodeLengths(uint hlit, uint hdist, uint hclen) {
	uint len = ReadLenCodes(hclen);

	// Init decoder
	uint hist = GetHistogram(19, len, 7);
	DecoderPair_init(hist, 7);
	SymbolTable_scatter(tid(), len, DecoderPair_getOffsets(len - 1));

	uint count = hlit + hdist;
	uint baseOffset = 0;
	uint lastlen = ~0;

	// Clear codelens array (4 bit lengths)
	Scratch_clear();
	g_tmp[tid()] = 0;

	sync();

	// Decode code length codes and expand into a shared memory array
	do {
		uint bits = BitReader_peek(7 + 7);

		uint len;
		uint sym = DecoderPair_decode(bits, len, false);

		uint idx = sym <= 15 ? 0 : (sym - 15);
		const uint base[4] = { 1, 3, 3, 11 };
		const uint xlen[4] = { 0, 2, 3, 7 };

		uint n = base[idx] + ((bits >> len) & mask(xlen[idx]));

		// Scan back to find the nearest lane which contains a valid symbol
		uint lane = findMSB(vote(sym != 16) & ltMask());

		uint codelen = sym;
		if (sym > 16) codelen = 0;
		uint prevlen = shuffle(codelen, lane);
		if (sym == 16) codelen = lane == ~0 ? lastlen : prevlen;

		lastlen = broadcast(codelen, NUM_THREADS - 1);
		baseOffset = scan(n) + baseOffset;

		if (baseOffset < count) {
			if (codelen != 0) {
				UpdateHistograms(codelen, int(baseOffset), int(n), int(hlit));
				Scratch_set4b(codelen, n, baseOffset);
			}
		}

		BitReader_eat(len + xlen[idx], baseOffset < count);

		baseOffset = broadcast(baseOffset + n, NUM_THREADS - 1);
	} while (all(baseOffset < count));

	return g_tmp[tid()];
}

void Copy(uint dst, uint dist, uint len, bool p) {
	const uint kMaxWideCopyLen = 16;

	// Fill in copy destinations in the previous round
	uint start = broadcast(dst, 0);
	uint mask = vote(p);

	while (mask != 0) {
		uint lane = findLSB(mask);

		uint offset = broadcast(dist, lane);
		uint length = broadcast(len, lane);
		uint outPos = broadcast(dst, lane);

		for (uint i = tid(); i < length; i += NUM_THREADS) {
			const uint byte = Dst_ReadByte(outPos + i%offset - offset);
			Dst_StoreByte(i + outPos, byte);
		}

		mask &= mask - 1;
	}
}

// Output literals and copies
void CoalesceOutput(uint dst, uint offset, uint dist, uint length, uint byte, bool iscopy) {
	dst += offset;
	// Output literals
	if (!iscopy && length != 0) Dst_StoreByte(dst, byte);

	// Fill in copy destinations
	Copy(dst, dist, length, iscopy);
}

// Translate a symbol to its value
uint TranslateSymbol(int sym, uint len, uint bits, bool isdist, bool p) {
	const uint lut[] = {	// base | bits
		// Distance
		(1<<16)|0, (2<<16)|0, (3<<16)|0, (4<<16)|0, (5<<16)|1, (7<<16)|1, (9<<16)|2, (13<<16)|2,
		(17<<16)|3, (25<<16)|3, (33<<16)|4, (49<<16)|4, (65<<16)|5, (97<<16)|5, (129<<16)|6, (193<<16)|6, 
		(257<<16)|7, (385<<16)|7, (513<<16)|8, (769<<16)|8, (1025<<16)|9, (1537<<16)|9, (2049<<16)|10, (3073<<16)|10,
		(4097<<16)|11, (6145<<16)|11, (8193<<16)|12, (12289<<16)|12, (16385<<16)|13, (24577<<16)|13,
		
#ifdef GDEFLATE_ENABLE_DEFLATE64
		(32769<<16)|14, (49153<<16)|14,
#else				
		0, 0,	// 32 entries
#endif
		// Literal/length
		(1<<16)|0, (0<<16|0), (3<<16)|0, (4<<16)|0, (5<<16)|0, (6<<16)|0, (7<<16)|0, (8<<16)|0, 
		(9<<16)|0, (10<<16)|0, (11<<16)|1, (13<<16)|1, (15<<16)|1, (17<<16)|1, (19<<16)|2, (23<<16)|2,
		(27<<16)|2, (31<<16)|2, (35<<16)|3, (43<<16)|3, (51<<16)|3, (59<<16)|3, (67<<16)|4, (83<<16)|4, 
		(99<<16)|4, (115<<16)|4, (131<<16)|5, (163<<16)|5, (195<<16)|5, (227<<16)|5,

#ifdef GDEFLATE_ENABLE_DEFLATE64
		(3<<16)|16,
#else
		(258<<16)|0,
#endif
		0, 		// 32 entries
	};

	uint lookup = lut[isdist ? sym : max(sym - 255 + 32, 32)];
	uint n = lookup & 0xffff;
	BitReader_eat(len + n, isdist || p);
	return (lookup >> 16) + ((bits >> len) & mask(n));
}

// Assumes code lengths have been stored in the shared memory array
uint CompressedBlock(uint hlit, uint counts, uint dst) {
	DecoderPair_init(counts, 15);
	SymbolTable_init(hlit, DecoderPair_getOffsets(tid()));

#ifdef GDEFLATE_ENABLE_DEFLATE64
	const uint kMaxCodeBits = 15 + 16;
#else
	const uint kMaxCodeBits = 15 + 13;	//+5
#endif

	// Initial round - no copy processing
	uint len = 0;
	int sym = DecoderPair_decode(BitReader_peek(kMaxCodeBits), len, false); // Initial round can't contain distance symbols

	uint eob = vote(sym == 256);
	bool oob = (eob & ltMask()) != 0;

	// Translate current symbol
	uint value = TranslateSymbol(sym, len, BitReader_peek(), false, !oob); // make conditional?

	// Compute output pointers for the current round
	uint length = oob ? 0 : value;
	uint offset = scan(length);

	bool iscopy = sym > 256;			// Copy predicate for the next round
	uint byte = sym;

	// .. for all symbols in the block
	while (eob == 0) {
		sym = DecoderPair_decode(BitReader_peek(kMaxCodeBits), len, iscopy);

		// Set predicates based on the current symbol
		eob = vote(sym == 256);	// end of block symbol
		oob = (eob & ltMask()) != 0;	// true in threads which looked at symbols past the end of the block

		// Translate current symbol
		value = TranslateSymbol(sym, len, BitReader_peek(), iscopy, !oob);
		CoalesceOutput(dst, offset, value, length, byte, iscopy);

		// Advance output pointers
		dst += broadcast(offset + length, NUM_THREADS - 1);

		// Compute output pointers for the current round
		length = (iscopy || oob) ? 0 : value;
		offset = scan(length);

		iscopy = sym > 256;			// Current symbol is a copy, transition to the new state
		byte = sym;
	}

	// One last round of copy processing
	sym = DecoderPair_decode(BitReader_peek(kMaxCodeBits), len, true);

	iscopy = iscopy && !oob;
	uint dist = TranslateSymbol(sym, len, BitReader_peek(), iscopy, false);
	CoalesceOutput(dst, offset, dist, length, byte, iscopy);

	return dst + broadcast(offset + length, NUM_THREADS - 1);	// Advance destination pointer
}

// Uncompressed block (raw copy)
uint UncompressedBlock(uint dst, uint size) {
	uint nrounds = size / NUM_THREADS;

	// Full rounds with no bounds checking
	while (nrounds-- > 0) {
		Dst_StoreByte(dst + tid(), BitReader_read(8, true));
		dst += NUM_THREADS;
	}

	uint rem = size % NUM_THREADS;

	// Last partial round with bounds check
	if (rem != 0) {
		uint byte = BitReader_read(8, tid() < rem);
		if (tid() < rem) Dst_StoreByte(dst + tid(), byte);
		dst += rem;
	}

	return dst;
}

// Initialize fixed code lengths
uint FixedCodeLengths() {
	g_buf.data[tid()] = tid() < 18 ? 0x88888888 : 0x99999999;
	g_buf.data[tid() + 32] = tid() < 3 ? 0x77777777 : (tid() < 4 ? 0x88888888 : 0x55555555);

	return tid() == 7 ? 24 : (tid() == 8 ? 152 : (tid() == 9 ? 112 : tid() == 16 + 5 ? 32 : 0));
}

// This is main entry point for tile decompressor
void DecompressTile() {
	uint dst = g_dstPos;

	// Init bit reader
	BitReader_init(g_srcPos);

	bool done;

	// .. for each block
	do {
		// Read block header and broadcast to all threads
		uint header = broadcast(BitReader_peek(), 0);
		done = extract(header, 0, 1, 0) != 0;

		// Parse block type
		uint btype = extract(header, 1, 2, 0);
		BitReader_eat(3, tid() == 0);

		uint counts, size, hlit, hdist;

		switch (btype) {
		case 2:	// Dynamic huffman block
			hlit = extract(header, 3, 5, 257);
			hdist = extract(header, 8, 5, 1);
			BitReader_eat(14, tid() == 0);
			counts = UnpackCodeLengths(hlit, hdist, extract(header, 13, 4, 4));

		case 1:	// Fixed huffman block
			if (btype == 1) counts = FixedCodeLengths();
			dst = CompressedBlock(btype == 1 ? 288 : hlit, counts, dst);
			break;

		case 0:	// Uncompressed block
			size = broadcast(BitReader_read(16, tid() == 0), 0);
			dst = UncompressedBlock(dst, size);
			break;

		default:
			break; // Should never happen
		}

	} while (!done);
}

#define DECOMPRESS_TILE DecompressTile
