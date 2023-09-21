/* Copyright (c) 2023 The Khronos Group Inc.
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
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
 * Author: Vikram Kushwaha <vkushwaha@nvidia.com>
 */

#pragma once
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#undef VK_NO_PROTOTYPES
#include <vector>
#include <unordered_set>
#include <vulkan/utility/vk_concurrent_unordered_map.hpp>

struct ByteCode {
    const uint8_t* code;
    size_t size;
};

namespace memory_decompression {
enum class SynchronizationScope { kFirst, kSecond };

struct DeviceData;
struct DeviceFeatures;

struct LayerSettings {
    bool force_enable{false};
    bool logging{false};
};

template <typename T>
using CmdVector = std::vector<T, extension_layer::CmdAlloc<T>>;

struct SubmitData {
    SubmitData(const VkSubmitInfo2KHR&, const VkAllocationCallbacks*, const DeviceFeatures& features);

    SubmitData(const VkSubmitInfo&, const VkAllocationCallbacks*, const DeviceFeatures& features);

    VkSubmitInfo info;
    CmdVector<VkSemaphore> wait_sem_vec;
    CmdVector<VkPipelineStageFlags> wait_dst_vec;
    CmdVector<VkCommandBuffer> cmd_vec;
    CmdVector<VkSemaphore> signal_vec;
};

struct PhysicalDeviceData {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t api_version;
    VkPhysicalDeviceMemoryProperties memoryProperties = {};
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
    LayerSettings layer_settings;
    const VkAllocationCallbacks* allocator;
    struct InstanceDispatchTable {
        DECLARE_HOOK(GetInstanceProcAddr);
        DECLARE_HOOK(CreateInstance);
        DECLARE_HOOK(DestroyInstance);
        DECLARE_HOOK(CreateDevice);
        DECLARE_HOOK(EnumeratePhysicalDevices);
        DECLARE_HOOK(EnumerateDeviceExtensionProperties);
        DECLARE_HOOK(EnumerateInstanceExtensionProperties);
        DECLARE_HOOK(GetPhysicalDeviceProperties2);
        DECLARE_HOOK(GetPhysicalDeviceFeatures2);
        DECLARE_HOOK(GetPhysicalDeviceProperties);
        DECLARE_HOOK(GetPhysicalDeviceMemoryProperties);
    } vtable;

    vku::concurrent::unordered_map<VkPhysicalDevice, std::shared_ptr<PhysicalDeviceData>> physical_device_map;
};

struct DeviceFeatures {
    DeviceFeatures(uint32_t api_version, const VkDeviceCreateInfo* create_info);
    DeviceFeatures() : decompression(false) {}
    bool decompression;
};

struct DeviceData {
    DeviceData(VkDevice device, PFN_vkGetDeviceProcAddr gpa, const DeviceFeatures& feat, bool enable_layer,
               const VkAllocationCallbacks* alloc);
    DeviceData() = delete;
    DeviceData(const DeviceData&) = delete;
    DeviceData& operator=(const DeviceData&) = delete;

    VkResult CreatePipelineState(VkDevice* pDevice, VkPhysicalDevice physicalDevice);
    void DestroyPipelineState();

    VkDevice device;
    const VkAllocationCallbacks* allocator;
    DeviceFeatures features;
    bool enable_layer;
    uint32_t api_version;

    VkBuffer indirectDispatchBuffer;
    VkDeviceMemory indirectDispatchBufferMemory;
    VkDeviceAddress indirectDispatchBufferAddress;

    VkPipelineLayout pipelineLayoutDecompressSingle;
    VkPipeline pipelineDecompressSingle;
    VkPipelineLayout pipelineLayoutDecompressMulti;
    VkPipeline pipelineDecompressMulti;
    VkPipelineLayout pipelineLayoutCopy;
    VkPipeline pipelineCopy;

    struct PushConstantDataCopy {
        uint64_t srcCountAddress;
        uint64_t destCountAddress;
    };
    struct PushConstantDataDecompressMulti {
        uint64_t paramsAddress;
        uint32_t stride;
    };
    struct DeviceDispatchTable {
        DECLARE_HOOK(GetDeviceProcAddr);
        DECLARE_HOOK(DestroyDevice);
        DECLARE_HOOK(CreateBuffer);
        DECLARE_HOOK(DestroyBuffer);
        DECLARE_HOOK(GetBufferDeviceAddress);
        DECLARE_HOOK(AllocateMemory);
        DECLARE_HOOK(BindBufferMemory);
        DECLARE_HOOK(MapMemory);
        DECLARE_HOOK(UnmapMemory);
        DECLARE_HOOK(FreeMemory);
        DECLARE_HOOK(GetBufferMemoryRequirements);
        DECLARE_HOOK(CreateShaderModule);
        DECLARE_HOOK(DestroyPipelineLayout);
        DECLARE_HOOK(DestroyShaderModule);
        DECLARE_HOOK(DestroyPipeline);
        DECLARE_HOOK(CreatePipelineLayout);
        DECLARE_HOOK(CreateComputePipelines);
        DECLARE_HOOK(BeginCommandBuffer);
        DECLARE_HOOK(CmdBindPipeline);
        DECLARE_HOOK(CmdPushConstants);
        DECLARE_HOOK(CmdDispatch);
        DECLARE_HOOK(CmdDispatchIndirect);
        DECLARE_HOOK(CmdPipelineBarrier);
        DECLARE_HOOK(EndCommandBuffer);
        DECLARE_HOOK(QueueSubmit);
        DECLARE_HOOK(CmdDecompressMemoryNV);
        DECLARE_HOOK(CmdDecompressMemoryIndirectCountNV);
    } vtable;
};
#undef DECLARE_HOOK
}  // namespace memory_decompression
