:: Copyright (c) 2023-2025 The Khronos Group Inc.
:: SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
:: SPDX-License-Identifier: Apache-2.0 or MIT
:: 
:: Licensed under either of
::   Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
::   or
::   MIT license (http://opensource.org/licenses/MIT)
:: at your option.
::
:: Any contribution submitted by you to Khronos for inclusion in this work shall be dual licensed as above.
:: 
:: Author: Ilya Terentiev <iterentiev@nvidia.com>
:: Author: Vikram Kushwaha <vkushwaha@nvidia.com>

glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. --vn copyCount -o spirv\copyCount_vk.h copyCount.glsl

glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DNONE --vn kGInflate8 -o spirv\GInflate8_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DGDEFLATE_HAVE_INT16  --vn kGInflate8_HAVE_INT16 -o spirv\GInflate8_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DGDEFLATE_HAVE_INT64  --vn kGInflate8_HAVE_INT64 -o spirv\GInflate8_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kGInflate8_HAVE_INT16_HAVE_INT64 -o spirv\GInflate8_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DNONE --vn kGInflate16 -o spirv\GInflate16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DGDEFLATE_HAVE_INT16  --vn kGInflate16_HAVE_INT16 -o spirv\GInflate16_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DGDEFLATE_HAVE_INT64  --vn kGInflate16_HAVE_INT64 -o spirv\GInflate16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kGInflate16_HAVE_INT16_HAVE_INT64 -o spirv\GInflate16_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DNONE --vn kGInflate32 -o spirv\GInflate32_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DGDEFLATE_HAVE_INT16  --vn kGInflate32_HAVE_INT16 -o spirv\GInflate32_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DGDEFLATE_HAVE_INT64  --vn kGInflate32_HAVE_INT64 -o spirv\GInflate32_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kGInflate32_HAVE_INT16_HAVE_INT64 -o spirv\GInflate32_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DNONE --vn kGInflate64 -o spirv\GInflate64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DGDEFLATE_HAVE_INT16  --vn kGInflate64_HAVE_INT16 -o spirv\GInflate64_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DGDEFLATE_HAVE_INT64  --vn kGInflate64_HAVE_INT64 -o spirv\GInflate64_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kGInflate64_HAVE_INT16_HAVE_INT64 -o spirv\GInflate64_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl

glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DNONE --vn kIndirectGInflate8 -o spirv\IndirectGInflate8_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DGDEFLATE_HAVE_INT16  --vn kIndirectGInflate8_HAVE_INT16 -o spirv\IndirectGInflate8_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate8_HAVE_INT64 -o spirv\IndirectGInflate8_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=8 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate8_HAVE_INT16_HAVE_INT64 -o spirv\IndirectGInflate8_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DNONE --vn kIndirectGInflate16 -o spirv\IndirectGInflate16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DGDEFLATE_HAVE_INT16  --vn kIndirectGInflate16_HAVE_INT16 -o spirv\IndirectGInflate16_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate16_HAVE_INT64 -o spirv\IndirectGInflate16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=16 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate16_HAVE_INT16_HAVE_INT64 -o spirv\IndirectGInflate16_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DNONE --vn kIndirectGInflate32 -o spirv\IndirectGInflate32_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DGDEFLATE_HAVE_INT16  --vn kIndirectGInflate32_HAVE_INT16 -o spirv\IndirectGInflate32_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate32_HAVE_INT64 -o spirv\IndirectGInflate32_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=32 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate32_HAVE_INT16_HAVE_INT64 -o spirv\IndirectGInflate32_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DNONE --vn kIndirectGInflate64 -o spirv\IndirectGInflate64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DGDEFLATE_HAVE_INT16  --vn kIndirectGInflate64_HAVE_INT16 -o spirv\IndirectGInflate64_HAVE_INT16_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate64_HAVE_INT64 -o spirv\IndirectGInflate64_HAVE_INT64_vk.h TileDecoder.glsl
glslangValidator --target-env vulkan1.2 -g0 -S comp -e main -DGDEFLATE_INDIRECT_DECOMPRESS -I. -DGDEFLATE_USING_BUFFER_REF -DSIMD_WIDTH=64 -DGDEFLATE_HAVE_INT16 -DGDEFLATE_HAVE_INT64  --vn kIndirectGInflate64_HAVE_INT16_HAVE_INT64 -o spirv\IndirectGInflate64_HAVE_INT16_HAVE_INT64_vk.h TileDecoder.glsl

