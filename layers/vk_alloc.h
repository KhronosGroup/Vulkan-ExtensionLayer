/*
 * Copyright © 2019 Intel Corporation
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

#ifndef VK_ALLOC_H_
#define VK_ALLOC_H_

#include <vulkan/vulkan.h>

static inline void *vk_alloc(const VkAllocationCallbacks *alloc,
                             size_t size, size_t alignment,
                             VkSystemAllocationScope scope)
{
    return alloc->pfnAllocation(alloc->pUserData, size, alignment, scope);
}

static inline void *vk_alloc2(const VkAllocationCallbacks *alloc,
                              const VkAllocationCallbacks *user_alloc,
                              size_t size, size_t alignment,
                              VkSystemAllocationScope scope)
{
    return vk_alloc(user_alloc ? user_alloc : alloc, size, alignment, scope);
}

static inline void *vk_zalloc(const VkAllocationCallbacks *alloc,
                              size_t size, size_t alignment,
                              VkSystemAllocationScope scope)
{
    void *ptr = alloc->pfnAllocation(alloc->pUserData, size, alignment, scope);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

static inline void vk_free(const VkAllocationCallbacks *alloc, void *ptr)
{
    alloc->pfnFree(alloc->pUserData, ptr);
}

static inline void vk_free2(const VkAllocationCallbacks *alloc,
                            const VkAllocationCallbacks *user_alloc,
                            void *ptr)
{
    vk_free(user_alloc ? user_alloc : alloc, ptr);
}

#endif /* VK_ALLOC_H_ */
