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

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require

#if defined(GDEFLATE_HAVE_INT16)
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#endif

#if defined(GDEFLATE_HAVE_INT64)
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#endif

#if SIMD_WIDTH > 16
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_vote : require
#extension GL_KHR_shader_subgroup_shuffle : require
#endif

#if SIMD_WIDTH > 8
#extension GL_KHR_shader_subgroup_arithmetic : require
#endif

// Thread blocks are sized to match GDeflate width
#define NUM_THREADS 32

layout(local_size_x = NUM_THREADS, local_size_y = 1) in;

layout(buffer_reference) buffer BufferRef32 { uint data[]; };
layout(buffer_reference) buffer BufferRef8 { uint8_t data[]; };

#if defined(GDEFLATE_INDIRECT_DECOMPRESS)

struct VkDecompressMemoryCommandNV
{
    uvec2 srcAddress;
    uvec2 dstAddress;
    uvec2 compressedSize;
    uvec2 decompressedSize;
    uvec2 decompressionMethod;
};

layout(buffer_reference) buffer BufferRef { VkDecompressMemoryCommandNV data; };

layout(push_constant) uniform constants
{
    uvec2 decompressionParamsAddr;
    uint stride;
} cbParams;

#else

layout(push_constant) uniform constants
{
    uvec2 srcAddress;
    uvec2 dstAddress;
    uvec2 compressedSize;
    uvec2 decompressedSize;
    uvec2 decompressionMethod;
} cbParams;

#endif

// Decompression parameters
BufferRef32 g_src;
BufferRef8  g_dst;
uint g_srcSize;
uint g_dstSize;
uint g_srcPos;
uint g_dstPos;

// Temp space that all methods can use
shared uint g_tmp[NUM_THREADS];

#include "Utils.glsl"
#include "ByteIO.glsl"
#include "GInflate.glsl"

void main()
{
#if defined(GDEFLATE_INDIRECT_DECOMPRESS)
    const uint i = gl_WorkGroupID.x;
    uvec2 paramsAddrs;
    uint carry = 0;
    paramsAddrs.x = uaddCarry(cbParams.decompressionParamsAddr.x, cbParams.stride * i, carry);
    paramsAddrs.y = cbParams.decompressionParamsAddr.y + carry;
    BufferRef paramsAddress = BufferRef(paramsAddrs);
    g_src = BufferRef32(paramsAddress.data.srcAddress);
    g_dst = BufferRef8(paramsAddress.data.dstAddress);
    g_srcSize = uint(paramsAddress.data.compressedSize);
    g_dstSize = uint(paramsAddress.data.decompressedSize);
    g_dstPos = 0;
    g_srcPos = 0;
    DECOMPRESS_TILE();
#else
    g_src = BufferRef32(cbParams.srcAddress);
    g_dst = BufferRef8(cbParams.dstAddress);
    g_srcSize = uint(cbParams.compressedSize);
    g_dstSize = uint(cbParams.decompressedSize);
    g_srcPos = 0;
    g_dstPos = 0;
    DECOMPRESS_TILE();
#endif
    return;
}
