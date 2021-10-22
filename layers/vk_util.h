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
 *
 * Author: Jason Ekstrand <jason@jlekstrand.net>
 */

#ifndef VK_UTIL_H_
#define VK_UTIL_H_

#include <vulkan/vulkan.h>

#include "vk_alloc.h"

// "restrict" is a C feature, and C++ compilers may not support it.
#ifdef __cplusplus
#define restrict
#endif

#ifdef _MSC_VER                 /* Visual Studio */
#pragma warning(disable : 4127) /* disable: C4127: conditional expression is constant */
#define FORCE_INLINE static __forceinline
#else
#if defined(__cplusplus) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#ifdef __GNUC__
#define FORCE_INLINE static inline __attribute__((always_inline))
#else
#define FORCE_INLINE static inline
#endif
#else
#define FORCE_INLINE static
#endif /* __STDC_VERSION__ */
#endif

/**
 * @file vk_util.h
 *
 * @brief Vulkan helpers
 */

#define NSEC_PER_SEC 1000000000ull

#define unreachable(expr) assert(!(expr))

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) < (b) ? (a) : (b))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define vk_foreach_struct(name, first) \
  for (VkBaseOutStructure *name = first; name != NULL; name = name->pNext)

#define vk_foreach_struct_const(name, first) \
  for (const VkBaseInStructure *name = first; name != NULL; name = name->pNext)

static inline const void *_vk_find_struct_const(const VkBaseInStructure *chain,
                                                VkStructureType sType)
{
  while (chain) {
    if (chain->sType == sType)
      return chain;
    chain = chain->pNext;
  }
  return NULL;
}

static inline void *_vk_find_struct(VkBaseOutStructure *chain,
                                    VkStructureType sType)
{
  while (chain) {
    if (chain->sType == sType)
      return chain;
    chain = chain->pNext;
  }
  return NULL;
}

