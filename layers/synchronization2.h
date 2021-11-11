/* Copyright (c) 2020-2021 The Khronos Group Inc.
 * Copyright (c) 2020-2021 LunarG, Inc.
 * Copyright (c) 2020-2021 Advanced Micro Devices, Inc.
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
 * Author: Tobias Hector <@tobski>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#undef VK_NO_PROTOTYPES
#include <vector>
#include <memory>
#include <unordered_set>

#include "allocator.h"
#include "vk_concurrent_unordered_map.h"

namespace synchronization2 {
enum SynchronizationScope { kFirst, kSecond };

enum ImageAspect { kColorOnly, kDepthAndStencil, kDepthOnly, kStencilOnly };

struct DeviceData;
struct DeviceFeatures;

// helper struct definitions to convert synchronization2 barriers

struct BufferMemoryBarrier : public VkBufferMemoryBarrier {
    BufferMemoryBarrier() {
        sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        pNext = nullptr;
        srcAccessMask = 0;
        dstAccessMask = 0;
        srcQueueFamilyIndex = 0;
        dstQueueFamilyIndex = 0;
        buffer = 0;
        offset = 0;
        size = 0;
    }
    BufferMemoryBarrier(const VkBufferMemoryBarrier2KHR& v2, const DeviceFeatures& features);
};

struct ImageMemoryBarrier : public VkImageMemoryBarrier {
    ImageMemoryBarrier() {
        sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        pNext = nullptr;
        srcAccessMask = 0;
        dstAccessMask = 0;
        srcQueueFamilyIndex = 0;
        dstQueueFamilyIndex = 0;
        oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image = 0;
        subresourceRange = {};
    }

    ImageMemoryBarrier(const VkImageMemoryBarrier2KHR& v2, const DeviceFeatures& features, ImageAspect aspect);
};

template <typename T>
using CmdVector = std::vector<T, extension_layer::CmdAlloc<T>>;

// help structs for converting submit infos.
struct TimelineSemaphoreSubmitInfo {
    TimelineSemaphoreSubmitInfo(const DeviceFeatures& features, const VkSubmitInfo2KHR& v2, const VkAllocationCallbacks*);
    explicit TimelineSemaphoreSubmitInfo(const VkAllocationCallbacks*);

    VkTimelineSemaphoreSubmitInfo info;
    CmdVector<uint64_t> wait_vec;
    CmdVector<uint64_t> signal_vec;
};

struct DeviceGroupSubmitInfo {
    DeviceGroupSubmitInfo(const DeviceFeatures& features, const VkSubmitInfo2KHR& v2, const VkAllocationCallbacks* alloc);
    explicit DeviceGroupSubmitInfo(const VkAllocationCallbacks* alloc);

    VkDeviceGroupSubmitInfo info;
    CmdVector<uint32_t> wait_vec;
    CmdVector<uint32_t> cmd_vec;
    CmdVector<uint32_t> signal_vec;
};

struct ProtectedSubmitInfo : public VkProtectedSubmitInfo {
    explicit ProtectedSubmitInfo(const VkSubmitInfo2KHR& v2) {
        sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        pNext = nullptr;
        protectedSubmit = (v2.flags & VK_SUBMIT_PROTECTED_BIT_KHR) != 0;
    }
    ProtectedSubmitInfo() {
        sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        pNext = nullptr;
        protectedSubmit = 0;
    }
};

struct SubmitData {
    SubmitData(const VkSubmitInfo2KHR&, const VkAllocationCallbacks*, const DeviceFeatures &features);

    SubmitData(const VkSubmitInfo&, const VkAllocationCallbacks*, const DeviceFeatures &features);

    VkSubmitInfo info;
    CmdVector<VkSemaphore> wait_sem_vec;
    CmdVector<VkPipelineStageFlags> wait_dst_vec;
    CmdVector<VkCommandBuffer> cmd_vec;
    CmdVector<VkSemaphore> signal_vec;
    ProtectedSubmitInfo protect;
    TimelineSemaphoreSubmitInfo timeline_sem;
    DeviceGroupSubmitInfo device_group;
};

struct DependencyInfoV1 {
    DependencyInfoV1(const DeviceData& device_data, uint32_t info_count, const VkDependencyInfoKHR* infos,
                     const VkAllocationCallbacks* allocator);

    VkPipelineStageFlags src_stage_mask = 0;
    VkPipelineStageFlags dst_stage_mask = 0;
    VkDependencyFlags flags = 0;
    VkMemoryBarrier barrier = {};
    CmdVector<BufferMemoryBarrier> buffer_barriers;
    CmdVector<ImageMemoryBarrier> image_barriers;
};

struct PhysicalDeviceData {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    bool lower_has_sync2 = false;
    uint32_t api_version;
};

#define DECLARE_HOOK(fn) PFN_vk##fn fn
struct InstanceData {
    InstanceData(VkInstance instance, PFN_vkGetInstanceProcAddr gpa, const VkAllocationCallbacks* alloc);
    InstanceData() = delete;
    InstanceData(const InstanceData&) = delete;
    InstanceData& operator=(const InstanceData&) = delete;

    std::shared_ptr<PhysicalDeviceData> GetPhysicalDeviceData(VkPhysicalDevice physical_device) const {
        const auto result = physical_device_map.find(physical_device);
        if (result == physical_device_map.end()) {
            return nullptr;
        }
        return result->second;
    }

    VkInstance instance;
    uint32_t api_version;
    bool force_enable;
    const VkAllocationCallbacks* allocator;
    struct InstanceDispatchTable {
        DECLARE_HOOK(GetInstanceProcAddr);
        DECLARE_HOOK(CreateInstance);
        DECLARE_HOOK(DestroyInstance);
        DECLARE_HOOK(CreateDevice);
        DECLARE_HOOK(EnumeratePhysicalDevices);
        DECLARE_HOOK(EnumerateDeviceExtensionProperties);
        DECLARE_HOOK(GetPhysicalDeviceFeatures2);
        DECLARE_HOOK(GetPhysicalDeviceFeatures2KHR);
        DECLARE_HOOK(GetPhysicalDeviceProperties);
    } vtable;

    vk_concurrent_unordered_map<VkPhysicalDevice, std::shared_ptr<PhysicalDeviceData>> physical_device_map;
};

// data stored per VkImage for ImageBarrier conversion
struct ImageData {
    ImageAspect aspect;
};

struct SwapchainData {
    VkFormat format;
    std::unordered_set<VkImage> images;
};

struct DeviceFeatures {
    DeviceFeatures(uint32_t api_version, const VkDeviceCreateInfo* create_info);
    DeviceFeatures()
        : sync2(false),
          geometry(false),
          tessellation(false),
          meshShader(false),
          taskShader(false),
          shadingRateImage(false),
          advancedBlend(false),
          timelineSemaphore(false),
          deviceGroup(false) {}

    bool sync2;
    bool geometry;
    bool tessellation;
    bool meshShader;
    bool taskShader;
    bool shadingRateImage;
    bool advancedBlend;
    bool timelineSemaphore;
    bool deviceGroup;
};

struct DeviceData {
    DeviceData(VkDevice device, PFN_vkGetDeviceProcAddr gpa, const DeviceFeatures& feat, bool enable_layer,
               const VkAllocationCallbacks* alloc);
    DeviceData() = delete;
    DeviceData(const DeviceData&) = delete;
    DeviceData& operator=(const DeviceData&) = delete;

    VkDevice device;
    const VkAllocationCallbacks* allocator;
    DeviceFeatures features;
    bool enable_layer;
    uint32_t api_version;
    vk_concurrent_unordered_map<VkImage, ImageData> image_map;
    vk_concurrent_unordered_map<VkSwapchainKHR, SwapchainData> swapchain_map;
    struct DeviceDispatchTable {
        DECLARE_HOOK(GetDeviceProcAddr);
        DECLARE_HOOK(DestroyDevice);
        DECLARE_HOOK(CreateImage);
        DECLARE_HOOK(DestroyImage);
        DECLARE_HOOK(CreateSwapchainKHR);
        DECLARE_HOOK(GetSwapchainImagesKHR);
        DECLARE_HOOK(DestroySwapchainKHR);
        DECLARE_HOOK(CmdSetEvent);
        DECLARE_HOOK(CmdResetEvent);
        DECLARE_HOOK(CmdWaitEvents);
        DECLARE_HOOK(CmdPipelineBarrier);
        DECLARE_HOOK(CmdWriteTimestamp);
        DECLARE_HOOK(QueueSubmit);
        DECLARE_HOOK(CreateRenderPass2);
        DECLARE_HOOK(CmdWriteBufferMarkerAMD);
        DECLARE_HOOK(GetQueueCheckpointDataNV);
    } vtable;
};
#undef DECLARE_HOOK
}  // namespace synchronization2
