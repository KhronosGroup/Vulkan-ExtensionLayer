# ~~~
# Copyright (c) 2014-2021 Valve Corporation
# Copyright (c) 2014-2021 LunarG, Inc.
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

# CMake project initialization ---------------------------------------------------------------------------------------------------
# This section contains pre-project() initialization, and ends with the project() command.

cmake_minimum_required(VERSION 3.10.2)

# Apple: Must be set before enable_language() or project() as it may influence configuration of the toolchain and flags.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project(Vulkan-ExtensionLayer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# User-interface declarations ----------------------------------------------------------------------------------------------------
# This section contains variables that affect development GUIs (e.g. CMake GUI and IDEs), such as option(), folders, and variables
# with the CACHE property.

# API_NAME allows renaming builds to avoid conflicts with installed SDKs.  It is referenced by layers/vk_loader_platform.h
set(API_NAME "Vulkan" CACHE STRING "API name to use when building")
string(TOLOWER ${API_NAME} API_LOWERCASE)
add_definitions(-DAPI_NAME="${API_NAME}")

# Enable beta Vulkan extensions
add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PythonInterp 3 QUIET)

if (TARGET Vulkan::Headers)
    message(STATUS "${PROJECT_NAME}: Using Vulkan headers from Vulkan::Headers target")
    get_target_property(VulkanHeaders_INCLUDE_DIRS Vulkan::Headers INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(VulkanRegistry_DIR Vulkan::Registry INTERFACE_INCLUDE_DIRECTORIES)
else()
    find_package(VulkanHeaders REQUIRED)

    # xxxnsubtil: this should eventually be replaced by exported targets
    add_library(Vulkan-Headers INTERFACE)
    target_include_directories(Vulkan-Headers INTERFACE ${VulkanHeaders_INCLUDE_DIRS})
    add_library(Vulkan::Headers ALIAS Vulkan-Headers)
endif()

option(USE_CCACHE "Use ccache" OFF)
if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Windows: if install locations not set by user, set install prefix to "<build_dir>\install".
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

if(APPLE)
    # CMake versions 3 or later need CMAKE_MACOSX_RPATH defined. This avoids the CMP0042 policy message.
    set(CMAKE_MACOSX_RPATH 1)
endif()

# Enable IDE GUI folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# "Helper" targets that don't have interesting source code should set their FOLDER property to this
set(LAYERS_HELPER_FOLDER "Helper Targets")

# Platform-specific compiler switches
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall
                        -Wextra
                        -Wno-unused-parameter
                        -Wno-missing-field-initializers
                        -Werror=int-to-pointer-cast
                        -fno-strict-aliasing
                        -fno-builtin-memcmp
                        -fvisibility=hidden
                        "$<$<COMPILE_LANGUAGE:C>:-Werror=int-conversion>"
                        "$<$<COMPILE_LANGUAGE:C>:-Werror=pointer-to-int-cast>")

    # Treat warnings as errors for versions of GCC and Clang that are shipped on Ubuntu 18.04 or older.
    if((CMAKE_COMPILER_IS_GNUCXX AND NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.3.0)) OR
       (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.8.0)))
        add_compile_options(-Werror)
    endif()

    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since there's no consistent way to satisfy
    # all compilers until they all accept the C++17 standard.
    if(CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
        add_compile_options(-Wimplicit-fallthrough=0)
    endif()
elseif(MSVC)
    # Treat warnings as errors
    add_compile_options("/WX")
    # Disable RTTI
    add_compile_options("/GR-")
    # Warn about nested declarations
    add_compile_options("/w34456")
    # Warn about potentially uninitialized variables
    add_compile_options("/w34701")
    add_compile_options("/w34703")
    # Warn about different indirection types.
    add_compile_options("/w34057")
    # Warn about signed/unsigned mismatch.
    add_compile_options("/w34245")
endif()

option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers

# Generate dependent helper files ------------------------------------------------------------------------------------------------

set(SCRIPTS_DIR "${PROJECT_SOURCE_DIR}/scripts")

# uninstall target ---------------------------------------------------------------------------------------------------------------
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE
                   @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    set_target_properties(uninstall PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
endif()

# Fetch header version from vulkan_core.h ----------------------------------------------------------------------------------------
file(STRINGS "${VulkanHeaders_INCLUDE_DIRS}/vulkan/vulkan_core.h" lines REGEX "^#define VK_HEADER_VERSION [0-9]+")
list(LENGTH lines len)
if(${len} EQUAL 1)
    string(REGEX MATCHALL
                 "[0-9]+"
                 vk_header_version
                 ${lines})
else()
    message(FATAL_ERROR "Unable to fetch version from vulkan_core.h")
endif()

# Optional codegen target --------------------------------------------------------------------------------------------------------
if(PYTHONINTERP_FOUND)
  add_custom_target(VulkanEL_generated_source
                      COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/generate_source.py
                              ${VulkanRegistry_DIR} --incremental
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/layers/generated
                      )
else()
  message("${PROJECT_NAME}: WARNING: VulkanEL_generated_source target requires python 3")
endif()

# Add subprojects ----------------------------------------------------------------------------------------------------------------

add_subdirectory(layers)
add_subdirectory(utils)
