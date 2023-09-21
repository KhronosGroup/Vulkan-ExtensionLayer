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

static bool logging_enabled = false;

#define PRINT(...)                        \
    {                                     \
        if (logging_enabled) {            \
            fprintf(stdout, __VA_ARGS__); \
            fflush(stdout);               \
        }                                 \
    }

#include <vulkan/vk_layer.h>
#include <vulkan/layer/vk_layer_settings.hpp>
#include <vulkan/utility/vk_safe_struct.hpp>
#include "allocator.h"
#include "log.h"
#include "vk_util.h"
#include "decompression.h"
#include "vk_common.h"

#include "shaders/spirv/copyCount_vk.h"

#include "shaders/spirv/GInflate8_vk.h"
#include "shaders/spirv/GInflate16_vk.h"
#include "shaders/spirv/GInflate32_vk.h"
#include "shaders/spirv/GInflate64_vk.h"
#include "shaders/spirv/GInflate8_HAVE_INT16_vk.h"
#include "shaders/spirv/GInflate16_HAVE_INT16_vk.h"
#include "shaders/spirv/GInflate32_HAVE_INT16_vk.h"
#include "shaders/spirv/GInflate64_HAVE_INT16_vk.h"
#include "shaders/spirv/GInflate8_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate16_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate32_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate64_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate8_HAVE_INT16_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate16_HAVE_INT16_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate32_HAVE_INT16_HAVE_INT64_vk.h"
#include "shaders/spirv/GInflate64_HAVE_INT16_HAVE_INT64_vk.h"

#include "shaders/spirv/IndirectGInflate8_vk.h"
#include "shaders/spirv/IndirectGInflate16_vk.h"
#include "shaders/spirv/IndirectGInflate32_vk.h"
#include "shaders/spirv/IndirectGInflate64_vk.h"
#include "shaders/spirv/IndirectGInflate8_HAVE_INT16_vk.h"
#include "shaders/spirv/IndirectGInflate16_HAVE_INT16_vk.h"
#include "shaders/spirv/IndirectGInflate32_HAVE_INT16_vk.h"
#include "shaders/spirv/IndirectGInflate64_HAVE_INT16_vk.h"
#include "shaders/spirv/IndirectGInflate8_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate16_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate32_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate64_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate8_HAVE_INT16_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate16_HAVE_INT16_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate32_HAVE_INT16_HAVE_INT64_vk.h"
#include "shaders/spirv/IndirectGInflate64_HAVE_INT16_HAVE_INT64_vk.h"

static const ByteCode kGInflateBytecode[] = {
    {(const uint8_t*)kGInflate8, sizeof(kGInflate8)},
    {(const uint8_t*)kGInflate16, sizeof(kGInflate16)},
    {(const uint8_t*)kGInflate32, sizeof(kGInflate32)},
    {(const uint8_t*)kGInflate64, sizeof(kGInflate64)},

    {(const uint8_t*)kGInflate8_HAVE_INT16, sizeof(kGInflate8_HAVE_INT16)},
    {(const uint8_t*)kGInflate16_HAVE_INT16, sizeof(kGInflate16_HAVE_INT16)},
    {(const uint8_t*)kGInflate32_HAVE_INT16, sizeof(kGInflate32_HAVE_INT16)},
    {(const uint8_t*)kGInflate64_HAVE_INT16, sizeof(kGInflate64_HAVE_INT16)},

    {(const uint8_t*)kGInflate8_HAVE_INT64, sizeof(kGInflate8_HAVE_INT64)},
    {(const uint8_t*)kGInflate16_HAVE_INT64, sizeof(kGInflate16_HAVE_INT64)},
    {(const uint8_t*)kGInflate32_HAVE_INT64, sizeof(kGInflate32_HAVE_INT64)},
    {(const uint8_t*)kGInflate64_HAVE_INT64, sizeof(kGInflate64_HAVE_INT64)},

    {(const uint8_t*)kGInflate8_HAVE_INT16_HAVE_INT64, sizeof(kGInflate8_HAVE_INT16_HAVE_INT64)},
    {(const uint8_t*)kGInflate16_HAVE_INT16_HAVE_INT64, sizeof(kGInflate16_HAVE_INT16_HAVE_INT64)},
    {(const uint8_t*)kGInflate32_HAVE_INT16_HAVE_INT64, sizeof(kGInflate32_HAVE_INT16_HAVE_INT64)},
    {(const uint8_t*)kGInflate64_HAVE_INT16_HAVE_INT64, sizeof(kGInflate64_HAVE_INT16_HAVE_INT64)},
};

