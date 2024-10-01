/*
 * Copyright 2023-2024 Nintendo
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

// clang-format off

#include <cassert>
#include <cctype>
#include <cstring>
#include <chrono>
#include <bitset>
#include <utility>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <vector>
#include <atomic>
#include <algorithm>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <vulkan/layer/vk_layer_settings.hpp>
#include <vulkan/utility/vk_safe_struct.hpp>

#include "log.h"
#include "vk_api_hash.h"
#include "vk_util.h"
#include "vk_common.h"

#include "shader_object/shader_object_structs.h"

#define kLayerSettingsForceEnable "force_enable"
#define kLayerSettingsDisablePipelinePreCaching "disable_pipeline_pre_caching"
#define kLayerSettingsCustomSTypeInfo "custom_stype_list"

#define SHADER_OBJECT_BINARY_VERSION 1

//#define ENABLE_DEBUG_LOG
//#define DEBUG_LOG_TO_OUTPUT

#if defined(ENABLE_DEBUG_LOG)
#if defined(DEBUG_LOG_TO_OUTPUT)
#define WIN32_MEAN_AND_LEAN
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>
#define DEBUG_LOG(...)                                                \
    {                                                                 \
        char msg[256] = {};                                           \
        sprintf(msg, "[VkLayer_khronos_shader_object] " __VA_ARGS__); \
        OutputDebugString(msg);                                       \
    }
#else
#define DEBUG_LOG(...)                                               \
    fprintf(stderr, "[VkLayer_khronos_shader_object] " __VA_ARGS__); \
    fflush(stderr)
#endif
#else
#define DEBUG_LOG(...)
#endif

#define ASSERT_VK_FALSE(state) ASSERT((state) == VK_FALSE)
#define ASSERT_VK_TRUE(state) ASSERT((state) == VK_TRUE)

#define UNIMPLEMENTED() ASSERT(!"Unimplemented")

#define UNUSED(x) ((void)x)

namespace shader_object {

static const char* kLayerName = "VK_LAYER_KHRONOS_shader_object";
static const VkExtensionProperties kExtensionProperties = {VK_EXT_SHADER_OBJECT_EXTENSION_NAME, VK_EXT_SHADER_OBJECT_SPEC_VERSION};

// Instance extensions that this layer provides:
const VkExtensionProperties kInstanceExtensionProperties[] = {
    VkExtensionProperties{VK_EXT_LAYER_SETTINGS_EXTENSION_NAME, VK_EXT_LAYER_SETTINGS_SPEC_VERSION}};
const uint32_t kInstanceExtensionPropertiesCount = static_cast<uint32_t>(std::size(kInstanceExtensionProperties));

template <typename T>
T* AllocateArray(VkAllocationCallbacks const& allocator, uint32_t count, VkSystemAllocationScope scope) {
    return static_cast<T*>(allocator.pfnAllocation(allocator.pUserData, sizeof(T) * count, alignof(T), scope));
}

static uint64_t ChecksumFletcher64(uint32_t const* data, size_t count) {
    constexpr uint32_t mod_value = 0xFFFFFFFF;
    uint64_t num1 = 0;
    uint64_t num2 = 0;

    for (size_t i = 0; i < count; ++i) {
        num1 = (num1 + data[i]) % mod_value;
        num2 = (num2 + num1) % mod_value;
    }

    return (num1 << 32) | num2;
}

#include "generated/shader_object_constants.h"
#include "generated/shader_object_entry_points_x_macros.inl"

// These LayerDispatch* structs hold pointers to the next layer's version of these functions so that we can call down the chain

struct LayerDispatchInstance {
#define ENTRY_POINT(name) PFN_vk##name name = nullptr;
#define ENTRY_POINT_ALIAS(alias, canon)
    ENTRY_POINTS_INSTANCE
    ADDITIONAL_INSTANCE_FUNCTIONS
    ENTRY_POINTS_DEVICE
    ADDITIONAL_DEVICE_FUNCTIONS
#undef ENTRY_POINT_ALIAS
#undef ENTRY_POINT

    void Initialize(VkInstance instance, PFN_vkGetInstanceProcAddr get_proc_addr) {
#define ENTRY_POINT_ALIAS(alias, canon) if (canon == nullptr) { canon = (PFN_vk##canon)get_proc_addr(instance, "vk" #alias); }
#define ENTRY_POINT(name) ENTRY_POINT_ALIAS(name, name)
        ENTRY_POINTS_INSTANCE
        ADDITIONAL_INSTANCE_FUNCTIONS
        ENTRY_POINTS_DEVICE
        ADDITIONAL_DEVICE_FUNCTIONS
#undef ENTRY_POINT
#undef ENTRY_POINT_ALIAS
    }
};

static ShaderType ShaderStageToShaderType(VkShaderStageFlagBits stage) {
   switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return VERTEX_SHADER;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return FRAGMENT_SHADER;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return TESSELLATION_CONTROL_SHADER;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return TESSELLATION_EVALUATION_SHADER;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return GEOMETRY_SHADER;
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return MESH_SHADER;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            return TASK_SHADER;
        default:
            ASSERT(false);
            return NUM_SHADERS;
    }
}

}  // namespace shader_object

#if defined(__GNUC__) && __GNUC__ >= 4
#define VEL_EXPORT __attribute__((visibility("default")))
#else
#define VEL_EXPORT
#endif

namespace shader_object {

struct LayerSettings {
    bool force_enable{false};
    bool disable_pipeline_pre_caching{false};
};

struct InstanceData {
    LayerDispatchInstance vtable;
    VkInstance            instance;
    VkPhysicalDevice*     physical_devices;
    uint32_t              physical_device_count;
    LayerSettings         layer_settings;
};

struct PhysicalDeviceData {
    InstanceData*            instance;
    uint32_t                 vendor_id;
    AdditionalExtensionFlags supported_additional_extensions;
};

static HashMap<VkInstance, InstanceData*>             instance_data_map;
static HashMap<VkDevice, DeviceData*>                 device_data_map;
static HashMap<VkPhysicalDevice, PhysicalDeviceData*> physical_device_data_map;
static HashMap<VkQueue, DeviceData*>                  queue_to_device_data_map;
static HashMap<VkCommandBuffer, VkCommandPool>        command_buffer_to_pool_map;

// Used if the device does not support private data
static HashMap<VkDescriptorUpdateTemplate, VkPipelineBindPoint> descriptor_update_template_to_bind_point_map;

// Used if device does not support private data or if there's more than one device
static HashMap<VkCommandBuffer, CommandBufferData*> command_buffer_to_command_buffer_data;

// Keeps track of the first created device
static ReaderWriterContainer<DeviceData*> first_device_container = nullptr;

static bool     ContainsValidShaderBinary(DeviceData const& deviceData, VkShaderCreateInfoEXT const& createInfo);

static VkResult CreatePipelineLayoutForShader(DeviceData const& deviceData, VkAllocationCallbacks const& allocator, Shader* shader) {
    ASSERT(shader->pipeline_layout == VK_NULL_HANDLE);
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        shader->num_descriptor_set_layouts,
        shader->descriptor_set_layouts,
        shader->num_push_constant_ranges,
        shader->push_constant_ranges
    };
    return deviceData.vtable.CreatePipelineLayout(deviceData.device, &pipeline_layout_create_info, &allocator, &shader->pipeline_layout);
}

ComparableShader::ComparableShader(Shader *shader) : shader_(shader), id_(shader ? shader->id : 0) {}

void DeviceData::AddDynamicState(VkDynamicState state) {
    ASSERT(dynamic_state_count < kMaxDynamicStates);
    dynamic_states[dynamic_state_count] = state;
    ++dynamic_state_count;
}

bool DeviceData::HasDynamicState(VkDynamicState state) const {
    for (uint32_t i = 0; i < dynamic_state_count; ++i) {
        if (dynamic_states[i] == state) {
            return true;
        }
    }
    return false;
}

void CommandBufferData::ReserveMemory(AlignedMemory& aligned_memory, VkPhysicalDeviceProperties const& properties) {
    aligned_memory.Add<CommandBufferData>();
    FullDrawStateData::ReserveMemory(aligned_memory, properties);
}

CommandBufferData* CommandBufferData::Create(DeviceData* data, VkAllocationCallbacks allocator) {
    AlignedMemory aligned_memory;
    ReserveMemory(aligned_memory, data->properties);

    aligned_memory.Allocate(allocator, VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (!aligned_memory) {
        return nullptr;
    }

    memset(aligned_memory.GetMemoryWritePtr(), 0, aligned_memory.GetSize());

    auto cmd_data = aligned_memory.GetNextAlignedPtr<CommandBufferData>();
    cmd_data->device_data      = data;
    cmd_data->allocator        = allocator;
    cmd_data->draw_state_data_ = aligned_memory.GetNextAlignedPtr<FullDrawStateData>();
    FullDrawStateData::InitializeMemory(cmd_data->draw_state_data_, data->properties, cmd_data->device_data->enabled_extensions & DYNAMIC_RENDERING_UNUSED_ATTACHMENTS);
    return cmd_data;
}

void CommandBufferData::Destroy(CommandBufferData** data) {
    ASSERT(data);
    VkAllocationCallbacks allocator = (*data)->allocator;
    allocator.pfnFree(allocator.pUserData, *data);
    *data = nullptr;
}

VkResult Shader::Create(DeviceData const& deviceData, VkShaderCreateInfoEXT const& createInfo, VkAllocationCallbacks const& allocator,
                        Shader** ppOutShader) {
    auto& vtable = deviceData.vtable;

    // Get SPIR-V information
    size_t spirv_size;
    void const* spirv_data;
    if (createInfo.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
        // SPIR-V is stored directly in the create info
        spirv_size = createInfo.codeSize;
        spirv_data = createInfo.pCode;
    } else if (createInfo.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT && ContainsValidShaderBinary(deviceData, createInfo)) {
        // SPIR-V is stored in the shader binary
        auto shader_binary = static_cast<ShaderBinary const*>(createInfo.pCode);
        spirv_size = shader_binary->spirv_data_size;
        spirv_data = shader_binary->GetSprivData();
    } else {
        return VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT;
    }

    size_t name_size = createInfo.pName == nullptr ? 0 : strlen(createInfo.pName) + 1;

    AlignedMemory aligned_memory;
    aligned_memory.Add<Shader>();
    aligned_memory.Add<char>(name_size);
    aligned_memory.Add<uint32_t>(spirv_size / sizeof(uint32_t));
    aligned_memory.Add<VkPushConstantRange>(createInfo.pushConstantRangeCount);
    aligned_memory.Add<VkDescriptorSetLayout>(createInfo.setLayoutCount);
    aligned_memory.Add<PrivateDataSlotPair>(deviceData.reserved_private_data_slot_count);
    if (createInfo.pSpecializationInfo) {
        aligned_memory.Add<uint32_t>(createInfo.pSpecializationInfo->dataSize / sizeof(uint32_t));
        aligned_memory.Add<VkSpecializationMapEntry>(createInfo.pSpecializationInfo->mapEntryCount);
    }

    aligned_memory.Allocate(allocator, VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (!aligned_memory) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    Shader* shader = new (aligned_memory.GetNextAlignedPtr<Shader>()) Shader();

    static std::atomic<uint64_t> id_counter{1};
    shader->id = id_counter.fetch_add(1, std::memory_order_relaxed);

    *ppOutShader = shader;
    shader->stage = createInfo.stage;
    if (createInfo.flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) {
        shader->flags |= VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;
    }
    if (createInfo.flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) {
        shader->flags |= VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
    }

    // Copy data over from create info struct

    aligned_memory.CopyBytes<char>(shader->name, shader->name_byte_count, createInfo.pName, name_size);
    
    aligned_memory.CopyBytes<uint32_t>(shader->spirv_data, shader->spirv_data_size, spirv_data, spirv_size);

    aligned_memory.CopyStruct(shader->push_constant_ranges, shader->num_push_constant_ranges, createInfo.pPushConstantRanges,
                              createInfo.pushConstantRangeCount);

    aligned_memory.CopyStruct(shader->descriptor_set_layouts, shader->num_descriptor_set_layouts, createInfo.pSetLayouts,
                              createInfo.setLayoutCount);

    shader->reserved_private_data_slots = aligned_memory.GetNextAlignedPtr<PrivateDataSlotPair>(deviceData.reserved_private_data_slot_count);

    if (createInfo.pSpecializationInfo) {
        shader->specialization_info = *createInfo.pSpecializationInfo;
        shader->specialization_info_ptr = &shader->specialization_info;

        aligned_memory.CopyBytes<uint32_t>(shader->specialization_info.pData, shader->specialization_info.dataSize,
                                 createInfo.pSpecializationInfo->pData, createInfo.pSpecializationInfo->dataSize);

        aligned_memory.CopyStruct(shader->specialization_info.pMapEntries, shader->specialization_info.mapEntryCount,
                                  createInfo.pSpecializationInfo->pMapEntries, createInfo.pSpecializationInfo->mapEntryCount);
    }

    // Create shader module from SPIR-V

    VkShaderModuleCreateInfo module_create_info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        spirv_size,
        static_cast<uint32_t const*>(spirv_data)
    };

    VkResult result = vtable.CreateShaderModule(deviceData.device, &module_create_info, &allocator, &shader->shader_module);
    if (result != VK_SUCCESS) {
        return result;
    }

    // Create pipeline caches for vertex/mesh (which are always present in a pipeline) and fragment (which is always present in a fragment shader pipeline library)
    if (createInfo.stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT)) {
        VkPipelineCacheCreateInfo cache_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO
        };

        // ShaderBinary may hold existing pipeline cache data
        if (createInfo.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
            auto shader_binary = static_cast<ShaderBinary const*>(createInfo.pCode);
            if (shader_binary->flags & ShaderBinary::HAS_PIPELINE_CACHE) {
                cache_create_info.initialDataSize = shader_binary->pipeline_cache_size;
                cache_create_info.pInitialData    = shader_binary->GetPipelineCacheData();
            }
        }

        // Shader has two caches:
        // `pristine_cache` is used for shader create time and is for serializing to/from the ShaderBinary
        // `cache` is used for pipeline creation at command buffer record time. Initially, it is equal to `pristine_cache`.

        result = vtable.CreatePipelineCache(deviceData.device, &cache_create_info, nullptr, &shader->pristine_cache);
        if (result != VK_SUCCESS) {
            return result;
        }

        result = vtable.CreatePipelineCache(deviceData.device, &cache_create_info, nullptr, &shader->cache);
        if (result != VK_SUCCESS) {
            return result;
        }
    } else if (createInfo.stage & VK_SHADER_STAGE_COMPUTE_BIT) {
        // Compute shaders do not require any additional data for a pipeline to be created

        result = CreatePipelineLayoutForShader(deviceData, allocator, shader);
        if (result != VK_SUCCESS) {
            return result;
        }

        VkPipelineShaderStageCreateInfo compute_stage{
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            shader->flags,
            shader->stage,
            shader->shader_module,
            shader->name,
            shader->specialization_info_ptr
        };
        VkComputePipelineCreateInfo compute_pipeline_create_info{
            VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            nullptr,
            0,
            compute_stage,
            shader->pipeline_layout
        };
        if (createInfo.flags & VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT) {
            compute_pipeline_create_info.flags |= VK_PIPELINE_CREATE_DISPATCH_BASE;
        }
        result = vtable.CreateComputePipelines(deviceData.device, shader->cache, 1, &compute_pipeline_create_info, &allocator, &shader->partial_pipeline.pipeline);
    }

    return result;
}

void Shader::Destroy(DeviceData const& device_data, Shader* pShader, VkAllocationCallbacks const& allocator) {
    if (pShader == nullptr) {
        return;
    }

    auto  device = device_data.device;
    auto& vtable = device_data.vtable;

    if (pShader->shader_module != VK_NULL_HANDLE) {
        vtable.DestroyShaderModule(device, pShader->shader_module, &allocator);
    }
    if (pShader->pristine_cache != VK_NULL_HANDLE) {
        vtable.DestroyPipelineCache(device, pShader->pristine_cache, nullptr);
    }
    if (pShader->cache != VK_NULL_HANDLE) {
        vtable.DestroyPipelineCache(device, pShader->cache, nullptr);
    }
    if (pShader->pipeline_layout != VK_NULL_HANDLE) {
        vtable.DestroyPipelineLayout(device, pShader->pipeline_layout, &allocator);
    }
    if (pShader->partial_pipeline.pipeline != VK_NULL_HANDLE) {
        vtable.DestroyPipeline(device, pShader->partial_pipeline.pipeline, &allocator);
        if (pShader->partial_pipeline.draw_state) {
            FullDrawStateData::Destroy(pShader->partial_pipeline.draw_state);
        }
    }

    auto& pipelines = pShader->pipelines.GetDataUnsafe();
    for (auto const& pair : pipelines) {
        vtable.DestroyPipeline(device, pair.value, nullptr);
    }
    pipelines.Clear();
    pShader->private_data.Clear();
    pShader->~Shader();

    allocator.pfnFree(allocator.pUserData, pShader);
}

uint64_t Shader::GetPrivateData(DeviceData const& device_data, VkPrivateDataSlot slot) {
    // first, search through the reserved slots
    for (uint32_t i = 0; i < device_data.reserved_private_data_slot_count; ++i) {
        if (reserved_private_data_slots[i].slot == slot) {
            return reserved_private_data_slots[i].data;
        }
    }

    // then, look up in the map
    const uint64_t* found = private_data.GetOrNullptr(slot);
    if (found) {
        return *found;
    }

    // otherwise, return default value
    return 0;
}

void Shader::SetPrivateData(DeviceData const& device_data, VkPrivateDataSlot slot, uint64_t data) {
    // first, search through the reserved slots
    for (uint32_t i = 0; i < device_data.reserved_private_data_slot_count; ++i) {
        if (reserved_private_data_slots[i].slot == VK_NULL_HANDLE || reserved_private_data_slots[i].slot == slot) {
            reserved_private_data_slots[i] = {slot, data};
            return;
        }
    }

    // otherwise, set it in the map
    private_data.Add(slot, data);
}

static VkResult CalculateBinarySizeForShader(DeviceData const& deviceData, Shader const& shader, size_t* out_binary_size, size_t* out_pipeline_cache_size) {
    VkResult result = VK_SUCCESS;
    size_t pipeline_cache_size = 0;
    if (shader.pristine_cache != VK_NULL_HANDLE) {
        result = deviceData.vtable.GetPipelineCacheData(deviceData.device, shader.pristine_cache, &pipeline_cache_size, nullptr);
    }
    if (out_pipeline_cache_size) {
        *out_pipeline_cache_size = pipeline_cache_size;
    }
    if (out_binary_size) {
        *out_binary_size = sizeof(ShaderBinary) + shader.spirv_data_size + pipeline_cache_size;
    }
    return result;
}

static uint64_t CalculateSpirvChecksum(void const* spirv_data, size_t spirv_data_size) {
    ASSERT(spirv_data_size % sizeof(uint32_t) == 0);
    return ChecksumFletcher64(static_cast<uint32_t const*>(spirv_data), spirv_data_size / sizeof(uint32_t));
}

VkResult ShaderBinary::Create(DeviceData const& deviceData, Shader const& shader, void* out) {
    auto& vtable = deviceData.vtable;
    auto  binary = static_cast<ShaderBinary*>(out);

    binary->magic           = kMagic;
    binary->version         = SHADER_OBJECT_BINARY_VERSION;
    binary->stage           = shader.stage;
    binary->spirv_data_size = shader.spirv_data_size;
    binary->spirv_checksum  = CalculateSpirvChecksum(shader.spirv_data, shader.spirv_data_size);
    memcpy(binary->GetSprivData(), shader.spirv_data, shader.spirv_data_size);

    if (shader.pristine_cache != VK_NULL_HANDLE) {
        binary->flags = ShaderBinary::HAS_PIPELINE_CACHE;
        VkResult result = vtable.GetPipelineCacheData(deviceData.device, shader.pristine_cache, &binary->pipeline_cache_size, nullptr);
        if (result != VK_SUCCESS) {
            return result;
        }
        return vtable.GetPipelineCacheData(deviceData.device, shader.pristine_cache, &binary->pipeline_cache_size, binary->GetPipelineCacheData());
    } else {
        binary->flags = 0;
        binary->pipeline_cache_size = 0;
    }
    return VK_SUCCESS;
}

static bool ContainsValidShaderBinary(DeviceData const& deviceData, VkShaderCreateInfoEXT const& createInfo) {
    if (createInfo.codeSize < sizeof(ShaderBinary)) {
        return false;
    }

    auto shader_binary = static_cast<ShaderBinary const*>(createInfo.pCode);
    if (shader_binary->magic != ShaderBinary::kMagic) {
        return false;
    }
    if (shader_binary->version != SHADER_OBJECT_BINARY_VERSION) {
        return false;
    }
    if (shader_binary->spirv_data_size == 0) {
        return false;
    }
    if (shader_binary->stage != createInfo.stage) {
        return false;
    }
    if (shader_binary->spirv_checksum != CalculateSpirvChecksum(shader_binary->GetSprivData(), shader_binary->spirv_data_size)) {
        return false;
    }
    if (shader_binary->flags & ShaderBinary::HAS_PIPELINE_CACHE) {
        if (shader_binary->pipeline_cache_size < sizeof(VkPipelineCacheHeaderVersionOne)) {
            return false;
        }
        auto pipeline_cache_header = reinterpret_cast<VkPipelineCacheHeaderVersionOne const*>(shader_binary->GetPipelineCacheData());
        if (memcmp(pipeline_cache_header->pipelineCacheUUID, deviceData.properties.pipelineCacheUUID, VK_UUID_SIZE) != 0) {
            return false;
        }
    }
    return true;
}

static CommandBufferData* GetCommandBufferData(VkCommandBuffer cmd) {
    // Ideal scenario is CommandBufferData* is stored in the private data for the command buffer. However, this can only
    // happen if there's only one device and that device supports private data.

    // A potential performance improvement may be caching the previous VkCommandBuffer and CommandBufferData in thread-local storage

    std::shared_lock<std::shared_mutex> lock;
    auto first_device = first_device_container.GetDataForReading(lock);
    if (device_data_map.NumEntries() != 1 || first_device->private_data.privateData == VK_FALSE) {
        // >1 existing device => Don't know which device the command buffer belongs to. Need to search the map
        // No private data    => Need to search the map
        auto found = command_buffer_to_command_buffer_data.GetOrNullptr(cmd);
        if (found) {
            return *found;
        }
    }

    // Command buffer belongs to the first device
    uint64_t cmd_data;
    first_device->vtable.GetPrivateData(first_device->device, VK_OBJECT_TYPE_COMMAND_BUFFER, reinterpret_cast<uint64_t>(cmd), first_device->private_data_slot, &cmd_data);
    return reinterpret_cast<CommandBufferData*>(cmd_data);
}

static VkPipelineBindPoint GetDescriptorUpdateTemplateBindPoint(DeviceData* deviceData, VkDescriptorUpdateTemplate descriptorUpdateTemplate) {
    auto& vtable = deviceData->vtable;
    if (deviceData->private_data.privateData == VK_TRUE) {
        uint64_t data;
        vtable.GetPrivateData(deviceData->device, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE, (uint64_t)(descriptorUpdateTemplate), deviceData->private_data_slot, &data);
        return static_cast<VkPipelineBindPoint>(data);
    }
    return descriptor_update_template_to_bind_point_map.Get(descriptorUpdateTemplate);
}

static void SetDescriptorUpdateTemplateBindPoint(DeviceData* deviceData, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineBindPoint bindPoint) {
    auto& vtable = deviceData->vtable;
    if (deviceData->private_data.privateData == VK_TRUE) {
        vtable.SetPrivateData(deviceData->device, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE, (uint64_t)(descriptorUpdateTemplate), deviceData->private_data_slot, static_cast<uint64_t>(bindPoint));
        return;
    }
    descriptor_update_template_to_bind_point_map.Add(descriptorUpdateTemplate, bindPoint);
}

static void RemoveDescriptorUpdateTemplateBindPoint(DeviceData* deviceData, VkDescriptorUpdateTemplate descriptorUpdateTemplate) {
    if (deviceData->private_data.privateData == VK_TRUE) {
        return;
    }
    descriptor_update_template_to_bind_point_map.Remove(descriptorUpdateTemplate);
}

static void SetCommandBufferDataForCommandBuffer(DeviceData* device_data, VkCommandBuffer cmd, CommandBufferData* cmd_data) {
    std::shared_lock<std::shared_mutex> lock;
    DeviceData* first_device = first_device_container.GetDataForReading(lock);

    if (device_data == first_device && device_data->private_data.privateData == VK_TRUE) {
        VkResult result = device_data->vtable.SetPrivateData(device_data->device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)cmd,
                                                             device_data->private_data_slot, (uint64_t)cmd_data);
        ASSERT(result == VK_SUCCESS);
        UNUSED(result);
    } else {
        command_buffer_to_command_buffer_data.Add(cmd, cmd_data);
    }
}

static void RemoveCommandBufferDataForCommandBuffer(DeviceData* device_data, VkCommandBuffer cmd) {
    std::shared_lock<std::shared_mutex> lock;
    DeviceData* first_device = first_device_container.GetDataForReading(lock);

    auto cmd_data = GetCommandBufferData(cmd);
    CommandBufferData::Destroy(&cmd_data);
    if (device_data != first_device) {
        command_buffer_to_command_buffer_data.Remove(cmd);
    }
}

static const char* GetShaderName(uint32_t shader_type) {
    switch (shader_type) {
        case VERTEX_SHADER:
            return "V";
        case FRAGMENT_SHADER:
            return "F";
        case TESSELLATION_CONTROL_SHADER:
            return "TC";
        case TESSELLATION_EVALUATION_SHADER:
            return "TE";
        case GEOMETRY_SHADER:
            return "G";
        case MESH_SHADER:
            return "M";
        case TASK_SHADER:
            return "T";
        default:
            break;
    }
    return "";
}

static void SetComputeShaderDebugUtilsName(DeviceData& data, Shader* shader, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    VkDebugUtilsObjectNameInfoEXT name_info{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_PIPELINE,
        (uint64_t)shader->partial_pipeline.pipeline,
        pNameInfo->pObjectName
    };
    data.vtable.SetDebugUtilsObjectNameEXT(data.device, &name_info);
}

static void SetComputeShaderDebugUtilsTag(DeviceData& data, Shader* shader, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    VkDebugUtilsObjectTagInfoEXT tag_info{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_PIPELINE,
        (uint64_t)shader->partial_pipeline.pipeline,
        pTagInfo->tagName,
        pTagInfo->tagSize,
        pTagInfo->pTag
    };
    data.vtable.SetDebugUtilsObjectTagEXT(data.device, &tag_info);
}

static void SetDebugUtilsNameAndTag(CommandBufferData& cmd_data, VkPipeline pipeline) {
    auto& device_data = *cmd_data.device_data;
    auto const state  = cmd_data.GetDrawStateData();

    if (device_data.debug_utils_object_name_map.NumEntries() > 0) {
        bool first_shader = true;
        bool same_name = true;
        char same_shader_name[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH];
        char pipeline_name[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH* 5 + 26];
        pipeline_name[0] = '\0';
        char temp[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH + 5];

        for (uint32_t shader_type = 0; shader_type < NUM_SHADERS; ++shader_type) {
            Shader* shader = state->GetComparableShader(shader_type).GetShaderPtr();

            if (shader == nullptr) {
                continue;
            }

            auto iter = device_data.debug_utils_object_name_map.Find(shader);
            if (iter != device_data.debug_utils_object_name_map.end()) {
                const auto& shader_name = iter.GetValue().name;
                if (first_shader) {
                    first_shader = false;
                    strncpy(same_shader_name, shader_name, sizeof(same_shader_name));
                } else if (strncmp(shader_name, same_shader_name, sizeof(shader_name)) == 0) {
                    same_name = false;
                    strncat(pipeline_name, "/", sizeof(pipeline_name) - strlen(pipeline_name) - 1);
                }

                bool has_space_or_slash = strchr(shader_name, ' ') != NULL || strchr(shader_name, '/') != NULL;
                if (has_space_or_slash) {
                    snprintf(temp, sizeof(temp), "%s:\"%s\"", GetShaderName(shader_type), shader_name);
                } else {
                    snprintf(temp, sizeof(temp), "%s:%s", GetShaderName(shader_type), shader_name);
                }
                strncat(pipeline_name, temp, sizeof(pipeline_name) - strlen(pipeline_name) - 1);
                pipeline_name[sizeof(pipeline_name) - 1] = '\0';
            }
        }

        if (!first_shader) {
            VkDebugUtilsObjectNameInfoEXT name_info{
                VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                nullptr,
                VK_OBJECT_TYPE_PIPELINE,
                (uint64_t)pipeline,
                same_name ? same_shader_name : pipeline_name
            };
            device_data.vtable.SetDebugUtilsObjectNameEXT(device_data.device, &name_info);
        }
    }
    if (device_data.debug_utils_object_tag_map.NumEntries() > 0) {
        bool first_shader = true;
        bool same_tag = true;
        char same_shader_tag[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH];
        char pipeline_tag[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH * 5 + 26];
        pipeline_tag[0] = '\0';
        char temp[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH + 5];
        uint64_t tagName = 0;

        for (uint32_t shader_type = 0; shader_type < NUM_SHADERS; ++shader_type) {
            Shader* shader = state->GetComparableShader(shader_type).GetShaderPtr();

            if (shader == nullptr) {
                continue;
            }

            auto iter = device_data.debug_utils_object_tag_map.Find(shader);
            if (iter != device_data.debug_utils_object_tag_map.end()) {
                const auto& shader_tag = iter.GetValue().tag;
                if (first_shader) {
                    first_shader = false;
                    strncpy(same_shader_tag, shader_tag, sizeof(same_shader_tag));
                } else if (strncmp(shader_tag, same_shader_tag, sizeof(same_shader_tag)) != 0) {
                    same_tag = false;
                    strncat(pipeline_tag, "/", sizeof(pipeline_tag) - strlen(pipeline_tag) - 1);
                }

                if (shader_type == VERTEX_SHADER || shader_type == MESH_SHADER) {
                    tagName = iter.GetValue().tagName;
                }

                bool has_space_or_slash = strchr(shader_tag, ' ') != NULL || strchr(shader_tag, '/') != NULL;
                if (has_space_or_slash) {
                    snprintf(temp, sizeof(temp), "%s:\"%s\"", GetShaderName(shader_type), shader_tag);
                } else {
                    snprintf(temp, sizeof(temp), "%s:%s", GetShaderName(shader_type), shader_tag);
                }
                strncat(pipeline_tag, temp, sizeof(pipeline_tag) - strlen(pipeline_tag) - 1);
                pipeline_tag[sizeof(pipeline_tag) - 1] = '\0';
            }
        }
        if (!first_shader) {
            VkDebugUtilsObjectTagInfoEXT tag_info{
                VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT,
                nullptr,
                VK_OBJECT_TYPE_PIPELINE,
                (uint64_t)pipeline,
                tagName,
                same_tag ? strlen(same_shader_tag) : strlen(pipeline_tag),
                same_tag ? same_shader_tag : pipeline_tag
            };
            device_data.vtable.SetDebugUtilsObjectTagEXT(device_data.device, &tag_info);
        }
    }
}

static VkPipeline CreateGraphicsPipelineForCommandBufferState(CommandBufferData& cmd_data) {
    auto& device_data = *cmd_data.device_data;
    auto const state  = cmd_data.GetDrawStateData();

    // gather shaders
    uint32_t num_stages = 0;
    VkPipelineShaderStageCreateInfo stages[NUM_SHADERS] = {};
    Shader* vertex_or_mesh_shader = nullptr;

    VkShaderStageFlags present_stages = 0;

    for (uint32_t shader_type = 0; shader_type < NUM_SHADERS; ++shader_type) {
        Shader* shader = state->GetComparableShader(shader_type).GetShaderPtr();

        if (shader == nullptr) {
            continue;
        }

        present_stages |= shader->stage;

        VkPipelineShaderStageCreateInfo stage = {};
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.flags = shader->flags;
        stage.module = shader->shader_module;
        stage.pName = shader->name;
        stage.stage = shader->stage;
        stage.pSpecializationInfo = shader->specialization_info_ptr;

        switch (shader_type) {
            case VERTEX_SHADER:
            case MESH_SHADER:
                vertex_or_mesh_shader = shader;
                break;
            default:
                break;
        }

        stages[num_stages] = stage;
        ++num_stages;
    }

    // vertex or mesh shader is required
    ASSERT(vertex_or_mesh_shader != nullptr);

    VkPipelineLayout pipeline_layout = vertex_or_mesh_shader->pipeline_layout;
    if (pipeline_layout == VK_NULL_HANDLE) {
        pipeline_layout = cmd_data.last_seen_pipeline_layout_;
    }
    if (pipeline_layout == VK_NULL_HANDLE) {
        pipeline_layout = cmd_data.device_data->dummy_pipeline_layout;
    }

    const uint32_t num_color_attachments = (device_data.enabled_extensions & DYNAMIC_RENDERING_UNUSED_ATTACHMENTS) ? device_data.properties.limits.maxColorAttachments : state->GetNumColorAttachments();

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = cmd_data.device_data->HasDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ? 0u : state->GetNumViewports();
    viewport_state.scissorCount = cmd_data.device_data->HasDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) ? 0u : state->GetNumScissors();
    VkPipelineViewportDepthClipControlCreateInfoEXT depth_clip_control_state{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT,
        nullptr,
        state->GetNegativeOneToOne()
    };
    VkPipelineViewportWScalingStateCreateInfoNV viewport_w_scaling_state{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV,
        nullptr,
        state->GetViewportWScalingEnable()
    };
    VkPipelineViewportSwizzleStateCreateInfoNV viewport_swizzle_state{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV,
        nullptr,
        0,
        state->GetViewportSwizzleCount(),
        state->GetViewportSwizzlePtr()
    };
    VkPipelineViewportShadingRateImageStateCreateInfoNV viewport_shading_rate{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV,
        nullptr,
        state->GetShadingRateImageEnable()
    };
    auto viewport_chain = reinterpret_cast<VkBaseOutStructure*>(&viewport_state);
    if (device_data.enabled_extensions & DEPTH_CLIP_CONTROL) {
        viewport_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&depth_clip_control_state);
        viewport_chain = viewport_chain->pNext;
    }
    if (device_data.enabled_extensions & NV_CLIP_SPACE_W_SCALING) {
        viewport_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&viewport_w_scaling_state);
        viewport_chain = viewport_chain->pNext;
    }
    if (device_data.enabled_extensions & NV_VIEWPORT_SWIZZLE) {
        viewport_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&viewport_swizzle_state);
        viewport_chain = viewport_chain->pNext;
    }
    if (device_data.enabled_extensions & NV_SHADING_RATE_IMAGE) {
        viewport_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&viewport_shading_rate);
        viewport_chain = viewport_chain->pNext;
    }

    // Set pointers to NULL when count is 0, workaround for a driver bug
    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = state->GetNumVertexInputBindingDescriptions();
    vertex_input.pVertexBindingDescriptions = vertex_input.vertexBindingDescriptionCount > 0 ? state->GetVertexInputBindingDescriptionPtr() : nullptr;
    vertex_input.vertexAttributeDescriptionCount = state->GetNumVertexInputAttributeDescriptions();
    vertex_input.pVertexAttributeDescriptions = vertex_input.vertexAttributeDescriptionCount > 0 ? state->GetVertexInputAttributeDescriptionPtr() : nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = state->GetPrimitiveTopology();
    input_assembly.primitiveRestartEnable = state->GetPrimitiveRestartEnable();

    VkPipelineTessellationDomainOriginStateCreateInfo domain_origin_state{};
    domain_origin_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
    domain_origin_state.domainOrigin = state->GetDomainOrigin();

    VkPipelineTessellationStateCreateInfo tessellation_state{};
    tessellation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellation_state.pNext = &domain_origin_state;
    tessellation_state.patchControlPoints = state->GetPatchControlPoints();

    VkPipelineRasterizationStateCreateInfo rasterization_state{};
    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.polygonMode = state->GetPolygonMode();
    rasterization_state.cullMode = state->GetCullMode();
    rasterization_state.frontFace = state->GetFrontFace();
    rasterization_state.rasterizerDiscardEnable = state->GetRasterizerDiscardEnable();
    rasterization_state.depthBiasEnable = state->GetDepthBiasEnable();
    rasterization_state.depthClampEnable = state->GetDepthClampEnable();
    rasterization_state.lineWidth = 1.0f;
    VkPipelineRasterizationStateStreamCreateInfoEXT rasterization_stream_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT,
        nullptr,
        0,
        state->GetRasterizationStream()
    };
    VkPipelineRasterizationConservativeStateCreateInfoEXT rasterization_conservative_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT,
        nullptr,
        0,
        state->GetConservativeRasterizationMode(),
        state->GetExtraPrimitiveOverestimationSize()
    };
    VkPipelineRasterizationDepthClipStateCreateInfoEXT rasterization_depth_clip_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT,
        nullptr,
        0,
        state->GetDepthClipEnable()
    };
    VkPipelineRasterizationProvokingVertexStateCreateInfoEXT provoking_vertex_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT,
        nullptr,
        state->GetProvokingVertexMode()
    };
    VkPipelineRasterizationLineStateCreateInfoEXT line_rasterization_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT,
        nullptr,
        state->GetLineRasterizationMode(),
        state->GetStippledLineEnable()
    };
    auto rasterization_chain = reinterpret_cast<VkBaseOutStructure*>(&rasterization_state);
    if (device_data.transform_feedback.transformFeedback == VK_TRUE) {
        rasterization_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&rasterization_stream_state);
        rasterization_chain = rasterization_chain->pNext;
    }
    if (device_data.enabled_extensions & CONSERVATIVE_RASTERIZATION) {
        rasterization_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&rasterization_conservative_state);
        rasterization_chain = rasterization_chain->pNext;
    }
    if (device_data.enabled_extensions & DEPTH_CLIP_ENABLE) {
        rasterization_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&rasterization_depth_clip_state);
        rasterization_chain = rasterization_chain->pNext;
    }
    if (device_data.enabled_extensions & PROVOKING_VERTEX) {
        rasterization_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&provoking_vertex_state);
        rasterization_chain = rasterization_chain->pNext;
    }
    if (device_data.enabled_extensions & LINE_RASTERIZATION) {
        rasterization_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&line_rasterization_state);
        rasterization_chain = rasterization_chain->pNext;
    }

    VkPipelineMultisampleStateCreateInfo multisample_state{};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.rasterizationSamples = state->GetRasterizationSamples();
    multisample_state.alphaToOneEnable = state->GetAlphaToOneEnable();
    multisample_state.alphaToCoverageEnable = state->GetAlphaToCoverageEnable();
    multisample_state.pSampleMask = state->GetSampleMaskPtr();
    VkPipelineSampleLocationsStateCreateInfoEXT sample_location_state{
        VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT,
        nullptr,
        state->GetSampleLocationsEnable(),
        {
            VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT
        }
    };
    VkPipelineCoverageModulationStateCreateInfoNV coverage_modulation_state{
        VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV,
        nullptr,
        0,
        state->GetCoverageModulationMode(),
        state->GetCoverageModulationTableEnable(),
        state->GetCoverageModulationTableCount(),
        state->GetCoverageModulationTableValuesPtr()
    };
    VkPipelineCoverageReductionStateCreateInfoNV coverage_reduction_state{
        VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV,
        nullptr,
        0,
        state->GetCoverageReductionMode()
    };
    VkPipelineCoverageToColorStateCreateInfoNV coverage_to_color_state{
        VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV,
        nullptr,
        0,
        state->GetCoverageToColorEnable(),
        state->GetCoverageToColorLocation()
    };
    auto multisample_chain = reinterpret_cast<VkBaseOutStructure*>(&multisample_state);
    if (device_data.enabled_extensions & SAMPLE_LOCATIONS) {
        multisample_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&sample_location_state);
        multisample_chain = multisample_chain->pNext;
    }
    if (device_data.enabled_extensions & NV_FRAMEBUFFER_MIXED_SAMPLES) {
        multisample_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&coverage_modulation_state);
        multisample_chain = multisample_chain->pNext;
    }
    if (device_data.enabled_extensions & NV_COVERAGE_REDUCTION_MODE) {
        multisample_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&coverage_reduction_state);
        multisample_chain = multisample_chain->pNext;
    }
    if (device_data.enabled_extensions & NV_FRAGMENT_COVERAGE_TO_COLOR) {
        multisample_chain->pNext = reinterpret_cast<VkBaseOutStructure*>(&coverage_to_color_state);
        multisample_chain = multisample_chain->pNext;
    }

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};
    depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.depthTestEnable = state->GetDepthTestEnable();
    depth_stencil_state.depthCompareOp = state->GetDepthCompareOp();
    depth_stencil_state.depthWriteEnable = state->GetDepthWriteEnable();
    depth_stencil_state.depthBoundsTestEnable = state->GetDepthBoundsTestEnable();
    depth_stencil_state.stencilTestEnable = state->GetStencilTestEnable();
    depth_stencil_state.front = state->GetStencilFront();
    depth_stencil_state.back = state->GetStencilBack();
    depth_stencil_state.maxDepthBounds = 1.0f;

    VkPipelineColorBlendStateCreateInfo color_blend_state{};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.logicOpEnable = state->GetLogicOpEnable();
    color_blend_state.logicOp = state->GetLogicOp();
    color_blend_state.attachmentCount = num_color_attachments;
    color_blend_state.pAttachments = state->GetColorBlendAttachmentStatePtr();

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = device_data.dynamic_state_count;
    dynamic_state.pDynamicStates = device_data.dynamic_states;

    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // create_info.flags;
    create_info.stageCount = num_stages;
    create_info.pStages = stages;
    create_info.pVertexInputState = &vertex_input;
    create_info.pInputAssemblyState = &input_assembly;
    create_info.pTessellationState = &tessellation_state;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &rasterization_state;
    create_info.pMultisampleState = &multisample_state;
    create_info.pDepthStencilState = &depth_stencil_state;
    create_info.pColorBlendState = &color_blend_state;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = pipeline_layout;
    create_info.renderPass = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;
    create_info.basePipelineHandle = VK_NULL_HANDLE;

    VkBaseOutStructure* prev_next = reinterpret_cast<VkBaseOutStructure*>(&create_info);
    auto const append_to_chain = [&prev_next](auto structure) {
        prev_next->pNext = reinterpret_cast<VkBaseOutStructure*>(structure);
        prev_next = prev_next->pNext;
    };

    if (device_data.enabled_extensions & DYNAMIC_RENDERING_UNUSED_ATTACHMENTS) {
        VkFormat lastFormat = VK_FORMAT_R8G8B8A8_UNORM;
        for (uint32_t i = 0; i < num_color_attachments; ++i) {
            VkFormat format = state->GetColorAttachmentFormatPtr()[i];
            if (format == VK_FORMAT_UNDEFINED) {
                state->SetColorAttachmentFormat(i, lastFormat);
            } else {
                lastFormat = format;
            }
        }
    }

    VkPipelineRenderingCreateInfo rendering_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        nullptr,
        0,
        num_color_attachments,
        state->GetColorAttachmentFormatPtr(),
        state->GetDepthAttachmentFormat(),
        state->GetStencilAttachmentFormat()
    };
    append_to_chain(&rendering_create_info);

    VkPipelineRepresentativeFragmentTestStateCreateInfoNV representative_fragment_test_state{
        VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV,
        nullptr,
        state->GetRepresentativeFragmentTestEnable()
    };
    if (device_data.enabled_extensions & NV_REPRESENTATIVE_FRAGMENT_TEST) {
        append_to_chain(&representative_fragment_test_state);
    }

    VkGraphicsPipelineLibraryCreateInfoEXT gpl_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT};
    VkPipelineLibraryCreateInfoKHR pl_create_info{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};

    // If we have graphics pipeline support, we might be able to use some precompiled pipelines
    VkPipeline libraries[2]{};
    if (device_data.enabled_extensions & GRAPHICS_PIPELINE_LIBRARY) {
        // describe full pipeline compilation, we'll remove flags for libraries that we have access to and can use
        gpl_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                                VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

        // Search through available precompiled pipelines
        for (uint32_t shader_type = 0; shader_type < NUM_SHADERS; ++shader_type) {
            Shader* shader = state->GetComparableShader(shader_type).GetShaderPtr();
            if (shader == nullptr) {
                continue;
            }

            // must have a partial pipeline to add
            if (shader->partial_pipeline.pipeline == VK_NULL_HANDLE) {
                continue;
            }

            // compiled pipeline must have been compiled with the same set of shaders
            if (shader->partial_pipeline.library_flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
                if ((shader->partial_pipeline.shader_stages & present_stages) != shader->partial_pipeline.shader_stages) {
                    continue;
                }
            } else if (shader->partial_pipeline.library_flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
                ASSERT(shader->stage == VK_SHADER_STAGE_FRAGMENT_BIT);
            } else {
                ASSERT(!"Unknown library flag");
            }

            // state that the partial pipeline was compiled with must match relevant part of current state
            if (!shader->partial_pipeline.draw_state->CompareStateSubset(*state, shader->partial_pipeline.library_flags)) {
                continue;
            }

            // Add the libraries which are valid/have matching state
            libraries[pl_create_info.libraryCount] = shader->partial_pipeline.pipeline;
            ++pl_create_info.libraryCount;
            ASSERT(pl_create_info.libraryCount <= GetArrayLength(libraries));

            // Remove this part from the pipeline compilation
            gpl_create_info.flags &= ~shader->partial_pipeline.library_flags;
        }

        if (pl_create_info.libraryCount > 0) {
            // use pipeline libraries if we found any we could use
            pl_create_info.pLibraries = libraries;

            // Strip stages from pipeline create info if we've already compiled the shaders
            if ((gpl_create_info.flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                          VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) == 0) {
                create_info.pStages = nullptr;
                create_info.stageCount = 0;
            }

            // add to pnext chain
            append_to_chain(&gpl_create_info);
            append_to_chain(&pl_create_info);
        }
    }

    VkPipeline pipeline;
    VkResult result = cmd_data.device_data->vtable.CreateGraphicsPipelines(
        cmd_data.device_data->device, vertex_or_mesh_shader->cache, 1, &create_info, nullptr, &pipeline);
    ASSERT(result == VK_SUCCESS);
    UNUSED(result);

    SetDebugUtilsNameAndTag(cmd_data, pipeline);

    return pipeline;
}

enum PipelineCreationFlagBits { INCLUDE_COLOR = 0x1, INCLUDE_DEPTH = 0x2 };
using PipelineCreationFlags = uint32_t;

void AddGraphicsPipelineToCache(DeviceData const& deviceData, VkAllocationCallbacks allocator, VkPipelineCache cache,
                                VkPipelineLayout layout, uint32_t stageCount, VkPipelineShaderStageCreateInfo stages[NUM_SHADERS],
                                PipelineCreationFlags options) {
    auto& vtable = deviceData.vtable;

    VkPrimitiveTopology primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    for (uint32_t i = 0; i < stageCount; ++i) {
        if (stages[i].stage & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
        }
    }

    VkFormat color_format         = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depth_stencil_format = (options & PipelineCreationFlagBits::INCLUDE_DEPTH) ? deviceData.supported_depth_stencil_format : VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo rendering_info{
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        nullptr,
        0,
        (options & PipelineCreationFlagBits::INCLUDE_COLOR) ? 1u : 0u,
        (options & PipelineCreationFlagBits::INCLUDE_COLOR) ? &color_format : nullptr,
        depth_stencil_format,
        depth_stencil_format,
    };

    VkPipelineColorBlendAttachmentState blend_state{
        VK_FALSE,
        VK_BLEND_FACTOR_SRC_COLOR,
        VK_BLEND_FACTOR_DST_COLOR,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_DST_ALPHA,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    VkPipelineInputAssemblyStateCreateInfo input_assembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
        nullptr,
        0, 
        primitive_topology
    };
    VkPipelineTessellationStateCreateInfo tessellation_state{
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        nullptr,
        0,
        1
    };
    VkViewport viewport = {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f};
    VkRect2D scissor    = {{0, 0}, {0, 0}};
    VkPipelineViewportStateCreateInfo viewport_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0u,
        deviceData.HasDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ? 0u : 1u,
        &viewport,
        deviceData.HasDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) ? 0u : 1u,
        &scissor
    };
    VkPipelineRasterizationStateCreateInfo rasterization_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0u,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_FRONT_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisample_state{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0u,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        0.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr,
        0u,
        VK_TRUE,
        VK_TRUE,
        VK_COMPARE_OP_LESS,
        VK_TRUE,
        VK_TRUE,
        {},
        {},
        0.0f,
        1.0f
    };
    VkPipelineColorBlendStateCreateInfo color_blend_state{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0u,
        VK_FALSE,
        VK_LOGIC_OP_CLEAR,
        rendering_info.colorAttachmentCount,
        &blend_state,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };
    VkPipelineDynamicStateCreateInfo dynamic_state{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr,
        0u,
        deviceData.dynamic_state_count,
        deviceData.dynamic_states
    };
    VkGraphicsPipelineCreateInfo create_info{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        &rendering_info,
        0u,
        stageCount,
        stages,
        &vertex_input_state,
        &input_assembly,
        &tessellation_state,
        &viewport_state,
        &rasterization_state,
        &multisample_state,
        &depth_stencil_state,
        &color_blend_state,
        &dynamic_state,
        layout,
        VK_NULL_HANDLE,
        0u,
        VK_NULL_HANDLE,
        -1
    };
 
    VkPipeline temp_pipeline;
    VkResult result = vtable.CreateGraphicsPipelines(deviceData.device, cache, 1, &create_info, nullptr, &temp_pipeline);
    if (result == VK_SUCCESS) {
        vtable.DestroyPipeline(deviceData.device, temp_pipeline, nullptr);
    } else {
        // If pipeline creation failed, silently fail in release
        ASSERT(!"Creation of pipeline to populate cache was not successful");
    }
}

PartialPipeline CreatePartiallyCompiledPipeline(DeviceData const& deviceData, VkAllocationCallbacks allocator, VkPipelineCache cache,
                                                VkPipelineLayout layout, VkGraphicsPipelineLibraryFlagBitsEXT pipelineLibraryFlags, 
                                                Shader** ppShaders, uint32_t shaderCount) {
    // Ensure that pipelineLibraryFlags doesn't have both VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT and VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT
    ASSERT((pipelineLibraryFlags &
           (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) !=
           (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT));

    VkPipelineShaderStageCreateInfo stages[NUM_SHADERS];
    VkGraphicsPipelineLibraryCreateInfoEXT gpl_create_info{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT,
        nullptr,
        static_cast<VkGraphicsPipelineLibraryFlagsEXT>(pipelineLibraryFlags)
    };
    VkShaderStageFlags shader_stage_flags{};
    for (uint32_t i = 0; i < shaderCount; ++i) {
        auto shader = ppShaders[i];
        stages[i] = {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            shader->flags,
            shader->stage,
            shader->shader_module,
            shader->name,
            shader->specialization_info_ptr
        };
        shader_stage_flags |= shader->stage;
        switch (shader->stage) {
            case VK_SHADER_STAGE_VERTEX_BIT:
            case VK_SHADER_STAGE_GEOMETRY_BIT:
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            case VK_SHADER_STAGE_TASK_BIT_EXT:
            case VK_SHADER_STAGE_MESH_BIT_EXT:
                ASSERT(pipelineLibraryFlags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT);
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                ASSERT(pipelineLibraryFlags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT);
                break;
            default:
                ASSERT(!"Unknown shader stage");
        }
    }
    PartialPipeline partial_pipeline{
        VK_NULL_HANDLE,
        FullDrawStateData::Create(deviceData.properties, allocator, deviceData.enabled_extensions & DYNAMIC_RENDERING_UNUSED_ATTACHMENTS),
        pipelineLibraryFlags,
        shader_stage_flags
    };

    auto viewport = VkViewport{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    auto scissor  = VkRect2D{{0, 0}, {1, 1}};
    VkPipelineViewportStateCreateInfo viewport_state{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, 
        nullptr, 
        0, 
        deviceData.HasDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ? 0u : 1u,
        &viewport, 
        deviceData.HasDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) ? 0u : 1u,
        &scissor
    };
    VkPipelineRasterizationStateCreateInfo rasterization_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_FRONT_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };
    VkPipelineTessellationStateCreateInfo tessellation_state{
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, 
        nullptr,
        0, 
        1
    };
    VkPipelineMultisampleStateCreateInfo multisample_state{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        0.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_TRUE,
        VK_TRUE,
        VK_COMPARE_OP_LESS,
        VK_TRUE,
        VK_TRUE,
        {},
        {},
        0.0f,
        1.0f
    };
    VkPipelineDynamicStateCreateInfo dynamic_state{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr,
        0,
        deviceData.dynamic_state_count,
        deviceData.dynamic_states
    };
    VkGraphicsPipelineCreateInfo create_info{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        &gpl_create_info,
        VK_PIPELINE_CREATE_LIBRARY_BIT_KHR,
        shaderCount,
        stages
    };
    for (uint32_t i = 0; i < shaderCount; ++i) {
        partial_pipeline.draw_state->SetComparableShader(ShaderStageToShaderType(ppShaders[i]->stage), ComparableShader(ppShaders[i]));
    }
    if (gpl_create_info.flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
        create_info.pViewportState = &viewport_state;
        partial_pipeline.draw_state->SetNumViewports(viewport_state.viewportCount);
        partial_pipeline.draw_state->SetNumScissors(viewport_state.scissorCount);

        create_info.pRasterizationState = &rasterization_state;
        partial_pipeline.draw_state->SetDepthClampEnable(rasterization_state.depthClampEnable);
        partial_pipeline.draw_state->SetPolygonMode(rasterization_state.polygonMode);
        partial_pipeline.draw_state->SetCullMode(rasterization_state.cullMode);
        partial_pipeline.draw_state->SetFrontFace(rasterization_state.frontFace);
        partial_pipeline.draw_state->SetDepthBiasEnable(rasterization_state.depthBiasEnable);

        create_info.pTessellationState = &tessellation_state;
        partial_pipeline.draw_state->SetPatchControlPoints(tessellation_state.patchControlPoints);
    }

    if (gpl_create_info.flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
        create_info.pMultisampleState = &multisample_state;
        partial_pipeline.draw_state->SetRasterizationSamples(multisample_state.rasterizationSamples);
        partial_pipeline.draw_state->SetAlphaToCoverageEnable(multisample_state.alphaToCoverageEnable);
        for (uint32_t i = 0; i < kMaxSampleMaskLength; ++i) {
            // Set sample mask to default value of all ones
            constexpr VkSampleMask all_ones = ~static_cast<VkSampleMask>(0);
            partial_pipeline.draw_state->SetSampleMask(i, all_ones);
        }
        multisample_state.pSampleMask = partial_pipeline.draw_state->GetSampleMaskPtr();
        partial_pipeline.draw_state->SetAlphaToOneEnable(multisample_state.alphaToOneEnable);

        create_info.pDepthStencilState = &depth_stencil_state;
        partial_pipeline.draw_state->SetDepthTestEnable(depth_stencil_state.depthTestEnable);
        partial_pipeline.draw_state->SetDepthWriteEnable(depth_stencil_state.depthWriteEnable);
        partial_pipeline.draw_state->SetDepthCompareOp(depth_stencil_state.depthCompareOp);
        partial_pipeline.draw_state->SetDepthBoundsTestEnable(depth_stencil_state.depthBoundsTestEnable);
        partial_pipeline.draw_state->SetStencilFront(depth_stencil_state.front);
        partial_pipeline.draw_state->SetStencilBack(depth_stencil_state.back);
    }
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = layout;
    create_info.basePipelineIndex = -1;

    VkResult result = deviceData.vtable.CreateGraphicsPipelines(deviceData.device, cache, 1, &create_info, &allocator, &partial_pipeline.pipeline);
    ASSERT(result == VK_SUCCESS);
    UNUSED(result);

    return partial_pipeline;
}

static VkResult PopulateCachesForShaders(DeviceData const& deviceData, VkAllocationCallbacks const& allocator, bool are_graphics_shaders_linked, uint32_t shaderCount, VkShaderEXT* pShaders) {
    if (deviceData.flags & DeviceData::DISABLE_PIPELINE_PRE_CACHING) {
        return VK_SUCCESS;
    }
    if (deviceData.graphics_pipeline_library.graphicsPipelineLibrary == VK_TRUE) {
        // Compile partial pipelines to fill cache and to keep around for first draw pipeline creation

        if (are_graphics_shaders_linked) {
            // Try to create a partial pipeline for all pre-rasterization stages and a partial pipeline for fragment shader

            uint32_t pre_rasterization_shader_count = 0;
            Shader* pre_rasterization_shaders[4]; // vertex + tesc + tese + geom or task + mesh
            Shader* vertex_or_mesh_shader = nullptr;
            Shader* fragment_shader = nullptr;
            for (uint32_t i = 0; i < shaderCount; ++i) {
                auto shader = *reinterpret_cast<Shader**>(&pShaders[i]);
                switch (shader->stage) {
                    case VK_SHADER_STAGE_VERTEX_BIT:
                    case VK_SHADER_STAGE_MESH_BIT_EXT:
                        vertex_or_mesh_shader = shader;
                        // fall-through
                    case VK_SHADER_STAGE_TASK_BIT_EXT:
                    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                    case VK_SHADER_STAGE_GEOMETRY_BIT:
                        pre_rasterization_shaders[pre_rasterization_shader_count] = shader;
                        ++pre_rasterization_shader_count;
                        break;
                    case VK_SHADER_STAGE_FRAGMENT_BIT:
                        fragment_shader = shader;
                        break;
                    default:
                        break;
                }
            }

            // Find the shader that owns the cache for this set of linked shaders
            VkPipelineCache cache_for_linked_shaders = VK_NULL_HANDLE;
            VkPipelineLayout layout_for_linked_shaders = VK_NULL_HANDLE;
            if (vertex_or_mesh_shader) {
                cache_for_linked_shaders = vertex_or_mesh_shader->cache;
                layout_for_linked_shaders = vertex_or_mesh_shader->pipeline_layout;
            } else if (fragment_shader) {
                cache_for_linked_shaders = fragment_shader->cache;
                layout_for_linked_shaders = fragment_shader->pipeline_layout;
            }

            ASSERT((!vertex_or_mesh_shader && !fragment_shader) || cache_for_linked_shaders != VK_NULL_HANDLE);
            ASSERT((!vertex_or_mesh_shader && !fragment_shader) || layout_for_linked_shaders != VK_NULL_HANDLE);

            if (vertex_or_mesh_shader) {
                vertex_or_mesh_shader->partial_pipeline = CreatePartiallyCompiledPipeline(
                    deviceData, allocator, cache_for_linked_shaders, layout_for_linked_shaders,
                    VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT, 
                    pre_rasterization_shaders, pre_rasterization_shader_count
                );
            }

            if (fragment_shader) {
                fragment_shader->partial_pipeline = CreatePartiallyCompiledPipeline(
                    deviceData, allocator, cache_for_linked_shaders, layout_for_linked_shaders, 
                    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT,
                    &fragment_shader, 1
                );
            }
        } else {
            // For unlinked shaders, there can only be a maximum of one shader per a partial pipeline
            for (uint32_t i = 0; i < shaderCount; ++i) {
                auto shader = *reinterpret_cast<Shader**>(&pShaders[i]);
                VkGraphicsPipelineLibraryFlagBitsEXT flag{};
                switch (shader->stage) {
                    case VK_SHADER_STAGE_VERTEX_BIT:
                    case VK_SHADER_STAGE_MESH_BIT_EXT:
                        flag = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;
                        break;
                    case VK_SHADER_STAGE_FRAGMENT_BIT:
                        flag = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
                        break;
                    default:
                        continue;
                }
                ASSERT(flag != 0);

                shader->partial_pipeline = CreatePartiallyCompiledPipeline(
                    deviceData, allocator, shader->cache, shader->pipeline_layout,
                    flag, &shader, 1);
            }
        }
    } else if (are_graphics_shaders_linked) {
        // Compile entire pipeline(s) to fill pipeline cache
        Shader* vertex_or_mesh_shader = nullptr;
        bool has_fragment_shader = false;
        bool has_tesc = false;
        bool has_tese = false;

        // Gather shaders into stages for pipeline compilation
        VkPipelineShaderStageCreateInfo stages[NUM_SHADERS];
        uint32_t graphics_shader_count = 0;
        for (uint32_t i = 0; i < shaderCount; ++i) {
            auto shader = *reinterpret_cast<Shader**>(&pShaders[i]);
            switch (shader->stage) {
                case VK_SHADER_STAGE_VERTEX_BIT:
                case VK_SHADER_STAGE_MESH_BIT_EXT:
                    vertex_or_mesh_shader = shader;
                    break;
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    has_fragment_shader = true;
                    break;
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                    has_tesc = true;
                    break;
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                    has_tese = true;
                    break;
                case VK_SHADER_STAGE_COMPUTE_BIT:
                    continue;
                default:
                    break;
            }

            stages[graphics_shader_count++] = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr,
                shader->flags,
                shader->stage,
                shader->shader_module,
                shader->name,
                shader->specialization_info_ptr
            };
        }

        if (vertex_or_mesh_shader == nullptr || (has_tesc && !has_tese)) {
            // Can't compile any pipeline
            return VK_SUCCESS;
        }

        AddGraphicsPipelineToCache(deviceData, allocator,
            vertex_or_mesh_shader->cache, vertex_or_mesh_shader->pipeline_layout, graphics_shader_count, stages,
            PipelineCreationFlagBits::INCLUDE_DEPTH);
        if (has_fragment_shader) {
            AddGraphicsPipelineToCache(deviceData, allocator,
                vertex_or_mesh_shader->cache, vertex_or_mesh_shader->pipeline_layout, graphics_shader_count, stages,
                PipelineCreationFlagBits::INCLUDE_COLOR);
            AddGraphicsPipelineToCache(deviceData, allocator,
                vertex_or_mesh_shader->cache, vertex_or_mesh_shader->pipeline_layout, graphics_shader_count, stages,
                PipelineCreationFlagBits::INCLUDE_DEPTH | PipelineCreationFlagBits::INCLUDE_COLOR);
        }
    }

    return VK_SUCCESS;
}

static VkLayerInstanceCreateInfo* GetChainInfo(const VkInstanceCreateInfo* pCreateInfo, VkLayerFunction func) {
    auto chain_info = reinterpret_cast<VkLayerInstanceCreateInfo*>(const_cast<void*>(pCreateInfo->pNext));
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == func)) {
        chain_info = reinterpret_cast<VkLayerInstanceCreateInfo*>(const_cast<void*>(chain_info->pNext));
    }
    ASSERT(chain_info != nullptr);
    return chain_info;
}

static VkLayerDeviceCreateInfo* GetChainInfo(const VkDeviceCreateInfo* pCreateInfo, VkLayerFunction func) {
    auto chain_info = reinterpret_cast<VkLayerDeviceCreateInfo*>(const_cast<void*>(pCreateInfo->pNext));
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = reinterpret_cast<VkLayerDeviceCreateInfo*>(const_cast<void*>(chain_info->pNext));
    }
    ASSERT(chain_info != nullptr);
    return chain_info;
}

static VkBaseOutStructure* FindStructureInChain(VkBaseOutStructure* pNext, VkStructureType sType) {
    auto current = pNext;
    while (current) {
        if (current->sType == sType) {
            return current;
        }
        current = current->pNext;
    }
    return nullptr;
}

void UpdateDrawState(CommandBufferData& data, VkCommandBuffer commandBuffer) {
    if (!data.graphics_bind_point_belongs_to_layer) {
        return;
    }

    auto state_data = data.GetDrawStateData();
    if (!state_data->is_dirty_) {
        return;
    }

    auto vertex_or_mesh_shader = state_data->GetComparableShader(VERTEX_SHADER).GetShaderPtr();
    if (vertex_or_mesh_shader == nullptr) {
        vertex_or_mesh_shader = state_data->GetComparableShader(MESH_SHADER).GetShaderPtr();
    }
    ASSERT(vertex_or_mesh_shader != nullptr);

    auto state_data_key     = state_data->GetKey();
    VkPipeline pipeline     = VK_NULL_HANDLE;
    uint32_t pipeline_count = 0;
    {
        std::shared_lock<std::shared_mutex> lock;
        auto const& pipelines   = vertex_or_mesh_shader->pipelines.GetDataForReading(lock);
        auto found_pipeline_ptr = pipelines.GetOrNullptr(state_data_key);
        if (found_pipeline_ptr) {
            pipeline = *found_pipeline_ptr;
        }
        pipeline_count = pipelines.NumEntries();
    }
    if (pipeline == VK_NULL_HANDLE) {
        std::unique_lock<std::shared_mutex> lock;
        auto& pipelines = vertex_or_mesh_shader->pipelines.GetDataForWriting(lock);
        // Ensure that a pipeline for this state wasn't created in another thread between the read lock above and the write lock
        if (pipelines.NumEntries() > pipeline_count) {
            auto iter = pipelines.Find(state_data_key);
            if (iter != pipelines.end()) {
                pipeline = iter.GetValue();
            }
        }
        if (pipeline == VK_NULL_HANDLE) {
            pipeline = CreateGraphicsPipelineForCommandBufferState(data);
            pipelines.Add(state_data_key, pipeline);
        }
    }

    data.device_data->vtable.CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    state_data->is_dirty_ = false;
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName);

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName);

void InitLayerSettings(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, LayerSettings* layer_settings) {
    assert(layer_settings != nullptr);

    const VkLayerSettingsCreateInfoEXT* create_info = vkuFindLayerSettingsCreateInfo(pCreateInfo);

    VkuLayerSettingSet layer_setting_set = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet(kLayerName, create_info, pAllocator, nullptr, &layer_setting_set);

    static const char* setting_names[] = {kLayerSettingsForceEnable, kLayerSettingsDisablePipelinePreCaching, kLayerSettingsCustomSTypeInfo};
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

    if (vkuHasLayerSetting(layer_setting_set, kLayerSettingsDisablePipelinePreCaching)) {
        vkuGetLayerSettingValue(layer_setting_set, kLayerSettingsDisablePipelinePreCaching, layer_settings->disable_pipeline_pre_caching);
    }

    if (vkuHasLayerSetting(layer_setting_set, kLayerSettingsCustomSTypeInfo)) {
        vkuGetLayerSettingValues(layer_setting_set, kLayerSettingsCustomSTypeInfo, vku::GetCustomStypeInfo());
    }

    vkuDestroyLayerSettingSet(layer_setting_set, pAllocator);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    auto allocator = pAllocator ? *pAllocator : kDefaultAllocator;
    auto chain_info = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);
    ASSERT(chain_info->u.pLayerInfo);

    // Get the next layer's vkGetInstanceProcAddr
    auto fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    auto fpCreateInstance      = reinterpret_cast<PFN_vkCreateInstance>(fpGetInstanceProcAddr(nullptr, "vkCreateInstance"));
    if (fpCreateInstance == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element of the chain.
    // This ensures that the next layer gets it's layer info and not
    // the info for our current layer.
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // Continue call down the chain
    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    // Enumerate physical devices
    auto fpEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(fpGetInstanceProcAddr(*pInstance, "vkEnumeratePhysicalDevices"));
    ASSERT(fpEnumeratePhysicalDevices != nullptr);

    uint32_t physical_device_count;
    result = fpEnumeratePhysicalDevices(*pInstance, &physical_device_count, nullptr);
    ASSERT(result == VK_SUCCESS);

    void* memory = allocator.pfnAllocation(allocator.pUserData, 
        sizeof(InstanceData) + physical_device_count * (sizeof(VkPhysicalDevice) + sizeof(PhysicalDeviceData)),
        alignof(InstanceData), VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (!memory) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    auto instance_data = new (memory) InstanceData();
    instance_data->instance = *pInstance;
    instance_data->physical_devices = reinterpret_cast<VkPhysicalDevice*>(instance_data + 1);
    instance_data->physical_device_count = physical_device_count;
    instance_data->vtable.Initialize(*pInstance, fpGetInstanceProcAddr);

    InitLayerSettings(pCreateInfo, pAllocator, &instance_data->layer_settings);

    result = fpEnumeratePhysicalDevices(*pInstance, &instance_data->physical_device_count, instance_data->physical_devices);
    ASSERT(result == VK_SUCCESS);

    // Check which physical devices support extensions that the layer cares about
    auto physical_device_datas = reinterpret_cast<PhysicalDeviceData*>(instance_data->physical_devices + instance_data->physical_device_count);
    uint32_t highest_property_count = 0;
    for (uint32_t i = 0; i < instance_data->physical_device_count; ++i) {
        VkPhysicalDeviceProperties properties;
        instance_data->vtable.GetPhysicalDeviceProperties(instance_data->physical_devices[i], &properties);

        physical_device_datas[i] = { instance_data, properties.vendorID };

        uint32_t property_count;
        result = instance_data->vtable.EnumerateDeviceExtensionProperties(instance_data->physical_devices[i], nullptr, &property_count, nullptr);
        highest_property_count = std::max(highest_property_count, property_count);
        ASSERT(result == VK_SUCCESS);
    }
    auto extension_properties = static_cast<VkExtensionProperties*>(allocator.pfnAllocation(allocator.pUserData, sizeof(VkExtensionProperties) * highest_property_count, alignof(VkExtensionProperties), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
    if (!extension_properties) {
        allocator.pfnFree(allocator.pUserData, memory);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    for (uint32_t i = 0; i < instance_data->physical_device_count; ++i) {
        uint32_t property_count = highest_property_count;
        result = instance_data->vtable.EnumerateDeviceExtensionProperties(instance_data->physical_devices[i], nullptr, &property_count, extension_properties);
        ASSERT(result == VK_SUCCESS);

        for (uint32_t extension_idx = 0; extension_idx < property_count; ++extension_idx) {
            physical_device_datas[i].supported_additional_extensions |= AdditionalExtensionStringToFlag(extension_properties[extension_idx].extensionName);
        }
    }
    allocator.pfnFree(allocator.pUserData, extension_properties);

    instance_data_map.Add(*pInstance, instance_data);
    for (uint32_t i = 0; i < instance_data->physical_device_count; ++i) {
        physical_device_data_map.Add(instance_data->physical_devices[i], physical_device_datas + i);
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    auto allocator = pAllocator ? *pAllocator : kDefaultAllocator;
    auto instance_data = instance_data_map.Get(instance);

    for (uint32_t i = 0; i < instance_data->physical_device_count; ++i) {
        physical_device_data_map.Remove(instance_data->physical_devices[i]);
    }
    instance_data->vtable.DestroyInstance(instance, pAllocator);

    // clean up allocation
    instance_data->~InstanceData();
    allocator.pfnFree(allocator.pUserData, instance_data);

    // stop keeping track of this instance
    instance_data_map.Remove(instance);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceFeatures2* pFeatures) {
    auto  physical_device_data = physical_device_data_map.Get(physicalDevice);
    auto& vtable               = physical_device_data->instance->vtable;

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* eds3_feature = nullptr;
    VkPhysicalDeviceShaderObjectFeaturesEXT* shader_object_feature = nullptr;
    auto feature_before_shader_object = reinterpret_cast<VkBaseOutStructure*>(pFeatures);
    for (auto chain = reinterpret_cast<VkBaseOutStructure*>(pFeatures); chain; chain = chain->pNext) {
        if (chain->pNext && chain->pNext->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT) {
            feature_before_shader_object = chain;
            shader_object_feature = reinterpret_cast<VkPhysicalDeviceShaderObjectFeaturesEXT*>(chain->pNext);
        }
        if (chain->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT) {
            eds3_feature = reinterpret_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT*>(chain);
        }
    }

    while (feature_before_shader_object) {
        if (feature_before_shader_object->pNext && feature_before_shader_object->pNext->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT) {
            shader_object_feature = reinterpret_cast<VkPhysicalDeviceShaderObjectFeaturesEXT*>(feature_before_shader_object->pNext);
            break;
        }
        feature_before_shader_object = feature_before_shader_object->pNext;
    }

    if (shader_object_feature && (physical_device_data->supported_additional_extensions & SHADER_OBJECT) == 0) {
        ASSERT(reinterpret_cast<uintptr_t>(feature_before_shader_object->pNext) == reinterpret_cast<uintptr_t>(shader_object_feature));
        
        // No native shader object in the driver, need to leave out shader object extension when calling underlying GetPhysicalDeviceFeatures2
        feature_before_shader_object->pNext = reinterpret_cast<VkBaseOutStructure*>(shader_object_feature->pNext);
        vtable.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
        feature_before_shader_object->pNext = reinterpret_cast<VkBaseOutStructure*>(shader_object_feature);

        // The layer requires maintenance2 and dynamic_rendering extensions
        uint32_t const required_extensions = DYNAMIC_RENDERING | MAINTENANCE_2;
        if ((physical_device_data->supported_additional_extensions & required_extensions) == required_extensions) {
            shader_object_feature->shaderObject = VK_TRUE;
        } else {
            shader_object_feature->shaderObject = VK_FALSE;
        }
    } else {
        vtable.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }

    // Also disable extended dynamic state 3 feature relating to VK_EXT_blend_operation_advanced
    if (eds3_feature) {
        eds3_feature->extendedDynamicState3ColorBlendAdvanced = VK_FALSE;
    }
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                               VkPhysicalDeviceProperties2* pProperties) {
    auto physical_device_data = physical_device_data_map.Get(physicalDevice);
    auto instance_data        = physical_device_data->instance;

    instance_data->vtable.GetPhysicalDeviceProperties2(physicalDevice, pProperties);

    VkBaseOutStructure* chain = reinterpret_cast<VkBaseOutStructure*>(pProperties->pNext);
    while (chain) {
        if (chain->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT) {
            auto features = reinterpret_cast<VkPhysicalDeviceShaderObjectPropertiesEXT*>(chain);
            memcpy(features->shaderBinaryUUID, pProperties->properties.pipelineCacheUUID,
                   sizeof(pProperties->properties.pipelineCacheUUID));
            features->shaderBinaryVersion = SHADER_OBJECT_BINARY_VERSION;
        }

        chain = chain->pNext;
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                         uint32_t* pPropertyCount,
                                                                         VkExtensionProperties* pProperties) {
    auto  physical_device_data = physical_device_data_map.Get(physicalDevice);
    auto  instance_data        = physical_device_data->instance;
    auto& vtable               = instance_data->vtable;

    if (pLayerName && strncmp(pLayerName, kLayerName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
        // User is asking for this layer
        *pPropertyCount = 1;
        if (pProperties) {
            *pProperties = kExtensionProperties;
        }
        return VK_SUCCESS;
    }
    if (pLayerName) {
        // User is asking for a different layer
        return vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    }

    // The application is asking about Vulkan implementation. If there's no native shader object:
    // Need to strip VK_EXT_discard_rectangles and VK_NV_scissor_exclusive if spec version is less than 2
    // Need to strip out VK_EXT_vertex_attribute_divisor
    // Need to strip out VK_EXT_blend_operation_advanced
    // Need to insert shader object

    uint32_t count  = 0;
    VkResult result = vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &count, nullptr);
    if (result != VK_SUCCESS) {
        *pPropertyCount = 0;
        return result;
    }
    auto all_properties = static_cast<VkExtensionProperties*>(kDefaultAllocator.pfnAllocation(nullptr, count * sizeof(VkExtensionProperties), std::alignment_of<VkExtensionProperties>::value, VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
    if (!all_properties) {
        *pPropertyCount = 0;
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    result = vtable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &count, all_properties);
    if (result != VK_SUCCESS) {
        kDefaultAllocator.pfnFree(nullptr, all_properties);
        *pPropertyCount = 0;
        return result;
    }

    bool has_native_shader_object = false;
    for (uint32_t i = 0; i < count; ++i) {
        if (strncmp(all_properties[i].extensionName, VK_EXT_SHADER_OBJECT_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
            has_native_shader_object = true;
            break;
        }
    }
    if (has_native_shader_object) {
        if (pProperties) {
            if (count < *pPropertyCount) {
                *pPropertyCount = count;
            }
            memcpy(pProperties, all_properties, *pPropertyCount * sizeof(VkExtensionProperties));
        } else {
            *pPropertyCount = count;
        }
        kDefaultAllocator.pfnFree(nullptr, all_properties);
        return count <= *pPropertyCount ? VK_SUCCESS : VK_INCOMPLETE;
    }

    uint32_t write_index = 0;
    for (uint32_t read_index = 0; read_index < count + 1; ++read_index) {
        // Append VK_EXT_shader_object
        auto& extension_property = (read_index == count) ? kExtensionProperties : all_properties[read_index];

        // Filter out extensions
        if (extension_property.specVersion < 2 && (
            strncmp(extension_property.extensionName, VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE) == 0 ||
            strncmp(extension_property.extensionName, VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME,   VK_MAX_EXTENSION_NAME_SIZE) == 0)) {
            continue;
        }
        if (strncmp(extension_property.extensionName, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
            continue;
        }
        if (strncmp(extension_property.extensionName, VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
            continue;
        }

        // Write the property if there's space
        if (pProperties) {
            if (write_index >= *pPropertyCount) {
                kDefaultAllocator.pfnFree(nullptr, all_properties);
                return VK_INCOMPLETE;
            }
            pProperties[write_index] = extension_property;
        }
        ++write_index;
    }

    *pPropertyCount = write_index;
    kDefaultAllocator.pfnFree(nullptr, all_properties);
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                    VkExtensionProperties* pProperties) {
    if (pLayerName && strncmp(pLayerName, shader_object::kLayerName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
        return EnumerateProperties(shader_object::kInstanceExtensionPropertiesCount,
                                   shader_object::kInstanceExtensionProperties, pPropertyCount, pProperties);
    }
    return VK_ERROR_LAYER_NOT_PRESENT;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    auto  allocator            = pAllocator ? *pAllocator : kDefaultAllocator;
    auto  physical_device_data = physical_device_data_map.Get(physicalDevice);
    auto  instance_data        = physical_device_data->instance;
    auto& instance_vtable      = instance_data->vtable;
    auto  chain_info           = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);

    ASSERT(chain_info->u.pLayerInfo);
    auto fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    auto fpGetDeviceProcAddr   = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    auto fpCreateDevice        = reinterpret_cast<PFN_vkCreateDevice>(fpGetInstanceProcAddr(instance_data->instance, "vkCreateDevice"));
    if (fpCreateDevice == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // Advance link info so that next layer gets the correct layer info
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    bool shader_object_feature_requested = false;
    for (auto chain = reinterpret_cast<VkBaseInStructure const*>(pCreateInfo->pNext); chain; chain = chain->pNext) {
        if (chain->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT) {
            continue;
        }
        auto shader_object_feature = reinterpret_cast<VkPhysicalDeviceShaderObjectFeaturesEXT const*>(chain);
        if (shader_object_feature->shaderObject == VK_TRUE) {
            shader_object_feature_requested = true;
        }
        break;
    }
    bool shader_object_extension_requested = false;
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i) {
        if (0 ==
            strncmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_SHADER_OBJECT_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) {
            shader_object_extension_requested = true;
            break;
        }
    }

    // only enable the layer if the application asked for the extension and feature AND the driver does not have a native
    // implementation (unless if the user specified to ignore the native implementation via environment variable)
    bool enable_layer = shader_object_feature_requested && shader_object_extension_requested;
    bool ignore_native_implementation = instance_data->layer_settings.force_enable;
    if (ignore_native_implementation) {
        DEBUG_LOG("ignoring native driver implementation of shader object\n");
    }
    if (enable_layer && !ignore_native_implementation && (physical_device_data->supported_additional_extensions & SHADER_OBJECT) != 0) {
        VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeature{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                                                                    nullptr};

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &shaderObjectFeature};
        instance_vtable.GetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures);

        if (shaderObjectFeature.shaderObject == VK_TRUE) {
            enable_layer = false;
        }
    }

    void* device_data_memory = allocator.pfnAllocation(allocator.pUserData, sizeof(DeviceData), std::alignment_of<DeviceData>::value, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!device_data_memory) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    DeviceData* device_data = new (device_data_memory) DeviceData();

    VkResult result;
    if (enable_layer) {
        // For consistent pipeline caches, VK_EXT_conservative_rasterization needs to be force enabled on some drivers
        auto const force_enable_conservative_rasterization = (physical_device_data->vendor_id == 0x10DE);
        auto const maximum_enabled_extension_count =
            pCreateInfo->enabledExtensionCount    +
            GetArrayLength(kAdditionalExtensions) +
            (force_enable_conservative_rasterization ? 1 : 0);

        auto enabled_extension_names = AllocateArray<const char*>(allocator, maximum_enabled_extension_count, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (!enabled_extension_names) {
            allocator.pfnFree(allocator.pUserData, device_data_memory);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        // Filter out shader object from pCreateInfo extensions
        uint32_t enabled_extension_count = 0;
        AdditionalExtensionFlags enabled_additional_extensions{};
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i) {
            auto name = pCreateInfo->ppEnabledExtensionNames[i];
            auto flag = AdditionalExtensionStringToFlag(name);
            if (flag & SHADER_OBJECT) {
                continue;
            }
            enabled_additional_extensions |= flag;
            enabled_extension_names[enabled_extension_count] = name;
            ++enabled_extension_count;
        }

        // Add extensions that allow the layer to perform better
        for (uint32_t i = 0; i < GetArrayLength(kAdditionalExtensions); ++i) {
            auto const& ext = kAdditionalExtensions[i];
            if ((physical_device_data->supported_additional_extensions & ext.flag) == 0) {
                // Physical device doesn't support the extension
                continue;
            }
            if (enabled_additional_extensions & ext.flag) {
                // The application already added this extension
                continue;
            }
            enabled_additional_extensions |= ext.flag;
            enabled_extension_names[enabled_extension_count] = ext.extension_name;
            ++enabled_extension_count;
        }

        // Handle force enabled conservative rasterization
        if (force_enable_conservative_rasterization &&
            (physical_device_data->supported_additional_extensions & CONSERVATIVE_RASTERIZATION) != 0 &&
            (enabled_additional_extensions                         & CONSERVATIVE_RASTERIZATION) == 0) {
            // Don't add the flag since we don't want the layer to expose related functionality (since the application didn't request it)
            enabled_extension_names[enabled_extension_count] = VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME;
            ++enabled_extension_count;
        }

        // Deep copy the create info pNext chain using vk_safe_struct.hpp
        auto const device_next_chain = static_cast<VkBaseOutStructure*>(vku::SafePnextCopy(pCreateInfo->pNext));
        if (!device_next_chain) {
            allocator.pfnFree(allocator.pUserData, enabled_extension_names);
            allocator.pfnFree(allocator.pUserData, device_data_memory);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        // Remove shader object from pNext chain
        auto before_device_next_chain = VkBaseOutStructure{{}, device_next_chain};
        for (auto current = &before_device_next_chain; current; current = current->pNext) {
            if (current->pNext && current->pNext->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT) {
                auto shader_object_feature = reinterpret_cast<vku::safe_VkPhysicalDeviceShaderObjectFeaturesEXT*>(current->pNext);
                current->pNext = current->pNext->pNext;
        
                // Structure was allocated with new in SafePnextCopy
                shader_object_feature->pNext = nullptr;
                delete shader_object_feature;
                break;
            }
        }

        // Generate a chain of feature structures for the extensions that were added to append to the device creation pNext chain
        VkPhysicalDeviceFeatures2 device_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
#include "generated/shader_object_create_device_feature_structs.inl"
        device_features.pNext = appended_features_chain;
        instance_vtable.GetPhysicalDeviceFeatures2(physicalDevice, &device_features);
        if ((!vulkan_1_3_ptr || vulkan_1_3_ptr->dynamicRendering == VK_FALSE) && (!dynamic_rendering_ptr || dynamic_rendering_ptr->dynamicRendering == VK_FALSE)) {
            // Dynamic rendering is required
            vku::FreePnextChain(device_next_chain);
            allocator.pfnFree(allocator.pUserData, enabled_extension_names);
            allocator.pfnFree(allocator.pUserData, device_data_memory);
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        VkBaseOutStructure* last = device_next_chain;
        uint32_t total_private_data_slot_request_count = 1;
        for (auto current = device_next_chain; current; current = current->pNext) {
            // Get pointer to last item in deep-copied pNext chain
            if (!current->pNext) {
                last = current;
            }

            // Count number of requested reserved slots
            if (current->sType == VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO) {
                total_private_data_slot_request_count += reinterpret_cast<VkDevicePrivateDataCreateInfo*>(current)->privateDataSlotRequestCount;
            }
        }

        // The layer requests a private data slot for command buffers
        VkDevicePrivateDataCreateInfo reserved_private_data_slot{
            VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO,
            appended_features_chain,
            1
        };

        // Append to deep-copied pNext chain
        if ((vulkan_1_3_ptr && vulkan_1_3_ptr->privateData == VK_TRUE) || (private_data_ptr && private_data_ptr->privateData == VK_TRUE)) {
            last->pNext = reinterpret_cast<VkBaseOutStructure*>(&reserved_private_data_slot);
        } else {
            last->pNext = appended_features_chain;
        }

        // Create VkDeviceCreateInfo with the modified extensions list and pNext chain
        VkDeviceCreateInfo device_create_info{
            pCreateInfo->sType,
            device_next_chain,
            pCreateInfo->flags,
            pCreateInfo->queueCreateInfoCount,
            pCreateInfo->pQueueCreateInfos,
            pCreateInfo->enabledLayerCount,
            pCreateInfo->ppEnabledLayerNames,
            enabled_extension_count,
            enabled_extension_names,
            pCreateInfo->pEnabledFeatures
        };

        // Create the device
        result = fpCreateDevice(physicalDevice, &device_create_info, pAllocator, pDevice);

        // Remove features that were appended to the deep-copied pNext chain and free memory that's no longer needed
        last->pNext = nullptr;
        allocator.pfnFree(allocator.pUserData, enabled_extension_names);

        // Handle device creation failure
        if (result != VK_SUCCESS) {
            vku::FreePnextChain(device_next_chain);
            allocator.pfnFree(allocator.pUserData, device_data_memory);
            return result;
        }

        // Fill in device data
        device_data->device                           = *pDevice;
        device_data->flags                            = enable_layer ? DeviceData::SHADER_OBJECT_LAYER_ENABLED : 0; 
        if (instance_data->layer_settings.disable_pipeline_pre_caching) {
            device_data->flags |= DeviceData::DISABLE_PIPELINE_PRE_CACHING;
        }
        device_data->reserved_private_data_slot_count = total_private_data_slot_request_count;
        device_data->enabled_extensions               = enabled_additional_extensions;

#include "generated/shader_object_device_data_set_extension_variables.inl"

        // Add dynamic states that are always available
        device_data->AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
        device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
        device_data->AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
        device_data->AddDynamicState(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
        device_data->AddDynamicState(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        device_data->AddDynamicState(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
#include "generated/shader_object_device_data_dynamic_state_adding.inl"

        // Handle special case dynamic states
        if (!extended_dynamic_state_1_ptr || extended_dynamic_state_1_ptr->extendedDynamicState == VK_FALSE) {
            // No VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, so use VK_DYNAMIC_STATE_VIEWPORT instead
            device_data->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
            device_data->AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
        }
        if (enabled_additional_extensions & SAMPLE_LOCATIONS) {
            device_data->AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
        }
        if (enabled_additional_extensions & NV_CLIP_SPACE_W_SCALING) {
            device_data->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
        }
        if (enabled_additional_extensions & NV_SHADING_RATE_IMAGE) {
            device_data->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV);
        }

        vku::FreePnextChain(device_next_chain);

        // Get properties for this device so we can allocate according to device limits
        instance_vtable.GetPhysicalDeviceProperties(physicalDevice, &device_data->properties);
        if (device_data->properties.apiVersion >= VK_API_VERSION_1_1) {
            VkPhysicalDeviceDriverPropertiesKHR driver_properties{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR
            };
            VkPhysicalDeviceExtendedDynamicState3PropertiesEXT eds3_properties{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT
            };
            if (device_data->enabled_extensions & DRIVER_PROPERTIES) {
                eds3_properties.pNext = &driver_properties;
            }
            VkPhysicalDeviceProperties2 properties2{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                &eds3_properties
            };
            instance_vtable.GetPhysicalDeviceProperties2(physicalDevice, &properties2);
            if (eds3_properties.dynamicPrimitiveTopologyUnrestricted == VK_TRUE) {
                device_data->flags |= DeviceData::HAS_PRIMITIVE_TOPLOGY_UNRESTRICTED;
            }
            // Disable graphics pipeline libraries on drivers that have issues with them
            if (driver_properties.driverID == VK_DRIVER_ID_MESA_RADV || driver_properties.driverID == VK_DRIVER_ID_MESA_LLVMPIPE) {
                device_data->enabled_extensions &= ~GRAPHICS_PIPELINE_LIBRARY;
            }
        }

        // Find a supported depth stencil format (either VK_FORMAT_D24_UNORM_S8_UINT or VK_FORMAT_D32_SFLOAT_S8_UINT)
        VkImageFormatProperties format_properties;
        VkResult format_result = instance_vtable.GetPhysicalDeviceImageFormatProperties(physicalDevice, 
                                                                                        VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TYPE_2D, 
                                                                                        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                                                                                        0, &format_properties);
        if (format_result == VK_SUCCESS) {
            device_data->supported_depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;
        } else if (format_result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
            device_data->supported_depth_stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        } else {
            allocator.pfnFree(allocator.pUserData, device_data_memory);
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    } else {
        // Pass call down the chain
        result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    }

    // Initialize device dispatch table using GetDeviceProcAddr of next layer in the chain
    auto& vtable = device_data->vtable;
    vtable.Initialize(*pDevice, fpGetDeviceProcAddr);

    if (device_data->flags & DeviceData::SHADER_OBJECT_LAYER_ENABLED) {
        // Create a dummy pipeline layout that will be used in some pipeline creations
        VkPipelineLayoutCreateInfo dummy_pipeline_layout_create_info{};
        dummy_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        VkResult pipeline_layout_result = vtable.CreatePipelineLayout(device_data->device, &dummy_pipeline_layout_create_info,
                                                                            &allocator, &device_data->dummy_pipeline_layout);
        ASSERT(pipeline_layout_result == VK_SUCCESS);
        UNUSED(pipeline_layout_result);

        // gather created queues
        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            for (uint32_t j = 0; j < pCreateInfo->pQueueCreateInfos[i].queueCount; ++j) {
                VkQueue queue;
                vtable.GetDeviceQueue(*pDevice, pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex, j, &queue);
                queue_to_device_data_map.Add(queue, device_data);
            }
        }

        if (device_data->private_data.privateData == VK_TRUE) {
            VkPrivateDataSlotCreateInfo private_data_create_info{VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO};
            VkResult data_slot_result =
                vtable.CreatePrivateDataSlotEXT(*pDevice, &private_data_create_info, &allocator, &device_data->private_data_slot);
            ASSERT(data_slot_result == VK_SUCCESS);
            UNUSED(data_slot_result);
        }

        std::unique_lock<std::shared_mutex> lock;
        auto& first_device = first_device_container.GetDataForWriting(lock);
        if (first_device == nullptr) {
            first_device = device_data;
        }
    }

    // Store device data
    device_data_map.Add(*pDevice, device_data);

    return result;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto  allocator      = pAllocator ? *pAllocator : kDefaultAllocator;
    auto  device_data    = device_data_map.Get(device);
    auto& vtable         = device_data->vtable;
    auto  destroy_device = vtable.DestroyDevice;

    // Remove references to device data
    queue_to_device_data_map.RemoveAllWithValue(device_data);
    device_data_map.Remove(device);

    // Update first device pointer if necessary
    std::unique_lock<std::shared_mutex> lock;
    auto& first_device = first_device_container.GetDataForWriting(lock);
    if (first_device == device_data) {
        first_device = nullptr;
    }
    lock.unlock();

    // Clean up device data resources
    if (device_data->private_data_slot != VK_NULL_HANDLE) {
        vtable.DestroyPrivateDataSlotEXT(device_data->device, device_data->private_data_slot, &allocator);
    }
    if (device_data->dummy_pipeline_layout != VK_NULL_HANDLE) {
        vtable.DestroyPipelineLayout(device_data->device, device_data->dummy_pipeline_layout, &allocator);
    }
    device_data->~DeviceData();
    allocator.pfnFree(allocator.pUserData, device_data);

    // Forward the destroy call down
    destroy_device(device, pAllocator);
}

static VKAPI_ATTR void VKAPI_CALL DestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    auto allocator = pAllocator ? *pAllocator : kDefaultAllocator;
    auto& data = *device_data_map.Get(device);

    Shader::Destroy(data, reinterpret_cast<Shader*>(shader), allocator);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                       const VkShaderCreateInfoEXT* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) {
    VkResult result = VK_SUCCESS;
    if (createInfoCount == 0) {
        return result;
    }

    auto const& allocator = pAllocator ? *pAllocator : kDefaultAllocator;
    auto const& device_data = *device_data_map.Get(device);
    auto const& vtable = device_data.vtable;
    memset(pShaders, 0, sizeof(VkShaderEXT) * createInfoCount);

    // First, create individual shaders
    bool are_graphics_shaders_linked = false;
    uint32_t successfulCreateCount = createInfoCount;
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        auto shader = reinterpret_cast<Shader**>(&pShaders[i]);
        result = Shader::Create(device_data, pCreateInfos[i], allocator, shader);
        if (result != VK_SUCCESS) {
            memset(shader, 0u, sizeof(VkShaderEXT));
            successfulCreateCount = i;
            break;
        }
        if ((pCreateInfos[i].stage & VK_SHADER_STAGE_ALL_GRAPHICS) != 0 && (pCreateInfos[i].flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            are_graphics_shaders_linked = true;
        }
    }

    if (result != VK_SUCCESS && are_graphics_shaders_linked) {
        return result;
    }

    bool incompatible_binary = false;
    if (result == VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT) {
        incompatible_binary = true;
        result = VK_SUCCESS;
    }

    // More work may be able to be done up-front depending on create options and feature support. There isn't enough information
    // to perform a full pipeline compile that will be exactly correct at draw time. However, if shaders are linked together,
    // a good "guess" can be made at a pipeline in hopes that enough data is stored in the pipeline cache to significantly speed
    // up the pipeline compile at first draw record time. Additionally, if graphics pipeline library is available, it's possible
    // to compile partial pipelines that can be used to speed up pipeline compilation at first draw record time.

    // Create pipeline layout for pipeline creation
    Shader* vertex_or_mesh_shader = nullptr;
    Shader* fragment_shader = nullptr;
    if (result == VK_SUCCESS) {
        for (uint32_t i = 0; i < successfulCreateCount; ++i) {
            auto shader = *reinterpret_cast<Shader**>(&pShaders[i]);
            switch (shader->stage) {
                case VK_SHADER_STAGE_VERTEX_BIT:
                case VK_SHADER_STAGE_MESH_BIT_EXT:
                    vertex_or_mesh_shader = shader;
                    break;
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    fragment_shader = shader;
                    break;
                default:
                    break;
            }
        }
    }
    if (result == VK_SUCCESS && are_graphics_shaders_linked && (vertex_or_mesh_shader || fragment_shader)) {
        Shader* shader_to_read_layout_from = vertex_or_mesh_shader ? vertex_or_mesh_shader : fragment_shader;
        result = CreatePipelineLayoutForShader(device_data, allocator, shader_to_read_layout_from);
    }
    if (result == VK_SUCCESS && !are_graphics_shaders_linked && device_data.graphics_pipeline_library.graphicsPipelineLibrary == VK_TRUE) {
        // Create layout for unlinked shaders that can have partial pipelines created
        for (uint32_t i = 0; i < successfulCreateCount; ++i) {
            auto shader = *reinterpret_cast<Shader**>(&pShaders[i]);
            switch (shader->stage) {
                case VK_SHADER_STAGE_VERTEX_BIT:
                case VK_SHADER_STAGE_MESH_BIT_EXT:
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    result = CreatePipelineLayoutForShader(device_data, allocator, shader);
                    break;
                default:
                    break;
            }

            if (result != VK_SUCCESS) {
                break;
            }
        }
    }

    // Generating pipelines to fill the cache is only relevant if codeType is not binary (i.e. SPIR-V) since the caches are
    // serialized in the shader binary
    if (result == VK_SUCCESS && pCreateInfos[0].codeType != VK_SHADER_CODE_TYPE_BINARY_EXT) {
        result = PopulateCachesForShaders(device_data, allocator, are_graphics_shaders_linked, successfulCreateCount, pShaders);

        for (uint32_t i = 0; i < successfulCreateCount; ++i) {
            auto shader = *reinterpret_cast<Shader**>(&pShaders[i]);
            if (shader->cache == VK_NULL_HANDLE) {
                continue;
            }

            // Save off the cache as it is right now into the pristine cache. We'll continue to use `cache` throughout the
            // application. `pristine_cache` is used for shader binary serialization
            result = vtable.MergePipelineCaches(device, shader->pristine_cache, 1, &shader->cache);
            if (result != VK_SUCCESS) {
                break;
            }
        }
    }

    if (incompatible_binary) {
        return VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT;
    }

    return result;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    auto& device_data = *device_data_map.Get(device);
    auto& shader_object = *reinterpret_cast<Shader*>(shader);

    size_t binary_size;
    if (VkResult result = CalculateBinarySizeForShader(device_data, shader_object, &binary_size, nullptr)) {
        return result;
    }

    if (pData == nullptr) {
        *pDataSize = binary_size;
        return VK_SUCCESS;
    }

    if (*pDataSize < binary_size) {
        *pDataSize = 0;
        return VK_INCOMPLETE;
    }

    return ShaderBinary::Create(device_data, shader_object, pData);
}

static VKAPI_ATTR void VKAPI_CALL CmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                    const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    auto& vtable = cmd_data->device_data->vtable;

    for (uint32_t i = 0; i < stageCount; ++i) {
        auto shader = pShaders ? reinterpret_cast<Shader*>(pShaders[i]) : nullptr;

        // Compute shaders are a special case and can have their pipeline bound immediately
        if (pStages[i] == VK_SHADER_STAGE_COMPUTE_BIT) {
            if (shader) {
                vtable.CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->partial_pipeline.pipeline);
            }
            continue;
        }

        // Graphics shaders require the layer to keep track of the graphics bind point
        if (shader) {
            cmd_data->graphics_bind_point_belongs_to_layer = true;
        }
        cmd_data->GetDrawStateData()->SetComparableShader(ShaderStageToShaderType(pStages[i]), ComparableShader(shader));
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        cmd_data->graphics_bind_point_belongs_to_layer = false;
    }
    cmd_data->device_data->vtable.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    DeviceData& data = *device_data_map.Get(device);

    VkResult result = data.vtable.CreateImageView(device, pCreateInfo, pAllocator, pView);
    if (result != VK_SUCCESS) {
        return result;
    }

    data.image_view_format_map.Add(*pView, pCreateInfo->format);

    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    auto device_data = device_data_map.Get(device);
    device_data->image_view_format_map.Remove(imageView);
    device_data->vtable.DestroyImageView(device, imageView, pAllocator);
}

static VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                             VkCommandBuffer* pCommandBuffers) {
    auto allocator   = kDefaultAllocator;
    auto device_data = device_data_map.Get(device);

    VkResult result = device_data->vtable.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (result != VK_SUCCESS) {
        return result;
    }

    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i) {
        auto cmd_data = CommandBufferData::Create(device_data, allocator);
        if (cmd_data == nullptr) {
            // Clean up references
            for (uint32_t j = 0; j < i; ++j) {
                command_buffer_to_pool_map.Remove(pCommandBuffers[j]);
                RemoveCommandBufferDataForCommandBuffer(device_data, pCommandBuffers[i]);
            }
            // Free all command buffers because we already allocated all of them
            device_data->vtable.FreeCommandBuffers(device, pAllocateInfo->commandPool, pAllocateInfo->commandBufferCount, pCommandBuffers);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        cmd_data->pool = pAllocateInfo->commandPool;
        SetCommandBufferDataForCommandBuffer(device_data, pCommandBuffers[i], cmd_data);
        command_buffer_to_pool_map.Add(pCommandBuffers[i], pAllocateInfo->commandPool);
    }

    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                     const VkCommandBuffer* pCommandBuffers) {
    DeviceData& data = *device_data_map.Get(device);

    for (uint32_t i = 0; i < commandBufferCount; ++i) {
        if (pCommandBuffers[i] == VK_NULL_HANDLE) {
            continue;
        }

        command_buffer_to_pool_map.Remove(pCommandBuffers[i]);

        RemoveCommandBufferDataForCommandBuffer(&data, pCommandBuffers[i]);
    }

    data.vtable.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

static VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                     const VkAllocationCallbacks* pAllocator) {
    DeviceData& data = *device_data_map.Get(device);

    // Find CommandBufferDatas associated with this command pool and destroy them
    command_buffer_to_pool_map.RemoveAllWithValueCustom(
        commandPool, [device_data = &data](VkCommandBuffer const& key, VkCommandPool const& value) {
            RemoveCommandBufferDataForCommandBuffer(device_data, key);
        });

    data.vtable.DestroyCommandPool(device, commandPool, pAllocator);
}

static VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                         const VkCommandBufferBeginInfo* pBeginInfo) {
    auto cmd_data    = GetCommandBufferData(commandBuffer);
    auto draw_state  = cmd_data->GetDrawStateData();
    auto device_data = cmd_data->device_data;

    // If every state setting call before the draw is dynamic, we still need to bind a pipeline
    draw_state->MarkDirty();

    draw_state->SetRasterizationSamples(VK_SAMPLE_COUNT_1_BIT);

    for (uint32_t i = 0; i < kMaxSampleMaskLength; ++i) {
        // Set sample mask to default value of all ones
        constexpr VkSampleMask all_ones = ~static_cast<VkSampleMask>(0);
        draw_state->SetSampleMask(i, all_ones);
    }

    if ((device_data->enabled_extensions & NV_VIEWPORT_SWIZZLE) != 0 && device_data->extended_dynamic_state_3.extendedDynamicState3ViewportSwizzle == VK_FALSE) {
        // Reset viewport swizzles
        for (uint32_t i = 0; i < device_data->properties.limits.maxViewports; ++i) {
            VkViewportSwizzleNV default_swizzle{
                VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV,
                VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
                VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV,
                VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV
            };
            draw_state->SetViewportSwizzle(i, default_swizzle);
        }
    }

    cmd_data->last_seen_pipeline_layout_ = VK_NULL_HANDLE;
    return device_data->vtable.BeginCommandBuffer(commandBuffer, pBeginInfo);
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    FullDrawStateData* state = cmd_data->GetDrawStateData();
    state->SetNumColorAttachments(pRenderingInfo->colorAttachmentCount);
    for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
        VkImageView view = pRenderingInfo->pColorAttachments[i].imageView;
        VkFormat format = (view == VK_NULL_HANDLE ? VK_FORMAT_UNDEFINED : cmd_data->device_data->image_view_format_map.Get(view));
        state->SetColorAttachmentFormat(i, format);
    }

    if (pRenderingInfo->pDepthAttachment) {
        VkImageView view = pRenderingInfo->pDepthAttachment->imageView;
        VkFormat format = view == VK_NULL_HANDLE ? VK_FORMAT_UNDEFINED : cmd_data->device_data->image_view_format_map.Get(view);
        state->SetDepthAttachmentFormat(format);
    }

    if (pRenderingInfo->pStencilAttachment) {
        VkImageView view = pRenderingInfo->pStencilAttachment->imageView;
        VkFormat format = view == VK_NULL_HANDLE ? VK_FORMAT_UNDEFINED : cmd_data->device_data->image_view_format_map.Get(view);
        state->SetStencilAttachmentFormat(format);
    }
    cmd_data->device_data->vtable.CmdBeginRendering(commandBuffer, pRenderingInfo);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                          const VkViewport* pViewports) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetNumViewports(viewportCount);
    cmd_data->device_data->vtable.CmdSetViewport(commandBuffer, 0, viewportCount, pViewports);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                         const VkRect2D* pScissors) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetNumScissors(scissorCount);
    cmd_data->device_data->vtable.CmdSetScissor(commandBuffer, 0, scissorCount, pScissors);
}

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, 
                                                        uint32_t firstBinding, uint32_t bindingCount, 
                                                        const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, 
                                                        const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    auto cmd_data   = GetCommandBufferData(commandBuffer);
    auto draw_state = cmd_data->GetDrawStateData();
    auto& vtable    = cmd_data->device_data->vtable;

    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    if (pStrides) {
        auto bindings = draw_state->GetVertexInputBindingDescriptionPtr();
        for (uint32_t i = 0; i < bindingCount; ++i) {
            uint32_t binding = firstBinding + i;

            // Find the binding
            for (uint32_t j = 0; j < draw_state->GetNumVertexInputBindingDescriptions(); ++j) {
                if (bindings[j].binding != binding) {
                    continue;
                }
                auto binding_description   = bindings[j];
                binding_description.stride = static_cast<uint32_t>(pStrides[i]);
                cmd_data->GetDrawStateData()->SetVertexInputBindingDescription(j, binding_description);
                break;
            }
        }
    }
    vtable.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                       const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                       uint32_t vertexAttributeDescriptionCount,
                                                       const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->vertex_input_dynamic.vertexInputDynamicState);
    FullDrawStateData* state = cmd_data->GetDrawStateData();

    state->SetNumVertexInputBindingDescriptions(vertexBindingDescriptionCount);
    for (uint32_t i = 0; i < vertexBindingDescriptionCount; ++i) {
        auto desc      = state->GetVertexInputBindingDescription(i);
        desc.binding   = pVertexBindingDescriptions[i].binding;
        desc.inputRate = pVertexBindingDescriptions[i].inputRate;
        desc.stride    = pVertexBindingDescriptions[i].stride;
        state->SetVertexInputBindingDescription(i, desc);
        ASSERT(pVertexBindingDescriptions[i].divisor == 1);
    }

    state->SetNumVertexInputAttributeDescriptions(vertexAttributeDescriptionCount);
    for (uint32_t i = 0; i < vertexAttributeDescriptionCount; ++i) {
        auto desc     = state->GetVertexInputAttributeDescription(i);
        desc.binding  = pVertexAttributeDescriptions[i].binding;
        desc.format   = pVertexAttributeDescriptions[i].format;
        desc.location = pVertexAttributeDescriptions[i].location;
        desc.offset   = pVertexAttributeDescriptions[i].offset;
        state->SetVertexInputAttributeDescription(i, desc);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto cmd_data    = GetCommandBufferData(commandBuffer);
    auto device_data = cmd_data->device_data;

    constexpr static VkPrimitiveTopology topology_class_to_common_table[] = {
        // point
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        // line
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        // triangle
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        // line
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        // triangle
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        // patch
        VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
    };

    if (device_data->extended_dynamic_state_1.extendedDynamicState == VK_TRUE) {
        ASSERT((device_data->flags & DeviceData::HAS_PRIMITIVE_TOPLOGY_UNRESTRICTED) == 0);

        // Collapse the primitive topology into a common primitive topology by class so that pipeline comparison is correct
        // All lines -> LINE_LIST, all triangles -> TRIANGLE_LIST, etc.
        cmd_data->GetDrawStateData()->SetPrimitiveTopology(topology_class_to_common_table[primitiveTopology]);
        device_data->vtable.CmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
    } else {
        cmd_data->GetDrawStateData()->SetPrimitiveTopology(primitiveTopology);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_2.extendedDynamicState2);
    cmd_data->GetDrawStateData()->SetPrimitiveRestartEnable(primitiveRestartEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_2.extendedDynamicState2);
    cmd_data->GetDrawStateData()->SetRasterizerDiscardEnable(rasterizerDiscardEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                                VkSampleCountFlagBits rasterizationSamples) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3RasterizationSamples);
    cmd_data->GetDrawStateData()->SetRasterizationSamples(rasterizationSamples);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3PolygonMode);
    cmd_data->GetDrawStateData()->SetPolygonMode(polygonMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetCullMode(cullMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetFrontFace(frontFace);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetDepthTestEnable(depthTestEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetDepthWriteEnable(depthWriteEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetDepthCompareOp(depthCompareOp);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetStencilTestEnable(stencilTestEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3LogicOpEnable);
    cmd_data->GetDrawStateData()->SetLogicOpEnable(logicOpEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount, const VkBool32* pColorBlendEnables) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ColorBlendEnable);
    FullDrawStateData* state = cmd_data->GetDrawStateData();
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        VkPipelineColorBlendAttachmentState blend_attachment_state = state->GetColorBlendAttachmentState(i + firstAttachment);
        blend_attachment_state.blendEnable = pColorBlendEnables[i];
        state->SetColorBlendAttachmentState(i + firstAttachment, blend_attachment_state);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                              uint32_t attachmentCount,
                                                              const VkColorBlendEquationEXT* pColorBlendEquations) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ColorBlendEquation);
    FullDrawStateData* state = cmd_data->GetDrawStateData();
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        VkPipelineColorBlendAttachmentState blend_attachment_state = state->GetColorBlendAttachmentState(i + firstAttachment);

        blend_attachment_state.srcColorBlendFactor = pColorBlendEquations[i].srcColorBlendFactor;
        blend_attachment_state.dstColorBlendFactor = pColorBlendEquations[i].dstColorBlendFactor;
        blend_attachment_state.colorBlendOp = pColorBlendEquations[i].colorBlendOp;
        blend_attachment_state.srcAlphaBlendFactor = pColorBlendEquations[i].srcAlphaBlendFactor;
        blend_attachment_state.dstAlphaBlendFactor = pColorBlendEquations[i].dstAlphaBlendFactor;
        blend_attachment_state.alphaBlendOp = pColorBlendEquations[i].alphaBlendOp;

        state->SetColorBlendAttachmentState(i + firstAttachment, blend_attachment_state);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ColorWriteMask);
    FullDrawStateData* state = cmd_data->GetDrawStateData();
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        VkPipelineColorBlendAttachmentState color_blend_attachment_state = state->GetColorBlendAttachmentState(i + firstAttachment);
        color_blend_attachment_state.colorWriteMask = pColorWriteMasks[i];

        state->SetColorBlendAttachmentState(i + firstAttachment, color_blend_attachment_state);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);
    cmd_data->GetDrawStateData()->SetDepthBoundsTestEnable(depthBoundsTestEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_2.extendedDynamicState2);
    cmd_data->GetDrawStateData()->SetDepthBiasEnable(depthBiasEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3DepthClampEnable);
    cmd_data->GetDrawStateData()->SetDepthClampEnable(depthClampEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                  VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_1.extendedDynamicState);

    FullDrawStateData* state = cmd_data->GetDrawStateData();
    VkStencilOpState stencil_op{};
    stencil_op.failOp = failOp;
    stencil_op.passOp = passOp;
    stencil_op.depthFailOp = depthFailOp;
    stencil_op.compareOp = compareOp;

    if (faceMask & VK_STENCIL_FACE_FRONT_BIT) {
        state->SetStencilFront(stencil_op);
    }
    if (faceMask & VK_STENCIL_FACE_BACK_BIT) {
        state->SetStencilBack(stencil_op);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_2.extendedDynamicState2LogicOp);
    cmd_data->GetDrawStateData()->SetLogicOp(logicOp);
}

static VKAPI_ATTR void VKAPI_CALL CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                          uint32_t firstVertex, uint32_t firstInstance) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                       stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                 uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                      uint32_t groupCountZ) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              uint32_t drawCount, uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkBuffer countBuffer,
                                                                   VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                   uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                   maxDrawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             uint32_t drawCount, uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    UpdateDrawState(*cmd_data, commandBuffer);
    cmd_data->device_data->vtable.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                  maxDrawCount, stride);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_2.extendedDynamicState2PatchControlPoints);
    cmd_data->GetDrawStateData()->SetPatchControlPoints(patchControlPoints);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                    VkTessellationDomainOrigin domainOrigin) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3TessellationDomainOrigin);
    cmd_data->GetDrawStateData()->SetDomainOrigin(domainOrigin);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                      const VkSampleMask* pSampleMask) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3SampleMask);
    uint32_t length = CalculateRequiredGroupSize(samples, 32);
    for (uint32_t i = 0; i < length; ++i) {
        cmd_data->GetDrawStateData()->SetSampleMask(i, pSampleMask[i]);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3AlphaToCoverageEnable);
    cmd_data->GetDrawStateData()->SetAlphaToCoverageEnable(alphaToCoverageEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3AlphaToOneEnable);
    cmd_data->GetDrawStateData()->SetAlphaToOneEnable(alphaToOneEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3RasterizationStream);
    ASSERT_VK_TRUE(cmd_data->device_data->transform_feedback.transformFeedback);
    cmd_data->GetDrawStateData()->SetRasterizationStream(rasterizationStream);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ConservativeRasterizationMode);
    cmd_data->GetDrawStateData()->SetConservativeRasterizationMode(conservativeRasterizationMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ExtraPrimitiveOverestimationSize);
    cmd_data->GetDrawStateData()->SetExtraPrimitiveOverestimationSize(extraPrimitiveOverestimationSize);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3DepthClipEnable);
    cmd_data->GetDrawStateData()->SetDepthClipEnable(depthClipEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3SampleLocationsEnable);
    cmd_data->GetDrawStateData()->SetSampleLocationsEnable(sampleLocationsEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ProvokingVertexMode);
    cmd_data->GetDrawStateData()->SetProvokingVertexMode(provokingVertexMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3LineRasterizationMode);
    cmd_data->GetDrawStateData()->SetLineRasterizationMode(lineRasterizationMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3LineStippleEnable);
    cmd_data->GetDrawStateData()->SetStippledLineEnable(stippledLineEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3DepthClipNegativeOneToOne);
    cmd_data->GetDrawStateData()->SetNegativeOneToOne(negativeOneToOne);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3CoverageModulationMode);
    cmd_data->GetDrawStateData()->SetCoverageModulationMode(coverageModulationMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3CoverageModulationTableEnable);
    cmd_data->GetDrawStateData()->SetCoverageModulationTableEnable(coverageModulationTableEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount, const float* pCoverageModulationTable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    auto draw_state          = cmd_data->GetDrawStateData();

    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3CoverageModulationTable);

    draw_state->SetCoverageModulationTableCount(coverageModulationTableCount);
    for (uint32_t i = 0; i < coverageModulationTableCount; ++i) {
        draw_state->SetCoverageModulationTableValues(i, pCoverageModulationTable[i]);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3CoverageReductionMode);
    cmd_data->GetDrawStateData()->SetCoverageReductionMode(coverageReductionMode);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3CoverageToColorEnable);
    cmd_data->GetDrawStateData()->SetCoverageToColorEnable(coverageToColorEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3CoverageToColorLocation);
    cmd_data->GetDrawStateData()->SetCoverageToColorLocation(coverageToColorLocation);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ViewportWScalingEnable);
    cmd_data->GetDrawStateData()->SetViewportWScalingEnable(viewportWScalingEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    auto draw_state          = cmd_data->GetDrawStateData();

    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ViewportSwizzle);

    draw_state->SetViewportSwizzleCount(firstViewport + viewportCount);
    for (uint32_t i = 0; i < viewportCount; ++i) {
        draw_state->SetViewportSwizzle(firstViewport + i, pViewportSwizzles[i]);
    }
}

static VKAPI_ATTR void VKAPI_CALL CmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3ShadingRateImageEnable);
    cmd_data->GetDrawStateData()->SetShadingRateImageEnable(shadingRateImageEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer, VkBool32 representativeFragmentTestEnable) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    ASSERT_VK_FALSE(cmd_data->device_data->extended_dynamic_state_3.extendedDynamicState3RepresentativeFragmentTestEnable);
    cmd_data->GetDrawStateData()->SetRepresentativeFragmentTestEnable(representativeFragmentTestEnable);
}

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                        const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                        const uint32_t* pDynamicOffsets) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        cmd_data->last_seen_pipeline_layout_ = layout;
    }
    cmd_data->device_data->vtable.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                                        pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                          VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                          const VkWriteDescriptorSet* pDescriptorWrites) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        cmd_data->last_seen_pipeline_layout_ = layout;
    }
    cmd_data->device_data->vtable.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                          pDescriptorWrites);
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                      VkPipelineLayout layout, uint32_t set, const void* pData) {
    auto cmd_data    = GetCommandBufferData(commandBuffer);
    auto device_data = cmd_data->device_data;
    if (GetDescriptorUpdateTemplateBindPoint(device_data, descriptorUpdateTemplate) == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        cmd_data->last_seen_pipeline_layout_ = layout;
    }
    device_data->vtable.CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                   VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                   const void* pValues) {
    auto cmd_data = GetCommandBufferData(commandBuffer);
    if (stageFlags & (VK_SHADER_STAGE_ALL_GRAPHICS)) {
        cmd_data->last_seen_pipeline_layout_ = layout;
    }
    cmd_data->device_data->vtable.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

static VKAPI_ATTR VkResult VKAPI_CALL SetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                     VkPrivateDataSlot privateDataSlot, uint64_t data) {
    auto device_data = device_data_map.Get(device);
    if (objectType != VK_OBJECT_TYPE_SHADER_EXT) {
        return device_data->vtable.SetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
    }

    auto shader = reinterpret_cast<Shader*>(objectHandle);
    shader->SetPrivateData(*device_data, privateDataSlot, data);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                 VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    auto device_data = device_data_map.Get(device);
    if (objectType != VK_OBJECT_TYPE_SHADER_EXT) {
        device_data->vtable.GetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
        return;
    }

    auto shader = reinterpret_cast<Shader*>(objectHandle);
    *pData = shader->GetPrivateData(*device_data, privateDataSlot);
}

static VKAPI_ATTR VkResult CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, 
                                                          VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    auto device_data = device_data_map.Get(device);
    VkResult result = device_data->vtable.CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    if (result == VK_SUCCESS) {
        SetDescriptorUpdateTemplateBindPoint(device_data, *pDescriptorUpdateTemplate, pCreateInfo->pipelineBindPoint);
    }
    return result;
}

static VKAPI_ATTR void DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, 
                                                       const VkAllocationCallbacks* pAllocator) {
    auto device_data = device_data_map.Get(device);
    device_data->vtable.DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    RemoveDescriptorUpdateTemplateBindPoint(device_data, descriptorUpdateTemplate);
}

static VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    DeviceData& data = *device_data_map.Get(device);
    VkResult result = VK_SUCCESS;
    if (pNameInfo->objectType == VK_OBJECT_TYPE_SHADER_EXT) {
        const auto shader = reinterpret_cast<Shader*>(pNameInfo->objectHandle);
        if (shader->stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            SetComputeShaderDebugUtilsName(data, shader, pNameInfo);
        } else {
            if (pNameInfo->pObjectName) {
                DeviceData::NameInfo objectName;
                strncpy(objectName.name, pNameInfo->pObjectName, sizeof(objectName.name) - 1);
                objectName.name[sizeof(objectName.name) - 1] = '\0';
                data.debug_utils_object_name_map.Add(shader, objectName);
            } else {
                data.debug_utils_object_name_map.Remove(shader);
            }
        }
    } else {
        result = data.vtable.SetDebugUtilsObjectNameEXT(device, pNameInfo);
    }
    return result;
}

static VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    DeviceData& data = *device_data_map.Get(device);
    VkResult result = VK_SUCCESS;
    if (pTagInfo->objectType == VK_OBJECT_TYPE_SHADER_EXT) {
        const auto shader = reinterpret_cast<Shader*>(pTagInfo->objectHandle);
        if (shader->stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            SetComputeShaderDebugUtilsTag(data, shader, pTagInfo);
        } else {
            DeviceData::TagInfo tagInfo;
            tagInfo.tagName = pTagInfo->tagName;
            size_t copySize = pTagInfo->tagSize < sizeof(tagInfo.tag) ? pTagInfo->tagSize : sizeof(tagInfo.tag) - 1;
            memcpy(tagInfo.tag, pTagInfo->pTag, copySize);
            tagInfo.tag[copySize] = '\0';
            data.debug_utils_object_tag_map.Add(shader, tagInfo);
        }
    } else {
        result = data.vtable.SetDebugUtilsObjectTagEXT(device, pTagInfo);
    }
    return result;
}

static VKAPI_ATTR void VKAPI_CALL FakeCmdSetColorBlendAdvancedEXT(VkCommandBuffer, uint32_t, uint32_t, const VkColorBlendAdvancedEXT*) {}

// Get Proc Addr

struct NameAndFunction {
    const char* name;
    void* func;
};

#define ENTRY_POINT_ALIAS(alias, canon) {"vk" #alias, (void*)shader_object::canon},
#define ENTRY_POINT(name) ENTRY_POINT_ALIAS(name, name)
static const NameAndFunction kFunctionMapInstance[] = {ENTRY_POINTS_INSTANCE ENTRY_POINTS_DEVICE};
static const NameAndFunction kFunctionMapDevice[] = {ENTRY_POINTS_DEVICE};
#undef ENTRY_POINT
#undef ENTRY_POINT_ALIAS

enum FunctionType { kInstanceFunctions = 0x1, kDeviceFunctions = 0x2 };

static void* FindFunctionByName(const char* pName, uint32_t functionType) {
    size_t num_elements = 0;
    const NameAndFunction* map = nullptr;

#define HANDLE_FUNCTION_TYPE(type)                                                           \
    if (map == nullptr && ((functionType & (k##type##Functions)) == (k##type##Functions))) { \
        num_elements = GetArrayLength(kFunctionMap##type);                                   \
        map = kFunctionMap##type;                                                            \
    }

    HANDLE_FUNCTION_TYPE(Instance);
    HANDLE_FUNCTION_TYPE(Device);

#undef HANDLE_FUNCTION_TYPE

    for (size_t i = 0; i < num_elements; ++i) {
        if (strcmp(pName, map[i].name) == 0) {
            return map[i].func;
        }
    }
    return nullptr;
}

void* DeviceData::FindStateSettingFunctionByName(const char* pName) {
    // check what features are enabled and return optimal functions
    // i.e. only intercept the functions that we have to emulate/are not supported by the device

#include "generated/shader_object_find_intercepted_dynamic_state_function_by_name.inl"

    // otherwise, intercept the function
    return FindFunctionByName(pName, kDeviceFunctions);
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName) {
    // See if this is a proc we want to intercept
    if (void* func = FindFunctionByName(pName, kInstanceFunctions | kDeviceFunctions)) {
        return reinterpret_cast<PFN_vkVoidFunction>(func);
    }
    // Otherwise, forward it to the next layer
    auto instance_data_it = instance_data_map.Find(instance);
    if (instance_data_it != instance_data_map.end()) {
        return instance_data_it.GetValue()->vtable.GetInstanceProcAddr(instance, pName);
    }
    return nullptr;
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName) {
    auto  device_data = device_data_map.Get(device);
    auto& vtable      = device_data->vtable;

    // See if this is a proc we want to intercept
    if (device_data->flags & DeviceData::SHADER_OBJECT_LAYER_ENABLED) {
        if (void* func = device_data->FindStateSettingFunctionByName(pName)) {
            return reinterpret_cast<PFN_vkVoidFunction>(func);
        }

        if (strcmp(pName, "vkCmdSetColorBlendAdvancedEXT") == 0) {
            // Special case. Even though it's illegal to call this function (because the extension and feature is missing), 
            // the layer must provide a non-null function pointer
            if (auto underlying_function = vtable.GetDeviceProcAddr(device, pName)) {
                return underlying_function;
            }
            return reinterpret_cast<PFN_vkVoidFunction>(shader_object::FakeCmdSetColorBlendAdvancedEXT);
        }
    } else {
        // Even if the layer isn't enabled for this device, destroy device still needs to be called to cleanup device data
        if (strcmp(pName, "vkDestroyDevice") == 0) {
            return reinterpret_cast<PFN_vkVoidFunction>(shader_object::DestroyDevice);
        }
    }

    // Otherwise, forward it to the next layer
    return vtable.GetDeviceProcAddr(device, pName);
}

}  // namespace shader_object

extern "C" VEL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    return shader_object::GetInstanceProcAddr(instance, pName);
}

extern "C" VEL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
    return shader_object::GetDeviceProcAddr(device, pName);
}
