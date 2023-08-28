# ~~~
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
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

set_directory_properties(PROPERTIES "COMPILE_OPTIONS" "") # Disable compiler warnings for android glue

enable_language(C) # NOTE: We need to enable the C language for android_native_app_glue.c

set(native_app_glue_dir "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")

if (NOT EXISTS ${native_app_glue_dir})
    message(FATAL_ERROR "Couldn't find Android Native Glue directory!")
endif()

# https://stackoverflow.com/questions/57189936/compiling-native-app-glue-c-results-in-an-invalid-library-file/76963564#76963564
add_library(android_glue OBJECT)

target_include_directories(android_glue PUBLIC ${native_app_glue_dir})
target_sources(android_glue PRIVATE
    ${native_app_glue_dir}/android_native_app_glue.c
    ${native_app_glue_dir}/android_native_app_glue.h
)

target_link_libraries(vk_extension_layer_tests PRIVATE android_glue)

target_link_options(vk_extension_layer_tests PUBLIC LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/android.map)

set_target_properties(vk_extension_layer_tests PROPERTIES OUTPUT_NAME "VulkanExtensionLayerTests")

install(TARGETS vk_extension_layer_tests DESTINATION ${CMAKE_INSTALL_LIBDIR})

# https://stackoverflow.com/questions/76631917/cmake-how-to-install-shared-stl-libraries-for-android/76656492#76656492
if ("${CMAKE_ANDROID_STL_TYPE}" MATCHES "shared")
    file(READ "${CMAKE_ANDROID_NDK}/meta/abis.json" JSON_FILE)
    string(JSON TRIPLE GET "${JSON_FILE}" "${CMAKE_ANDROID_ARCH_ABI}" "triple")
    install(FILES "${CMAKE_SYSROOT}/usr/lib/${TRIPLE}/libc++_shared.so" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