static const ByteCode kIndirectGInflateBytecode[] = {
    {(const uint8_t*)kIndirectGInflate8, sizeof(kIndirectGInflate8)},
    {(const uint8_t*)kIndirectGInflate16, sizeof(kIndirectGInflate16)},
    {(const uint8_t*)kIndirectGInflate32, sizeof(kIndirectGInflate32)},
    {(const uint8_t*)kIndirectGInflate64, sizeof(kIndirectGInflate64)},

    {(const uint8_t*)kIndirectGInflate8_HAVE_INT16, sizeof(kIndirectGInflate8_HAVE_INT16)},
    {(const uint8_t*)kIndirectGInflate16_HAVE_INT16, sizeof(kIndirectGInflate16_HAVE_INT16)},
    {(const uint8_t*)kIndirectGInflate32_HAVE_INT16, sizeof(kIndirectGInflate32_HAVE_INT16)},
    {(const uint8_t*)kIndirectGInflate64_HAVE_INT16, sizeof(kIndirectGInflate64_HAVE_INT16)},

    {(const uint8_t*)kIndirectGInflate8_HAVE_INT64, sizeof(kIndirectGInflate8_HAVE_INT64)},
    {(const uint8_t*)kIndirectGInflate16_HAVE_INT64, sizeof(kIndirectGInflate16_HAVE_INT64)},
    {(const uint8_t*)kIndirectGInflate32_HAVE_INT64, sizeof(kIndirectGInflate32_HAVE_INT64)},
    {(const uint8_t*)kIndirectGInflate64_HAVE_INT64, sizeof(kIndirectGInflate64_HAVE_INT64)},

    {(const uint8_t*)kIndirectGInflate8_HAVE_INT16_HAVE_INT64, sizeof(kIndirectGInflate8_HAVE_INT16_HAVE_INT64)},
    {(const uint8_t*)kIndirectGInflate16_HAVE_INT16_HAVE_INT64, sizeof(kIndirectGInflate16_HAVE_INT16_HAVE_INT64)},
    {(const uint8_t*)kIndirectGInflate32_HAVE_INT16_HAVE_INT64, sizeof(kIndirectGInflate32_HAVE_INT16_HAVE_INT64)},
    {(const uint8_t*)kIndirectGInflate64_HAVE_INT16_HAVE_INT64, sizeof(kIndirectGInflate64_HAVE_INT16_HAVE_INT64)},
};

#define kLayerSettingsForceEnable "force_enable"
#define kLayerSettingsCustomSTypeInfo "custom_stype_list"
#define kLayerSettingsLogging "logging"

namespace memory_decompression {

static const VkLayerProperties kGlobalLayer = {
    "VK_LAYER_KHRONOS_memory_decompression",
    VK_HEADER_VERSION_COMPLETE,
    1,
    "Default memory decompression layer",
};

// Instance extensions that this layer provides:
const VkExtensionProperties kInstanceExtensionProperties[] = {
    VkExtensionProperties{VK_EXT_LAYER_SETTINGS_EXTENSION_NAME, VK_EXT_LAYER_SETTINGS_SPEC_VERSION}};
const uint32_t kInstanceExtensionPropertiesCount = static_cast<uint32_t>(std::size(kInstanceExtensionProperties));

static const VkExtensionProperties kDeviceExtension = {VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME,
                                                       VK_NV_MEMORY_DECOMPRESSION_SPEC_VERSION};

static vku::concurrent::unordered_map<uintptr_t, std::shared_ptr<InstanceData>> instance_data_map;
static vku::concurrent::unordered_map<uintptr_t, std::shared_ptr<DeviceData>> device_data_map;

uintptr_t DispatchKey(const void* object) {
    auto tmp = reinterpret_cast<const struct VkLayerDispatchTable_* const*>(object);
    return reinterpret_cast<uintptr_t>(*tmp);
}

// helper to pass vector sizes into vulkan without noisy casting everywhere
template <typename V>
static inline uint32_t VecSize(const V& v) {
    return static_cast<uint32_t>(v.size());
}

static std::shared_ptr<InstanceData> GetInstanceData(const void* object) {
    auto result = instance_data_map.find(DispatchKey(object));
    return result != instance_data_map.end() ? result->second : nullptr;
}

static std::shared_ptr<DeviceData> GetDeviceData(const void* object) {
    auto result = device_data_map.find(DispatchKey(object));
    return result != device_data_map.end() ? result->second : nullptr;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                  uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    if (pLayerName && strncmp(pLayerName, kGlobalLayer.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
        auto instance_data = GetInstanceData(physicalDevice);
        return instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    }

    if (!pLayerName) {
        uint32_t count = 0;
        auto instance_data = GetInstanceData(physicalDevice);
        instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &count, nullptr);
        std::vector<VkExtensionProperties> extProps(count);
        instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &count, extProps.data());
        bool decompressionExtFound = false;
        for (uint32_t t = 0; t < count; t++) {
            if (0 == strcmp(extProps[t].extensionName, kDeviceExtension.extensionName)) {
                decompressionExtFound = true;
                break;
            }
        }
        // Do not add the extension, if the implementation already supports VK_NV_MEMORY_DECOMPRESSION_EXTENSION
        uint32_t extensionCount = decompressionExtFound ? count : count + 1;
        if (!pProperties) {
            *pPropertyCount = extensionCount;
            return VK_SUCCESS;
        }
        if (*pPropertyCount < extensionCount) {
            instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
            return VK_INCOMPLETE;
        }
        instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &count, pProperties);
        *pPropertyCount = extensionCount;
        // Add VK_NV_MEMORY_DECOMPRESSION_EXTENSION extension if the implementation doesn't support it
        if (!decompressionExtFound) {
            pProperties[count] = kDeviceExtension;
        }
        return VK_SUCCESS;
    }

    VK_OUTARRAY_MAKE(out, pProperties, pPropertyCount);
    vk_outarray_append(&out, prop) { *prop = kDeviceExtension; }
    return vk_outarray_status(&out);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto instance_data = GetInstanceData(physicalDevice);
    auto pdd = instance_data->GetPhysicalDeviceData(physicalDevice);

    if (instance_data->vtable.GetPhysicalDeviceProperties2 != nullptr) {
        instance_data->vtable.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto instance_data = GetInstanceData(physicalDevice);
    auto pdd = instance_data->GetPhysicalDeviceData(physicalDevice);

    if (instance_data->vtable.GetPhysicalDeviceFeatures2 != nullptr) {
        instance_data->vtable.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
}

#define INIT_HOOK(_vt, _inst, fn) _vt.fn = reinterpret_cast<PFN_vk##fn>(vtable.GetInstanceProcAddr(_inst, "vk" #fn))
InstanceData::InstanceData(VkInstance inst, PFN_vkGetInstanceProcAddr gpa, const VkAllocationCallbacks* alloc)
    : instance(inst), api_version(0), allocator(alloc) {
    vtable.GetInstanceProcAddr = gpa;
    INIT_HOOK(vtable, instance, CreateInstance);
    INIT_HOOK(vtable, instance, DestroyInstance);
    INIT_HOOK(vtable, instance, CreateDevice);
    INIT_HOOK(vtable, instance, EnumeratePhysicalDevices);
    INIT_HOOK(vtable, instance, EnumerateDeviceExtensionProperties);
    INIT_HOOK(vtable, instance, EnumerateInstanceExtensionProperties);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceProperties2);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceFeatures2);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceProperties);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceMemoryProperties);
}
#undef INIT_HOOK

