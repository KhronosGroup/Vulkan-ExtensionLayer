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

#ifdef GDEFLATE_USING_BUFFER_REF

uint Src_ReadAlignedDword(uint index)
{
#ifdef GDEFLATE_BOUNDS_CHECK
    if (index*4 > g_srcSize)
        return 0;
#endif
    return g_src.data[index];
}

uint8_t Dst_ReadByte(uint index)
{
#ifdef GDEFLATE_BOUNDS_CHECK
    if (index > g_dstSize)
        return uint8_t(0);
#endif
    return g_dst.data[index];
}

void Dst_StoreByte(uint index, uint byte)
{
#ifdef GDEFLATE_BOUNDS_CHECK
    if (index > g_dstSize)
        return;
#endif
    g_dst.data[index] = uint8_t(byte);
}

#else

layout(std430, binding = 0) buffer inputLayout { uint g_Input[]; };
layout(std430, binding = 1) buffer outputLayout { uint8_t g_Output[]; };

uint Src_ReadByte(uint offset)
{
    uint offsetMod4 = offset&3;
    offset -= offsetMod4;
    uint shift = offsetMod4<<3;

    return (g_Input[offset/4] >> shift)&0xff;
}

uint Src_ReadAlignedDword(uint offset)
{
    return g_Input[offset];
}

uint Dst_ReadByte(uint offset)
{
    return uint(g_Output[offset]);
}

void Dst_StoreByte(uint offset, uint data)
{
    g_Output[offset] = uint8_t(data);
}

#endif
