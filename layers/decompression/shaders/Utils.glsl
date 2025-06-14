/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 or MIT
 *
 * Licensed under either of
 *   Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 *   or
 *   MIT license (http://opensource.org/licenses/MIT)
 * at your option.
 *
 * Any contribution submitted by you to Khronos for inclusion in this work shall be dual licensed as above.
 *
 * Author: Ilya Terentiev <iterentiev@nvidia.com>
 * Author: Vikram Kushwaha <vkushwaha@nvidia.com>
 */

// Helpers
uint tid() { return gl_LocalInvocationID.x; }
uint mask(uint n) { return (1<<n) - 1; }
uint ltMask() { return mask(tid()); }
uint extract(uint data, uint pos, uint n, uint base) { return ((data >> pos) & mask(n)) + base; }

#if SIMD_WIDTH >= NUM_THREADS

uint vote(bool p) { return subgroupBallot(p).x; }
uint shuffle(uint value, uint idx) { return subgroupShuffle(value, idx); }
uint broadcast(uint value, uint idx) { return subgroupShuffle(value, idx); }
bool all(bool p) { return subgroupAll(p); }
uint scan(uint value) { return subgroupExclusiveAdd(value); }

void sync() {}

#else

// Temp space for group-synchronous operations
shared uint g_tmpGroupSync[NUM_THREADS];

uint vote(bool p) {
	uint tid = tid();

#if SIMD_WIDTH == 16
	g_tmpGroupSync[tid] = subgroupBallot(p).x;
	barrier();

	uint ballot = g_tmpGroupSync[0] | (g_tmpGroupSync[16] << 16);
#else
	g_tmpGroupSync[tid] = int(p) << tid;
	barrier();

	uint ballot = g_tmpGroupSync[0];

	for (uint i = 1; i < NUM_THREADS; i++)
		ballot |= g_tmpGroupSync[i];
#endif

	barrier();
	return ballot;
}

uint shuffle(uint value, uint idx) {
	g_tmpGroupSync[tid()] = value;
	barrier();
	uint res = g_tmpGroupSync[idx&(NUM_THREADS - 1)];
	barrier();
	return res;
}

uint broadcast(uint value, uint idx) {
	if (tid() == idx) g_tmpGroupSync[0] = value;
	barrier();
	uint res = g_tmpGroupSync[0];
	barrier();
	return res;
}

bool all(bool p) {
	return vote(p) == (~0 >> (32 - NUM_THREADS));
} 

// Prefix sum
uint scan(uint value) {
	uint tid = tid();

#if SIMD_WIDTH == 16
	uint sum = subgroupExclusiveAdd(value);
	if (tid == SIMD_WIDTH - 1) g_tmpGroupSync[0] = sum + value;
	barrier();
	if (tid >= gl_SubgroupSize) sum += g_tmpGroupSync[0];
	barrier();
	return sum;
#else
	uint sum = value;

	for (uint i = 1; i < NUM_THREADS; i *= 2) {
		uint s = shuffle(sum, tid - i);
		sum += tid >= i ? s : 0;
	}

	return sum - value;
#endif
}

void sync() {
	barrier();
}

#endif

// Segmented prefix sum
uint scan16(uint value) {
#if SIMD_WIDTH == 16
	return subgroupInclusiveAdd(value);
#else
	for (uint i = 1; i < 16; i *= 2) {
		uint s = shuffle(value, tid() - i);
		value += (tid() & 15) >= i ? s : 0;
	}
#endif

	return value;
}

uint match(uint value) {
	uint mask = 0;

#if SIMD_WIDTH >= NUM_THREADS
	for (uint k = 0; k < NUM_THREADS; k++)
		mask |= (subgroupShuffle(value, k) == value ? 1 : 0) << k;
#else
	g_tmpGroupSync[tid()] = value;
	barrier();

	for (uint i = 0; i < NUM_THREADS; i++)
		mask |= g_tmpGroupSync[i] == value ? (1<<i) : 0;

	barrier();
#endif

	return mask;
}
