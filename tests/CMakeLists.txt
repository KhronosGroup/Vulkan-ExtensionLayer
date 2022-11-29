# ~~~
# Copyright (c) 2014-2022 Valve Corporation
# Copyright (c) 2014-2022 LunarG, Inc.
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

# Needed to make structure definitions match with glslang libraries
add_definitions(-DNV_EXTENSIONS -DAMD_EXTENSIONS)

if(ANNOTATED_SPEC_LINK)
    add_definitions(-DANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    # Workaround for TR1 deprecation in Visual Studio 15.5 until Google Test is updated
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    # If MSVC, allow Windows to use multiprocessor compilation
    if(MSVC)
    add_compile_options(/MP)
    endif()
elseif(ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
endif()

if(WIN32)
    file(COPY vk_extension_layer_tests.vcxproj.user DESTINATION ${CMAKE_BINARY_DIR}/tests)
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
               ${COMMON_CPP})
add_test(NAME vk_extension_layer_tests COMMAND vk_extension_layer_tests)
add_dependencies(vk_extension_layer_tests VkLayer_khronos_synchronization2 VkLayer_khronos_synchronization2-json)
# Note that there is no need to add GTEST directories here due to googletest exporting them via the gtest target.
target_include_directories(vk_extension_layer_tests
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${PROJECT_SOURCE_DIR}/utils
                                  ${PROJECT_SOURCE_DIR}/utils/generated
                                  ${GLSLANG_SPIRV_INCLUDE_DIR}
                                  ${SPIRV_TOOLS_INCLUDE_DIR}
                                  ${SPIRV_HEADERS_INCLUDE_DIR}
                                  ${CMAKE_CURRENT_BINARY_DIR}
                                  ${CMAKE_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}
                                  ${PROJECT_BINARY_DIR}/layers)
add_dependencies(vk_extension_layer_tests
	         VkExtLayer_utils)
target_link_libraries(vk_extension_layer_tests PRIVATE
    VkExtLayer_utils
    Vulkan::Headers
    GTest::gtest
    GTest::gtest_main
    ${GLSLANG_LIBRARIES}
)

# Specify target_link_libraries
if(NOT WIN32)
    target_compile_options(vk_extension_layer_tests PRIVATE "-Wno-sign-compare")
    target_link_libraries(vk_extension_layer_tests PRIVATE ${CMAKE_DL_LIBS} VkExtLayer_utils)
    if(BUILD_WSI_XCB_SUPPORT)
        target_link_libraries(vk_extension_layer_tests PRIVATE PkgConfig::XCB)
    endif()

    if (BUILD_WSI_XLIB_SUPPORT)
        target_link_libraries(vk_extension_layer_tests PRIVATE PkgConfig::X11)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        target_link_libraries(vk_extension_layer_tests PRIVATE PkgConfig::WAYLAND)
    endif()
endif()

if(INSTALL_TESTS)
    install(TARGETS vk_extension_layer_tests DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
