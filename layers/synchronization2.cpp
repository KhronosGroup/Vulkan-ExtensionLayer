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
#include <vulkan/vk_layer.h>
#include <ctype.h>
#include <cstring>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include "synchronization2.h"
#include "allocator.h"
#include "log.h"
#include "vk_format_utils.h"
#include "vk_layer_config.h"
#include "vk_safe_struct.h"

#error
// required by vk_safe_struct
std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};

namespace synchronization2 {

static const VkLayerProperties kGlobalLayer = {
    "VK_LAYER_KHRONOS_synchronization2",
    VK_HEADER_VERSION_COMPLETE,
    1,
    "Default synchronization2 layer",
};

static const VkExtensionProperties kDeviceExtension = {VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
                                                       VK_KHR_SYNCHRONIZATION_2_SPEC_VERSION};

static const char* const kEnvarForceEnable =
#if defined(__ANDROID__)
    "debug.vulkan.sync2.force_enable";
#else
    "VK_SYNC2_FORCE_ENABLE";
#endif
static const char* const kLayerSettingsForceEnable = "khronos_synchronization2.force_enable";

// TODO: should we try to use the same fields as ValidationLayers, so that list only needs
// to be defined once?
static const char* const kEnvarCustomStypeList =
#if defined(__ANDROID__)
    "debug.vulkan.sync2.custom_stype_list";
#else
    "VK_LAYER_SYNC2_CUSTOM_STYPE_LIST";
#endif
static const char* const kLayerSettingsCustomStypeList = "khronos_synchronization2.custom_stype_list";

static vk_concurrent_unordered_map<uintptr_t, std::shared_ptr<InstanceData>> instance_data_map;
static vk_concurrent_unordered_map<uintptr_t, std::shared_ptr<DeviceData>> device_data_map;

static void string_tolower(std::string &s) {
    for (auto& c: s) {
        c = tolower(c);
    }
}

static bool GetForceEnable() {
    bool result = false;
    std::string setting = GetLayerEnvVar(kEnvarForceEnable);
    if (setting.empty()) {
        setting = getLayerOption(kLayerSettingsForceEnable);
    }
    if (!setting.empty()) {
        string_tolower(setting);
        if (setting == "true") {
            result = true;
        } else {
            result = std::atoi(setting.c_str()) != 0;
        }
    }
    return result;
}

static std::string GetNextToken(std::string *token_list, const std::string &delimiter, size_t *pos) {
    std::string token;
    *pos = token_list->find(delimiter);
    if (*pos != std::string::npos) {
        token = token_list->substr(0, *pos);
    } else {
        *pos = token_list->length() - delimiter.length();
        token = *token_list;
    }
    token_list->erase(0, *pos + delimiter.length());

    // Remove quotes from quoted strings
    if ((token.length() > 0) && (token[0] == '\"')) {
        token.erase(token.begin());
        if ((token.length() > 0) && (token[token.length() - 1] == '\"')) {
            token.erase(--token.end());
        }
    }
    return token;
}

static uint32_t TokenToUint(const std::string &token) {
    uint32_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

static void SetCustomStypeInfo(std::string raw_id_list, const std::string &delimiter) {
    size_t pos = 0;
    std::string token;
    // List format is a list of integer pairs
    while (raw_id_list.length() != 0) {
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t stype_id = TokenToUint(token);
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t struct_size_in_bytes = TokenToUint(token);
        if ((stype_id != 0) && (struct_size_in_bytes != 0)) {
            bool found = false;
            // Prevent duplicate entries
            for (auto item : custom_stype_info) {
                if (item.first == stype_id) {
                    found = true;
                    break;
                }
            }
            if (!found) custom_stype_info.push_back(std::make_pair(stype_id, struct_size_in_bytes));
        }
    }
}

static void SetupCustomStypes() {
    const std::string kEnvDelim =
#if defined(_WIN32)
        ";";
#else
        ":";
#endif
    SetCustomStypeInfo(getLayerOption(kLayerSettingsCustomStypeList), ",");
    SetCustomStypeInfo(GetLayerEnvVar(kEnvarCustomStypeList), kEnvDelim);
}

uintptr_t DispatchKey(const void* object) {
    auto tmp = reinterpret_cast<const struct VkLayerDispatchTable_ * const *>(object);
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

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount,
                                            VkExtensionProperties* pProperties) {
    if (pLayerName && strncmp(pLayerName, kGlobalLayer.layerName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
        if (!pProperties) {
            *pPropertyCount = 1;
            return VK_SUCCESS;
        }
        if (*pPropertyCount < 1) {
            return VK_INCOMPLETE;
        }
        pProperties[0] = kDeviceExtension;
        *pPropertyCount = 1;
        return VK_SUCCESS;
    } else {
        // Only call down if not the layer
        // Android will pass a null physicalDevice with the layer name so can't get instance data from it
        auto instance_data = GetInstanceData(physicalDevice);
        return instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    }
}

static void CheckDeviceFeatures(PhysicalDeviceData &pdd, VkPhysicalDeviceFeatures2* pFeatures) {
    auto chain = reinterpret_cast<VkBaseInStructure*>(pFeatures->pNext);
    while (chain != nullptr) {
        if (chain->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR) {
            auto feature = reinterpret_cast<VkPhysicalDeviceSynchronization2FeaturesKHR*>(chain);
            if (feature->synchronization2) {
                pdd.lower_has_sync2 = true;
            } else  {
                pdd.lower_has_sync2 = false;
                feature->synchronization2 = true;
            }
        }
        chain = const_cast<VkBaseInStructure*>(chain->pNext);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto instance_data = GetInstanceData(physicalDevice);
    auto pdd = instance_data->GetPhysicalDeviceData(physicalDevice);

    if (instance_data->vtable.GetPhysicalDeviceFeatures2 != nullptr) {
        instance_data->vtable.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }

    CheckDeviceFeatures(*pdd, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto instance_data = GetInstanceData(physicalDevice);
    auto pdd = instance_data->GetPhysicalDeviceData(physicalDevice);

    if (instance_data->vtable.GetPhysicalDeviceFeatures2KHR != nullptr) {
        instance_data->vtable.GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }

    CheckDeviceFeatures(*pdd, pFeatures);
}

#define INIT_HOOK(_vt, _inst, fn) _vt.fn = reinterpret_cast<PFN_vk##fn>(vtable.GetInstanceProcAddr(_inst, "vk" #fn))
InstanceData::InstanceData(VkInstance inst, PFN_vkGetInstanceProcAddr gpa, const VkAllocationCallbacks* alloc)
    : instance(inst), allocator(alloc) {
    vtable.GetInstanceProcAddr = gpa;
    INIT_HOOK(vtable, instance, CreateInstance);
    INIT_HOOK(vtable, instance, DestroyInstance);
    INIT_HOOK(vtable, instance, CreateDevice);
    INIT_HOOK(vtable, instance, EnumeratePhysicalDevices);
    INIT_HOOK(vtable, instance, EnumerateDeviceExtensionProperties);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceFeatures2);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceFeatures2KHR);
    INIT_HOOK(vtable, instance, GetPhysicalDeviceProperties);
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

// Get all elements from a vkEnumerate*() lambda into a std::vector.
template <typename T>
VkResult EnumerateAll(std::vector<T>* vect, std::function<VkResult(uint32_t*, T*)> func) {
    VkResult result = VK_INCOMPLETE;
    do {
        uint32_t count = 0;
        result = func(&count, nullptr);
        ASSERT(result == VK_SUCCESS);
        vect->resize(count);
        result = func(&count, vect->data());
    } while (result == VK_INCOMPLETE);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
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
        auto instance_data = std::make_shared<InstanceData>(*pInstance, gpa,
                                                            pAllocator ? pAllocator : &extension_layer::kDefaultAllocator);

        instance_data_map.insert(DispatchKey(*pInstance), instance_data);

        instance_data->force_enable = GetForceEnable();
        instance_data->api_version = pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : 0;
        SetupCustomStypes();
    } catch (const std::bad_alloc&) {
        auto destroy_instance = reinterpret_cast<PFN_vkDestroyInstance>(gpa(NULL, "vkDestroyInstance"));
        destroy_instance(*pInstance, pAllocator);
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    auto instance_data = GetInstanceData(instance);
    VkResult result =
        instance_data->vtable.EnumeratePhysicalDevices(instance_data->instance, pPhysicalDeviceCount, pPhysicalDevices);
    if (result == VK_SUCCESS && pPhysicalDevices != nullptr) {
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
    : device(device), allocator(alloc), features(feat), enable_layer(enable), image_map() {
    vtable.GetDeviceProcAddr = gpa;
    if (enable_layer) {
        INIT_HOOK(vtable, device, DestroyDevice);
        INIT_HOOK(vtable, device, CreateImage);
        INIT_HOOK(vtable, device, DestroyImage);
        INIT_HOOK(vtable, device, CmdSetEvent);
        INIT_HOOK(vtable, device, CmdResetEvent);
        INIT_HOOK(vtable, device, CmdWaitEvents);
        INIT_HOOK(vtable, device, CmdPipelineBarrier);
        INIT_HOOK(vtable, device, CmdWriteTimestamp);
        INIT_HOOK(vtable, device, QueueSubmit);
        INIT_HOOK(vtable, device, CreateRenderPass2);
        INIT_HOOK_ALIAS(vtable, device, CreateRenderPass2KHR, CreateRenderPass2);
        INIT_HOOK(vtable, device, CmdWriteBufferMarkerAMD);
        INIT_HOOK(vtable, device, GetQueueCheckpointDataNV);
    }
}
#undef INIT_HOOK
#undef INIT_HOOK_ALIAS

static VkLayerDeviceCreateInfo* GetChainInfo(const VkDeviceCreateInfo* pCreateInfo, VkLayerFunction func) {
    auto chain_info = reinterpret_cast<VkLayerDeviceCreateInfo*>(const_cast<void*>(pCreateInfo->pNext));
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = reinterpret_cast<VkLayerDeviceCreateInfo*>(const_cast<void*>(chain_info->pNext));
    }
    ASSERT(chain_info != NULL);
    return chain_info;
}

DeviceFeatures::DeviceFeatures(uint32_t api_version, const VkDeviceCreateInfo* create_info)
    : sync2(false),
      geometry(false),
      tessellation(false),
      meshShader(false),
      taskShader(false),
      shadingRateImage(false),
      advancedBlend(false),
      timelineSemaphore(false),
      deviceGroup(false) {
    if (create_info->pEnabledFeatures != nullptr) {
        //Note: explicit checks against 0 are required to avoid warnings from VS2015
        geometry = 0 != create_info->pEnabledFeatures->geometryShader;
        tessellation = 0 != create_info->pEnabledFeatures->tessellationShader;
    }
    const VkBaseInStructure* chain = reinterpret_cast<const VkBaseInStructure*>(create_info->pNext);

    if (api_version >= VK_MAKE_VERSION(1, 2, 0)) {
        timelineSemaphore = true;
    }
    if (create_info->pEnabledFeatures != nullptr) {
        geometry = 0 != create_info->pEnabledFeatures->geometryShader;
        tessellation = 0 != create_info->pEnabledFeatures->tessellationShader;
    }
    while (chain != nullptr) {
        switch (chain->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR: {
                auto sync2_features = reinterpret_cast<const VkPhysicalDeviceSynchronization2FeaturesKHR*>(chain);
                sync2 = 0 != sync2_features->synchronization2;
            } break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2: {
                auto features2 = reinterpret_cast<const VkPhysicalDeviceFeatures2*>(chain);
                geometry = 0 != features2->features.geometryShader;
                tessellation = 0 != features2->features.tessellationShader;
            } break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: {
                auto advanced_blend = reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT*>(chain);
                advancedBlend = 0 != advanced_blend->advancedBlendCoherentOperations;
            } break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV: {
                auto mesh_shader = reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV*>(chain);
                meshShader = 0 != mesh_shader->meshShader;
                taskShader = 0 != mesh_shader->taskShader;;
            } break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV: {
                auto shading_rate = reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV*>(chain);
                shadingRateImage = 0 != shading_rate->shadingRateImage;
            } break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: {
                auto timeline_sem = reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures*>(chain);
                timelineSemaphore = 0 != timeline_sem->timelineSemaphore;
            } break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES_KHR: {
                deviceGroup = true;
            } break;

            default:
                break;
        }
        chain = chain->pNext;
    }
}

// This code depends on the allocation behavior of SafeStringCopy() in vk_safe_struct.cpp
static void RemoveExtensionString(char** string_array, uint32_t* size, const char* s) {
    for (uint32_t i = 0; i < *size; i++) {
        if (strcmp(string_array[i], s) == 0) {
            delete[] string_array[i];
            string_array[i] = nullptr;
            if ((i + 1) < *size) {
                memmove(&string_array[i], &string_array[i + 1], sizeof(char*) * (*size - (i + 1)));
            }
            *size -= 1;
            break;
        }
    }
}

// this code depends on the allocation behavior of SafePnextCopy() in vk_safe_struct.cpp
static void RemoveDeviceFeature(safe_VkDeviceCreateInfo* create_info, uint32_t s_type) {
    auto cur = reinterpret_cast<const VkBaseInStructure*>(create_info->pNext);
    auto prev_ptr = const_cast<VkBaseInStructure**>(reinterpret_cast<const VkBaseInStructure**>(&create_info->pNext));
    while (cur != nullptr) {
        if (cur->sType == s_type) {
            *prev_ptr = const_cast<VkBaseInStructure*>(cur->pNext);
            delete cur;
            break;
        }
        prev_ptr = const_cast<VkBaseInStructure**>(&cur->pNext);
        cur = cur->pNext;
    }
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

    try {
        bool enable_layer = (features.sync2 && (!pdd->lower_has_sync2 || instance_data->force_enable));
        // Filter out our extension name and feature struct, in a copy of the create info.
        // Only enable device hooks if synchronization2 extension is enabled AND
        // the physical device doesn't support it already or we are force enabled.
        if (enable_layer) {
            safe_VkDeviceCreateInfo create_info(pCreateInfo);

            RemoveExtensionString(const_cast<char**>(create_info.ppEnabledExtensionNames), &create_info.enabledExtensionCount,
                                  VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

            RemoveDeviceFeature(&create_info, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR);

            result = create_device(physicalDevice, create_info.ptr(), pAllocator, pDevice);
        } else {
            result = create_device(physicalDevice, pCreateInfo, pAllocator, pDevice);
        }

        if (result != VK_SUCCESS) {
            return result;
        }
        auto alloccb = pAllocator ? pAllocator : instance_data->allocator;
        auto device_data = std::make_shared<DeviceData>(*pDevice, gdpa, features, enable_layer, alloccb);

        device_data_map.insert(DispatchKey(*pDevice), device_data);
    } catch (const std::bad_alloc&) {
        auto destroy_device = reinterpret_cast<PFN_vkDestroyDevice>(gdpa(*pDevice, "vkDestroyDevice"));
        destroy_device(*pDevice, pAllocator);
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto key = DispatchKey(device);
    auto result = device_data_map.find(key);
    if (result != device_data_map.end()) {
        auto device_data = result->second;

        device_data->vtable.DestroyDevice(device, pAllocator);

        device_data_map.erase(key);
    }
}

static VkPipelineStageFlags ConvertPipelineStageMask(VkPipelineStageFlags2KHR stage_mask, SynchronizationScope scope, const DeviceFeatures &features) {
    VkPipelineStageFlags result_stage_mask = (stage_mask & 0x7fffffff);

    if ((stage_mask & VK_PIPELINE_STAGE_2_COPY_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR) ||
        (stage_mask & VK_PIPELINE_STAGE_2_BLIT_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR)) {
        result_stage_mask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if ((stage_mask & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT_KHR) ||
        (stage_mask & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR)) {
        result_stage_mask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }

    if (stage_mask & VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR) {
        result_stage_mask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        if (features.tessellation) {
            result_stage_mask |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                 VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        }
        if (features.geometry) {
            result_stage_mask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        }
        if (features.meshShader) {
            result_stage_mask |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;
        }
        if (features.taskShader) {
            result_stage_mask |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
        }
    }

    if (result_stage_mask == 0 && scope == kFirst) {
        result_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (result_stage_mask == 0 && scope == kSecond) {
        result_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    return result_stage_mask;
}

static VkAccessFlags ConvertAccessMask(VkAccessFlags2KHR flags2, VkPipelineStageFlags2KHR stage_mask,
                                       const DeviceFeatures& features) {
    VkAccessFlags flags = VkAccessFlags(
        flags2 & ~(VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR |
                   VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR));

    if (flags2 & VK_ACCESS_2_MEMORY_READ_BIT_KHR) {
        if (stage_mask & VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT_KHR) {
            flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR)) {
            flags |= VK_ACCESS_INDEX_READ_BIT;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR)) {
            flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV) ||
            (stage_mask & VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV) || (stage_mask & VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)) {
            flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
            flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_NV) {
            flags |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR)) {
            flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR) {
            flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            if (features.advancedBlend) flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_COPY_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_BLIT_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR)) {
            flags |= VK_ACCESS_TRANSFER_READ_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT_KHR) {
            flags |= VK_ACCESS_HOST_READ_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR) {
            flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                     VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                     VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
                     VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV | VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
                     VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV | VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) {
            flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                     VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                     VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
                     VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT | VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV |
                     VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT | VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV |
                     VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT) {
            flags |= VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT) {
            flags |= VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV) {
            flags |= VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV) {
            flags |= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR) {
            flags |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT) {
            flags |= VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
        }
    }
    if (flags2 & VK_ACCESS_2_MEMORY_WRITE_BIT_KHR) {
        if ((stage_mask & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV) ||
            (stage_mask & VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV) || (stage_mask & VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)) {
            flags |= VK_ACCESS_SHADER_WRITE_BIT;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR)) {
            flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR) {
            flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }

        if ((stage_mask & VK_PIPELINE_STAGE_2_COPY_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR) || (stage_mask & VK_PIPELINE_STAGE_2_BLIT_BIT_KHR) ||
            (stage_mask & VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR)) {
            flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT_KHR) {
            flags |= VK_ACCESS_HOST_WRITE_BIT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR) {
            flags |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT |
                     VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT | VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) {
            flags |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT |
                     VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT |
                     VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT) {
            flags |= VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT | VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
        }

        if (stage_mask & VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR) {
            flags |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        }
    }

    if (flags2 & (VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR)) {
        flags |= VK_ACCESS_SHADER_READ_BIT;
    }

    if (flags2 & VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR) {
        flags |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    return flags;
}

static ImageAspect ImageAspectFromFormat(VkFormat format) {
    if (FormatIsDepthAndStencil(format)) {
        return kDepthAndStencil;
    } else if (FormatIsDepthOnly(format)) {
        return kDepthOnly;
    } else if (FormatIsStencilOnly(format)) {
        return kStencilOnly;
    } else {
        return kColorOnly;
    }
}

static VkImageLayout ImageLayoutFromAspect(VkImageLayout layout, ImageAspect aspect, VkImageAspectFlags aspectMask) {
    if (aspect == kDepthAndStencil) {
        if ((aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) && !(aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
            aspect = kDepthOnly;
        }
        if ((aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) && !(aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)) {
            aspect = kStencilOnly;
        }
    }

    switch (aspect) {
        case kColorOnly: {
            if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
        }
        case kDepthAndStencil: {
            if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            break;
        }
        case kDepthOnly: {
            if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
            break;
        }
        case kStencilOnly: {
            if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            break;
        }
    }

    return layout;
}

BufferMemoryBarrier::BufferMemoryBarrier(const VkBufferMemoryBarrier2KHR& v2, const DeviceFeatures& features) {
    sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    pNext = v2.pNext;
    srcAccessMask = ConvertAccessMask(v2.srcAccessMask, v2.srcStageMask, features);
    dstAccessMask = ConvertAccessMask(v2.dstAccessMask, v2.dstStageMask, features);
    srcQueueFamilyIndex = v2.srcQueueFamilyIndex;
    dstQueueFamilyIndex = v2.dstQueueFamilyIndex;
    buffer = v2.buffer;
    offset = v2.offset;
    size = v2.size;
}

ImageMemoryBarrier::ImageMemoryBarrier(const VkImageMemoryBarrier2KHR& v2, const DeviceFeatures& features, ImageAspect aspect) {
    sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    pNext = v2.pNext;
    srcAccessMask = ConvertAccessMask(v2.srcAccessMask, v2.srcStageMask, features);
    dstAccessMask = ConvertAccessMask(v2.dstAccessMask, v2.dstStageMask, features);

    srcQueueFamilyIndex = v2.srcQueueFamilyIndex;
    dstQueueFamilyIndex = v2.dstQueueFamilyIndex;

    oldLayout = ImageLayoutFromAspect(v2.oldLayout, aspect, v2.subresourceRange.aspectMask);
    newLayout = ImageLayoutFromAspect(v2.newLayout, aspect, v2.subresourceRange.aspectMask);

    image = v2.image;
    subresourceRange = v2.subresourceRange;
}

DependencyInfoV1::DependencyInfoV1(const DeviceData& device_data, uint32_t info_count, const VkDependencyInfoKHR* infos,
                                   const VkAllocationCallbacks* allocator)
    : src_stage_mask(0),
      dst_stage_mask(0),
      flags(0),
      barrier(),
      buffer_barriers(decltype(buffer_barriers)::allocator_type(allocator)),
      image_barriers(decltype(image_barriers)::allocator_type(allocator)) {
    uint32_t buffer_barrier_count = 0;
    uint32_t image_barrier_count = 0;

    // Calculate how many image and buffer memory barriers are required.
    for (uint32_t i = 0; i < info_count; ++i) {
        buffer_barrier_count += infos[i].bufferMemoryBarrierCount;
        for (uint32_t j = 0; j < infos[i].imageMemoryBarrierCount; ++j) {
            const VkImageMemoryBarrier2KHR* image_barrier = &infos[i].pImageMemoryBarriers[j];
            if ((image_barrier->oldLayout != image_barrier->newLayout) ||
                (image_barrier->srcQueueFamilyIndex != image_barrier->dstQueueFamilyIndex)) {
                image_barrier_count++;
            }
        }
    }

    // Allocate the required memory barriers.
    buffer_barriers.reserve(buffer_barrier_count);
    image_barriers.reserve(image_barrier_count);

    // Only ever use a single global barrier since they can always be combined
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    // Loop through each dependency info in order to combine them into a single info structure
    for (uint32_t i = 0; i < info_count; ++i) {
        const VkDependencyInfoKHR* info = &infos[i];

        // Collapse global barriers into the single output barrier
        for (uint32_t j = 0; j < info->memoryBarrierCount; ++j) {
            const VkMemoryBarrier2KHR* barrier_v2 = &info->pMemoryBarriers[j];

            src_stage_mask |= ConvertPipelineStageMask(barrier_v2->srcStageMask, kFirst, device_data.features);
            dst_stage_mask |= ConvertPipelineStageMask(barrier_v2->dstStageMask, kSecond, device_data.features);
            barrier.srcAccessMask |= ConvertAccessMask(barrier_v2->srcAccessMask, barrier_v2->srcStageMask, device_data.features);
            barrier.dstAccessMask |= ConvertAccessMask(barrier_v2->dstAccessMask, barrier_v2->dstStageMask, device_data.features);
        }

        // Buffer Barriers
        for (uint32_t j = 0; j < info->bufferMemoryBarrierCount; ++j) {
            const VkBufferMemoryBarrier2KHR& barrier_v2 = info->pBufferMemoryBarriers[j];

            src_stage_mask |= ConvertPipelineStageMask(barrier_v2.srcStageMask, kFirst, device_data.features);
            dst_stage_mask |= ConvertPipelineStageMask(barrier_v2.dstStageMask, kSecond, device_data.features);

            buffer_barriers.emplace_back(barrier_v2, device_data.features);
        }

        // Image Barriers
        for (uint32_t j = 0; j < info->imageMemoryBarrierCount; ++j) {
            const VkImageMemoryBarrier2KHR& barrier_v2 = info->pImageMemoryBarriers[j];

            auto image_data = device_data.image_map.find(barrier_v2.image);
            ASSERT(image_data != device_data.image_map.end());
            ImageAspect aspect = image_data->second.aspect;

            src_stage_mask |= ConvertPipelineStageMask(barrier_v2.srcStageMask, kFirst, device_data.features);
            dst_stage_mask |= ConvertPipelineStageMask(barrier_v2.dstStageMask, kSecond, device_data.features);

            // If there's no valid image layout transition, it needs to be turned into a regular memory barrier to be valid in
            // sync1.  But check for this on the converted v1 barrier so that we use the final image layouts rather than
            // the new 'generic' ones.
            ImageMemoryBarrier barrier_v1(barrier_v2, device_data.features, aspect);

            if ((barrier_v1.oldLayout == barrier_v1.newLayout) &&
                (barrier_v1.srcQueueFamilyIndex == barrier_v1.dstQueueFamilyIndex)) {
                barrier.srcAccessMask |=
                    ConvertAccessMask(barrier_v2.srcAccessMask, barrier_v2.srcStageMask, device_data.features);
                barrier.dstAccessMask |=
                    ConvertAccessMask(barrier_v2.dstAccessMask, barrier_v2.dstStageMask, device_data.features);
            } else {
                image_barriers.push_back(barrier_v1);
            }
        }
    }
    // if we didn't get stage masks on any of the barriers, use the defaults.
    if (src_stage_mask == 0) {
        src_stage_mask = ConvertPipelineStageMask(src_stage_mask, kFirst, device_data.features);
    }
    if (dst_stage_mask == 0) {
        dst_stage_mask = ConvertPipelineStageMask(dst_stage_mask, kSecond, device_data.features);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                     VkImage* pImage) {
    auto device_data = GetDeviceData(device);
    VkResult result = device_data->vtable.CreateImage(device, pCreateInfo, pAllocator, pImage);
    if (result == VK_SUCCESS) {
        ImageData image_data{ImageAspectFromFormat(pCreateInfo->format)};
        device_data->image_map.insert(*pImage, image_data);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    auto device_data = GetDeviceData(device);
    device_data->vtable.DestroyImage(device, image, pAllocator);
    device_data->image_map.erase(image);
}

VKAPI_ATTR void VKAPI_CALL CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stage_mask) {
    auto device_data = GetDeviceData(commandBuffer);

    device_data->vtable.CmdSetEvent(commandBuffer, event, ConvertPipelineStageMask(stage_mask, kFirst, device_data->features));
}

VKAPI_ATTR void VKAPI_CALL CmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR* pDependencyInfo) {
    auto device_data = GetDeviceData(commandBuffer);
    const auto& features = device_data->features;

    VkPipelineStageFlags stage_mask = 0;
    for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; ++i) {
        stage_mask |= ConvertPipelineStageMask(pDependencyInfo->pMemoryBarriers[i].srcStageMask, kFirst, features);
    }
    for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; ++i) {
        stage_mask |= ConvertPipelineStageMask(pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask, kFirst, features);
    }
    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        stage_mask |= ConvertPipelineStageMask(pDependencyInfo->pImageMemoryBarriers[i].srcStageMask, kFirst, features);
    }
    // set the default stage mask if we don't have one from the barriers
    if (stage_mask == VK_PIPELINE_STAGE_NONE_KHR) {
        stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    device_data->vtable.CmdSetEvent(commandBuffer, event, stage_mask);
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stage_mask) {
    auto device_data = GetDeviceData(commandBuffer);

    device_data->vtable.CmdResetEvent(commandBuffer, event, ConvertPipelineStageMask(stage_mask, kFirst, device_data->features));
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stage_mask) {
    auto device_data = GetDeviceData(commandBuffer);

    device_data->vtable.CmdResetEvent(commandBuffer, event, ConvertPipelineStageMask(stage_mask, kFirst, device_data->features));
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask,
                   VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                   uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                   uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto device_data = GetDeviceData(commandBuffer);
    const auto& features = device_data->features;

    device_data->vtable.CmdWaitEvents(commandBuffer, eventCount, pEvents, ConvertPipelineStageMask(srcStageMask, kFirst, features),
                                      ConvertPipelineStageMask(dstStageMask, kSecond, features), memoryBarrierCount, pMemoryBarriers,
                                      bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                      pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                       const VkDependencyInfoKHR* pDependencyInfos) {
    try {
        auto device_data = GetDeviceData(commandBuffer);

        DependencyInfoV1 dep_info(*device_data, eventCount, pDependencyInfos, device_data->allocator);

        // don't pass in an invalid VkMemoryBarrier
        uint32_t mem_barrier_count = 0;
        VkMemoryBarrier* mem_barrier = nullptr;
        if (dep_info.barrier.srcAccessMask != 0 || dep_info.barrier.dstAccessMask != 0) {
            mem_barrier_count = 1;
            mem_barrier = &dep_info.barrier;
        }

        device_data->vtable.CmdWaitEvents(
            commandBuffer, eventCount, pEvents, dep_info.src_stage_mask, dep_info.dst_stage_mask, mem_barrier_count, mem_barrier,
            VecSize(dep_info.buffer_barriers), reinterpret_cast<VkBufferMemoryBarrier*>(dep_info.buffer_barriers.data()),
            VecSize(dep_info.image_barriers), reinterpret_cast<VkImageMemoryBarrier*>(dep_info.image_barriers.data()));
    } catch (const std::bad_alloc& e) {
        // We don't have a way to return an error here.
        LOG("bad_alloc: %s\n", e.what());
        return;
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                        VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                        uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                        uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto device_data = GetDeviceData(commandBuffer);
    const auto& features = device_data->features;

    device_data->vtable.CmdPipelineBarrier(commandBuffer, ConvertPipelineStageMask(srcStageMask, kFirst, features),
                                           ConvertPipelineStageMask(dstStageMask, kSecond, features), dependencyFlags, memoryBarrierCount,
                                           pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                           imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* dependencyInfo) {
    try {
        auto device_data = GetDeviceData(commandBuffer);

        DependencyInfoV1 dep_info(*device_data, 1, dependencyInfo, device_data->allocator);
        uint32_t mem_barrier_count = 0;
        VkMemoryBarrier* mem_barrier = nullptr;
        if (dep_info.barrier.srcAccessMask != 0 || dep_info.barrier.dstAccessMask != 0) {
            mem_barrier_count = 1;
            mem_barrier = &dep_info.barrier;
        }
        device_data->vtable.CmdPipelineBarrier(commandBuffer, dep_info.src_stage_mask, dep_info.dst_stage_mask, dep_info.flags,
                                               mem_barrier_count, mem_barrier, VecSize(dep_info.buffer_barriers),
                                               dep_info.buffer_barriers.data(), VecSize(dep_info.image_barriers),
                                               dep_info.image_barriers.data());
    } catch (const std::bad_alloc& e) {
        LOG("bad_alloc: %s\n", e.what());
        return;
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                       uint32_t query) {
    auto device_data = GetDeviceData(commandBuffer);

    if (pipelineStage == VK_PIPELINE_STAGE_NONE_KHR) {
        pipelineStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    device_data->vtable.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage_mask, VkQueryPool queryPool,
                           uint32_t query) {
    auto device_data = GetDeviceData(commandBuffer);

    auto pipelineStage = static_cast<VkPipelineStageFlagBits>(ConvertPipelineStageMask(stage_mask, kFirst, device_data->features));
    device_data->vtable.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

DeviceGroupSubmitInfo::DeviceGroupSubmitInfo(const VkAllocationCallbacks* alloc)
    : info{},
      wait_vec(decltype(wait_vec)::allocator_type(alloc)),
      cmd_vec(decltype(cmd_vec)::allocator_type(alloc)),
      signal_vec(decltype(cmd_vec)::allocator_type(alloc)) {
    info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO;
}

DeviceGroupSubmitInfo::DeviceGroupSubmitInfo(const DeviceFeatures& features, const VkSubmitInfo2KHR& v2,
                                             const VkAllocationCallbacks* alloc)
    : info{},
      wait_vec(decltype(wait_vec)::allocator_type(alloc)),
      cmd_vec(decltype(cmd_vec)::allocator_type(alloc)),
      signal_vec(decltype(cmd_vec)::allocator_type(alloc)) {
    info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO;
    // skip translation if this feature isn't enabled
    if (!features.deviceGroup) {
        return;
    }

    if (v2.waitSemaphoreInfoCount > 0) {
        wait_vec.reserve(v2.waitSemaphoreInfoCount);
        for (uint32_t i = 0; i < v2.waitSemaphoreInfoCount; i++) {
            wait_vec.push_back(v2.pWaitSemaphoreInfos[i].deviceIndex);
        }
        info.waitSemaphoreCount = VecSize(wait_vec);
        info.pWaitSemaphoreDeviceIndices = wait_vec.data();
    }
    if (v2.commandBufferInfoCount > 0) {
        cmd_vec.reserve(v2.commandBufferInfoCount);
        for (uint32_t i = 0; i < v2.commandBufferInfoCount; i++) {
            cmd_vec.push_back(v2.pCommandBufferInfos[i].deviceMask);
        }
        info.commandBufferCount = VecSize(cmd_vec);
        info.pCommandBufferDeviceMasks = cmd_vec.data();
    }

    if (v2.signalSemaphoreInfoCount > 0) {
        signal_vec.reserve(v2.signalSemaphoreInfoCount);
        for (uint32_t i = 0; i < v2.signalSemaphoreInfoCount; i++) {
            signal_vec.push_back(v2.pSignalSemaphoreInfos[i].deviceIndex);
        }
        info.signalSemaphoreCount = VecSize(signal_vec);
        info.pSignalSemaphoreDeviceIndices = signal_vec.data();
    }
}

TimelineSemaphoreSubmitInfo::TimelineSemaphoreSubmitInfo(const VkAllocationCallbacks* alloc)
    : info{}, wait_vec(decltype(wait_vec)::allocator_type(alloc)), signal_vec(decltype(signal_vec)::allocator_type(alloc)) {
    info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
}

TimelineSemaphoreSubmitInfo::TimelineSemaphoreSubmitInfo(const DeviceFeatures& features, const VkSubmitInfo2KHR& v2,
                                                         const VkAllocationCallbacks* alloc)
    : info{}, wait_vec(decltype(wait_vec)::allocator_type(alloc)), signal_vec(decltype(signal_vec)::allocator_type(alloc)) {
    info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;

    // skip translation if this feature isn't enabled
    if (!features.timelineSemaphore) {
        return;
    }

    if (v2.waitSemaphoreInfoCount > 0) {
        wait_vec.reserve(v2.waitSemaphoreInfoCount);
        for (uint32_t i = 0; i < v2.waitSemaphoreInfoCount; i++) {
            wait_vec.push_back(v2.pWaitSemaphoreInfos[i].value);
        }
        info.waitSemaphoreValueCount = VecSize(wait_vec);
        info.pWaitSemaphoreValues = wait_vec.data();
    }
    if (v2.signalSemaphoreInfoCount > 0) {
        signal_vec.reserve(v2.signalSemaphoreInfoCount);
        for (uint32_t i = 0; i < v2.signalSemaphoreInfoCount; i++) {
            signal_vec.push_back(v2.pSignalSemaphoreInfos[i].value);
        }
        info.signalSemaphoreValueCount = VecSize(signal_vec);
        info.pSignalSemaphoreValues = signal_vec.data();
    }
}

SubmitData::SubmitData(const VkSubmitInfo2KHR& v2, const VkAllocationCallbacks* alloc, const DeviceFeatures& features)
    : info{},
      wait_sem_vec(decltype(wait_sem_vec)::allocator_type(alloc)),
      wait_dst_vec(decltype(wait_dst_vec)::allocator_type(alloc)),
      cmd_vec(decltype(cmd_vec)::allocator_type(alloc)),
      signal_vec(decltype(signal_vec)::allocator_type(alloc)),
      protect(v2),
      timeline_sem(features, v2, alloc),
      device_group(features, v2, alloc) {
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    if (v2.waitSemaphoreInfoCount > 0) {
        wait_sem_vec.reserve(v2.waitSemaphoreInfoCount);
        wait_dst_vec.reserve(v2.waitSemaphoreInfoCount);
        for (uint32_t i = 0; i < v2.waitSemaphoreInfoCount; i++) {
            wait_sem_vec.push_back(v2.pWaitSemaphoreInfos[i].semaphore);
            wait_dst_vec.push_back(ConvertPipelineStageMask(v2.pWaitSemaphoreInfos[i].stageMask, kSecond, features));
        }
        info.waitSemaphoreCount = VecSize(wait_sem_vec);
        info.pWaitSemaphores = wait_sem_vec.data();
        info.pWaitDstStageMask = wait_dst_vec.data();
    }

    if (v2.commandBufferInfoCount > 0) {
        cmd_vec.reserve(v2.commandBufferInfoCount);
        for (uint32_t i = 0; i < v2.commandBufferInfoCount; i++) {
            cmd_vec.push_back(v2.pCommandBufferInfos[i].commandBuffer);
        }
        info.commandBufferCount = VecSize(cmd_vec);
        info.pCommandBuffers = cmd_vec.data();
    }

    if (v2.signalSemaphoreInfoCount > 0) {
        signal_vec.reserve(v2.signalSemaphoreInfoCount);
        for (uint32_t i = 0; i < v2.signalSemaphoreInfoCount; i++) {
            signal_vec.push_back(v2.pSignalSemaphoreInfos[i].semaphore);
        }
        info.signalSemaphoreCount = VecSize(signal_vec);
        info.pSignalSemaphores = signal_vec.data();
    }
    const void* tail = v2.pNext;
    // This structure is only needed for a protected submit. Not
    // including it is equivalent to setting protectedSubmit to false.
    if (protect.protectedSubmit) {
        protect.pNext = tail;
        tail = &protect;
    }
    if (features.timelineSemaphore) {
        timeline_sem.info.pNext = tail;
        tail = &timeline_sem.info;
    }
    if (features.deviceGroup) {
        device_group.info.pNext = tail;
        tail = &device_group.info;
    }
    info.pNext = tail;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence) {
    VkResult ret;
    try {
        auto device_data = GetDeviceData(queue);
        if (submitCount == 0 || pSubmits == nullptr) {
            return device_data->vtable.QueueSubmit(queue, 0, nullptr, fence);
        }

        CmdVector<SubmitData> submit_data_vec(extension_layer::CmdAlloc<SubmitData>(device_data->allocator));
        CmdVector<VkSubmitInfo> submit_infos(extension_layer::CmdAlloc<VkSubmitInfo>(device_data->allocator));

        submit_data_vec.reserve(submitCount);
        submit_infos.reserve(submitCount);
        for (uint32_t i = 0; i < submitCount; i++) {
            submit_data_vec.emplace_back(pSubmits[i], device_data->allocator, device_data->features);
            submit_infos.push_back(submit_data_vec.back().info);
        }

        ret = device_data->vtable.QueueSubmit(queue, VecSize(submit_infos), submit_infos.data(), fence);
    } catch (const std::bad_alloc&) {
        ret = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return ret;
}

SubmitData::SubmitData(const VkSubmitInfo& orig, const VkAllocationCallbacks* alloc, const DeviceFeatures &features)
    : info{},
      wait_sem_vec(decltype(wait_sem_vec)::allocator_type(alloc)),
      wait_dst_vec(decltype(wait_dst_vec)::allocator_type(alloc)),
      cmd_vec(decltype(cmd_vec)::allocator_type(alloc)),
      signal_vec(decltype(signal_vec)::allocator_type(alloc)),
      protect(),
      timeline_sem(alloc),
      device_group(alloc) {
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // All we need to do is fix up any uses of VK_PIPELINE_STAGE_NONE_KHR.
    info.waitSemaphoreCount = orig.waitSemaphoreCount;
    info.pWaitSemaphores = orig.pWaitSemaphores;
    if (orig.waitSemaphoreCount > 0) {
        wait_dst_vec.reserve(orig.waitSemaphoreCount);
        for (uint32_t i = 0; i < orig.waitSemaphoreCount; i++) {
            wait_dst_vec.push_back(ConvertPipelineStageMask(orig.pWaitDstStageMask[i], kSecond, features));
        }
        info.pWaitDstStageMask = wait_dst_vec.data();
    } else {
        info.pWaitDstStageMask = nullptr;
    }

    info.commandBufferCount = orig.commandBufferCount;
    info.pCommandBuffers = orig.pCommandBuffers;

    info.signalSemaphoreCount = orig.signalSemaphoreCount;
    info.pSignalSemaphores = orig.pSignalSemaphores;

    info.pNext = orig.pNext;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    VkResult ret;
    try {
        auto device_data = GetDeviceData(queue);
        if (submitCount == 0 || pSubmits == nullptr) {
            return device_data->vtable.QueueSubmit(queue, 0, nullptr, fence);
        }

        CmdVector<SubmitData> submit_data_vec(extension_layer::CmdAlloc<SubmitData>(device_data->allocator));
        CmdVector<VkSubmitInfo> submit_infos(extension_layer::CmdAlloc<VkSubmitInfo>(device_data->allocator));

        submit_data_vec.reserve(submitCount);
        submit_infos.reserve(submitCount);
        for (uint32_t i = 0; i < submitCount; i++) {
            submit_data_vec.emplace_back(pSubmits[i], device_data->allocator, device_data->features);
            submit_infos.push_back(submit_data_vec.back().info);
        }

        ret = device_data->vtable.QueueSubmit(queue, VecSize(submit_infos), submit_infos.data(), fence);
    } catch (const std::bad_alloc&) {
        ret = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return ret;
}

VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage_mask, VkBuffer dstBuffer,
                              VkDeviceSize dstOffset, uint32_t marker) {
    auto device_data = GetDeviceData(commandBuffer);

    if (device_data->vtable.CmdWriteBufferMarkerAMD != nullptr) {
        auto pipelineStage = static_cast<VkPipelineStageFlagBits>(ConvertPipelineStageMask(stage_mask, kFirst, device_data->features));
        device_data->vtable.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    }
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData2) {
    try {
        auto device_data = GetDeviceData(queue);
        if (device_data->vtable.GetQueueCheckpointDataNV == nullptr) {
            return;
        }

        CmdVector<VkCheckpointDataNV> checkpoint_data(extension_layer::CmdAlloc<VkCheckpointDataNV>(device_data->allocator));

        if (pCheckpointData2 != nullptr) {
            checkpoint_data.reserve(*pCheckpointDataCount);
            for (uint32_t i = 0; i < *pCheckpointDataCount; ++i) {
                checkpoint_data[i].sType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;
                checkpoint_data[i].pNext = pCheckpointData2[i].pNext;
            }
        }

        device_data->vtable.GetQueueCheckpointDataNV(queue, pCheckpointDataCount, checkpoint_data.data());

        if (pCheckpointData2 != nullptr) {
            for (uint32_t i = 0; i < *pCheckpointDataCount; ++i) {
                pCheckpointData2[i].stage = checkpoint_data[i].stage;
                pCheckpointData2[i].pCheckpointMarker = checkpoint_data[i].pCheckpointMarker;
            }
        }
    } catch (const std::bad_alloc& e) {
        // We don't have a way to return an error here.
        LOG("bad_alloc: %s\n", e.what());
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                           VkRenderPass* pRenderPass) {
    auto device_data = GetDeviceData(device);

    if (device_data->vtable.CreateRenderPass2 == nullptr) {
        LOG("Device does not support CreateRenderPass2\n");
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    // make a shallow copy of the structure so we can mess with it.
    VkRenderPassCreateInfo2 create_info = *pCreateInfo;
    // we'll make our own versions of these arrays
    create_info.attachmentCount = 0;
    create_info.pAttachments = nullptr;
    create_info.dependencyCount = 0;
    create_info.pDependencies = nullptr;

    const auto allocator = pAllocator ? pAllocator : device_data->allocator;
    extension_layer::CmdAlloc<VkAttachmentDescription2> a1(allocator);
    CmdVector<VkAttachmentDescription2> attachment_vec(a1);
    extension_layer::CmdAlloc<VkSubpassDependency2> a2(allocator);
    CmdVector<VkSubpassDependency2> dependency_vec(a2);

    try {
        if (pCreateInfo->attachmentCount > 0) {
            attachment_vec.reserve(pCreateInfo->attachmentCount);

            for (uint32_t i = 0; i < pCreateInfo->attachmentCount; i++) {
                // pNext is shallow copied here.
                VkAttachmentDescription2 attachment = pCreateInfo->pAttachments[i];

                ImageAspect aspect = ImageAspectFromFormat(attachment.format);
                if (aspect == kDepthAndStencil) {
                    auto chain = const_cast<VkBaseInStructure*>(reinterpret_cast<const VkBaseInStructure*>(attachment.pNext));
                    while (chain != nullptr) {
                        if (chain->sType == VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT_KHR) {
                            auto stencil_layout = reinterpret_cast<VkAttachmentDescriptionStencilLayoutKHR*>(chain);
                            stencil_layout->stencilInitialLayout =
                                ImageLayoutFromAspect(stencil_layout->stencilInitialLayout, kStencilOnly, 0);
                            stencil_layout->stencilFinalLayout =
                                ImageLayoutFromAspect(stencil_layout->stencilFinalLayout, kStencilOnly, 0);
                            aspect = kDepthOnly;
                            break;
                        }
                        chain = const_cast<VkBaseInStructure*>(chain->pNext);
                    }
                }
                attachment.initialLayout = ImageLayoutFromAspect(attachment.initialLayout, aspect, 0);
                attachment.finalLayout = ImageLayoutFromAspect(attachment.finalLayout, aspect, 0);

                attachment_vec.emplace_back(std::move(attachment));
            }
            create_info.attachmentCount = VecSize(attachment_vec);
            create_info.pAttachments = attachment_vec.data();
        }

        if (pCreateInfo->dependencyCount > 0) {
            dependency_vec.reserve(pCreateInfo->dependencyCount);
            const auto& features = device_data->features;
            for (uint32_t i = 0; i < pCreateInfo->dependencyCount; i++) {
                VkSubpassDependency2 dependency = pCreateInfo->pDependencies[i];
                // so far, the only extension struct for this type is VkMemoryBarrier2KHR
                auto chain = const_cast<VkBaseInStructure*>(reinterpret_cast<const VkBaseInStructure*>(dependency.pNext));
                while (chain != nullptr) {
                    if (chain->sType == VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR) {
                        auto barrier = reinterpret_cast<VkMemoryBarrier2KHR*>(chain);
                        dependency.srcStageMask = ConvertPipelineStageMask(barrier->srcStageMask, kFirst, features);
                        dependency.dstStageMask = ConvertPipelineStageMask(barrier->dstStageMask, kSecond, features);
                        dependency.srcAccessMask = ConvertAccessMask(barrier->srcAccessMask, barrier->srcStageMask, features);
                        dependency.dstAccessMask = ConvertAccessMask(barrier->dstAccessMask, barrier->dstStageMask, features);
                    }
                    chain = const_cast<VkBaseInStructure*>(chain->pNext);
                }
                dependency.pNext = nullptr;

                if (dependency.srcStageMask == 0) {
                    dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                }
                if (dependency.dstStageMask == 0) {
                    dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                }
                dependency_vec.emplace_back(std::move(dependency));
            }
            create_info.dependencyCount = VecSize(dependency_vec);
            create_info.pDependencies = dependency_vec.data();
        }
    } catch (std::bad_alloc&) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return device_data->vtable.CreateRenderPass2(device, &create_info, pAllocator, pRenderPass);
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
    ADD_HOOK(GetPhysicalDeviceFeatures2),
    ADD_HOOK(GetPhysicalDeviceFeatures2KHR),
};

static const std::unordered_map<std::string, PFN_vkVoidFunction> kDeviceFunctions = {
    ADD_HOOK(DestroyDevice),
    ADD_HOOK(CreateImage),
    ADD_HOOK(DestroyImage),

    // NOTE: we need to hook the original synchronization functions
    // to translate VK_PIPELINE_STAGE_NONE_KHR.
    ADD_HOOK(CmdSetEvent),
    ADD_HOOK(CmdSetEvent2KHR),
    ADD_HOOK(CmdResetEvent),
    ADD_HOOK(CmdResetEvent2KHR),
    ADD_HOOK(CmdWaitEvents),
    ADD_HOOK(CmdWaitEvents2KHR),
    ADD_HOOK(CmdPipelineBarrier),
    ADD_HOOK(CmdPipelineBarrier2KHR),
    ADD_HOOK(CmdWriteTimestamp),
    ADD_HOOK(CmdWriteTimestamp2KHR),
    ADD_HOOK(QueueSubmit2KHR),
    ADD_HOOK(CreateRenderPass2),
    ADD_HOOK_ALIAS(CreateRenderPass2KHR, CreateRenderPass2),

    ADD_HOOK(CmdWriteBufferMarker2AMD),
    ADD_HOOK(GetQueueCheckpointData2NV),

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

}  // namespace synchronization2

extern "C" VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    return synchronization2::GetInstanceProcAddr(instance, pName);
}

extern "C" VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
    return synchronization2::GetDeviceProcAddr(device, pName);
}

extern "C" VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
    ASSERT(pVersionStruct != nullptr);
    ASSERT(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->loaderLayerInterfaceVersion = 2;
        pVersionStruct->pfnGetInstanceProcAddr = synchronization2::GetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = synchronization2::GetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;
    }

    return VK_SUCCESS;
}

// loader-layer interface v0 - Needed for Android loader using explicit layers
extern "C" VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    if (pLayerName && strncmp(pLayerName, synchronization2::kGlobalLayer.layerName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
        // VK_KHR_synchronization2 is a device extension and don't want to have it labeled as both instance and device extension
        *pPropertyCount = 0;
        return VK_SUCCESS;
    }
    return VK_ERROR_LAYER_NOT_PRESENT;
}

// loader-layer interface v0 - Needed for Android loader using explicit layers
extern "C" VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                                                                                             VkLayerProperties* pProperties) {
    if (pProperties == NULL) {
        *pPropertyCount = 1;
        return VK_SUCCESS;
    }
    if (*pPropertyCount < 1) {
        return VK_INCOMPLETE;
    }
    *pPropertyCount = 1;
    pProperties[0] = synchronization2::kGlobalLayer;
    return VK_SUCCESS;
}

// loader-layer interface v0 - Needed for Android loader using explicit layers
extern "C" VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                               const char* pLayerName,
                                                                                               uint32_t* pPropertyCount,
                                                                                               VkExtensionProperties* pProperties) {
    // Want to have this call down chain if multiple layers are enabling extenions
    return synchronization2::EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

// Deprecated, but needed or else Android loader will not call into the exported vkEnumerateDeviceExtensionProperties
extern "C" VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice,
                                                                                           uint32_t* pPropertyCount,
                                                                                           VkLayerProperties* pProperties) {
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}
