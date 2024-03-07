# ~~~
# Copyright (c) 2014-2024 Valve Corporation
# Copyright (c) 2014-2024 LunarG, Inc.
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


if(IOS)
    add_library(VkLayer_khronos_synchronization2 SHARED)
else()
    add_library(VkLayer_khronos_synchronization2 MODULE)
endif()

target_sources(VkLayer_khronos_synchronization2 PRIVATE
    synchronization2.cpp
)


if(IOS)
    add_library(VkLayer_khronos_shader_object SHARED)
else()
    add_library(VkLayer_khronos_shader_object MODULE)
endif()

target_sources(VkLayer_khronos_shader_object PRIVATE
    shader_object.cpp
)

add_library(VkLayer_khronos_memory_decompression MODULE)
target_sources(VkLayer_khronos_memory_decompression PRIVATE
    decompression/decompression.cpp
)

set(EXTENSION_LAYERS VkLayer_khronos_synchronization2 VkLayer_khronos_shader_object VkLayer_khronos_memory_decompression)

if (NOT WIN32)
    # timeline_semaphore.c requires a C compiler
    enable_language(C)

    add_library(VkLayer_khronos_timeline_semaphore MODULE)
    target_sources(VkLayer_khronos_timeline_semaphore PRIVATE
        hash_table.cpp
        timeline_semaphore.c
    )
    list(APPEND EXTENSION_LAYERS "VkLayer_khronos_timeline_semaphore")
endif()

foreach(extension_layer ${EXTENSION_LAYERS})
    lunarg_target_compiler_configurations(${extension_layer} ${BUILD_WERROR})
    target_include_directories(${extension_layer} PRIVATE .)
    target_link_libraries(${extension_layer} PRIVATE VkExtLayer_utils Vulkan::LayerSettings Vulkan::UtilityHeaders)

    if(MSVC)
        target_link_options(${extension_layer} PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/${extension_layer}.def)
        target_compile_definitions(${extension_layer} PUBLIC NOMINMAX)
    elseif(MINGW)
        target_sources(${extension_layer} PRIVATE ${extension_layer}.def)
    elseif(APPLE)
        message(DEBUG "Functions are exported via VEL_EXPORT")
        set_target_properties(${extension_layer} PROPERTIES SUFFIX ".dylib")
    elseif(ANDROID)
        target_link_options(${extension_layer} PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/lib${extension_layer}.map)
    else()
        target_link_options(${extension_layer} PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/lib${extension_layer}.map,-Bsymbolic,--exclude-libs,ALL)
    endif()

    if (ANDROID)
        install(TARGETS ${extension_layer} DESTINATION ${CMAKE_INSTALL_LIBDIR})
        continue()
    endif()

    set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/json/${extension_layer}.json.in")
    set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/intermediate-${extension_layer}.json")
    set(OUTPUT_FILE_FINAL_NAME "${extension_layer}.json")
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
    if (WIN32)
        set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}) # WIN32/MINGW expect the dll in the `bin` dir, this matches our WIN32 SDK process
    endif()

    if (WIN32)
        set(JSON_LIBRARY_PATH ".\\\\${extension_layer}.dll")
    elseif(APPLE)

	if(IOS)
            set_target_properties(${extension_layer} PROPERTIES
		FRAMEWORK			TRUE
                MACOSX_FRAMEWORK_BUNDLE_VERSION "${VulkanHeaders_VERSION}"
                MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${VulkanHeaders_VERSION}"
		MACOSX_FRAMEWORK_IDENTIFIER 	com.khronos.${extension_layer}
            )
	else()
	    set(JSON_LIBRARY_PATH "./lib${extension_layer}.dylib")
        endif()

    else()
        set(JSON_LIBRARY_PATH "./lib${extension_layer}.so")
    endif()

    set(JSON_VERSION ${VulkanHeaders_VERSION})

    configure_file(${INPUT_FILE} ${INTERMEDIATE_FILE} @ONLY)

    add_custom_command(TARGET ${extension_layer} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} $<TARGET_FILE_DIR:${extension_layer}>/${OUTPUT_FILE_FINAL_NAME}
    )

    # Don't install VkLayer_khronos_memory_decompression. We don't plan to release this for the 1.3.250 SDK release.
    if (${extension_layer} STREQUAL "VkLayer_khronos_memory_decompression")
        continue()
    endif()

    if (UNIX)
        set(UNIX_INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/unix_install_${extension_layer}.json")

        if(APPLE)

            if(IOS)
                set_target_properties(${extension_layer} PROPERTIES
		        FRAMEWORK			TRUE
		        MACOSX_FRAMEWORK_IDENTIFIER 	com.khronos.${extension_layer}
                )
            else()
                set(JSON_LIBRARY_PATH "lib${extension_layer}.dylib")
            endif()

        else()
            set(JSON_LIBRARY_PATH "lib${extension_layer}.so")
        endif()

        configure_file(${INPUT_FILE} ${UNIX_INTERMEDIATE_FILE} @ONLY)

        install(FILES ${UNIX_INTERMEDIATE_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/explicit_layer.d RENAME ${OUTPUT_FILE_FINAL_NAME})
    endif()

    if (WIN32)
        install(FILES ${INTERMEDIATE_FILE} DESTINATION ${LAYER_INSTALL_DIR} RENAME ${OUTPUT_FILE_FINAL_NAME})
    endif()
    if (MSVC)
        install(FILES $<TARGET_PDB_FILE:${extension_layer}> DESTINATION ${LAYER_INSTALL_DIR})
    endif()

    install(TARGETS ${extension_layer} DESTINATION ${LAYER_INSTALL_DIR})
endforeach()
