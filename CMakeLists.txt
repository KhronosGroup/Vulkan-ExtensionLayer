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

project(Vulkan-ExtensionLayer)

add_subdirectory(scripts)

if (NOT TARGET Vulkan::Headers)
    find_package(VulkanHeaders CONFIG REQUIRED QUIET)
endif()

add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

# Enable IDE GUI folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# "Helper" targets that don't have interesting source code should set their FOLDER property to this
set(LAYERS_HELPER_FOLDER "Helper Targets")

# NOTE: The idiom for open source projects is to not enable warnings as errors.
# This reduces problems for users who simply want to build the repository.
option(BUILD_WERROR "Treat compiler warnings as errors" OFF)
if (BUILD_WERROR)
    add_compile_options("$<IF:$<CXX_COMPILER_ID:MSVC>,/WX,-Werror>")
endif()

# Platform-specific compiler switches
if(MAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wconversion

                        # TODO These warnings also get turned on with -Wconversion in some versions of clang.
                        #      Leave off until further investigation.
                        -Wno-sign-conversion
                        -Wno-shorten-64-to-32
                        -Wno-string-conversion
                        -Wno-implicit-int-conversion
                        -Wno-enum-enum-conversion)

endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall
                        -Wextra
                        -Wno-unused-parameter
                        -Wno-missing-field-initializers
                        -fno-strict-aliasing
                        -fno-builtin-memcmp
                        -fvisibility=hidden)

    set(CMAKE_C_STANDARD 99)

    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since there's no consistent way to satisfy
    # all compilers until they all accept the C++17 standard.
    if(CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
        add_compile_options(-Wimplicit-fallthrough=0)
    endif()
elseif(MSVC)
    add_compile_options("/W3")
    # Warn about nested declarations
    add_compile_options("/w34456")
    # Warn about potentially uninitialized variables
    add_compile_options("/w34701")
    add_compile_options("/w34703")
    # Warn about different indirection types.
    add_compile_options("/w34057")
    # Warn about signed/unsigned mismatch.
    add_compile_options("/w34245")
    # Warn zero sized arrays in structs or unions
    add_compile_options("/wd4200")

    # PDBs aren't generated on Release builds by default.
    add_compile_options("$<$<CONFIG:Release>:/Zi>")
    add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")
    # Remove unreferenced code/data.
    add_link_options("$<$<CONFIG:Release>:/OPT:REF>")
    # Merge duplicate data/functions
    add_link_options("$<$<CONFIG:Release>:/OPT:ICF>")
endif()

if(MAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wconversion

                        # TODO These warnings also get turned on with -Wconversion in some versions of clang.
                        #      Leave off until further investigation.
                        -Wno-sign-conversion
                        -Wno-shorten-64-to-32
                        -Wno-string-conversion
                        -Wno-implicit-int-conversion
                        -Wno-enum-enum-conversion)

endif()

option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers

option(BUILD_TESTS "Build tests")
if(BUILD_TESTS)
    # Options for Linux only
    if(UNIX AND NOT APPLE) # i.e. Linux
        find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

        option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
        option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
        option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)

        if(BUILD_WSI_XCB_SUPPORT)
            pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
            add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
        endif()

        if(BUILD_WSI_XLIB_SUPPORT)
            pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
            add_definitions(-DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_XLIB_XRANDR_EXT)
        endif()

        if(BUILD_WSI_WAYLAND_SUPPORT)
            pkg_check_modules(WAYLAND REQUIRED QUIET IMPORTED_TARGET wayland-client)
            add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
        endif()
    endif()
endif()

option(VEL_CODEGEN "Enable code generation")
if (VEL_CODEGEN)
    find_package(Python3 REQUIRED QUIET)
    add_custom_target(vel_codegen
        COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/generate_source.py
            ${VULKAN_HEADERS_REGISTRY_DIRECTORY} --incremental
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/utils/generated
    )
endif()

add_subdirectory(layers)
add_subdirectory(utils)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
