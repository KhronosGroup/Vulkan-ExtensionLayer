/* Copyright (c) 2021 The Khronos Group Inc.
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
 */
#pragma once

#include <stdlib.h>

// Basic Logging Support
#ifdef __ANDROID__
#include <android/log.h>
#define LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "VulkanExtensionLayer", __VA_ARGS__))
#define LOG_FATAL(...)                                                                 \
    (void)__android_log_print(ANDROID_LOG_FATAL, "VulkanExtensionLayer", __VA_ARGS__); \
    exit(1);
#else  // __ANDROID__
#include <stdio.h>
#define LOG(...)                  \
    fprintf(stdout, __VA_ARGS__); \
    fflush(stdout);
#endif

// Define own assert because <cassert> on android will not actually assert anything
#ifdef __ANDROID__
#define ASSERT(condition)                                                      \
    do {                                                                       \
        if (!(condition)) {                                                    \
            LOG_FATAL("ASSERT: %s at %s:%d\n", #condition, __FILE__, __LINE__) \
        }                                                                      \
    } while (0)
#else  // __ANDROID__
#include <cassert>
#define ASSERT(condition) assert(condition);
#endif
