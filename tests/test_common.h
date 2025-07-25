/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Charles Giessen <charles@LunarG.com>
 */

#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
// WinSock2.h must be included *BEFORE* windows.h
#include <winsock2.h>
#endif

#include <vulkan/vulkan.h>

#ifdef _MSC_VER
#pragma warning(push)
/*
    warnings 4251 and 4275 have to do with potential dll-interface mismatch
    between library (gtest) and users. Since we build the gtest library
    as part of the test build we know that the dll-interface will match and
    can disable these warnings.
 */
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#endif

// GTest and Xlib collide due to redefinitions of "None" and "Bool"
#ifdef VK_USE_PLATFORM_XLIB_KHR
#pragma push_macro("None")
#pragma push_macro("Bool")
#undef None
#undef Bool
#endif

#include <gtest/gtest.h>

// Redefine Xlib definitions
#ifdef VK_USE_PLATFORM_XLIB_KHR
#pragma pop_macro("Bool")
#pragma pop_macro("None")
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "vktestbinding.h"

#define ASSERT_VK_SUCCESS(err)                                                 \
    {                                                                          \
        const VkResult resolved_err = err;                                     \
        ASSERT_EQ(VK_SUCCESS, resolved_err) << vk_result_string(resolved_err); \
    }

static inline const char *vk_result_string(VkResult err) {
    switch (err) {
#define STR(r) \
    case r:    \
        return #r
        STR(VK_SUCCESS);
        STR(VK_NOT_READY);
        STR(VK_TIMEOUT);
        STR(VK_EVENT_SET);
        STR(VK_EVENT_RESET);
        STR(VK_INCOMPLETE);
        STR(VK_ERROR_OUT_OF_HOST_MEMORY);
        STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        STR(VK_ERROR_INITIALIZATION_FAILED);
        STR(VK_ERROR_DEVICE_LOST);
        STR(VK_ERROR_MEMORY_MAP_FAILED);
        STR(VK_ERROR_LAYER_NOT_PRESENT);
        STR(VK_ERROR_EXTENSION_NOT_PRESENT);
        STR(VK_ERROR_FEATURE_NOT_PRESENT);
        STR(VK_ERROR_INCOMPATIBLE_DRIVER);
        STR(VK_ERROR_TOO_MANY_OBJECTS);
        STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
        STR(VK_ERROR_FRAGMENTED_POOL);
        STR(VK_ERROR_UNKNOWN);
        STR(VK_ERROR_OUT_OF_POOL_MEMORY);
        STR(VK_ERROR_INVALID_EXTERNAL_HANDLE);
        STR(VK_ERROR_FRAGMENTATION);
        STR(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
        STR(VK_ERROR_SURFACE_LOST_KHR);
        STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(VK_SUBOPTIMAL_KHR);
        STR(VK_ERROR_OUT_OF_DATE_KHR);
        STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(VK_ERROR_VALIDATION_FAILED_EXT);
        STR(VK_ERROR_INVALID_SHADER_NV);
        STR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
        STR(VK_ERROR_NOT_PERMITTED_EXT);
        STR(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
#undef STR
        default:
            return "UNKNOWN_RESULT";
    }
}

#if defined(__linux__) || defined(__APPLE__)
/* Linux-specific common code: */

#include <pthread.h>

// Threads:
typedef pthread_t test_platform_thread;

static inline int test_platform_thread_create(test_platform_thread *thread, void *(*func)(void *), void *data) {
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    return pthread_create(thread, &thread_attr, func, data);
}
static inline int test_platform_thread_join(test_platform_thread thread, void **retval) { return pthread_join(thread, retval); }

// Thread IDs:
typedef pthread_t test_platform_thread_id;
static inline test_platform_thread_id test_platform_get_thread_id() { return pthread_self(); }

// Thread mutex:
typedef pthread_mutex_t test_platform_thread_mutex;
static inline void test_platform_thread_create_mutex(test_platform_thread_mutex *pMutex) { pthread_mutex_init(pMutex, NULL); }
static inline void test_platform_thread_lock_mutex(test_platform_thread_mutex *pMutex) { pthread_mutex_lock(pMutex); }
static inline void test_platform_thread_unlock_mutex(test_platform_thread_mutex *pMutex) { pthread_mutex_unlock(pMutex); }
static inline void test_platform_thread_delete_mutex(test_platform_thread_mutex *pMutex) { pthread_mutex_destroy(pMutex); }
typedef pthread_cond_t test_platform_thread_cond;
static inline void test_platform_thread_init_cond(test_platform_thread_cond *pCond) { pthread_cond_init(pCond, NULL); }
static inline void test_platform_thread_cond_wait(test_platform_thread_cond *pCond, test_platform_thread_mutex *pMutex) {
    pthread_cond_wait(pCond, pMutex);
}
static inline void test_platform_thread_cond_broadcast(test_platform_thread_cond *pCond) { pthread_cond_broadcast(pCond); }

#elif defined(_WIN32)  // defined(__linux__)
// Threads:
typedef HANDLE test_platform_thread;
static inline int test_platform_thread_create(test_platform_thread *thread, void *(*func)(void *), void *data) {
    DWORD threadID;
    *thread = CreateThread(NULL,  // default security attributes
                           0,     // use default stack size
                           (LPTHREAD_START_ROUTINE)func,
                           data,        // thread function argument
                           0,           // use default creation flags
                           &threadID);  // returns thread identifier
    return (*thread != NULL);
}
static inline int test_platform_thread_join(test_platform_thread thread, void **retval) {
    return WaitForSingleObject(thread, INFINITE);
}

// Thread IDs:
typedef DWORD test_platform_thread_id;
static test_platform_thread_id test_platform_get_thread_id() { return GetCurrentThreadId(); }

// Thread mutex:
typedef CRITICAL_SECTION test_platform_thread_mutex;
static void test_platform_thread_create_mutex(test_platform_thread_mutex *pMutex) { InitializeCriticalSection(pMutex); }
static void test_platform_thread_lock_mutex(test_platform_thread_mutex *pMutex) { EnterCriticalSection(pMutex); }
static void test_platform_thread_unlock_mutex(test_platform_thread_mutex *pMutex) { LeaveCriticalSection(pMutex); }
static void test_platform_thread_delete_mutex(test_platform_thread_mutex *pMutex) { DeleteCriticalSection(pMutex); }
typedef CONDITION_VARIABLE test_platform_thread_cond;
static void test_platform_thread_init_cond(test_platform_thread_cond *pCond) { InitializeConditionVariable(pCond); }
static void test_platform_thread_cond_wait(test_platform_thread_cond *pCond, test_platform_thread_mutex *pMutex) {
    SleepConditionVariableCS(pCond, pMutex, INFINITE);
}
static void test_platform_thread_cond_broadcast(test_platform_thread_cond *pCond) { WakeAllConditionVariable(pCond); }
#else                  // defined(_WIN32)

#error The "test_common.h" file must be modified for this OS.

// NOTE: In order to support another OS, an #elif needs to be added (above the
// "#else // defined(_WIN32)") for that OS, and OS-specific versions of the
// contents of this file must be created.

// NOTE: Other OS-specific changes are also needed for this OS.  Search for
// files with "WIN32" in it, as a quick way to find files that must be changed.

#endif  // defined(_WIN32)
