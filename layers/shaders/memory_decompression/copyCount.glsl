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