static VkLayerInstanceCreateInfo* GetChainInfo(const VkInstanceCreateInfo* pCreateInfo, VkLayerFunction func) {
    auto chain_info = reinterpret_cast<VkLayerInstanceCreateInfo*>(const_cast<void*>(pCreateInfo->pNext));
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == func)) {
        chain_info = reinterpret_cast<VkLayerInstanceCreateInfo*>(const_cast<void*>(chain_info->pNext));
    }
    ASSERT(chain_info != NULL);
    return chain_info;
}

void InitLayerSettings(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, LayerSettings* layer_settings) {
    assert(layer_settings != nullptr);

    const VkLayerSettingsCreateInfoEXT* create_info = vkuFindLayerSettingsCreateInfo(pCreateInfo);

    VkuLayerSettingSet layer_setting_set = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet(kGlobalLayer.layerName, create_info, pAllocator, nullptr, &layer_setting_set);

    static const char* setting_names[] = {kLayerSettingsForceEnable, kLayerSettingsLogging, kLayerSettingsCustomSTypeInfo};
    uint32_t setting_name_count = static_cast<uint32_t>(std::size(setting_names));

    uint32_t unknown_setting_count = 0;
    vkuGetUnknownSettings(create_info, setting_name_count, setting_names, &unknown_setting_count, nullptr);

    if (unknown_setting_count > 0) {
        std::vector<const char*> unknown_settings;
        unknown_settings.resize(unknown_setting_count);

        vkuGetUnknownSettings(create_info, setting_name_count, setting_names, &unknown_setting_count, &unknown_settings[0]);

        for (std::size_t i = 0, n = unknown_settings.size(); i < n; ++i) {
            LOG("Unknown %s setting listed in VkLayerSettingsCreateInfoEXT, this setting is ignored.\n", unknown_settings[i]);
        }
    }

    if (vkuHasLayerSetting(layer_setting_set, kLayerSettingsForceEnable)) {
        vkuGetLayerSettingValue(layer_setting_set, kLayerSettingsForceEnable, layer_settings->force_enable);
    }

    if (vkuHasLayerSetting(layer_setting_set, kLayerSettingsLogging)) {
        vkuGetLayerSettingValue(layer_setting_set, kLayerSettingsLogging, layer_settings->logging);
    }

    if (vkuHasLayerSetting(layer_setting_set, kLayerSettingsCustomSTypeInfo)) {
        vkuGetLayerSettingValues(layer_setting_set, kLayerSettingsCustomSTypeInfo, vku::GetCustomStypeInfo());
    }

    vkuDestroyLayerSettingSet(layer_setting_set, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                              VkInstance* pInstance) {
    VkLayerInstanceCreateInfo* chain_info = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);

    ASSERT(chain_info->u.pLayerInfo);
    auto gpa = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    auto create_instance = reinterpret_cast<PFN_vkCreateInstance>(gpa(NULL, "vkCreateInstance"));
    if (create_instance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = create_instance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }
    try {
        auto instance_data =
            std::make_shared<InstanceData>(*pInstance, gpa, pAllocator ? pAllocator : &extension_layer::kDefaultAllocator);

        instance_data_map.insert(DispatchKey(*pInstance), instance_data);

        instance_data->api_version = pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : 0;

        InitLayerSettings(pCreateInfo, pAllocator, &instance_data->layer_settings);
        logging_enabled = instance_data->layer_settings.logging;
    } catch (const std::bad_alloc&) {
        auto destroy_instance = reinterpret_cast<PFN_vkDestroyInstance>(gpa(NULL, "vkDestroyInstance"));
        destroy_instance(*pInstance, pAllocator);
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                        VkPhysicalDevice* pPhysicalDevices) {
    auto instance_data = GetInstanceData(instance);
    VkResult result =
        instance_data->vtable.EnumeratePhysicalDevices(instance_data->instance, pPhysicalDeviceCount, pPhysicalDevices);
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pPhysicalDevices != nullptr) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            VkPhysicalDeviceProperties properties{};
            auto physical_device = pPhysicalDevices[i];

            if (instance_data->physical_device_map.find(physical_device) != instance_data->physical_device_map.end()) {
                continue;
            }
            auto pdd = std::make_shared<PhysicalDeviceData>();
            pdd->physical_device = physical_device;

            instance_data->vtable.GetPhysicalDeviceProperties(physical_device, &properties);
            pdd->api_version = properties.apiVersion;

            instance_data->vtable.GetPhysicalDeviceMemoryProperties(physical_device, &pdd->memoryProperties);

            instance_data->physical_device_map.insert(physical_device, pdd);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    auto key = DispatchKey(instance);
    auto result = instance_data_map.find(key);
    if (result != instance_data_map.end()) {
        auto instance_data = result->second;

        instance_data->vtable.DestroyInstance(instance, pAllocator);

        instance_data_map.erase(key);
    }
}

