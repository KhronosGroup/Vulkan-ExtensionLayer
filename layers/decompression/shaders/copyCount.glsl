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
 * Author: Vikram Kushwaha <vkushwaha@nvidia.com>
 */

#version 450 core
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout (local_size_x=1,local_size_y=1,local_size_z=1) in;
layout(buffer_reference) buffer BufferRef32 { uint d[]; };

layout(push_constant) uniform PushConstants {
   uvec2 srcAddress;
   uvec2 dstAddress;
} pushConstants;

void main()
{
   // Copy a single integer value (count from indirect address) from srcAddress to dstAddress
   BufferRef32 srcAddr = BufferRef32(pushConstants.srcAddress);
   BufferRef32 dstAddr = BufferRef32(pushConstants.dstAddress);
   dstAddr.d[0] = srcAddr.d[0];
}
