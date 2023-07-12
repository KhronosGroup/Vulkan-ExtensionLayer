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

# TODO: Remove this macro. It's more confusing than helpful.
macro(AddVkLayer target)
    add_library(VkLayer_${target} MODULE)

    target_include_directories(VkLayer_${target} PRIVATE .)

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

if(NOT WIN32)
    AddVkLayer(khronos_timeline_semaphore)
    target_sources(VkLayer_khronos_timeline_semaphore PRIVATE
        hash_table.cpp
        timeline_semaphore.c
    )
endif()

AddVkLayer(khronos_synchronization2)
target_sources(VkLayer_khronos_synchronization2 PRIVATE
    synchronization2.cpp
)

AddVkLayer(khronos_shader_object)
target_sources(VkLayer_khronos_shader_object PRIVATE
    shader_object.cpp
)

AddVkLayer(khronos_memory_decompression)
target_sources(VkLayer_khronos_memory_decompression PRIVATE
    decompression/decompression.cpp
)

set(TARGET_NAMES VkLayer_khronos_timeline_semaphore VkLayer_khronos_synchronization2 VkLayer_khronos_shader_object VkLayer_khronos_memory_decompression)

# timeline_semaphore doesn't build on windows yet
if (WIN32)
    list(REMOVE_ITEM TARGET_NAMES "VkLayer_khronos_timeline_semaphore")
endif()

foreach(TARGET_NAME ${TARGET_NAMES})
    set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/json/${TARGET_NAME}.json.in")
    set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/intermediate-${TARGET_NAME}.json")
    set(OUTPUT_FILE_FINAL_NAME "${TARGET_NAME}.json")
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
    if (WIN32)
        set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}) # WIN32/MINGW expect the dll in the `bin` dir, this matches our WIN32 SDK process
    endif()

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

    # Don't install VkLayer_khronos_memory_decompression. We don't plan to release this for the 1.3.250 SDK release.
    if (${TARGET_NAME} STREQUAL "VkLayer_khronos_memory_decompression")
        continue()
    endif()

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
    if (MSVC)
        install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> DESTINATION ${LAYER_INSTALL_DIR})
    endif()

    install(TARGETS ${TARGET_NAME} DESTINATION ${LAYER_INSTALL_DIR})
endforeach()