#define INIT_HOOK(_vt, _dev, fn) _vt.fn = reinterpret_cast<PFN_vk##fn>(vtable.GetDeviceProcAddr(_dev, "vk" #fn))
#define INIT_HOOK_ALIAS(_vt, _dev, fn, fn_alias) \
    _vt.fn_alias = (_vt.fn_alias != nullptr) ? _vt.fn_alias : reinterpret_cast<PFN_vk##fn>(vtable.GetDeviceProcAddr(_dev, "vk" #fn))

DeviceData::DeviceData(VkDevice device, PFN_vkGetDeviceProcAddr gpa, const DeviceFeatures& feat, bool enable,
                       const VkAllocationCallbacks* alloc)
    : device(device), allocator(alloc), features(feat), enable_layer(enable) {
    vtable.GetDeviceProcAddr = gpa;

    if (enable_layer) {
        INIT_HOOK(vtable, device, DestroyDevice);
        INIT_HOOK(vtable, device, CreateBuffer);
        INIT_HOOK(vtable, device, DestroyBuffer);
        INIT_HOOK(vtable, device, GetBufferDeviceAddress);
        INIT_HOOK(vtable, device, AllocateMemory);
        INIT_HOOK(vtable, device, BindBufferMemory);
        INIT_HOOK(vtable, device, MapMemory);
        INIT_HOOK(vtable, device, UnmapMemory);
        INIT_HOOK(vtable, device, FreeMemory);
        INIT_HOOK(vtable, device, GetBufferMemoryRequirements);
        INIT_HOOK(vtable, device, CreateShaderModule);
        INIT_HOOK(vtable, device, DestroyPipelineLayout);
        INIT_HOOK(vtable, device, DestroyShaderModule);
        INIT_HOOK(vtable, device, DestroyPipeline);
        INIT_HOOK(vtable, device, CreatePipelineLayout);
        INIT_HOOK(vtable, device, CreateComputePipelines);
        INIT_HOOK(vtable, device, BeginCommandBuffer);
        INIT_HOOK(vtable, device, CmdBindPipeline);
        INIT_HOOK(vtable, device, CmdPushConstants);
        INIT_HOOK(vtable, device, CmdDispatch);
        INIT_HOOK(vtable, device, CmdDispatchIndirect);
        INIT_HOOK(vtable, device, CmdPipelineBarrier);
        INIT_HOOK(vtable, device, EndCommandBuffer);
        INIT_HOOK(vtable, device, QueueSubmit);
        INIT_HOOK(vtable, device, CmdDecompressMemoryNV);
        INIT_HOOK(vtable, device, CmdDecompressMemoryIndirectCountNV);
    }
}
#undef INIT_HOOK
#undef INIT_HOOK_ALIAS

static inline uint32_t findFirstSetBit(uint32_t a) {
    uint32_t n = 0;
    while (0 == (a & 1) && n < sizeof(uint32_t) * 8) {
        a >>= 1;
        ++n;
    }
    return n;
}

static VkLayerDeviceCreateInfo* GetChainInfo(const VkDeviceCreateInfo* pCreateInfo, VkLayerFunction func) {
    auto chain_info = reinterpret_cast<VkLayerDeviceCreateInfo*>(const_cast<void*>(pCreateInfo->pNext));
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = reinterpret_cast<VkLayerDeviceCreateInfo*>(const_cast<void*>(chain_info->pNext));
    }
    ASSERT(chain_info != NULL);
    return chain_info;
}

DeviceFeatures::DeviceFeatures(uint32_t api_version, const VkDeviceCreateInfo* create_info) : decompression(false) {
    const VkBaseInStructure* chain = reinterpret_cast<const VkBaseInStructure*>(create_info->pNext);

    while (chain != nullptr) {
        switch (chain->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV: {
                auto decompression_features = reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV*>(chain);
                decompression = 0 != decompression_features->memoryDecompression;
            } break;
            default:
                break;
        }
        chain = chain->pNext;
    }
}