#define vk_find_struct_const(chain, type) \
  _vk_find_struct_const(chain, VK_STRUCTURE_TYPE_##type)

#define vk_find_struct(chain, type) \
  _vk_find_struct(chain, VK_STRUCTURE_TYPE_##type)

/**
 * A wrapper for a Vulkan output array. A Vulkan output array is one that
 * follows the convention of the parameters to
 * vkGetPhysicalDeviceQueueFamilyProperties().
 *
 * Example Usage:
 *
 *    VkResult
 *    vkGetPhysicalDeviceQueueFamilyProperties(
 *       VkPhysicalDevice           physicalDevice,
 *       uint32_t*                  pQueueFamilyPropertyCount,
 *       VkQueueFamilyProperties*   pQueueFamilyProperties)
 *    {
 *       VK_OUTARRAY_MAKE(props, pQueueFamilyProperties,
 *                         pQueueFamilyPropertyCount);
 *
 *       vk_outarray_append(&props, p) {
 *          p->queueFlags = ...;
 *          p->queueCount = ...;
 *       }
 *
 *       vk_outarray_append(&props, p) {
 *          p->queueFlags = ...;
 *          p->queueCount = ...;
 *       }
 *
 *       return vk_outarray_status(&props);
 *    }
 */
struct _vk_outarray {
   /** May be null. */
   void *data;

   /**
    * Capacity, in number of elements. Capacity is unlimited (UINT32_MAX) if
    * data is null.
    */
   uint32_t cap;

   /**
    * Count of elements successfully written to the array. Every write is
    * considered successful if data is null.
    */
   uint32_t *filled_len;

   /**
    * Count of elements that would have been written to the array if its
    * capacity were sufficient. Vulkan functions often return VK_INCOMPLETE
    * when `*filled_len < wanted_len`.
    */
   uint32_t wanted_len;
};

static inline void
_vk_outarray_init(struct _vk_outarray *a,
                  void *data, uint32_t *restrict len)
{
   a->data = data;
   a->cap = *len;
   a->filled_len = len;
   *a->filled_len = 0;
   a->wanted_len = 0;

   if (a->data == NULL)
      a->cap = UINT32_MAX;
}

static inline VkResult
_vk_outarray_status(const struct _vk_outarray *a)
{
   if (*a->filled_len < a->wanted_len)
      return VK_INCOMPLETE;
   else
      return VK_SUCCESS;
}

static inline void *
_vk_outarray_next(struct _vk_outarray *a, size_t elem_size)
{
   void *p = NULL;

   a->wanted_len += 1;

   if (*a->filled_len >= a->cap)
      return NULL;

   if (a->data != NULL)
      p = (uint8_t *)a->data + (*a->filled_len) * elem_size;

   *a->filled_len += 1;

   return p;
}

#define vk_outarray(elem_t) \
   struct { \
      struct _vk_outarray base; \
      elem_t meta[]; \
   }
#ifdef __cplusplus
#define vk_outarray_typeof_elem(a) std::remove_reference<decltype((a)->meta[0])>::type
#else
#define vk_outarray_typeof_elem(a) __typeof__((a)->meta[0])
#endif
#define vk_outarray_sizeof_elem(a) sizeof((a)->meta[0])

#define vk_outarray_init(a, data, len) \
   _vk_outarray_init(&(a)->base, (data), (len))

#ifdef __cplusplus
#define VK_OUTARRAY_MAKE(name, data, len)                               \
    vk_outarray(std::remove_reference<decltype((data)[0])>::type) name; \
    vk_outarray_init(&name, (data), (len))
#else
#define VK_OUTARRAY_MAKE(name, data, len) \
   vk_outarray(__typeof__((data)[0])) name; \
   vk_outarray_init(&name, (data), (len))
#endif

#define vk_outarray_status(a) \
   _vk_outarray_status(&(a)->base)

#define vk_outarray_next(a) \
   ((vk_outarray_typeof_elem(a) *) \
      _vk_outarray_next(&(a)->base, vk_outarray_sizeof_elem(a)))

/**
 * Append to a Vulkan output array.
 *
 * This is a block-based macro. For example:
 *
 *    vk_outarray_append(&a, elem) {
 *       elem->foo = ...;
 *       elem->bar = ...;
 *    }
 *
 * The array `a` has type `vk_outarray(elem_t) *`. It is usually declared with
 * VK_OUTARRAY_MAKE(). The variable `elem` is block-scoped and has type
 * `elem_t *`.
 *
 * The macro unconditionally increments the array's `wanted_len`. If the array
 * is not full, then the macro also increment its `filled_len` and then
 * executes the block. When the block is executed, `elem` is non-null and
 * points to the newly appended element.
 */
#define vk_outarray_append(a, elem) \
   for (vk_outarray_typeof_elem(a) *elem = vk_outarray_next(a); \
        elem != NULL; elem = NULL)

/* A multi-pointer allocator
 *
 * When copying data structures from the user (such as a render pass), it's
 * common to need to allocate data for a bunch of different things.  Instead
 * of doing several allocations and having to handle all of the error checking
 * that entails, it can be easier to do a single allocation.  This struct
 * helps facilitate that.  The intended usage looks like this:
 *
 *    VK_MULTIALLOC(ma)
 *    vk_multialloc_add(&ma, &main_ptr, 1);
 *    vk_multialloc_add(&ma, &substruct1, substruct1Count);
 *    vk_multialloc_add(&ma, &substruct2, substruct2Count);
 *
 *    if (!vk_multialloc_alloc(&ma, pAllocator, VK_ALLOCATION_SCOPE_FOO))
 *       return vk_error(VK_ERROR_OUT_OF_HOST_MEORY);
 */
struct vk_multialloc {
    size_t size;
    size_t align;

    uint32_t ptr_count;
    void **ptrs[10];
};

#define VK_MULTIALLOC_INIT \
   ((struct vk_multialloc) { 0, })

#define VK_MULTIALLOC(_name) \
   struct vk_multialloc _name = VK_MULTIALLOC_INIT

static inline uint64_t
align_u64(uint64_t v, uint64_t a)
{
    assert(a != 0 && a == (a & -(int64_t)a));
    return (v + a - 1) & ~(a - 1);
}

FORCE_INLINE void _vk_multialloc_add(struct vk_multialloc *ma, void **ptr, size_t size, size_t align) {
    size_t offset = (size_t)align_u64(ma->size, align);
    ma->size = offset + size;
    ma->align = MAX2(ma->align, align);

    /* Store the offset in the pointer. */
    *ptr = (void *)(uintptr_t)offset;

    assert(ma->ptr_count < ARRAY_SIZE(ma->ptrs));
    ma->ptrs[ma->ptr_count++] = ptr;
}

#define vk_multialloc_add_size(_ma, _ptr, _size) \
   _vk_multialloc_add((_ma), (void **)(_ptr), (_size), __alignof__(**(_ptr)))

#define vk_multialloc_add(_ma, _ptr, _count) \
   vk_multialloc_add_size(_ma, _ptr, (_count) * sizeof(**(_ptr)));

FORCE_INLINE void *vk_multialloc_alloc(struct vk_multialloc *ma, const VkAllocationCallbacks *alloc,
                                       VkSystemAllocationScope scope) {
    void *ptr = vk_alloc(alloc, ma->size, ma->align, scope);
    if (!ptr) return NULL;

    /* Fill out each of the pointers with their final value.
     *
     *   for (uint32_t i = 0; i < ma->ptr_count; i++)
     *      *ma->ptrs[i] = ptr + (uintptr_t)*ma->ptrs[i];
     *
     * Unfortunately, even though ma->ptr_count is basically guaranteed to be a
     * constant, GCC is incapable of figuring this out and unrolling the loop
     * so we have to give it a little help.
     */
    assert(ARRAY_SIZE(ma->ptrs) == 10);
#define _VK_MULTIALLOC_UPDATE_POINTER(_i) \
   if ((_i) < ma->ptr_count) \
       *ma->ptrs[_i] = (void *)((uintptr_t) ptr + (uintptr_t)*ma->ptrs[_i])
   _VK_MULTIALLOC_UPDATE_POINTER(0);
   _VK_MULTIALLOC_UPDATE_POINTER(1);
   _VK_MULTIALLOC_UPDATE_POINTER(2);
   _VK_MULTIALLOC_UPDATE_POINTER(3);
   _VK_MULTIALLOC_UPDATE_POINTER(4);
   _VK_MULTIALLOC_UPDATE_POINTER(5);
   _VK_MULTIALLOC_UPDATE_POINTER(6);
   _VK_MULTIALLOC_UPDATE_POINTER(7);
   _VK_MULTIALLOC_UPDATE_POINTER(8);
   _VK_MULTIALLOC_UPDATE_POINTER(9);
#undef _VK_MULTIALLOC_UPDATE_POINTER

   return ptr;
}

FORCE_INLINE void *vk_multialloc_alloc2(struct vk_multialloc *ma, const VkAllocationCallbacks *parent_alloc,
                                        const VkAllocationCallbacks *alloc, VkSystemAllocationScope scope) {
    return vk_multialloc_alloc(ma, alloc ? alloc : parent_alloc, scope);
}

#endif /* VK_UTIL_H_ */
