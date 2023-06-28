# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
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

# run the generator expression on the test_layer_location file
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/test_layer_location_$<CONFIG>.h" INPUT "${CMAKE_CURRENT_LIST_DIR}/generator_source/test_layer_location.h")

# copy $<BUILD>/test_layer_location_$<CONFIG> to $<BUILD>/test_layer_location.h
add_custom_command(
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "${CMAKE_BINARY_DIR}/test_layer_location_$<CONFIG>.h" "${CMAKE_BINARY_DIR}/test_layer_location.h"
    VERBATIM
    DEPENDS  "${CMAKE_BINARY_DIR}/test_layer_location_$<CONFIG>.h"
    OUTPUT   "${CMAKE_BINARY_DIR}/test_layer_location.h"
    COMMENT  "creating test_layer_location.h file ({event: PRE_BUILD}, {filename: test_layer_location.h })"
    )
add_custom_target (generate_test_layer_location DEPENDS "${CMAKE_BINARY_DIR}/test_layer_location.h")

if(ANNOTATED_SPEC_LINK)
    add_definitions(-DANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    # Workaround for TR1 deprecation in Visual Studio 15.5 until Google Test is updated
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    # If MSVC, allow Windows to use multiprocessor compilation
    if(MSVC)
    add_compile_options(/MP)
    endif()
endif()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")

    # If MSVC, disable some signed/unsigned mismatch warnings.
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
    endif()
endif()

set(LIBGLM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libs)

set(COMMON_CPP
    vkrenderframework.cpp
    vktestbinding.cpp
    vktestframework.cpp
    test_environment.cpp)

if(NOT TARGET vulkan)
    set(
        CMAKE_PREFIX_PATH
        ${CMAKE_PREFIX_PATH};${VULKAN_HEADERS_INSTALL_DIR};$ENV{VULKAN_HEADERS_INSTALL_DIR}
        )
endif()

set_source_files_properties(${PROJECT_BINARY_DIR}/vk_safe_struct.cpp PROPERTIES GENERATED TRUE)
add_executable(vk_extension_layer_tests
               extension_layer_tests.cpp
               synchronization2_tests.cpp
               decompression_tests.cpp
               shader_object_tests.cpp
               ${COMMON_CPP})
add_test(NAME vk_extension_layer_tests COMMAND vk_extension_layer_tests)

if(MSVC_IDE)
    set_target_properties(vk_extension_layer_tests PROPERTIES VS_DEBUGGER_ENVIRONMENT "VK_LAYER_PATH=$<TARGET_FILE_DIR:VkLayer_khronos_synchronization2>")
endif()

if (NOT MSVC)
    target_compile_options(vk_extension_layer_tests PRIVATE "-Wno-sign-compare")
endif()

add_dependencies(vk_extension_layer_tests VkLayer_khronos_synchronization2 VkLayer_khronos_memory_decompression generate_test_layer_location)

# Note that there is no need to add GTEST directories here due to googletest exporting them via the gtest target.
target_include_directories(vk_extension_layer_tests
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${PROJECT_SOURCE_DIR}/utils
                                  ${PROJECT_SOURCE_DIR}/utils/generated
                                  ${CMAKE_CURRENT_BINARY_DIR}
                                  ${CMAKE_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}/layers)
add_dependencies(vk_extension_layer_tests VkExtLayer_utils)

find_package(SPIRV-Headers REQUIRED CONFIG QUIET)
target_link_libraries(vk_extension_layer_tests PRIVATE SPIRV-Headers::SPIRV-Headers)

find_package(SPIRV-Tools-opt REQUIRED CONFIG QUIET)
target_link_libraries(vk_extension_layer_tests PRIVATE SPIRV-Tools-opt)

find_package(SPIRV-Tools REQUIRED CONFIG QUIET)

# See https://github.com/KhronosGroup/SPIRV-Tools/issues/3909 for background on this.
# The targets available from SPIRV-Tools change depending on how SPIRV_TOOLS_BUILD_STATIC is set.
# Try to handle all possible combinations so that we work with externally built packages.
if (TARGET SPIRV-Tools)
    target_link_libraries(vk_extension_layer_tests PRIVATE SPIRV-Tools)
elseif(TARGET SPIRV-Tools-static)
    target_link_libraries(vk_extension_layer_tests PRIVATE SPIRV-Tools-static)
elseif(TARGET SPIRV-Tools-shared)
    target_link_libraries(vk_extension_layer_tests PRIVATE SPIRV-Tools-shared)
else()
    message(FATAL_ERROR "Cannot determine SPIRV-Tools target name")
endif()

find_package(GTest REQUIRED CONFIG QUIET)

find_package(glslang REQUIRED CONFIG QUIET)

target_link_libraries(vk_extension_layer_tests PRIVATE
    VkExtLayer_utils
    Vulkan::Headers
    GTest::gtest
    GTest::gtest_main
    glslang::glslang
    glslang::OGLCompiler
    glslang::OSDependent
    glslang::MachineIndependent
    glslang::GenericCodeGen
    glslang::HLSL
    glslang::SPIRV
    glslang::SPVRemapper
    $<TARGET_NAME_IF_EXISTS:PkgConfig::XCB>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::X11>
    $<TARGET_NAME_IF_EXISTS:PkgConfig::WAYlAND_CLIENT>
    ${CMAKE_DL_LIBS}
)

target_compile_definitions(vk_extension_layer_tests PUBLIC SHADER_OBJECT_BINARY_PATH="$<TARGET_FILE_DIR:VkLayer_khronos_shader_object>")

install(TARGETS vk_extension_layer_tests)
