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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

#include "hash_table.h"
#include "list.h"

#include "vk_alloc.h"
#include "vk_util.h"

#define ENTRY_POINTS \
    ENTRY_POINT(GetInstanceProcAddr) \
    \
    ENTRY_POINT(EnumeratePhysicalDevices) \
    ENTRY_POINT(EnumerateDeviceExtensionProperties) \
    ENTRY_POINT(GetPhysicalDeviceFeatures2) \
    ENTRY_POINT(GetPhysicalDeviceProperties2) \
    ENTRY_POINT(GetPhysicalDeviceQueueFamilyProperties) \
    ENTRY_POINT(GetPhysicalDeviceExternalSemaphoreProperties) \
    \
    ENTRY_POINT(DestroyInstance) \
    \
    ENTRY_POINT(GetDeviceProcAddr) \
    ENTRY_POINT(GetDeviceQueue) \
    ENTRY_POINT(DestroyDevice) \
    ENTRY_POINT(DeviceWaitIdle) \
    ENTRY_POINT(ImportSemaphoreFdKHR) \
    \
    ENTRY_POINT(CreateFence) \
    ENTRY_POINT(DestroyFence) \
    ENTRY_POINT(ResetFences) \
    ENTRY_POINT(GetFenceStatus) \
    ENTRY_POINT(WaitForFences) \
    \
    ENTRY_POINT(CreateSemaphore) \
    ENTRY_POINT(DestroySemaphore) \
    \
    ENTRY_POINT(QueueWaitIdle) \
    ENTRY_POINT(QueueSubmit) \
    ENTRY_POINT(QueueBindSparse) \
    \
    ENTRY_POINT(AcquireNextImageKHR) \
    ENTRY_POINT(QueuePresentKHR)

    struct vulkan_vtable {
#define ENTRY_POINT(name) PFN_vk##name name;
        ENTRY_POINTS
#undef ENTRY_POINT
    };

struct object_map {
    struct hash_table *map;
    pthread_mutex_t lock;
};

/* Attached to VkInstance/VkPhysicalDevice objects */
struct instance_data {
    struct vulkan_vtable vtable;
    VkInstance instance;

    VkAllocationCallbacks alloc;
};

struct device_data;

/* Attached to VkQueue objects */
struct queue_data {
    struct device_data *device;

    VkQueue queue;
    VkQueueFamilyProperties props;

    struct list_head waiting_submissions; /* list of struct queue_submit */

    /* Used to transfer temporary payloads from one */
    struct list_head wait_points; /* list of struct timeline_wait_point */
};

/* Attached to VkDevice objects */
struct device_data {
    pthread_mutex_t lock;
    pthread_cond_t queue_submit;

    struct instance_data *instance;

    PFN_vkSetDeviceLoaderData set_device_loader_data;

    struct vulkan_vtable vtable;
    VkPhysicalDevice physical_device;
    VkDevice device;

    struct queue_data *queues;
    uint32_t n_queues;

    struct object_map semaphores;
    struct object_map temporary_import_semaphores;

    VkAllocationCallbacks alloc;

    struct list_head free_points; /* List of struct timeline_points */
    struct list_head free_wait_points; /* list of struct timeline_wait_point */
    struct list_head free_point_semaphores; /* list of struct timeline_point_semaphore */
    struct list_head free_point_fences; /* list of struct timeline_point_fence */
};


/* Attached to VkSemaphore objects of timeline type */
struct timeline_semaphore {
    struct device_data *device;

    /* Last point to have been signaled */
    uint64_t highest_past;
    /* Last point to have been submitted for signaling */
    uint64_t highest_pending;

    VkAllocationCallbacks alloc;

    struct list_head points; /* list of struct timeline_point */

    struct list_head wait_points; /* list of struct timeline_wait_point */

    struct list_head link;
};

/* Fence associated with timeline values to allow for host wait operations. */
struct timeline_point_fence {
    /* Fence associated with a semaphore signaling, we use this to check the
     * status of the semaphore.
     */
    VkFence fence;

    uint32_t refcount;

    struct list_head link;
};

/* Semaphore associated with timeline values to allow for device
 * synchronization.
 */
struct timeline_point_semaphore {
    /* Semaphore substitued on QueueSubmit() */
    VkSemaphore semaphore;

    /* If this point has never been waited upon & signaled. We can directly use
     * the semaphore that has been signaled but not yet waited upon, otherwise
     * we need to created a new semaphore submit it on the queue and wait on
     * that new semaphore.
     */
    bool device_waited;
    bool device_signaled;

    /* Where the semaphore was submitted for signaling. */
    struct queue_data *queue;

    uint32_t refcount;

    struct list_head link;
};

/* Represents a point on the timeline */
struct timeline_point {
    struct timeline_semaphore *timeline;

    struct list_head link;

    int waiting;
    uint64_t serial;

    /* Where the semaphore was submitted for signaling. */
    struct queue_data *queue;

    /* Semaphore for device side wait/signal operations. */
    struct timeline_point_semaphore *semaphore;

    /* Fence we use to do host side wait operations of the timeline point. */
    struct timeline_point_fence *fence;
};

/* For a given point we might need multiple semaphore to allow for the 1 to N
 * wait relationship that doesn't exist with binary semaphores.
 */
struct timeline_wait_point {
    struct timeline_point *point;

    struct list_head link;

    struct timeline_point_semaphore *semaphore;

    /* This is the fence we use to do host side wait operations of the timeline
     * point.
     */
    struct timeline_point_fence *fence;
};

struct queue_submit {
    VkStructureType stype;
    VkBaseOutStructure pnext;

    union {
        /* Copied from VkSubmitInfo */
        struct {
            VkPipelineStageFlags *wait_stage_mask;

            VkCommandBuffer *command_buffers;
            uint32_t n_command_buffers;
        };

        /* Copied from VkBindSparseInfo */
        struct {
            VkSparseBufferMemoryBindInfo *buffer_binds;
            uint32_t n_buffer_binds;

            VkSparseImageOpaqueMemoryBindInfo *image_opaque_binds;
            uint32_t n_image_opaque_binds;

            VkSparseImageMemoryBindInfo *image_binds;
            uint32_t n_image_binds;
        };
    };

    /* Fence coming from the application. */
    VkFence fence;

    /* Translated timeline semaphores into binary semaphores by the layer.
     * Includes a first set of clone binary semaphores followed by semaphores
     * coming from the layer to emulate timeline semaphores.
     */
    VkSemaphore *wait_semaphores;
    VkSemaphore *signal_semaphores;

    uint32_t n_wait_semaphores;
    uint32_t n_signal_semaphores;

    /* These are semaphores we need to signal before doing the actual
     * QueueSubmit onto the implementation. Each semaphore might be signaled on
     * a different queue.
     */
    struct timeline_point_semaphore **serialize_semaphores;
    uint32_t n_serialize_semaphores;

    /* Array of timeline semaphores the layer has to wait on to be available
     * before submitting them to the implementation.
     */
    struct timeline_semaphore_ref {
        struct timeline_semaphore *semaphore;
        uint64_t value;
    } *wait_timeline_semaphores, *signal_timeline_semaphores;
    uint32_t n_wait_timeline_semaphores;
    uint32_t n_signal_timeline_semaphores;

    struct list_head link;
};

static void free_submit_info(struct device_data *device, struct queue_submit *info);
static void timeline_wait_point_free_locked(struct device_data *device,
                                            struct timeline_wait_point *wait_point);
static VkResult device_submit_deferred_locked(struct device_data *device);

/**/

static uint64_t gettime_ns(void)
{
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return (uint64_t)current.tv_sec * NSEC_PER_SEC + current.tv_nsec;
}


static uint64_t absolute_timeout(uint64_t timeout)
{
    if (timeout == 0)
        return 0;
    uint64_t current_time = gettime_ns();
    uint64_t max_timeout = (uint64_t) INT64_MAX - current_time;

    timeout = MIN2(max_timeout, timeout);

    return (current_time + timeout);
}

/* Mapping of dispatchable objects */

static struct object_map global_map = {
    .map = NULL,
    .lock = PTHREAD_MUTEX_INITIALIZER,
};

static inline void object_map_ensure_initialized(struct object_map *map)
{
    if (!map->map)
        map->map = hash_table_new();

}
static void object_map(struct object_map *map, void *obj, void *data)
{
    pthread_mutex_lock(&map->lock);
    object_map_ensure_initialized(map);
    hash_table_insert(map->map, (uintptr_t) obj, data);
    pthread_mutex_unlock(&map->lock);
}

static void object_unmap(struct object_map *map, void *obj)
{
    pthread_mutex_lock(&map->lock);
    hash_table_remove(map->map, (uintptr_t) obj);
    pthread_mutex_unlock(&map->lock);
}

