/* Copyright (c) 2020-2021 The Khronos Group Inc.
 * Copyright (c) 2020-2025 LunarG, Inc.
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
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once

#include <memory>
#include <new>
#include <limits>
#include <vulkan/vulkan_core.h>

namespace extension_layer {

// default allocation callbacks to use within extension layers, this struct should not be passed to lower levels
extern const VkAllocationCallbacks kDefaultAllocator;

// adaptor to use VkAllocationCallbacks where a std::allocator is needed
template <typename T, VkSystemAllocationScope AllocScope>
class Allocator {
  public:
    // type definitions
    typedef T type;
    typedef T value_type;

    template <typename T2>
    struct rebind {
        typedef Allocator<T2, AllocScope> other;
    };

    Allocator() = delete;
    explicit Allocator(const VkAllocationCallbacks* allocator) : alloc(allocator) {}
    Allocator(const Allocator&) = default;

    template <typename T2>
    Allocator(const Allocator<T2, AllocScope>& other) : alloc(other.alloc) {}

    T* allocate(size_t num, const void* = 0) {
        T* result = reinterpret_cast<T*>(alloc->pfnAllocation(alloc->pUserData, num * sizeof(T), alignof(T), AllocScope));
#ifndef VEL_NO_EXCEPTIONS
        if (result == nullptr) {
            throw std::bad_alloc();
        }
#endif
        return result;
    }

    void deallocate(T* p, size_t num) { alloc->pfnFree(alloc->pUserData, p); }

    const VkAllocationCallbacks* alloc;
};

// helpers to declare allocators that use different scopes
template <typename T>
using CmdAlloc = Allocator<T, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND>;

template <typename T>
using ObjAlloc = Allocator<T, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>;

template <typename T>
using CacheAlloc = Allocator<T, VK_SYSTEM_ALLOCATION_SCOPE_CACHE>;

template <typename T>
using DevAlloc = Allocator<T, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>;

template <typename T>
using InstAlloc = Allocator<T, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>;

}  // namespace extension_layer
