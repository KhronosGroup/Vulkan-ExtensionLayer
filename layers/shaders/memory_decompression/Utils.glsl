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

shared uint g_tmp1[NUM_THREADS];

uint vote(bool p) {
	g_tmp1[tid()] = p ? 1 : 0;
	barrier();

	uint ballot = g_tmp1[0];

	for (uint i = 1; i < NUM_THREADS; i++)
		ballot |= g_tmp1[i] << i;

	barrier();
	return ballot;
}

uint shuffle(uint value, uint idx) {
	g_tmp1[tid()] = value;
	barrier();
	uint res = g_tmp1[idx];
	barrier();
	return res;
}

uint broadcast(uint value, uint idx) {
	barrier();
	if (tid() == idx) g_tmp1[0] = value;
	barrier();
	return g_tmp1[0];
}

bool all(bool p) {
	return vote(p) == (~0 >> (32 - NUM_THREADS));
} 

// Prefix sum
uint scan(uint value) {
	uint tid = tid();

#if SIMD_WIDTH == 16
	uint sum = subgroupExclusiveAdd(value);
	if (tid == SIMD_WIDTH - 1) g_tmp1[0] = sum + value;
	barrier();
	if (tid >= SIMD_WIDTH) sum += g_tmp1[0];
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
	g_tmp1[tid()] = value;
	barrier();

	for (uint i = 0; i < NUM_THREADS; i++)
		mask |= g_tmp1[i] == value ? (1<<i) : 0;

	barrier();
#endif

	return mask;
}