VkResult DeviceData::CreatePipelineState(VkDevice* pDevice, VkPhysicalDevice physicalDevice) {
    VkResult result;
    auto instance_data = GetInstanceData(physicalDevice);
    auto pdd = instance_data->GetPhysicalDeviceData(physicalDevice);

    VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    VkPhysicalDeviceSubgroupSizeControlProperties subgroupsizeProps = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES};
    props.pNext = &subgroupsizeProps;
    GetPhysicalDeviceProperties2(physicalDevice, &props);

    VkPhysicalDeviceFeatures2 devFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    VkPhysicalDeviceSubgroupSizeControlFeatures subgroupFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES};
    devFeatures.pNext = &subgroupFeatures;
    GetPhysicalDeviceFeatures2(physicalDevice, &devFeatures);

    uint32_t subgroupSize = 32;
    if (!subgroupFeatures.subgroupSizeControl || subgroupsizeProps.minSubgroupSize < 16) {
        // If we do not have size control extension, or width < 16
        // Force a shader variant with full syncronization.
        subgroupSize = 8;
    } else if (subgroupFeatures.subgroupSizeControl) {
        // Use a shader with the narrowest supported subgroup.
        subgroupSize = subgroupsizeProps.minSubgroupSize;
    }

    PRINT("Info: subgroupSize %u\n", subgroupSize);

    if (subgroupSize != 8 && subgroupSize != 16 && subgroupSize != 32 && subgroupSize != 64) {
        // Only 8, 16, 32 and 64 are supported
        PRINT("Error: Unsupported subgroupSize %u\n", subgroupSize);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t bytecodeIndex = 0;
    bytecodeIndex += findFirstSetBit(subgroupSize) - 3;
    bytecodeIndex += (devFeatures.features.shaderInt16 ? 4 : 0);
    bytecodeIndex += (devFeatures.features.shaderInt64 ? 8 : 0);
    PRINT("Info: bytecodeIndex %u\n", bytecodeIndex);

    size_t byteCodeArrLength = sizeof(kGInflateBytecode) / sizeof(kGInflateBytecode[0]);
    size_t byteCodeIndirectArrLength = sizeof(kIndirectGInflateBytecode) / sizeof(kIndirectGInflateBytecode[0]);

    // Must have all shaders for both direct/indirect mode
    // byteCodeIndex must pick a valid shader
    if ((byteCodeArrLength != byteCodeIndirectArrLength) && (bytecodeIndex >= byteCodeArrLength)) {
        PRINT("Error: Unsupported bytecodeIndex %u\n", bytecodeIndex);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // create indirect buffer with data {[indirectCommandsAddress], 1,1}
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = 3 * sizeof(uint32_t);
    bufferInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    result = vtable.CreateBuffer(device, &bufferInfo, 0, &indirectDispatchBuffer);
    if (result != VK_SUCCESS) {
        PRINT("Error: CreateBuffer with error %u\n", result);
        return result;
    }

    {
        PRINT("Info: Memory Heaps/Types:\n");
        for (uint32_t t = 0; t < pdd->memoryProperties.memoryHeapCount; t++) {
            PRINT("    Heap %u: Size %llu, Flags %u\n", t, (unsigned long long)pdd->memoryProperties.memoryHeaps[t].size,
                  pdd->memoryProperties.memoryHeaps[t].flags);
        }
        for (uint32_t t = 0; t < pdd->memoryProperties.memoryTypeCount; t++) {
            PRINT("    Memory Type %u: HeapIndex %u, Flags %u\n", t, pdd->memoryProperties.memoryTypes[t].heapIndex,
                  pdd->memoryProperties.memoryTypes[t].propertyFlags);
        }
    }

    VkMemoryRequirements reqs;
    vtable.GetBufferMemoryRequirements(*pDevice, indirectDispatchBuffer, &reqs);
    VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t mem_type_index = 0;
    for (mem_type_index = 0; mem_type_index < pdd->memoryProperties.memoryTypeCount; ++mem_type_index) {
        if (property_flags == (property_flags & pdd->memoryProperties.memoryTypes[mem_type_index].propertyFlags)) break;
    }
    indirectDispatchBufferMemory = 0;
    if (mem_type_index < pdd->memoryProperties.memoryTypeCount) {
        PRINT("Info: Using memory index %u for indirectDispatch Buffer.\n", mem_type_index);
        VkMemoryAllocateFlagsInfo memFlagsInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
        memFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        VkMemoryAllocateInfo memInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memInfo.pNext = &memFlagsInfo;
        memInfo.allocationSize = reqs.size;
        memInfo.memoryTypeIndex = mem_type_index;
        result = vtable.AllocateMemory(device, &memInfo, 0, &indirectDispatchBufferMemory);
        if (result != VK_SUCCESS) {
            return result;
        }
        result = vtable.BindBufferMemory(device, indirectDispatchBuffer, indirectDispatchBufferMemory, 0);
        if (result != VK_SUCCESS) {
            return result;
        }
        uint32_t* dispatchData;
        result = vtable.MapMemory(device, indirectDispatchBufferMemory, 0, reqs.size, 0, (void**)&dispatchData);
        if (result != VK_SUCCESS) {
            return result;
        }
        *(dispatchData) = 1;
        *(dispatchData + 1) = 1;
        *(dispatchData + 2) = 1;
        vtable.UnmapMemory(device, indirectDispatchBufferMemory);
    }
    if (!indirectDispatchBufferMemory) {
        // Could not find appropriate memory type
        PRINT("Error: Could not create memory for indirect Buffer\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkBufferDeviceAddressInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, indirectDispatchBuffer};
    indirectDispatchBufferAddress = vtable.GetBufferDeviceAddress(device, &buffer_info);

    VkShaderModule decompressShaderModule;
    VkShaderModule copyCountModule;
    VkShaderModule indirectDecompressShaderModule;

    // Create Decompression shader pipeline
    ByteCode bytecode = kGInflateBytecode[bytecodeIndex];
    VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    shaderModuleInfo.codeSize = bytecode.size;
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.code);
    result = vtable.CreateShaderModule(device, &shaderModuleInfo, 0, &decompressShaderModule);
    if (result != VK_SUCCESS) {
        return result;
    }
    // create pipeline
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(VkDecompressMemoryRegionNV);
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    result = vtable.CreatePipelineLayout(device, &pipelineLayoutCreateInfo, 0, &pipelineLayoutDecompressSingle);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkComputePipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipelineInfo.layout = pipelineLayoutDecompressSingle;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = decompressShaderModule;
    pipelineInfo.stage.pName = "main";
    result = vtable.CreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &pipelineDecompressSingle);
    if (result != VK_SUCCESS) {
        return result;
    }
    vtable.DestroyShaderModule(device, decompressShaderModule, 0);

    // Create copy shader pipeline
    ByteCode copyCountBytecode = {(const uint8_t*)copyCount, sizeof(copyCount)};
    shaderModuleInfo.codeSize = copyCountBytecode.size;
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(copyCountBytecode.code);
    result = vtable.CreateShaderModule(device, &shaderModuleInfo, 0, &copyCountModule);
    if (result != VK_SUCCESS) {
        return result;
    }

    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantDataCopy);
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    result = vtable.CreatePipelineLayout(device, &pipelineLayoutCreateInfo, 0, &pipelineLayoutCopy);
    if (result != VK_SUCCESS) {
        return result;
    }
    pipelineInfo.layout = pipelineLayoutCopy;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = copyCountModule;
    pipelineInfo.stage.pName = "main";
    result = vtable.CreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &pipelineCopy);
    if (result != VK_SUCCESS) {
        return result;
    }
    vtable.DestroyShaderModule(device, copyCountModule, 0);

    // Create indirect decompression shader pipeline
    ByteCode bytecodeIndirect = kIndirectGInflateBytecode[bytecodeIndex];
    shaderModuleInfo.codeSize = bytecodeIndirect.size;
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(bytecodeIndirect.code);
    result = vtable.CreateShaderModule(device, &shaderModuleInfo, 0, &indirectDecompressShaderModule);
    if (result != VK_SUCCESS) {
        return result;
    }

    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantDataDecompressMulti);
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    result = vtable.CreatePipelineLayout(device, &pipelineLayoutCreateInfo, 0, &pipelineLayoutDecompressMulti);
    if (result != VK_SUCCESS) {
        return result;
    }

    pipelineInfo.layout = pipelineLayoutDecompressMulti;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = indirectDecompressShaderModule;
    pipelineInfo.stage.pName = "main";
    result = vtable.CreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &pipelineDecompressMulti);
    if (result != VK_SUCCESS) {
        return result;
    }
    vtable.DestroyShaderModule(device, indirectDecompressShaderModule, 0);

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    VkResult result;
    auto instance_data = GetInstanceData(physicalDevice);
    auto pdd = instance_data->GetPhysicalDeviceData(physicalDevice);

    VkLayerDeviceCreateInfo* chain_info = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);

    ASSERT(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr instance_proc_addr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateDevice create_device = (PFN_vkCreateDevice)instance_proc_addr(instance_data->instance, "vkCreateDevice");
    PFN_vkGetDeviceProcAddr gdpa = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    if (instance_data->vtable.CreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    uint32_t effective_api_version =
        (instance_data->api_version != 0) ? std::min(instance_data->api_version, pdd->api_version) : pdd->api_version;

    DeviceFeatures features(effective_api_version, pCreateInfo);

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkPhysicalDeviceFeatures2 devFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    VkPhysicalDeviceMemoryDecompressionFeaturesNV decompressionFeature = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV};
    VkPhysicalDeviceVulkan12Features vulkan12Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    decompressionFeature.pNext = &vulkan12Features;
    devFeatures.pNext = &decompressionFeature;
    GetPhysicalDeviceFeatures2(physicalDevice, &devFeatures);

    VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    VkPhysicalDeviceSubgroupProperties subgroupProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES};
    props.pNext = &subgroupProps;
    GetPhysicalDeviceProperties2(physicalDevice, &props);

    const bool computeStageSupport = (subgroupProps.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT);
    const bool subgroupBasicSupport = subgroupProps.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT;

    try {
        bool enable_layer = false;
        if (!decompressionFeature.memoryDecompression) {
            PRINT("Memory decompression feature not available in the driver, enabling decompression layer.\n");
            enable_layer = true;
        } else {
            if (instance_data->layer_settings.force_enable) {
                PRINT("Memory decompression feature available in the driver, but force enabling decompression layer.\n");
                enable_layer = true;
            } else {
                PRINT("Memory decompression feature available in the driver, not using decompression layer.\n");
            }
        }

        // Filter out our extension name and feature struct, in a copy of the create info.
        // Only enable device hooks if memory decompression extension is enabled AND
        // the physical device doesn't support it already or we are force enabled.
        if (enable_layer) {
            // The layer requires 8-bit integer support,
            // basic subgroup feature in the compute stage and
            // bufferDeviceAddress feature
            if (!computeStageSupport || !subgroupBasicSupport || !vulkan12Features.shaderInt8 ||
                !vulkan12Features.bufferDeviceAddress) {
                PRINT("Info: computeStageSupport %u\n", computeStageSupport);
                PRINT("Info: subgroupBasicSupport %u\n", subgroupBasicSupport);
                PRINT("Info: vulkan12Features.shaderInt8 %u\n", vulkan12Features.shaderInt8);
                PRINT("Info: vulkan12Features.bufferDeviceAddress %u\n", vulkan12Features.bufferDeviceAddress);
                PRINT("Error: Required features not present to use decompression layer.\n");
                return VK_ERROR_FEATURE_NOT_PRESENT;
            }

            vku::safe_VkDeviceCreateInfo create_info(pCreateInfo);
            vku::RemoveExtension(create_info, VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME);
            vku::RemoveFromPnext(create_info, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV);

            result = create_device(physicalDevice, create_info.ptr(), pAllocator, pDevice);
        } else {
            result = create_device(physicalDevice, pCreateInfo, pAllocator, pDevice);
        }
        if (result != VK_SUCCESS) {
            PRINT("Error: CreateDevice failed with error %u\n", result);
            return result;
        }
        auto alloccb = pAllocator ? pAllocator : instance_data->allocator;
        auto device_data = std::make_shared<DeviceData>(*pDevice, gdpa, features, enable_layer, alloccb);

        if (enable_layer) {
            result = device_data->CreatePipelineState(pDevice, physicalDevice);
            if (result != VK_SUCCESS) {
                PRINT("Error: CreatePipelineState failed with error %u\n", result);
                return result;
            }
        }

        device_data_map.insert(DispatchKey(*pDevice), device_data);
    } catch (const std::bad_alloc&) {
        auto destroy_device = reinterpret_cast<PFN_vkDestroyDevice>(gdpa(*pDevice, "vkDestroyDevice"));
        destroy_device(*pDevice, pAllocator);
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return result;
}