static void *object_find(struct object_map *map, void *obj)
{
    pthread_mutex_lock(&map->lock);
    object_map_ensure_initialized(map);
    void *data = hash_table_search(map->map, (uintptr_t) obj);
    pthread_mutex_unlock(&map->lock);
    return data;
}

/**/

#define VK_CHECK(expr) \
    do { \
        VkResult __result = (expr); \
        if (__result != VK_SUCCESS) { \
            fprintf(stderr, "'%s' line %i failed with %i\n", \
                    #expr, __LINE__, __result); \
        } \
    } while (0)

/**/

static void *default_alloc_func(void *pUserData, size_t size, size_t align,
                                VkSystemAllocationScope allocationScope)
{
    return malloc(size);
}

static void *default_realloc_func(void *pUserData, void *pOriginal, size_t size,
                                  size_t align, VkSystemAllocationScope allocationScope)
{
    return realloc(pOriginal, size);
}

static void default_free_func(void *pUserData, void *pMemory)
{
    free(pMemory);
}

static const VkAllocationCallbacks default_alloc = {
    .pUserData = NULL,
    .pfnAllocation = default_alloc_func,
    .pfnReallocation = default_realloc_func,
    .pfnFree = default_free_func,
};

/**/

static void fill_instance_vtable(struct vulkan_vtable *vtable, VkInstance instance,
                                 PFN_vkGetInstanceProcAddr get_proc_addr)
{
#define ENTRY_POINT(name) \
    vtable->name = (PFN_vk##name) get_proc_addr(instance, "vk"#name);
    ENTRY_POINTS
#undef ENTRY_POINT
        }

static void fill_device_vtable(struct vulkan_vtable *vtable, VkDevice device,
                               PFN_vkGetDeviceProcAddr get_proc_addr)
{
#define ENTRY_POINT(name) vtable->name = (PFN_vk##name) get_proc_addr(device, "vk"#name);
    ENTRY_POINTS
#undef ENTRY_POINT
}

/**/

static VkLayerInstanceCreateInfo *get_instance_chain_info(const VkInstanceCreateInfo *pCreateInfo,
                                                          VkLayerFunction func)
{
    vk_foreach_struct_const(item, pCreateInfo->pNext) {
        if (item->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
            ((VkLayerInstanceCreateInfo *) item)->function == func)
            return (VkLayerInstanceCreateInfo *) item;
    }
    unreachable("instance chain info not found");
    return NULL;
}

static VkLayerDeviceCreateInfo *get_device_chain_info(const VkDeviceCreateInfo *pCreateInfo,
                                                      VkLayerFunction func)
{
    vk_foreach_struct_const(item, pCreateInfo->pNext) {
        if (item->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO &&
            ((VkLayerDeviceCreateInfo *) item)->function == func)
            return (VkLayerDeviceCreateInfo *)item;
    }
    unreachable("device chain info not found");
    return NULL;
}

/**/

static void instance_destroy(struct instance_data *instance)
{
    VkAllocationCallbacks alloc = instance->alloc;

    object_unmap(&global_map, instance->instance);
    vk_free(&alloc, instance);
}

static VkResult instance_new(VkInstance _instance,
                             PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr,
                             const VkAllocationCallbacks *allocator)
{
    struct instance_data *instance =
        vk_zalloc(allocator, sizeof(*instance), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (!instance)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    instance->instance = _instance;
    instance->alloc = *allocator;
    fill_instance_vtable(&instance->vtable,
                         instance->instance,
                         fpGetInstanceProcAddr);

    object_map(&global_map, instance->instance, instance);

    return VK_SUCCESS;
}

/**/

static void device_destroy(struct device_data *device)
{
    VkAllocationCallbacks alloc = device->alloc;

    for (uint32_t i = 0; i < device->n_queues; i++) {
        struct queue_data *queue = &device->queues[i];

        list_for_each_entry_safe(struct timeline_wait_point, wait_point,
                                 &queue->wait_points, link) {
            list_del(&wait_point->link);
            timeline_wait_point_free_locked(device, wait_point);
        }

        list_for_each_entry_safe(struct queue_submit, submit,
                                 &queue->waiting_submissions, link) {
            list_del(&submit->link);
            free_submit_info(device, submit);
        }

        object_unmap(&global_map, queue->queue);
    }
    list_for_each_entry_safe(struct timeline_point_fence, fence,
                             &device->free_point_fences, link) {
        list_del(&fence->link);
        device->vtable.DestroyFence(device->device, fence->fence, &device->alloc);
        vk_free(&device->alloc, fence);
    }
    list_for_each_entry_safe(struct timeline_point, point,
                             &device->free_points, link) {
        list_del(&point->link);
        vk_free(&device->alloc, point);
    }
    list_for_each_entry_safe(struct timeline_wait_point, wait_point,
                             &device->free_wait_points, link) {
        assert(wait_point->fence == NULL);
        assert(wait_point->semaphore == NULL);
        list_del(&wait_point->link);
        vk_free(&device->alloc, wait_point);
    }
    list_for_each_entry_safe(struct timeline_point_semaphore, semaphore,
                             &device->free_point_semaphores, link) {
        device->vtable.DestroySemaphore(device->device,
                                        semaphore->semaphore,
                                        &device->alloc);
        list_del(&semaphore->link);
        vk_free(&device->alloc, semaphore);
    }

    pthread_mutex_destroy(&device->lock);
    object_unmap(&global_map, device->device);
    pthread_mutex_destroy(&device->semaphores.lock);
    hash_table_destroy(device->semaphores.map);
    vk_free(&alloc, device);
}

static VkResult device_new(VkDevice _device,
                           VkPhysicalDevice physical_device,
                           PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr,
                           PFN_vkSetDeviceLoaderData pfnSetDeviceLoaderData,
                           const VkDeviceCreateInfo *create_info,
                           const VkAllocationCallbacks *allocator,
                           struct instance_data *instance)
{
    uint32_t n_queues = 0;
    for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++)
        n_queues += create_info->pQueueCreateInfos[i].queueCount;

    struct device_data *device =
        vk_zalloc(allocator,
                  sizeof(*device) + n_queues * sizeof(*device->queues),
                  8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!device)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    device->alloc = *allocator;
    device->instance = instance;
    device->device = _device;
    device->physical_device = physical_device;
    device->set_device_loader_data = pfnSetDeviceLoaderData;
    device->queues = (struct queue_data *) (device + 1);
    device->n_queues = n_queues;

    fill_device_vtable(&device->vtable, device->device, fpGetDeviceProcAddr);

    if (pthread_mutex_init(&device->lock, NULL) != 0)
        goto err;

    pthread_condattr_t condattr;
    if (pthread_condattr_init(&condattr) != 0)
        goto err;

    if (pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC) != 0) {
        pthread_condattr_destroy(&condattr);
        goto err;
    }
    if (pthread_cond_init(&device->queue_submit, &condattr) != 0) {
        pthread_condattr_destroy(&condattr);
        goto err;
    }
    pthread_condattr_destroy(&condattr);

    if (pthread_mutex_init(&device->semaphores.lock, NULL) != 0)
        goto err;
    device->semaphores.map = hash_table_new();

    if (pthread_mutex_init(&device->temporary_import_semaphores.lock, NULL) != 0)
        goto err;
    device->temporary_import_semaphores.map = hash_table_new();

    list_inithead(&device->free_point_fences);
    list_inithead(&device->free_point_semaphores);
    list_inithead(&device->free_wait_points);
    list_inithead(&device->free_points);

    object_map(&global_map, device->device, device);

    uint32_t queue_family_count;
    instance->vtable.GetPhysicalDeviceQueueFamilyProperties(device->physical_device,
                                                            &queue_family_count,
                                                            NULL);
    VkQueueFamilyProperties *queue_family_props =
        vk_alloc(&device->alloc, sizeof(*queue_family_props) * queue_family_count,
                 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (!queue_family_props)
        goto err;
    instance->vtable.GetPhysicalDeviceQueueFamilyProperties(device->physical_device,
                                                            &queue_family_count,
                                                            queue_family_props);


    uint32_t queue_idx = 0;
    for (uint32_t qci = 0; qci < create_info->queueCreateInfoCount; qci++) {
        uint32_t queue_family_idx = create_info->pQueueCreateInfos[qci].queueFamilyIndex;

        for (uint32_t qi = 0; qi < create_info->pQueueCreateInfos[qci].queueCount; qi++) {
            struct queue_data *queue = &device->queues[queue_idx++];

            queue->device = device;
            queue->props = queue_family_props[queue_family_idx];
            list_inithead(&queue->waiting_submissions);
            list_inithead(&queue->wait_points);

            device->vtable.GetDeviceQueue(device->device,
                                          queue_family_idx, qi, &queue->queue);
            VK_CHECK(device->set_device_loader_data(device->device, queue->queue));

            object_map(&global_map, queue->queue, queue);
        }
    }

    return VK_SUCCESS;

err:
    device_destroy(device);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

/**/

static VkResult device_get_point_fence_locked(struct device_data *device,
                                              struct timeline_point_fence **fence)
{
    if (!list_empty(&device->free_point_fences)) {
        *fence = list_first_entry(&device->free_point_fences,
                                  struct timeline_point_fence, link);
        list_del(&(*fence)->link);

        (*fence)->refcount = 1;

        return device->vtable.ResetFences(device->device, 1, &(*fence)->fence);
    }

    *fence = vk_zalloc(&device->alloc, sizeof(**fence), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!(*fence))
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    (*fence)->refcount = 1;

    VkResult result = device->vtable.CreateFence(
        device->device,
        &(VkFenceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                },
        &device->alloc,
        &(*fence)->fence);
    if (result != VK_SUCCESS)
        vk_free(&device->alloc, *fence);

    return result;
}

static struct timeline_point_fence *point_fence_ref_locked(struct timeline_point_fence *fence)
{
    fence->refcount++;
    return fence;
}

static void point_fence_unref_locked(struct device_data *device,
                                     struct timeline_point_fence *fence)
{
    assert(!fence || fence->refcount > 0);
    if (!fence || --fence->refcount)
        return;

    list_add(&fence->link, &device->free_point_fences);
}

static VkResult device_get_point_semaphore_locked(struct device_data *device,
                                                  struct timeline_point_semaphore **semaphore)
{
    if (!list_empty(&device->free_point_semaphores)) {
        *semaphore = list_first_entry(&device->free_point_semaphores,
                                      struct timeline_point_semaphore, link);
        list_del(&(*semaphore)->link);
    } else {
        *semaphore = vk_zalloc(&device->alloc, sizeof(**semaphore), 8,
                               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!(*semaphore))
            return VK_ERROR_OUT_OF_HOST_MEMORY;

        VkResult result = device->vtable.CreateSemaphore(
            device->device,
            &(VkSemaphoreCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                    },
            &device->alloc,
            &(*semaphore)->semaphore);

        if (result != VK_SUCCESS) {
            vk_free(&device->alloc, *semaphore);
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }
    }

    (*semaphore)->queue = NULL;
    (*semaphore)->refcount = 1;
    (*semaphore)->device_waited = false;
    (*semaphore)->device_signaled = false;

    return VK_SUCCESS;
}

static struct timeline_point_semaphore *point_semaphore_ref_locked(struct timeline_point_semaphore *semaphore)
{
    semaphore->refcount++;
    return semaphore;
}

static void point_semaphore_unref_locked(struct device_data *device,
                                         struct timeline_point_semaphore *semaphore)
{
    assert(!semaphore || semaphore->refcount > 0);
    if (!semaphore || --semaphore->refcount)
        return;

    /* If the semaphore was waited upon, it's a candidate to be reused,
     * otherwise we need to destroy it.
     */
    if (semaphore->device_waited) {
        list_add(&semaphore->link, &device->free_point_semaphores);
    } else {
        device->vtable.DestroySemaphore(device->device, semaphore->semaphore, &device->alloc);
        vk_free(&device->alloc, semaphore);
    }
}

static void timeline_wait_point_free_locked(struct device_data *device,
                                            struct timeline_wait_point *wait_point)
{
    point_semaphore_unref_locked(device, wait_point->semaphore);
    wait_point->semaphore = NULL;
    point_fence_unref_locked(device, wait_point->fence);
    wait_point->fence = NULL;
    list_add(&wait_point->link, &device->free_wait_points);
}

static void timeline_point_free_locked(struct device_data *device,
                                       struct timeline_point *point)
{
    point_semaphore_unref_locked(device, point->semaphore);
    point->semaphore = NULL;
    point_fence_unref_locked(device, point->fence);
    point->fence = NULL;
    list_add(&point->link, &device->free_points);
}

static VkResult gc_wait_point_list_locked(struct device_data *device, struct list_head *list)
{
    list_for_each_entry_safe(struct timeline_wait_point, wait_point, list, link) {
        VkResult result = device->vtable.GetFenceStatus(device->device,
                                                        wait_point->fence->fence);
        if (result == VK_NOT_READY) {
            break;
        } else if (result != VK_SUCCESS) {
            return result;
        }

        list_del(&wait_point->link);
        timeline_wait_point_free_locked(device, wait_point);
    }

    return VK_SUCCESS;
}

static VkResult timeline_gc_locked(struct device_data *device,
                                   struct timeline_semaphore *semaphore)
{
    /* Garbage collect all serializing semaphores. */
    VkResult result = gc_wait_point_list_locked(device, &semaphore->wait_points);
    if (result != VK_SUCCESS)
        return result;

    /* Now look at the points in order to garbage collect the timeline. */
    list_for_each_entry_safe(struct timeline_point, point,
                             &semaphore->points, link) {
        /* If someone is waiting on this time point, consider it busy and don't
         * try to recycle it. There's a slim possibility that it's no longer
         * busy by the time we look at it but we would be recycling it out from
         * under a waiter and that can lead to weird races.
         *
         * We walk the list in-order so if this time point is still busy so is
         * every following time point
         */
        assert(point->waiting >= 0);
        if (point->waiting)
            return VK_SUCCESS;

        /* We might have points that have been signaled but another submission
         * depended on them and we had to create serialization semaphores,
         * therefore adding more wait points. We try to garbage collect those.
         */
        result = device->vtable.GetFenceStatus(device->device,
                                               point->fence->fence);
        if (result == VK_NOT_READY) {
            /* We walk the list in-order so if this time point is still busy so
             * is every following time point
             */
            return VK_SUCCESS;
        } else if (result != VK_SUCCESS) {
            return result;
        }

        assert(semaphore->highest_past < point->serial);
        semaphore->highest_past = point->serial;

        list_del(&point->link);
        timeline_point_free_locked(device, point);
    }

    return VK_SUCCESS;
}

static VkResult timeline_wait_locked(struct device_data *device,
                                     struct timeline_semaphore **semaphores,
                                     const uint64_t *serials,
                                     uint32_t n_semaphores,
                                     bool wait_all,
                                     uint64_t abs_timeout_ns)
{
    void *alloc;
    struct timeline_point **points;
    VkFence *fences;
    VkResult result = VK_TIMEOUT;
    VK_MULTIALLOC(ma);

    vk_multialloc_add(&ma, &points, n_semaphores);
    vk_multialloc_add(&ma, &fences, n_semaphores);

    if (!(alloc = vk_multialloc_alloc(&ma, &device->alloc, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE)))
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    do {
        uint32_t n_fences = 0;
        uint32_t n_ready = 0;

        /* First look for any signaled semaphore or pick up the fences
         * associated with the semaphores.
         */
        for (uint32_t i = 0; i < n_semaphores; i++) {
            timeline_gc_locked(device, semaphores[i]);
            if (semaphores[i]->highest_past >= serials[i]) {
                if (!wait_all) {
                    result = VK_SUCCESS;
                    goto out;
                }
                n_ready++;
            } else {
                list_for_each_entry(struct timeline_point, point, &semaphores[i]->points, link) {
                    if (point->serial < serials[i])
                        continue;

                    points[n_fences] = point;
                    fences[n_fences] = point->fence->fence;
                    n_fences++;

                    if (!wait_all) {
                        result = device->vtable.GetFenceStatus(device->device, point->fence->fence);
                        if (result != VK_NOT_READY)
                            goto out;
                    }
                }
            }
        }

        if (n_ready == n_semaphores) {
            result = VK_SUCCESS;
            goto out;
        }

        if ((n_fences + n_ready) == n_semaphores) {
            /* If we have all fences available, wait on them, otherwise wait for the
             * broadcast queue_submit to collect more fences.
             */

            for (uint32_t i = 0; i < n_semaphores; i++)
                points[i]->waiting++;

            uint64_t now = gettime_ns();
            pthread_mutex_unlock(&device->lock);
            result = device->vtable.WaitForFences(device->device, n_fences, fences, true,
                                                  now > abs_timeout_ns ? 0 : abs_timeout_ns - now);
            pthread_mutex_lock(&device->lock);

            for (uint32_t i = 0; i < n_semaphores; i++)
                points[i]->waiting--;

            goto out;
        } else {
            struct timespec abstime = {
                .tv_sec = abs_timeout_ns / NSEC_PER_SEC,
                .tv_nsec = abs_timeout_ns % NSEC_PER_SEC,
            };

            int ret = pthread_cond_timedwait(&device->queue_submit,
                                             &device->lock, &abstime);
            assert(ret != EINVAL);
            (void)ret;
        }
    } while (gettime_ns() < abs_timeout_ns);

out:
    vk_free(&device->alloc, alloc);

    return result;
}

static VkResult timeline_create_wait_point_locked(struct timeline_semaphore *semaphore,
                                                  uint64_t serial,
                                                  struct timeline_point_fence *fence,
                                                  struct timeline_point_semaphore **out_point_semaphore)
{
    struct device_data *device = semaphore->device;

    *out_point_semaphore = NULL;

    if (semaphore->highest_past >= serial)
        return VK_SUCCESS;

    assert(serial <= semaphore->highest_pending);

    struct timeline_point *point = NULL;
    list_for_each_entry_rev(struct timeline_point, _point, &semaphore->points, link) {
        if (_point->serial < serial)
            continue;
        point = _point;
        break;
    }

    /* highest_past < serial <= semaphore->highest_pending
     *
     * implies there is at least one point in the list that match our criteria.
     */
    assert(point);

    struct timeline_wait_point *wait_point;
    if (!list_empty(&device->free_wait_points)) {
        wait_point = list_first_entry(&device->free_wait_points,
                                      struct timeline_wait_point, link);
        list_del(&wait_point->link);
    } else {
        wait_point = vk_alloc(&device->alloc, sizeof(*wait_point), 8,
                              VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!wait_point)
            return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    struct timeline_point_semaphore *point_semaphore = NULL;
    /* If the point we created in the timeline was never waited upon but
     * already signaled, we can just use it.
     */
    if (!point->semaphore->device_waited &&
        point->semaphore->device_signaled) {
        point_semaphore = point_semaphore_ref_locked(point->semaphore);
    }

    VkResult result = VK_SUCCESS;

    if (!point_semaphore) {
        result = device_get_point_semaphore_locked(device, &point_semaphore);
        if (result != VK_SUCCESS) {
            vk_free(&device->alloc, wait_point);
            return result;
        }
        point_semaphore->queue = point->queue;
    }

    wait_point->point = point;
    wait_point->semaphore = point_semaphore;
    wait_point->fence = point_fence_ref_locked(fence);

    list_addtail(&wait_point->link, &semaphore->wait_points);

    *out_point_semaphore = wait_point->semaphore;

    return result;
}

static VkResult timeline_create_point_locked(struct queue_data *queue,
                                             struct timeline_semaphore *semaphore,
                                             uint64_t serial,
                                             struct timeline_point_fence *fence,
                                             struct timeline_point **out_point)
{
    struct device_data *device = queue->device;

    /* Timelines must always increase */
    assert(serial > semaphore->highest_pending);

    struct timeline_point *point;
    if (!list_empty(&device->free_points)) {
        point = list_first_entry(&device->free_points,
                                 struct timeline_point, link);
        list_del(&point->link);
    } else {
        point = vk_zalloc(&device->alloc, sizeof(*point), 8,
                          VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!point)
            return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    assert(!point->waiting);
    point->timeline = semaphore;
    point->serial = serial;
    point->queue = queue;

    VkResult result = device_get_point_semaphore_locked(device, &point->semaphore);
    if (result != VK_SUCCESS) {
        vk_free(&device->alloc, point);
        return result;
    }

    point->fence = point_fence_ref_locked(fence);

    semaphore->highest_pending = serial;
    list_addtail(&point->link, &semaphore->points);

    *out_point = point;

    return VK_SUCCESS;
}

/**/

static VkResult timeline_CreateSemaphore(
    VkDevice                                    _device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore)
{
    struct device_data *device = object_find(&global_map, _device);

    const VkSemaphoreTypeCreateInfoKHR *type_create_info =
        vk_find_struct_const(pCreateInfo->pNext, SEMAPHORE_TYPE_CREATE_INFO_KHR);
    if (!type_create_info ||
        type_create_info->semaphoreType != VK_SEMAPHORE_TYPE_TIMELINE_KHR) {
        return device->vtable.CreateSemaphore(_device, pCreateInfo, pAllocator, pSemaphore);
    }

    struct timeline_semaphore *semaphore =
        vk_alloc2(&device->alloc, pAllocator, sizeof(*semaphore), 8,
                  VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    object_map(&device->semaphores, semaphore, semaphore);
    semaphore->device = device;
    semaphore->highest_past = type_create_info->initialValue;
    semaphore->highest_pending = type_create_info->initialValue;
    list_inithead(&semaphore->points);
    list_inithead(&semaphore->wait_points);

    if (pAllocator)
        semaphore->alloc = *pAllocator;
    else
        semaphore->alloc = device->alloc;

    *pSemaphore = (VkSemaphore)(uintptr_t) semaphore;

    return VK_SUCCESS;
}

static void timeline_DestroySemaphore(
    VkDevice                                    _device,
    VkSemaphore                                 _semaphore,
    const VkAllocationCallbacks*                pAllocator)
{
    struct device_data *device = object_find(&global_map, _device);
    struct timeline_semaphore *semaphore = object_find(&device->semaphores, (void*)(uintptr_t)_semaphore);

    if (!semaphore)
        return device->vtable.DestroySemaphore(_device, _semaphore, pAllocator);

    pthread_mutex_lock(&device->lock);

    timeline_gc_locked(device, semaphore);

    list_for_each_entry_safe(struct timeline_wait_point, wait_point,
                             &semaphore->wait_points, link) {
        list_del(&wait_point->link);
        timeline_wait_point_free_locked(device, wait_point);
    }

    list_for_each_entry_safe(struct timeline_point, point,
                             &semaphore->points, link) {
        list_del(&point->link);
        timeline_point_free_locked(device, point);
    }

    pthread_mutex_unlock(&device->lock);

    vk_free2(&device->alloc, pAllocator, semaphore);
}

static VkResult timeline_ImportSemaphoreFdKHR(
    VkDevice                                    _device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo)
{
    struct device_data *device = object_find(&global_map, _device);
    VkResult result = device->vtable.ImportSemaphoreFdKHR(_device, pImportSemaphoreFdInfo);

    /* Mark the semaphore as temporary so we know it needs special treatment on
     * QueueSubmit.
     */
    if (result == VK_SUCCESS &&
        (pImportSemaphoreFdInfo->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT)) {
        pthread_mutex_lock(&device->lock);
        object_map(&device->temporary_import_semaphores, (void*)(uintptr_t)pImportSemaphoreFdInfo->semaphore, device);
        pthread_mutex_unlock(&device->lock);
    }
    return result;
}

static VkResult timeline_GetSemaphoreCounterValueKHR(
    VkDevice                                    _device,
    VkSemaphore                                 _semaphore,
    uint64_t*                                   pValue)
{
    struct device_data *device = object_find(&global_map, _device);
    struct timeline_semaphore *semaphore = object_find(&device->semaphores, (void*)(uintptr_t)_semaphore);

    assert(semaphore);

    pthread_mutex_lock(&device->lock);

    VkResult result = timeline_gc_locked(device, semaphore);
    *pValue = semaphore->highest_past;

    pthread_mutex_unlock(&device->lock);

    return result;
}

static VkResult timeline_WaitSemaphoresKHR(
    VkDevice                                    _device,
    const VkSemaphoreWaitInfoKHR*               pWaitInfo,
    uint64_t                                    timeout)
{
    struct device_data *device = object_find(&global_map, _device);
    struct timeline_semaphore **semaphores =
        vk_alloc(&device->alloc, sizeof(*semaphores) * pWaitInfo->semaphoreCount, 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

    if (!semaphores)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++)
        semaphores[i] = object_find(&device->semaphores, (void*)(uintptr_t)pWaitInfo->pSemaphores[i]);

    pthread_mutex_lock(&device->lock);

    VkResult result =
        timeline_wait_locked(device,
                             semaphores,
                             pWaitInfo->pValues,
                             pWaitInfo->semaphoreCount,
                             (pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT_KHR) == 0,
                             absolute_timeout(timeout));

    pthread_mutex_unlock(&device->lock);

    vk_free(&device->alloc, semaphores);

    return result;
}

static VkResult timeline_SignalSemaphoreKHR(
    VkDevice                                    _device,
    const VkSemaphoreSignalInfoKHR*             pSignalInfo)
{
    struct device_data *device = object_find(&global_map, _device);
    struct timeline_semaphore *semaphore = object_find(&device->semaphores, (void*)(uintptr_t)pSignalInfo->semaphore);

    assert(semaphore);

    pthread_mutex_lock(&device->lock);

    VkResult result = timeline_gc_locked(device, semaphore);

    /* It is not allowed to signal a semaphore from the host while other device
     * operations are still ongoing.
     *
     * It is also not allowed to submit a value <= to the last submitted value.
     *
     * Therefore once garbage collected, we should have no value left in the
     * timeline before we update the value.
     */
    assert(list_empty(&semaphore->points));

    semaphore->highest_pending = semaphore->highest_past = pSignalInfo->value;

    /* Signaling a point might make operations runnable on the variable
     * queues.
     */
    if (result == VK_SUCCESS)
        result = device_submit_deferred_locked(device);

    pthread_cond_broadcast(&device->queue_submit);

    pthread_mutex_unlock(&device->lock);

    return result;
}

static void
free_submit_info(struct device_data *device, struct queue_submit *submit)
{
    for (VkBaseOutStructure *item = (VkBaseOutStructure *) submit->pnext.pNext, *next = item ? item->pNext : NULL;
         item;
         item = next, next = next ? next->pNext : NULL)
        vk_free(&device->alloc, item);
    vk_free(&device->alloc, submit);
}

static VkResult
maybe_clone_semaphore(struct queue_data *queue,
                      VkSemaphore app_semaphore,
                      VkSemaphore *out_layer_semaphore)
{
    struct device_data *device = queue->device;

    /* No special treatement if the semaphore doesn't hold a temporary
     * payload.
     */
    if (!object_find(&device->temporary_import_semaphores, (void*)(uintptr_t)app_semaphore)) {
        *out_layer_semaphore = app_semaphore;
        return VK_SUCCESS;
    }

    /* Otherwise, create a wait_point which holds a tuple (semaphore,fence) and
     * submit it to the implementation. This will drop the temporary payload
     * from the app_semaphore and we can use the new semaphore within the
     * layer.
     */
    struct timeline_wait_point *wait_point;
    if (!list_empty(&device->free_wait_points)) {
        wait_point = list_first_entry(&device->free_wait_points,
                                      struct timeline_wait_point, link);
        list_del(&wait_point->link);
    } else {
        wait_point = vk_alloc(&device->alloc, sizeof(*wait_point), 8,
                              VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!wait_point)
            return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    VkResult result = device_get_point_fence_locked(device, &wait_point->fence);
    if (result != VK_SUCCESS) {
        timeline_wait_point_free_locked(device, wait_point);
        return result;
    }

    result = device_get_point_semaphore_locked(device, &wait_point->semaphore);
    if (result != VK_SUCCESS) {
        timeline_wait_point_free_locked(device, wait_point);
        return result;
    }

    wait_point->point = NULL;
    wait_point->semaphore->queue = queue;

    VkPipelineStageFlags wait_stage_mask = 0;
    VkSubmitInfo info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitSemaphores = &app_semaphore,
        .waitSemaphoreCount = 1,
        .pSignalSemaphores = &wait_point->semaphore->semaphore,
        .signalSemaphoreCount = 1,
        .pWaitDstStageMask = &wait_stage_mask,
    };
    result = device->vtable.QueueSubmit(queue->queue, 1, &info, wait_point->fence->fence);
    if (result != VK_SUCCESS) {
        timeline_wait_point_free_locked(device, wait_point);
        return result;
    }

    list_addtail(&wait_point->link, &queue->wait_points);

    /* The semaphore doesn't hold a temporary payload anymore. */
    object_unmap(&device->temporary_import_semaphores, (void*)(uintptr_t)app_semaphore);

    *out_layer_semaphore = wait_point->semaphore->semaphore;

    return VK_SUCCESS;
}

static VkResult
clone_submit_semaphores(struct queue_data *queue,
                        struct queue_submit *submit,
                        uint32_t n_wait_semaphores,
                        const VkSemaphore *wait_semaphores,
                        const uint64_t *wait_semaphore_values,
                        uint32_t n_signal_semaphores,
                        const VkSemaphore *signal_semaphores,
                        const uint64_t *signal_semaphore_values)
{
    struct device_data *device = queue->device;
    VkResult result = VK_SUCCESS;

    for (uint32_t i = 0; i < n_wait_semaphores; i++) {
        struct timeline_semaphore *semaphore =
            object_find(&device->semaphores, (void*)(uintptr_t)wait_semaphores[i]);
        if (semaphore) {
            assert(wait_semaphore_values);
            submit->wait_timeline_semaphores[submit->n_wait_timeline_semaphores++] =
                (struct timeline_semaphore_ref) {
                .semaphore = semaphore,
                .value = wait_semaphore_values[i],
            };
        } else {
            assert(!wait_semaphore_values || wait_semaphore_values[i] == 0);
            result = maybe_clone_semaphore(queue, wait_semaphores[i],
                                           &submit->wait_semaphores[submit->n_wait_semaphores]);
            if (result != VK_SUCCESS)
                return result;
        }
    }
    for (uint32_t i = 0; i < n_signal_semaphores; i++) {
        struct timeline_semaphore *semaphore =
            object_find(&device->semaphores, (void*)(uintptr_t)signal_semaphores[i]);
        if (semaphore) {
            assert(signal_semaphore_values);
            submit->signal_timeline_semaphores[submit->n_signal_timeline_semaphores++] =
                (struct timeline_semaphore_ref) {
                .semaphore = semaphore,
                .value = signal_semaphore_values[i],
            };
        } else {
            assert(!signal_semaphore_values || signal_semaphore_values[i] == 0);
            /* No need to clone signal semaphores as they do not get their
             * temporary payload reset.
             */
            submit->signal_semaphores[submit->n_signal_semaphores++] = signal_semaphores[i];
        }
    }

    return result;
}

static size_t
vk_submit_structure_type_size(const VkBaseInStructure *item)
{
    switch (item->sType) {
    case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO:
        return sizeof(VkProtectedSubmitInfo);
    case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
        return sizeof(VkDeviceGroupSubmitInfo);
    default:
        unreachable("Unknown structure for VkSubmitInfo::pNext");
    }
    return 0;
}

static VkResult
clone_submit_pnext(struct device_data *device,
                   struct queue_submit *submit,
                   const void *pNext)
{
    VkBaseOutStructure *tail = &submit->pnext;
    vk_foreach_struct_const(item, pNext) {
        /* Skip this element. */
        if (item->sType == VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR)
            continue;

        size_t item_size = vk_submit_structure_type_size(item);
        struct VkBaseOutStructure *new_item =
            vk_alloc(&device->alloc, item_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!new_item) {
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        memcpy(new_item, item, item_size);
        new_item->pNext = NULL;

        if (tail)
            tail->pNext = new_item;
        tail = new_item;
    }

    return VK_SUCCESS;
}

static struct queue_submit *
clone_submit_info(struct queue_data *queue,
                  const VkSubmitInfo *in_info,
                  const VkTimelineSemaphoreSubmitInfoKHR *in_timeline_info,
                  VkFence fence)
{
    struct device_data *device = queue->device;
    struct queue_submit *submit, _submit = { 0 };
    VK_MULTIALLOC(ma);

    vk_multialloc_add(&ma, &submit, 1);
    vk_multialloc_add(&ma, &_submit.command_buffers, in_info->commandBufferCount);
    vk_multialloc_add(&ma, &_submit.wait_stage_mask, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.wait_semaphores, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.serialize_semaphores, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.signal_semaphores, in_info->signalSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.wait_timeline_semaphores, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.signal_timeline_semaphores, in_info->signalSemaphoreCount);

    if (!vk_multialloc_alloc(&ma, &device->alloc, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
        return NULL;

    *submit = _submit;

    submit->stype = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit->fence = fence;
    submit->n_command_buffers = in_info->commandBufferCount;

    if (in_info->pWaitDstStageMask) {
        memcpy(submit->wait_stage_mask, in_info->pWaitDstStageMask,
               in_info->waitSemaphoreCount * sizeof(*submit->wait_stage_mask));
    } else {
        submit->wait_stage_mask = NULL;
    }
    memcpy(submit->command_buffers, in_info->pCommandBuffers,
           submit->n_command_buffers * sizeof(*submit->command_buffers));

    VkResult result = clone_submit_pnext(device, submit, in_info->pNext);
    if (result != VK_SUCCESS) {
        free_submit_info(device, submit);
        return NULL;
    }

    result = clone_submit_semaphores(queue, submit,
                                     in_info->waitSemaphoreCount,
                                     in_info->pWaitSemaphores,
                                     in_timeline_info ? in_timeline_info->pWaitSemaphoreValues : NULL,
                                     in_info->signalSemaphoreCount,
                                     in_info->pSignalSemaphores,
                                     in_timeline_info ? in_timeline_info->pSignalSemaphoreValues : NULL);
    if (result != VK_SUCCESS) {
        free_submit_info(device, submit);
        return NULL;
    }

    return submit;
}

static struct queue_submit *
clone_bind_sparse_info(struct queue_data *queue,
                       const VkBindSparseInfo *in_info,
                       const VkTimelineSemaphoreSubmitInfoKHR *in_timeline_info,
                       VkFence fence)
{
    struct device_data *device = queue->device;
    struct queue_submit *submit, _submit = { 0 };
    VK_MULTIALLOC(ma);

    vk_multialloc_add(&ma, &submit, 1);
    vk_multialloc_add(&ma, &_submit.buffer_binds, in_info->bufferBindCount);
    vk_multialloc_add(&ma, &_submit.image_opaque_binds, in_info->imageOpaqueBindCount);
    vk_multialloc_add(&ma, &_submit.image_binds, in_info->imageBindCount);
    vk_multialloc_add(&ma, &_submit.wait_stage_mask, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.wait_semaphores, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.serialize_semaphores, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.signal_semaphores, in_info->signalSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.wait_timeline_semaphores, in_info->waitSemaphoreCount);
    vk_multialloc_add(&ma, &_submit.signal_timeline_semaphores, in_info->signalSemaphoreCount);

    if (!vk_multialloc_alloc(&ma, &device->alloc, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
        return NULL;

    *submit = _submit;
    submit->fence = fence;
    submit->stype = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;

    submit->n_buffer_binds = in_info->bufferBindCount;
    submit->n_image_opaque_binds = in_info->imageOpaqueBindCount;
    submit->n_image_binds = in_info->imageBindCount;

    VkResult result = clone_submit_pnext(device, submit, in_info->pNext);
    if (result != VK_SUCCESS) {
        free_submit_info(device, submit);
        return NULL;
    }

    result = clone_submit_semaphores(queue, submit,
                                     in_info->waitSemaphoreCount,
                                     in_info->pWaitSemaphores,
                                     in_timeline_info ? in_timeline_info->pWaitSemaphoreValues : NULL,
                                     in_info->signalSemaphoreCount,
                                     in_info->pSignalSemaphores,
                                     in_timeline_info ? in_timeline_info->pSignalSemaphoreValues : NULL);
    if (result != VK_SUCCESS) {
        free_submit_info(device, submit);
        return NULL;
    }

    return submit;
}

static struct queue_submit *
creace_fence_submit_info(struct device_data *device, VkStructureType stype, VkFence fence)
{
    struct queue_submit *submit =
        vk_zalloc(&device->alloc, sizeof(submit), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!submit)
        return NULL;

    submit->stype = stype;
    submit->fence = fence;

    return submit;
}

static bool
queue_submit_wait_fences_available(struct queue_submit *submit)
{
    for (uint32_t i = 0; i < submit->n_wait_timeline_semaphores; i++) {
        if (submit->wait_timeline_semaphores[i].semaphore->highest_pending <
            submit->wait_timeline_semaphores[i].value)
            return false;
    }

    return true;
}

static VkResult
queue_submit_deferred_locked(struct queue_data *queue, uint32_t *out_advance)
{
    struct device_data *device = queue->device;
    VkResult result = VK_SUCCESS;

    list_for_each_entry_safe(struct queue_submit, submit,
                             &queue->waiting_submissions, link) {
        /* Check we can proceed as all the wait semaphores are available. */
        if (!queue_submit_wait_fences_available(submit))
            break;

        /* Number of points actually submitted to the implementation. If 0 we
         * have no use for point_fence.
         */
        uint32_t n_submitted_points = 0;

        /* Fence associated with all submitted points. */
        struct timeline_point_fence *point_fence = NULL;
        result = device_get_point_fence_locked(device, &point_fence);

        /* Rewrite the timeline semaphores instances with underlying
         * semaphores.
         */
        for (uint32_t i = 0; result == VK_SUCCESS && i < submit->n_wait_timeline_semaphores; i++) {
            struct timeline_semaphore *semaphore =
                submit->wait_timeline_semaphores[i].semaphore;
            uint64_t value = submit->wait_timeline_semaphores[i].value;

            struct timeline_point_semaphore *point_semaphore = NULL;
            result = timeline_create_wait_point_locked(semaphore, value,
                                                       point_fence, &point_semaphore);
            if (result == VK_SUCCESS && point_semaphore) {
                submit->wait_semaphores[submit->n_wait_semaphores++] = point_semaphore->semaphore;
                point_semaphore->device_waited = true;
                if (!point_semaphore->device_signaled) {
                    point_semaphore->device_signaled = true;
                    submit->serialize_semaphores[submit->n_serialize_semaphores++] = point_semaphore;
                }
                n_submitted_points++;
            }
        }

        for (uint32_t i = 0; result == VK_SUCCESS && i < submit->n_signal_timeline_semaphores; i++) {
            struct timeline_semaphore *semaphore =
                submit->signal_timeline_semaphores[i].semaphore;
            uint64_t value = submit->signal_timeline_semaphores[i].value;

            struct timeline_point *point = NULL;
            result = timeline_create_point_locked(queue, semaphore, value,
                                                  point_fence, &point);
            if (result == VK_SUCCESS) {
                assert(point);
                submit->signal_semaphores[submit->n_signal_semaphores++] = point->semaphore->semaphore;
                point->semaphore->device_signaled = true;
                n_submitted_points++;
            }
        }

        /* Serialize the waiting semaphores.
         *
         * TODO: This somewhat ineffecient, but batching semaphores per queue is
         * more work...
         */
        for (uint32_t i = 0; result == VK_SUCCESS && i < submit->n_serialize_semaphores; i++) {
            struct timeline_point_semaphore *point_semaphore = submit->serialize_semaphores[i];
            VkSubmitInfo serialize_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pSignalSemaphores = &point_semaphore->semaphore,
                .signalSemaphoreCount = 1,
            };

            result = device->vtable.QueueSubmit(point_semaphore->queue->queue,
                                                1, &serialize_info, VK_NULL_HANDLE);
        }

        /* Now submit the actual work. */
        if (result == VK_SUCCESS) {
            switch (submit->stype) {
            case VK_STRUCTURE_TYPE_SUBMIT_INFO: {
                VkSubmitInfo info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .pNext = submit->pnext.pNext,
                    .pCommandBuffers = submit->command_buffers,
                    .commandBufferCount = submit->n_command_buffers,
                    .pWaitDstStageMask = submit->wait_stage_mask,
                    .pWaitSemaphores = submit->wait_semaphores,
                    .waitSemaphoreCount = submit->n_wait_semaphores,
                    .pSignalSemaphores = submit->signal_semaphores,
                    .signalSemaphoreCount = submit->n_signal_semaphores,
                };

                result = device->vtable.QueueSubmit(queue->queue, 1, &info,
                                                    n_submitted_points > 0 ? point_fence->fence : VK_NULL_HANDLE);
                break;
            }

            case VK_STRUCTURE_TYPE_BIND_SPARSE_INFO: {
                VkBindSparseInfo info = {
                    .sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
                    .pNext = submit->pnext.pNext,
                    .pBufferBinds = submit->buffer_binds,
                    .bufferBindCount = submit->n_buffer_binds,
                    .pImageOpaqueBinds = submit->image_opaque_binds,
                    .imageOpaqueBindCount = submit->n_image_opaque_binds,
                    .pImageBinds = submit->image_binds,
                    .imageBindCount = submit->n_image_binds,
                    .pWaitSemaphores = submit->wait_semaphores,
                    .waitSemaphoreCount = submit->n_wait_semaphores,
                    .pSignalSemaphores = submit->signal_semaphores,
                    .signalSemaphoreCount = submit->n_signal_semaphores,
                };

                result = device->vtable.QueueBindSparse(queue->queue, 1, &info,
                                                        n_submitted_points > 0 ? point_fence->fence : VK_NULL_HANDLE);
                break;
            }

            default:
                unreachable("invalid submit type");
            }

            (*out_advance)++;

            pthread_cond_broadcast(&device->queue_submit);

            if (n_submitted_points > 0)
                assert(point_fence->refcount > 1);
        }

        /* Finally if the user provided a fence, get it signaled. */
        if (result == VK_SUCCESS && submit->fence != VK_NULL_HANDLE) {
            switch (submit->stype) {
            case VK_STRUCTURE_TYPE_SUBMIT_INFO:
                result = device->vtable.QueueSubmit(queue->queue, 0, NULL, submit->fence);
                break;

            case VK_STRUCTURE_TYPE_BIND_SPARSE_INFO:
                result = device->vtable.QueueBindSparse(queue->queue, 0, NULL, submit->fence);
                break;

            default:
                unreachable("invalid submit type");
            }
        }

        point_fence_unref_locked(device, point_fence);

        list_del(&submit->link);
        free_submit_info(device, submit);

        if (result != VK_SUCCESS)
            break;
    }

    return result;
}

static VkResult
device_submit_deferred_locked(struct device_data *device)
{
    VkResult result = VK_SUCCESS;
    uint32_t advance;

    /* Keep posting to the implementation as long as we make progress. */
    do {
        advance = 0;
        for (uint32_t i = 0; result == VK_SUCCESS && i < device->n_queues; i++)
            result = queue_submit_deferred_locked(&device->queues[i], &advance);
    } while (result == VK_SUCCESS && advance > 0);

    return result;
}

static VkResult timeline_QueueSubmit(
    VkQueue                                     _queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence)
{
    struct queue_data *queue = object_find(&global_map, _queue);
    struct device_data *device = queue->device;
    VkResult result = VK_SUCCESS;

    pthread_mutex_lock(&device->lock);

    /* Garbage collect temporary semaphore payload "clones". */
    result = gc_wait_point_list_locked(device, &queue->wait_points);

    for (uint32_t i = 0; i < submitCount && result == VK_SUCCESS; i++) {
        const VkTimelineSemaphoreSubmitInfoKHR *timeline_submit_info =
            vk_find_struct_const(pSubmits[i].pNext, TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR);
        VkFence submit_fence = (i == (submitCount - 1)) ? fence : VK_NULL_HANDLE;

        if (list_empty(&queue->waiting_submissions) && !timeline_submit_info) {
            /* With no timeline semaphores and nothing waiting in the queue, just
             * pass everything onto the implementation.
             */
            result = device->vtable.QueueSubmit(_queue, 1, &pSubmits[i], submit_fence);
        } else {
            /* Otherwise copy all the data we need and put it in the layer's
             * queue.
             */
            struct queue_submit *submit =
                clone_submit_info(queue, &pSubmits[i], timeline_submit_info, submit_fence);
            if (submit) {
                list_addtail(&submit->link, &queue->waiting_submissions);

                /* Try to advance the queue. */
                uint32_t temp = 0;
                result = queue_submit_deferred_locked(queue, &temp);
            } else {
                result = VK_ERROR_OUT_OF_HOST_MEMORY;
            }
        }
    }

    if (submitCount == 0 && fence != VK_NULL_HANDLE && result == VK_SUCCESS) {
        if (list_empty(&queue->waiting_submissions))
            result = device->vtable.QueueSubmit(_queue, 0, NULL, fence);
        else {
            struct queue_submit *submit =
                creace_fence_submit_info(device, VK_STRUCTURE_TYPE_SUBMIT_INFO, fence);
            list_addtail(&submit->link, &queue->waiting_submissions);
        }
    }

    /* Run through the queue to kick-off any work that isn't waiting
     * anymore.
     */
    device_submit_deferred_locked(device);

    pthread_mutex_unlock(&device->lock);

    return result;
}

static VkResult timeline_QueueBindSparse(
    VkQueue                                     _queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence)
{
    struct queue_data *queue = object_find(&global_map, _queue);
    struct device_data *device = queue->device;

    VkResult result = VK_SUCCESS;

    pthread_mutex_lock(&device->lock);

    /* Garbage collect temporary semaphore payload "clones". */
    result = gc_wait_point_list_locked(device, &queue->wait_points);

    for (uint32_t i = 0; i < bindInfoCount && result == VK_SUCCESS; i++) {
        const VkTimelineSemaphoreSubmitInfoKHR *timeline_submit_info =
            vk_find_struct_const(pBindInfo[i].pNext, TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR);
        VkFence submit_fence = (i == (bindInfoCount - 1)) ? fence : VK_NULL_HANDLE;

        if (list_empty(&queue->waiting_submissions) && !timeline_submit_info) {
            /* With no timeline semaphores and nothing waiting in the queue, just
             * pass everything onto the implementation.
             */
            result = device->vtable.QueueBindSparse(_queue, 1, &pBindInfo[i], submit_fence);
        } else {
            /* Otherwise copy all the data we need and put it in the layer's
             * queue.
             */
            struct queue_submit *submit =
                clone_bind_sparse_info(queue, &pBindInfo[i], timeline_submit_info, submit_fence);
            if (submit) {
                list_addtail(&submit->link, &queue->waiting_submissions);

                /* Try to advance the queue. */
                uint32_t temp = 0;
                result = queue_submit_deferred_locked(queue, &temp);
            } else {
                result = VK_ERROR_OUT_OF_HOST_MEMORY;
            }
        }
    }

    if (bindInfoCount == 0 && fence != VK_NULL_HANDLE) {
        assert(result == VK_SUCCESS);
        if (list_empty(&queue->waiting_submissions))
            result = device->vtable.QueueBindSparse(_queue, 0, NULL, fence);
        else {
            struct queue_submit *submit =
                creace_fence_submit_info(device, VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, fence);
            list_addtail(&submit->link, &queue->waiting_submissions);
        }
    }

    /* Run through the queue to kick-off any work that isn't waiting
     * anymore.
     */
    device_submit_deferred_locked(device);

    pthread_mutex_unlock(&device->lock);

    return result;
}

static VkResult timeline_QueuePresentKHR(
    VkQueue                                     _queue,
    const VkPresentInfoKHR*                     pPresentInfo)
{
    struct queue_data *queue = object_find(&global_map, _queue);
    struct device_data *device = queue->device;

    pthread_mutex_lock(&device->lock);

    do {
        device_submit_deferred_locked(device);

        if (list_empty(&queue->waiting_submissions))
            break;

        int ret = pthread_cond_wait(&device->queue_submit,
                                    &device->lock);
        assert(ret != EINVAL);
        (void)ret;
    } while (!list_empty(&queue->waiting_submissions));

    pthread_mutex_unlock(&device->lock);

    return device->vtable.QueuePresentKHR(_queue, pPresentInfo);
}

static VkResult timeline_AcquireNextImageKHR(
    VkDevice                                    _device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex)
{
    struct device_data *device = object_find(&global_map, _device);
    VkResult result = device->vtable.AcquireNextImageKHR(_device, swapchain,
                                                         timeout, semaphore,
                                                         fence, pImageIndex);

    /* Mark the semaphore as temporarly imported :
     *
     * "Because the exportable handle types of an imported semaphore correspond
     *  to its current imported payload, and vkAcquireNextImageKHR behaves the
     *  same as a temporary import operation for which the source semaphore is
     *  opaque to the application, applications have no way of determining
     *  whether any external handle types can be exported from a semaphore in
     *  this state. Therefore, applications must not attempt to export external
     *  handles from semaphores using a temporarily imported payload from
     *  vkAcquireNextImageKHR."
     */
    if (result == VK_SUCCESS && semaphore != VK_NULL_HANDLE) {
        pthread_mutex_lock(&device->lock);
        object_map(&device->temporary_import_semaphores, (void*)(uintptr_t)semaphore, device);
        pthread_mutex_unlock(&device->lock);
    }
    return result;
}

static VkResult timeline_QueueWaitIdle(
    VkQueue                                     _queue)
{
    struct queue_data *queue = object_find(&global_map, _queue);
    struct device_data *device = queue->device;

    pthread_mutex_lock(&device->lock);

    do {
        device_submit_deferred_locked(device);

        if (list_empty(&queue->waiting_submissions))
            break;

        int ret = pthread_cond_wait(&device->queue_submit,
                                    &device->lock);
        assert(ret != EINVAL);
        (void)ret;
    } while (!list_empty(&queue->waiting_submissions));

    pthread_mutex_unlock(&device->lock);

    return device->vtable.QueueWaitIdle(_queue);
}

static bool all_queues_empty(struct device_data *device)
{
    for (uint32_t i = 0; i < device->n_queues; i++) {
        if (!list_empty(&device->queues[i].waiting_submissions)) {
            return false;
        }
    }
    return true;
}

static VkResult timeline_DeviceWaitIdle(
    VkDevice                                    _device)
{
    struct device_data *device = object_find(&global_map, _device);

    pthread_mutex_lock(&device->lock);

    /* Submit anything queued to the implementation. */
    do {
        device_submit_deferred_locked(device);

        if (all_queues_empty(device))
            break;

        int ret = pthread_cond_wait(&device->queue_submit,
                                    &device->lock);
        assert(ret != EINVAL);
        (void)ret;
    } while (!all_queues_empty(device));

    pthread_mutex_unlock(&device->lock);

    return device->vtable.DeviceWaitIdle(_device);
}

static VkResult timeline_EnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{
    if (!pLayerName ||
        strcmp(pLayerName, "VK_LAYER_MESA_timeline_semaphore")) {
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VK_OUTARRAY_MAKE(out, pProperties, pPropertyCount);

    vk_outarray_append(&out, prop) {
        *prop = (VkExtensionProperties) { .extensionName = "VK_KHR_timeline_semaphore", .specVersion = 1, };
    }

    return vk_outarray_status(&out);
}

/* Unfortunately we have to override this function to make sure the extension
 * is picked by the loader, otherwise the application has to specifically
 * request this layer to be enabled.
 */
static VkResult timeline_EnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{
    struct instance_data *instance = object_find(&global_map, physicalDevice);

    if (pLayerName &&
        strcmp(pLayerName, "VK_LAYER_MESA_timeline_semaphore")) {
        return instance->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                                                   pPropertyCount, pProperties);
    }

    if (!pLayerName) {
        uint32_t count = 0;
        instance->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                                            &count, NULL);

        if (!pProperties) {
            *pPropertyCount = count + 1;
            return VK_SUCCESS;
        }

        if (*pPropertyCount <= count) {
            instance->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                                                pPropertyCount, pProperties);
            return VK_INCOMPLETE;
        }

        instance->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                                            &count, pProperties);

        pProperties[count] = (VkExtensionProperties) { .extensionName = "VK_KHR_timeline_semaphore",
                                                       .specVersion = 1, };
        *pPropertyCount = count + 1;

        return VK_SUCCESS;
    }

    VK_OUTARRAY_MAKE(out, pProperties, pPropertyCount);

    vk_outarray_append(&out, prop) {
        *prop = (VkExtensionProperties) { .extensionName = "VK_KHR_timeline_semaphore", .specVersion = 1, };
    }

    return vk_outarray_status(&out);
}

static void timeline_GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures)
{
    struct instance_data *instance = object_find(&global_map, physicalDevice);

    instance->vtable.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);

    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR *timeline_features =
        vk_find_struct(pFeatures->pNext, PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR);
    if (timeline_features) {
        timeline_features->timelineSemaphore = true;
    }
}

static void timeline_GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties)
{
    struct instance_data *instance = object_find(&global_map, physicalDevice);

    instance->vtable.GetPhysicalDeviceProperties2(physicalDevice, pProperties);

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR *timeline_properties =
        vk_find_struct(pProperties->pNext, PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR);
    if (timeline_properties) {
        timeline_properties->maxTimelineSemaphoreValueDifference = UINT64_MAX;
    }
}

static void timeline_GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties)
{
    struct instance_data *instance = object_find(&global_map, physicalDevice);
    const VkSemaphoreTypeCreateInfoKHR *type_info =
        vk_find_struct_const(pExternalSemaphoreInfo->pNext, SEMAPHORE_TYPE_CREATE_INFO_KHR);

    if (!type_info || type_info->semaphoreType != VK_SEMAPHORE_TYPE_TIMELINE_KHR) {
        return instance->vtable.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice,
                                                                             pExternalSemaphoreInfo,
                                                                             pExternalSemaphoreProperties);
    }

    pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0;
    pExternalSemaphoreProperties->compatibleHandleTypes = 0;
    pExternalSemaphoreProperties->externalSemaphoreFeatures = 0;
}

static VkResult timeline_CreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice)
{
    struct instance_data *instance = object_find(&global_map, physicalDevice);
    VkLayerDeviceCreateInfo *chain_info =
        get_device_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(NULL, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    /* Advance the link info for the next element on the chain */
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS)
        return result;

    VkLayerDeviceCreateInfo *load_data_info =
        get_device_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);

    result = device_new(*pDevice, physicalDevice,
                        fpGetDeviceProcAddr,
                        load_data_info->u.pfnSetDeviceLoaderData,
                        pCreateInfo,
                        pAllocator ? pAllocator : &instance->alloc,
                        instance);
    if (result != VK_SUCCESS) {
        PFN_vkDestroyDevice fpDestroyDevice = (PFN_vkDestroyDevice)fpGetInstanceProcAddr(NULL, "vkDestroyDevice");
        fpDestroyDevice(*pDevice, pAllocator);
    }

    return result;
}

