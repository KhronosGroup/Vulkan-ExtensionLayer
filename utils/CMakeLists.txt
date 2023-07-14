# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
# Copyright (c) 2019      Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~
add_library(VkExtLayer_utils STATIC)

target_sources(VkExtLayer_utils PRIVATE
    allocator.cpp
    vk_format_utils.cpp
    vk_layer_config.cpp
    generated/vk_safe_struct.cpp
    generated/lvt_function_pointers.cpp
)

# TODO: Fix warnings
target_compile_options(VkExtLayer_utils PUBLIC "$<IF:$<CXX_COMPILER_ID:MSVC>,/wd4100,-Wno-unused-parameter>")
if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(VkExtLayer_utils PUBLIC -Wno-missing-field-initializers)
endif()
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(VkExtLayer_utils PUBLIC
        -Wno-implicit-int-conversion
        -Wno-sign-conversion
        -Wno-string-conversion
        -Wno-implicit-fallthrough
        -Wno-implicit-int-float-conversion
        -Wno-shorten-64-to-32
        -Wno-incompatible-function-pointer-types
    )
endif()
if (MSVC)
    target_compile_options(VkExtLayer_utils PUBLIC
        /wd4389
        /wd4324
        /wd4244
        /wd4815
        /wd4200
        /wd4267
        /wd4505
        /wd4706
    )
endif()

target_link_libraries(VkExtLayer_utils PUBLIC Vulkan::Headers)

if(ANDROID)
    target_link_libraries(VkExtLayer_utils PUBLIC log)
endif()

target_include_directories(VkExtLayer_utils PUBLIC
    .
    generated
)
