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
cmake_minimum_required(VERSION 3.10.2)

project(Vulkan-ExtensionLayer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# Enable beta Vulkan extensions
add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (UPDATE_DEPS)
    find_package(PythonInterp 3 REQUIRED)

    if (CMAKE_GENERATOR_PLATFORM)
        set(_target_arch ${CMAKE_GENERATOR_PLATFORM})
    else()
        if (MSVC_IDE)
            message(WARNING "CMAKE_GENERATOR_PLATFORM not set. Using x64 as target architecture.")
        endif()
        set(_target_arch x64)
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        message(WARNING "CMAKE_BUILD_TYPE not set. Using Debug for dependency build type")
        set(_build_type Debug)
    else()
        set(_build_type ${CMAKE_BUILD_TYPE})
    endif()

    message("********************************************************************************")
    message("* NOTE: Adding target vvl_update_deps to run as needed for updating            *")
    message("*       dependencies.                                                          *")
    message("********************************************************************************")

    # Add a target so that update_deps.py will run when necessary
    # NOTE: This is triggered off of the timestamps of known_good.json and helper.cmake
    add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake
                       COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/update_deps.py --dir ${CMAKE_CURRENT_LIST_DIR}/external --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}"
                       DEPENDS ${CMAKE_CURRENT_LIST_DIR}/scripts/known_good.json)

    add_custom_target(vvl_update_deps DEPENDS ${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake)

    # Check if update_deps.py needs to be run on first cmake run
    if (${CMAKE_CURRENT_LIST_DIR}/scripts/known_good.json IS_NEWER_THAN ${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake)
        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/update_deps.py --dir ${CMAKE_CURRENT_LIST_DIR}/external --arch ${_target_arch} --config ${_build_type} --generator "${CMAKE_GENERATOR}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            RESULT_VARIABLE _update_deps_result
        )
        if (NOT (${_update_deps_result} EQUAL 0))
            message(FATAL_ERROR "Could not run update_deps.py which is necessary to download dependencies.")
        endif()
    endif()
    include(${CMAKE_CURRENT_LIST_DIR}/external/helper.cmake)
else()
    message("********************************************************************************")
    message("* NOTE: Not adding target to run update_deps.py automatically.                 *")
    message("********************************************************************************")
    find_package(PythonInterp 3 QUIET)
endif()
if (SPIRV_HEADERS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${SPIRV_HEADERS_INSTALL_DIR})
endif()
if (SPIRV_TOOLS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${SPIRV_TOOLS_INSTALL_DIR})
endif()
if (GOOGLETEST_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${GOOGLETEST_INSTALL_DIR})
endif()
if (VULKAN_HEADERS_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${VULKAN_HEADERS_INSTALL_DIR})
endif()

if (NOT TARGET Vulkan::Headers)
    find_package(VulkanHeaders CONFIG REQUIRED QUIET)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Windows: if install locations not set by user, set install prefix to "<build_dir>\install".
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

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

option(BUILD_TESTS "Build tests" ON)
option(INSTALL_TESTS "Install tests" OFF)
option(BUILD_LAYERS "Build layers" ON)
option(BUILD_LAYER_SUPPORT_FILES "Generate layer files" OFF) # For generating files when not building layers

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

    set(GLSLANG_INSTALL_DIR "GLSLANG-NOTFOUND" CACHE PATH "Absolute path to a glslang install directory")
    if(NOT GLSLANG_INSTALL_DIR AND NOT DEFINED ENV{GLSLANG_INSTALL_DIR} AND NOT TARGET glslang)
        message(FATAL_ERROR "Must define location of glslang binaries -- see BUILD.md")
    endif()

    # CMake command line option overrides environment variable
    if(NOT GLSLANG_INSTALL_DIR)
        set(GLSLANG_INSTALL_DIR $ENV{GLSLANG_INSTALL_DIR})
    endif()

    # CMake command line option overrides environment variable
    if(NOT SPIRV_HEADERS_INSTALL_DIR)
        set(SPIRV_HEADERS_INSTALL_DIR $ENV{SPIRV_HEADERS_INSTALL_DIR})
    endif()

    if(NOT SPIRV_TOOLS_INSTALL_DIR)
        set(SPIRV_TOOLS_INSTALL_DIR $ENV{SPIRV_TOOLS_INSTALL_DIR})
    endif()

    if (NOT TARGET glslang)
        if(NOT SPIRV_HEADERS_INSTALL_DIR)
            message(FATAL_ERROR "Must define location of SPIRV-Headers -- see BUILD.md")
        endif()

        message(STATUS "Using glslang install located at ${GLSLANG_INSTALL_DIR}")
        set(GLSLANG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(GLSLANG_DEBUG_SEARCH_PATH "${GLSLANG_INSTALL_DIR}/lib")
        set(GLSLANG_SPIRV_INCLUDE_DIR "${GLSLANG_INSTALL_DIR}/include/glslang" CACHE PATH "Path to glslang spirv headers")
        set(SPIRV_HEADERS_INCLUDE_DIR "${SPIRV_HEADERS_INSTALL_DIR}/include" CACHE PATH "Path to spirv-headers")

        set(GLSLANG_LIBRARIES glslang OGLCompiler OSDependent MachineIndependent GenericCodeGen HLSL SPIRV SPVRemapper)

        # Add glslang static libraries as imported targets
        foreach(GLSLANG_LIBRARY ${GLSLANG_LIBRARIES})
            add_library(${GLSLANG_LIBRARY} STATIC IMPORTED)
            find_library("${GLSLANG_LIBRARY}_PATH" NAMES "${GLSLANG_LIBRARY}" HINTS ${GLSLANG_SEARCH_PATH})
            set_target_properties(${GLSLANG_LIBRARY}
                                  PROPERTIES IMPORTED_LOCATION
                                             "${${GLSLANG_LIBRARY}_PATH}")
            if (WIN32)
                find_library("${GLSLANG_LIBRARY}_DEBUG_PATH" NAMES "${GLSLANG_LIBRARY}d" HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
                set_target_properties(${GLSLANG_LIBRARY}
                                      PROPERTIES IMPORTED_LOCATION_DEBUG
                                                 "${${GLSLANG_LIBRARY}_DEBUG_PATH}")
            endif()
        endforeach()

        # Circular dependencies exist between glslang static libs, add them all as link dependencies for each other
        foreach(GLSLANG_LIBRARY ${GLSLANG_LIBRARIES})
            set_target_properties(${GLSLANG_LIBRARY}
                                  PROPERTIES INTERFACE_LINK_LIBRARIES
                                             "${GLSLANG_LIBRARIES}")
        endforeach()
    else()
        set(GLSLANG_SPIRV_INCLUDE_DIR "${glslang_SOURCE_DIR}" CACHE PATH "Path to glslang spirv headers")
        set(GLSLANG_LIBRARIES glslang SPIRV SPVRemapper)
    endif()

    # spirv-tools
    if (NOT TARGET SPIRV-Tools)
        if(NOT SPIRV_TOOLS_INSTALL_DIR)
            set(SPIRV_TOOLS_INSTALL_DIR "${GLSLANG_INSTALL_DIR}")
        endif()

        set(SPIRV_TOOLS_BINARY_ROOT "${SPIRV_TOOLS_INSTALL_DIR}/lib"
            CACHE PATH "User defined path to the SPIRV-Tools binaries for this project")
        set(SPIRV_TOOLS_OPT_BINARY_ROOT "${SPIRV_TOOLS_INSTALL_DIR}/lib"
            CACHE PATH "User defined path to the SPIRV-Tools-opt binaries for this project")
        set(SPIRV_TOOLS_INCLUDE_DIR "${SPIRV_TOOLS_INSTALL_DIR}/include" CACHE PATH "Path to spirv tools headers")
        set(SPIRV_TOOLS_SEARCH_PATH "${SPIRV_TOOLS_INSTALL_DIR}/lib")
        set(SPIRV_TOOLS_DEBUG_SEARCH_PATH "${SPIRV_TOOLS_INSTALL_DIR}/lib")
        set(SPIRV_TOOLS_OPT_SEARCH_PATH "${SPIRV_TOOLS_INSTALL_DIR}/lib")
        set(SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH "${SPIRV_TOOLS_INSTALL_DIR}/lib")

        find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools HINTS ${SPIRV_TOOLS_SEARCH_PATH})
        find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_SEARCH_PATH})

        if(WIN32)
            add_library(SPIRV-Tools-opt STATIC IMPORTED)
            add_library(SPIRV-Tools STATIC IMPORTED)

            find_library(SPIRV_TOOLS_DLIB NAMES SPIRV-Toolsd HINTS ${SPIRV_TOOLS_DEBUG_SEARCH_PATH})
            find_library(SPIRV_TOOLS_OPT_DLIB NAMES SPIRV-Tools-optd HINTS ${SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH})

            set_target_properties(SPIRV-Tools
                                  PROPERTIES IMPORTED_LOCATION
                                             "${SPIRV_TOOLS_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${SPIRV_TOOLS_DLIB}")
            set_target_properties(SPIRV-Tools-opt
                                  PROPERTIES IMPORTED_LOCATION
                                             "${SPIRV_TOOLS_OPT_LIB}"
                                             IMPORTED_LOCATION_DEBUG
                                             "${SPIRV_TOOLS_OPT_DLIB}")

            set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools-opt SPIRV-Tools)
        else()
            set(SPIRV_TOOLS_LIBRARIES ${SPIRV_TOOLS_OPT_LIB} ${SPIRV_TOOLS_LIB})
        endif()
    else()
        set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools SPIRV-Tools-opt)
        set(SPIRV_TOOLS_INCLUDE_DIR "${spirv-tools_SOURCE_DIR}/include" CACHE PATH "Path to spirv tools headers")
    endif()

    set(GLSLANG_LIBRARIES ${GLSLANG_LIBRARIES} ${SPIRV_TOOLS_LIBRARIES})
endif()

# uninstall target ---------------------------------------------------------------------------------------------------------------
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE
                   @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    set_target_properties(uninstall PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
endif()

# Optional codegen target --------------------------------------------------------------------------------------------------------
if(PYTHONINTERP_FOUND)
    add_custom_target(VulkanEL_generated_source
        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/generate_source.py ${VULKAN_HEADERS_REGISTRY_DIRECTORY} --incremental
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/utils/generated
    )
else()
    message("${PROJECT_NAME}: WARNING: VulkanEL_generated_source target requires python 3")
endif()

# Add subprojects ----------------------------------------------------------------------------------------------------------------

add_subdirectory(layers)
add_subdirectory(utils)
if(BUILD_TESTS)
    # Attempt to enable googletest if available.
    find_package(GTest REQUIRED CONFIG)
    add_subdirectory(tests)
endif()