static void timeline_DestroyDevice(
    VkDevice                                    _device,
    const VkAllocationCallbacks*                pAllocator)
{
    struct device_data *device = object_find(&global_map, _device);
    PFN_vkDestroyDevice destroy_cb = device->vtable.DestroyDevice;

    device_destroy(device);
    destroy_cb(_device, pAllocator);
}

static VkResult timeline_EnumeratePhysicalDevices(
    VkInstance                                  _instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices)
{
    struct instance_data *instance = object_find(&global_map, _instance);
    VkResult result = instance->vtable.EnumeratePhysicalDevices(_instance,
                                                                pPhysicalDeviceCount,
                                                                pPhysicalDevices);

    if (pPhysicalDevices && (result == VK_SUCCESS || result == VK_INCOMPLETE)) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            if (object_find(&global_map, pPhysicalDevices[i]))
                object_unmap(&global_map, pPhysicalDevices[i]);
            object_map(&global_map, pPhysicalDevices[i], instance);
        }
    }

    return result;
}

static VkResult timeline_CreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance)
{
    VkLayerInstanceCreateInfo *chain_info =
        get_instance_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr =
        chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance =
        (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    /* Advance the link info for the next element on the chain */
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS)
        return result;

    result = instance_new(*pInstance,
                          fpGetInstanceProcAddr,
                          pAllocator ? pAllocator : &default_alloc);
    if (result != VK_SUCCESS) {
        PFN_vkDestroyInstance fpDestroyInstance =
            (PFN_vkDestroyInstance)fpGetInstanceProcAddr(NULL, "vkDestroyInstance");
        fpDestroyInstance(*pInstance, pAllocator);
    }

    return result;
}