void DeviceData::DestroyPipelineState() {
    vtable.DestroyPipelineLayout(device, pipelineLayoutDecompressSingle, 0);
    vtable.DestroyPipelineLayout(device, pipelineLayoutDecompressMulti, 0);
    vtable.DestroyPipelineLayout(device, pipelineLayoutCopy, 0);

    vtable.DestroyPipeline(device, pipelineDecompressMulti, 0);
    vtable.DestroyPipeline(device, pipelineDecompressSingle, 0);
    vtable.DestroyPipeline(device, pipelineCopy, 0);
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto key = DispatchKey(device);
    auto result = device_data_map.find(key);
    if (result != device_data_map.end()) {
        auto device_data = result->second;

        device_data->DestroyPipelineState();
        device_data->vtable.DestroyBuffer(device, device_data->indirectDispatchBuffer, pAllocator);
        if (device_data->indirectDispatchBufferMemory) {
            device_data->vtable.FreeMemory(device, device_data->indirectDispatchBufferMemory, pAllocator);
        }
        device_data->vtable.DestroyDevice(device, pAllocator);

        device_data_map.erase(key);
    }
}

static VkPipelineStageFlags ConvertPipelineStageMask(VkPipelineStageFlags2KHR stage_mask, SynchronizationScope scope,
                                                     const DeviceFeatures& features) {
    VkPipelineStageFlags result_stage_mask = (stage_mask & 0x7fffffff);

    if ((stage_mask & VK_PIPELINE_STAGE_2_COPY_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR) ||
        (stage_mask & VK_PIPELINE_STAGE_2_BLIT_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR)) {
        result_stage_mask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if ((stage_mask & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT_KHR) ||
        (stage_mask & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR)) {
        result_stage_mask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }

    if (result_stage_mask == 0 && scope == SynchronizationScope::kFirst) {
        result_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (result_stage_mask == 0 && scope == SynchronizationScope::kSecond) {
        result_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    return result_stage_mask;
}

VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                 VkDecompressMemoryRegionNV const* pDecompressMemoryRegions) {
    try {
        auto device_data = GetDeviceData(commandBuffer);

        if (device_data->vtable.CmdDecompressMemoryNV) {
            device_data->vtable.CmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
        } else {
            device_data->vtable.CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                                device_data->pipelineDecompressSingle);
            for (uint32_t t = 0; t < decompressRegionCount; t++) {
                device_data->vtable.CmdPushConstants(commandBuffer, device_data->pipelineLayoutDecompressSingle,
                                                     VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDecompressMemoryRegionNV),
                                                     (void*)&pDecompressMemoryRegions[t]);
                device_data->vtable.CmdDispatch(commandBuffer, 1, 1, 1);
                PRINT("Info: vkCmdDecompressMemoryNV: Using VK_LAYER_KHRONOS_memory_decompression layer\n");
            }
        }
    } catch (const std::bad_alloc& e) {
        // We don't have a way to return an error here.
        PRINT("bad_alloc: %s\n", e.what());
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                              VkDeviceAddress indirectCommandsAddress,
                                                              VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    try {
        auto device_data = GetDeviceData(commandBuffer);
        if (device_data->vtable.CmdDecompressMemoryIndirectCountNV) {
            device_data->vtable.CmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress,
                                                                   indirectCommandsCountAddress, stride);
        } else {
            DeviceData::PushConstantDataCopy pushConstantDataCopy = {indirectCommandsCountAddress,
                                                                     device_data->indirectDispatchBufferAddress};
            device_data->vtable.CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, device_data->pipelineCopy);
            device_data->vtable.CmdPushConstants(commandBuffer, device_data->pipelineLayoutCopy, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                                 sizeof(DeviceData::PushConstantDataCopy), &pushConstantDataCopy);
            device_data->vtable.CmdDispatch(commandBuffer, 1, 1, 1);

            DeviceData::PushConstantDataDecompressMulti pushConstantData = {indirectCommandsAddress, stride};
            {
                VkBufferMemoryBarrier bufferBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
                bufferBarrier.buffer = device_data->indirectDispatchBuffer;
                bufferBarrier.offset = 0;
                bufferBarrier.size = VK_WHOLE_SIZE;
                bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                bufferBarrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                device_data->vtable.CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0, 1, &bufferBarrier, 0, 0);
            }

            device_data->vtable.CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                                device_data->pipelineDecompressMulti);
            device_data->vtable.CmdPushConstants(commandBuffer, device_data->pipelineLayoutDecompressMulti,
                                                 VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                                 sizeof(DeviceData::PushConstantDataDecompressMulti), &pushConstantData);
            device_data->vtable.CmdDispatchIndirect(commandBuffer, device_data->indirectDispatchBuffer, 0);
            PRINT("Info: vkCmdDecompressMemoryIndirectCountNV: Using VK_LAYER_KHRONOS_memory_decompression layer\n");
        }
    } catch (const std::bad_alloc& e) {
        // We don't have a way to return an error here.
        PRINT("bad_alloc: %s\n", e.what());
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto device_data = GetDeviceData(commandBuffer);
    const auto& features = device_data->features;

    device_data->vtable.CmdPipelineBarrier(
        commandBuffer, ConvertPipelineStageMask(srcStageMask, SynchronizationScope::kFirst, features),
        ConvertPipelineStageMask(dstStageMask, SynchronizationScope::kSecond, features), dependencyFlags, memoryBarrierCount,
        pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName);
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName);

