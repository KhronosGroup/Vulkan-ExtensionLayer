# ~~~
# Copyright (c) 2014-2021 Valve Corporation
# Copyright (c) 2014-2021 LunarG, Inc.
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

if(WIN32)
    add_custom_target(${PROJECT_NAME}-mk_layer_config_dir ALL
                      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    set_target_properties(${PROJECT_NAME}-mk_layer_config_dir PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
elseif(ANDROID)
elseif(APPLE)
    if(CMAKE_GENERATOR MATCHES "^Xcode.*")
        add_custom_target(${PROJECT_NAME}-mk_layer_config_dir ALL
                          COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()
elseif(UNIX AND NOT APPLE) # i.e. Linux
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

if(WIN32)
# timeline_semaphore doesn't build on windows yet
set(TARGET_NAMES
    VkLayer_khronos_synchronization2)
else()
set(TARGET_NAMES
    VkLayer_khronos_timeline_semaphore
    VkLayer_khronos_synchronization2)
endif()

# Install the layer json files
if(WIN32)
    if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
        foreach(TARGET_NAME ${TARGET_NAMES})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${TARGET_NAME}.json DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endforeach()
    else()
        foreach(TARGET_NAME ${TARGET_NAMES})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.json DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endforeach()
    endif()
elseif(UNIX) # UNIX includes APPLE
    foreach(TARGET_NAME ${TARGET_NAMES})
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/staging-json/${TARGET_NAME}.json
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/explicit_layer.d)
    endforeach()
endif()

# System-specific macros to create a library target.
if(WIN32)
    macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def DEF_FILE)
        add_custom_target(copy-${target}-def-file ALL
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} VkLayer_${target}.def
                          VERBATIM)
        set_target_properties(copy-${target}-def-file PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})

        add_library(VkLayer_${target} SHARED ${ARGN} VkLayer_${target}.def)
        target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
        target_link_libraries(VkLayer_${target} PRIVATE VkExtLayer_utils)
        add_dependencies(VkLayer_${target} VkExtLayer_utils)
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endmacro()
elseif(APPLE)
    macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
        add_library(VkLayer_${target} SHARED ${ARGN})
        target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
        target_link_libraries(VkLayer_${target} PRIVATE VkExtLayer_utils)
        add_dependencies(VkLayer_${target} VkExtLayer_utils)
        set_target_properties(VkLayer_${target}
                              PROPERTIES LINK_FLAGS
                                         "-Wl"
                                         INSTALL_RPATH
                                         "@loader_path/")
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endmacro()
else(UNIX AND NOT APPLE) # i.e.: Linux
    macro(AddVkLayer target LAYER_COMPILE_DEFINITIONS)
        add_library(VkLayer_${target} SHARED ${ARGN})
        target_compile_definitions(VkLayer_${target} PUBLIC ${LAYER_COMPILE_DEFINITIONS})
        target_link_libraries(VkLayer_${target} PRIVATE VkExtLayer_utils)
        add_dependencies(VkLayer_${target} VkExtLayer_utils)
        set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libVkLayer_${target}.map,-Bsymbolic,--exclude-libs,ALL")
        install(TARGETS VkLayer_${target} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endmacro()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/utils
                    ${PROJECT_SOURCE_DIR}/utils/generated
                    ${VulkanHeaders_INCLUDE_DIRS})

if(WIN32)
    # Applies to all configurations
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
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

# The output file needs Unix "/" separators or Windows "\" separators On top of that, Windows separators actually need to be doubled
# because the json format uses backslash escapes
file(TO_NATIVE_PATH "./" RELATIVE_PATH_PREFIX)
string(REPLACE "\\"
               "\\\\"
               RELATIVE_PATH_PREFIX
               "${RELATIVE_PATH_PREFIX}")

# Run each .json.in file through the generator We need to create the generator.cmake script so that the generator can be run at
# compile time, instead of configure time Running at compile time lets us use cmake generator expressions (TARGET_FILE_NAME and
# TARGET_FILE_DIR, specifically)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generator.cmake" "configure_file(\"\${INPUT_FILE}\" \"\${OUTPUT_FILE}\")")
foreach(TARGET_NAME ${TARGET_NAMES})
    set(CONFIG_DEFINES -DINPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/json/${TARGET_NAME}.json.in" -DVK_VERSION=1.2.${vk_header_version})
    # If this json file is not a metalayer, get the needed properties from that target
    if(TARGET ${TARGET_NAME})
        set(CONFIG_DEFINES
            ${CONFIG_DEFINES}
            -DOUTPUT_FILE="$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.json"
            -DRELATIVE_LAYER_BINARY="${RELATIVE_PATH_PREFIX}$<TARGET_FILE_NAME:${TARGET_NAME}>")
        # If this json file is a metalayer, make the output path match core validation, and there is no layer binary file
    endif()
    add_custom_target(${TARGET_NAME}-json ALL
                      COMMAND ${CMAKE_COMMAND} ${CONFIG_DEFINES} -P "${CMAKE_CURRENT_BINARY_DIR}/generator.cmake")
    if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
        set_target_properties(${TARGET_NAME}-json PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
    endif()
endforeach()

# For UNIX-based systems, `library_path` should not contain a relative path (indicated by "./") before installing to system
# directories, so we do not include it in the staging-json files which are used for installation
if(UNIX)
    foreach(TARGET_NAME ${TARGET_NAMES})
        set(INSTALL_DEFINES
            -DINPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/json/${TARGET_NAME}.json.in"
            -DOUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/staging-json/${TARGET_NAME}.json"
            -DVK_VERSION=1.1.${vk_header_version})
        # If this json file is not a metalayer, get the needed properties from that target
        if(TARGET ${TARGET_NAME})
            set(INSTALL_DEFINES ${INSTALL_DEFINES} -DRELATIVE_LAYER_BINARY="$<TARGET_FILE_NAME:${TARGET_NAME}>")
        endif()
        add_custom_target(${TARGET_NAME}-staging-json ALL
                          COMMAND ${CMAKE_COMMAND} ${INSTALL_DEFINES} -P "${CMAKE_CURRENT_BINARY_DIR}/generator.cmake")
    endforeach()
endif()
