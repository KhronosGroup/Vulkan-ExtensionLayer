/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

/*
 * This file is processed using CMake to define the build folder of the layers.
 * Thus, the layers can set VK_LAYER_PATH on startup and be able to find all of the layers immediately, rather than relying on
 * the user to set the path properly.
 *
 * File Usage: Just include "test_layer_location.h" in the executable
 */

#pragma once

#define LAYER_BUILD_LOCATION "$<TARGET_FILE_DIR:VkLayer_khronos_synchronization2>"