#define ADD_HOOK(fn) \
    { "vk" #fn, (PFN_vkVoidFunction)fn }
#define ADD_HOOK_ALIAS(fn, fn_alias) \
    { "vk" #fn, (PFN_vkVoidFunction)fn_alias }

static const std::unordered_map<std::string, PFN_vkVoidFunction> kInstanceFunctions = {
    ADD_HOOK(CreateInstance),
    ADD_HOOK(DestroyInstance),
    ADD_HOOK(EnumeratePhysicalDevices),
    ADD_HOOK(EnumerateDeviceExtensionProperties),
    ADD_HOOK(CreateDevice),
    ADD_HOOK(GetPhysicalDeviceProperties2),
    ADD_HOOK(GetPhysicalDeviceFeatures2),
};

static const std::unordered_map<std::string, PFN_vkVoidFunction> kDeviceFunctions = {
    ADD_HOOK(DestroyDevice),
    ADD_HOOK(CmdPipelineBarrier),
    ADD_HOOK(CmdDecompressMemoryNV),
    ADD_HOOK(CmdDecompressMemoryIndirectCountNV),

    // Needs to point to itself as Android loaders calls vkGet*ProcAddr to itself. Without these hooks, when the app calls
    // vkGetDeviceProcAddr to get layer functions it will fail on Android
    ADD_HOOK(GetInstanceProcAddr),
    ADD_HOOK(GetDeviceProcAddr),
};
#undef ADD_HOOK
#undef ADD_HOOK_ALIAS

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName) {
    auto instance_result = kInstanceFunctions.find(pName);
    if (instance_result != kInstanceFunctions.end()) {
        return instance_result->second;
    }
    auto dev_result = kDeviceFunctions.find(pName);
    if (dev_result != kDeviceFunctions.end()) {
        return dev_result->second;
    }
    auto instance_data = GetInstanceData(instance);
    if (instance_data != nullptr && instance_data->vtable.GetInstanceProcAddr) {
        PFN_vkVoidFunction result = instance_data->vtable.GetInstanceProcAddr(instance, pName);
        return result;
    }
    return nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName) {
    auto device_data = GetDeviceData(device);
    if (device_data && device_data->enable_layer) {
        auto result = kDeviceFunctions.find(pName);
        if (result != kDeviceFunctions.end()) {
            return result->second;
        }
    }
    if (device_data && device_data->vtable.GetDeviceProcAddr) {
        PFN_vkVoidFunction result = device_data->vtable.GetDeviceProcAddr(device, pName);
        return result;
    }
    return nullptr;
}

}  // namespace memory_decompression

