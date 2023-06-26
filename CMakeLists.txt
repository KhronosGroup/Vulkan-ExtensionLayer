# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
# Copyright (c) 2019      Intel Corporation.
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
cmake_minimum_required(VERSION 3.17.2)

project(VEL LANGUAGES CXX)

add_subdirectory(scripts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN "YES")

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT TARGET Vulkan::Headers)
    find_package(VulkanHeaders CONFIG REQUIRED QUIET)
endif()

target_compile_definitions(Vulkan::Headers INTERFACE VK_ENABLE_BETA_EXTENSIONS) # Enable beta Vulkan extensions
if(WIN32)
    target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT)
else()
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)

    find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_XCB_KHR)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        pkg_check_modules(WAYlAND_CLIENT QUIET REQUIRED IMPORTED_TARGET wayland-client)
        target_compile_definitions(Vulkan::Headers INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
endif()

find_package(VulkanUtilityLibraries CONFIG REQUIRED)

option(BUILD_WERROR "Treat compiler warnings as errors")
if (BUILD_WERROR)
    add_compile_options("$<IF:$<CXX_COMPILER_ID:MSVC>,/WX,-Werror>")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    add_compile_options(
        -Wall
        -Wextra
        -Wpointer-arith
    )
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        add_compile_options(
            -Wconversion
            -Wimplicit-fallthrough
            -Wstring-conversion
        )
    endif()
elseif(MSVC)
    add_compile_options(
        /W4
        /we5038 # Enable warning about MIL ordering in constructors
    )

    # Enforce stricter ISO C++
    add_compile_options(/permissive-)

    # PDBs aren't generated on Release builds by default.
    add_compile_options("$<$<CONFIG:Release>:/Zi>")
    add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")
    # Remove unreferenced code/data.
    add_link_options("$<$<CONFIG:Release>:/OPT:REF>")
    # Merge duplicate data/functions
    add_link_options("$<$<CONFIG:Release>:/OPT:ICF>")

    # Allow usage of unsafe CRT functions and minimize what Windows.h leaks
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)

    add_compile_options($<$<BOOL:${MSVC_IDE}>:/MP>) # Speed up Visual Studio builds
endif()

option(VEL_CODEGEN "Enable code generation")
if (VEL_CODEGEN)
    find_package(Python3 REQUIRED QUIET)
    add_custom_target(vel_codegen
        COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/generate_source.py
            ${VULKAN_HEADERS_INSTALL_DIR}/${CMAKE_INSTALL_DATADIR}/vulkan/registry --incremental
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/utils/generated
    )
endif()

add_subdirectory(layers)
add_subdirectory(utils)

option(BUILD_TESTS "Build tests")
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
