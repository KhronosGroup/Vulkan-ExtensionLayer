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

# System-specific macros to create a library target.
macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
    add_library(VkLayer_${target} MODULE ${ARGN})
    target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
    target_link_libraries(VkLayer_${target} PRIVATE VkExtLayer_utils Vulkan::Headers)
    if(MSVC)
        target_link_options(VkLayer_${target} PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def)
        target_compile_definitions(VkLayer_${target} PUBLIC NOMINMAX)
    elseif(MINGW)
        target_sources(VkLayer_${target} PRIVATE VkLayer_${target}.def)
    elseif(APPLE)
        message(DEBUG "Functions are exported via VEL_EXPORT")
        set_target_properties(VkLayer_${target} PROPERTIES SUFFIX ".dylib")
    elseif(ANDROID)
        message(DEBUG "Functions are exported via VEL_EXPORT")
    else()
        target_link_options(VkLayer_${target} PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libVkLayer_${target}.map,-Bsymbolic,--exclude-libs,ALL)
    endif()
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/utils
                    ${PROJECT_SOURCE_DIR}/utils/generated)

if(MSVC)
    # Applies to all configurations
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
    # Avoid: fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj
    add_compile_options("/bigobj")
    # Allow Windows to use multiprocessor compilation
    add_compile_options(/MP)
    # Turn off transitional "changed behavior" warning message for Visual Studio versions prior to 2015. The changed behavior is
    # that constructor initializers are now fixed to clear the struct members.
    add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>:/wd4351>")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wno-unused-function -Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wno-unused-function -Wno-sign-compare")
    # Use of pthread_condattr_setclock requires a "recent" POSIX setting.
    add_definitions(-D_POSIX_C_SOURCE=200112)
endif()

if(NOT WIN32)
set(TIMELINE_SEMAPHORE_FILES
    hash_table.cpp
    timeline_semaphore.c)

AddVkLayer(khronos_timeline_semaphore ""
    ${TIMELINE_SEMAPHORE_FILES})
endif()

set(SYNCHRONIZATION2_FILES
    synchronization2.cpp)

AddVkLayer(khronos_synchronization2 ""
    ${SYNCHRONIZATION2_FILES})

set(SHADER_OBJECT_FILES
    shader_object.cpp)

AddVkLayer(khronos_shader_object ""
    ${SHADER_OBJECT_FILES})

set(TARGET_NAMES VkLayer_khronos_timeline_semaphore VkLayer_khronos_synchronization2 VkLayer_khronos_shader_object)

# timeline_semaphore doesn't build on windows yet
if (WIN32)
    list(REMOVE_ITEM TARGET_NAMES "VkLayer_khronos_timeline_semaphore")
endif()

# The Vulkan SDK has deliberately chosen not to ship 32-bit VkLayer_khronos_shader_object files
if (CMAKE_SIZEOF_VOID_P EQUAL "4")
    list(REMOVE_ITEM TARGET_NAMES "VkLayer_khronos_shader_object")
endif()

foreach(TARGET_NAME ${TARGET_NAMES})
    set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/json/${TARGET_NAME}.json.in")
    set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/intermediate-${TARGET_NAME}.json")
    set(OUTPUT_FILE_FINAL_NAME "${TARGET_NAME}.json")
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})

    if (WIN32)
        set(JSON_LIBRARY_PATH ".\\\\${TARGET_NAME}.dll")
    elseif(APPLE)
        set(JSON_LIBRARY_PATH "./lib${TARGET_NAME}.dylib")
    else()
        set(JSON_LIBRARY_PATH "./lib${TARGET_NAME}.so")
    endif()

    set(JSON_VERSION ${VulkanHeaders_VERSION})

    configure_file(${INPUT_FILE} ${INTERMEDIATE_FILE} @ONLY)

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} $<TARGET_FILE_DIR:${TARGET_NAME}>/${OUTPUT_FILE_FINAL_NAME}
    )
    if (UNIX)
        set(UNIX_INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/unix_install_${TARGET_NAME}.json")

        if(APPLE)
            set(JSON_LIBRARY_PATH "lib${TARGET_NAME}.dylib")
        else()
            set(JSON_LIBRARY_PATH "lib${TARGET_NAME}.so")
        endif()

        configure_file(${INPUT_FILE} ${UNIX_INTERMEDIATE_FILE} @ONLY)

        install(FILES ${UNIX_INTERMEDIATE_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/explicit_layer.d RENAME ${OUTPUT_FILE_FINAL_NAME})
    endif()

    if (WIN32)
        install(FILES ${INTERMEDIATE_FILE} DESTINATION ${LAYER_INSTALL_DIR} RENAME ${OUTPUT_FILE_FINAL_NAME})
    endif()

    install(TARGETS ${TARGET_NAME} DESTINATION ${LAYER_INSTALL_DIR})
endforeach()