#if defined(__GNUC__) && __GNUC__ >= 4
#define VEL_EXPORT __attribute__((visibility("default")))
#else
#define VEL_EXPORT
#endif

extern "C" VEL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    return memory_decompression::GetInstanceProcAddr(instance, pName);
}

extern "C" VEL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
    return memory_decompression::GetDeviceProcAddr(device, pName);
}

extern "C" VEL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
    ASSERT(pVersionStruct != nullptr);
    ASSERT(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->loaderLayerInterfaceVersion = 2;
        pVersionStruct->pfnGetInstanceProcAddr = memory_decompression::GetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = memory_decompression::GetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;
    }

    return VK_SUCCESS;
}

// loader-layer interface v0 - Needed for Android loader using explicit layers
extern "C" VEL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char* pLayerName,
                                                                                            uint32_t* pPropertyCount,
                                                                                            VkExtensionProperties* pProperties) {
    if (pLayerName && strncmp(pLayerName, memory_decompression::kGlobalLayer.layerName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
        return EnumerateProperties(memory_decompression::kInstanceExtensionPropertiesCount,
                                   memory_decompression::kInstanceExtensionProperties, pPropertyCount, pProperties);
    }
    return VK_ERROR_LAYER_NOT_PRESENT;
}

// loader-layer interface v0 - Needed for Android loader using explicit layers
extern "C" VEL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                                                                                        VkLayerProperties* pProperties) {
    if (pProperties == NULL) {
        *pPropertyCount = 1;
        return VK_SUCCESS;
    }
    if (*pPropertyCount < 1) {
        return VK_INCOMPLETE;
    }
    *pPropertyCount = 1;
    pProperties[0] = memory_decompression::kGlobalLayer;
    return VK_SUCCESS;
}

// loader-layer interface v0 - Needed for Android loader using explicit layers
extern "C" VEL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                          const char* pLayerName,
                                                                                          uint32_t* pPropertyCount,
                                                                                          VkExtensionProperties* pProperties) {
    // Want to have this call down chain if multiple layers are enabling extenions
    return memory_decompression::EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

// Deprecated, but needed or else Android loader will not call into the exported vkEnumerateDeviceExtensionProperties
extern "C" VEL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice, uint32_t* pPropertyCount,
                                                                                      VkLayerProperties* pProperties) {
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}
