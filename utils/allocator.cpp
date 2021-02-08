/* Copyright (c) 2020-2021 The Khronos Group Inc.
 * Copyright (c) 2020-2021 LunarG, Inc.
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
#include "allocator.h"
#include <cstdlib>

namespace extension_layer {
// currently none of the extension layers have special alignment requirements so we just use malloc / free.
static VKAPI_ATTR void* VKAPI_CALL DefaultAlloc(void*, size_t size, size_t alignment, VkSystemAllocationScope) {
    return std::malloc(size);
}

static VKAPI_ATTR void VKAPI_CALL DefaultFree(void*, void* pMem) { std::free(pMem); }

static VKAPI_ATTR void* VKAPI_CALL DefaultRealloc(void*, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope) {
    return std::realloc(pOriginal, size);
}

const VkAllocationCallbacks kDefaultAllocator = {
    nullptr, DefaultAlloc, DefaultRealloc, DefaultFree, nullptr, nullptr,
};

}  // namespace extension_layer