static void timeline_DestroyInstance(
    VkInstance                                  _instance,
    const VkAllocationCallbacks*                pAllocator)
{
    struct instance_data *instance = object_find(&global_map, _instance);
    PFN_vkDestroyInstance destroy_cb = instance->vtable.DestroyInstance;

    instance_destroy(instance);
    destroy_cb(_instance, pAllocator);
}

static const struct {
    const char *name;
    void *ptr;
} name_to_funcptr_map[] = {
    { "vkGetDeviceProcAddr", (void *) vkGetDeviceProcAddr },
#define ADD_HOOK(fn) { "vk" # fn, (void *) timeline_ ## fn }
    ADD_HOOK(CreateSemaphore),
    ADD_HOOK(DestroySemaphore),
    ADD_HOOK(ImportSemaphoreFdKHR),
    ADD_HOOK(GetSemaphoreCounterValueKHR),
    ADD_HOOK(WaitSemaphoresKHR),
    ADD_HOOK(SignalSemaphoreKHR),

    ADD_HOOK(QueueSubmit),
    ADD_HOOK(QueueBindSparse),
    ADD_HOOK(QueueWaitIdle),
    ADD_HOOK(DeviceWaitIdle),
    ADD_HOOK(QueuePresentKHR),
    ADD_HOOK(AcquireNextImageKHR),

    ADD_HOOK(CreateInstance),
    ADD_HOOK(DestroyInstance),
    ADD_HOOK(CreateDevice),
    ADD_HOOK(DestroyDevice),
    ADD_HOOK(EnumeratePhysicalDevices),
    ADD_HOOK(EnumerateDeviceExtensionProperties),
    ADD_HOOK(EnumerateInstanceExtensionProperties),
    ADD_HOOK(GetPhysicalDeviceFeatures2),
    ADD_HOOK(GetPhysicalDeviceProperties2),
    ADD_HOOK(GetPhysicalDeviceExternalSemaphoreProperties),
#undef ADD_HOOK
};

static void *find_ptr(const char *name)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(name_to_funcptr_map); i++) {
        if (strcmp(name, name_to_funcptr_map[i].name) == 0)
            return name_to_funcptr_map[i].ptr;
    }

    return NULL;
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice _device,
                                                                             const char *funcName)
{
    void *ptr = find_ptr(funcName);
    if (ptr)
        return ptr;

    if (_device == NULL)
        return NULL;

    struct device_data *device = object_find(&global_map, _device);
    if (device->vtable.GetDeviceProcAddr == NULL) return NULL;
    return device->vtable.GetDeviceProcAddr(_device, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance _instance,
                                                                               const char *funcName)
{
    void *ptr = find_ptr(funcName);
    if (ptr)
        return ptr;

    if (_instance == NULL)
        return NULL;

    struct instance_data *instance = object_find(&global_map, _instance);
    if (instance->vtable.GetInstanceProcAddr == NULL) return NULL;
    return instance->vtable.GetInstanceProcAddr(_instance, funcName);
}
