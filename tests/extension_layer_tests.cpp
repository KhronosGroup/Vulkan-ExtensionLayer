/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Copyright (c) 2015-2023 Nvidia Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Vikram Kushwaha <vkushwaha@nvidia.com>
 */
#include "extension_layer_tests.h"
#include "vk_typemap_helper.h"

#if !defined(ANDROID)
#include "test_layer_location.h"
#endif

// Global list of sType,size identifiers
std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};

VkFormat FindSupportedDepthOnlyFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

VkFormat FindSupportedStencilOnlyFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice phy) {
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(phy, ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return ds_formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

bool ImageFormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (0 != (phy_features & features));
}

bool ImageFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(phy, format, &format_props);
    VkFormatFeatureFlags phy_features =
        (VK_IMAGE_TILING_OPTIMAL == tiling ? format_props.optimalTilingFeatures : format_props.linearTilingFeatures);
    return (features == (phy_features & features));
}

bool ImageFormatAndFeaturesSupported(const VkInstance inst, const VkPhysicalDevice phy, const VkImageCreateInfo info,
                                     const VkFormatFeatureFlags features) {
    // Verify physical device support of format features
    if (!ImageFormatAndFeaturesSupported(phy, info.format, info.tiling, features)) {
        return false;
    }

    // Verify that PhysDevImageFormatProp() also claims support for the specific usage
    VkImageFormatProperties props;
    VkResult err =
        vk::GetPhysicalDeviceImageFormatProperties(phy, info.format, info.imageType, info.tiling, info.usage, info.flags, &props);
    if (VK_SUCCESS != err) {
        return false;
    }

#if 0  // Convinced this chunk doesn't currently add any additional info, but leaving in place because it may be
       // necessary with future extensions

    // Verify again using version 2, if supported, which *can* return more property data than the original...
    // (It's not clear that this is any more definitive than using the original version - but no harm)
    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR p_GetPDIFP2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vk::GetInstanceProcAddr(inst,
                                                                                "vkGetPhysicalDeviceImageFormatProperties2KHR");
    if (NULL != p_GetPDIFP2KHR) {
        VkPhysicalDeviceImageFormatInfo2KHR fmt_info{};
        fmt_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
        fmt_info.pNext = nullptr;
        fmt_info.format = info.format;
        fmt_info.type = info.imageType;
        fmt_info.tiling = info.tiling;
        fmt_info.usage = info.usage;
        fmt_info.flags = info.flags;

        VkImageFormatProperties2KHR fmt_props = {};
        fmt_props.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
        err = p_GetPDIFP2KHR(phy, &fmt_info, &fmt_props);
        if (VK_SUCCESS != err) {
            return false;
        }
    }
#endif

    return true;
}

bool operator==(const VkDebugUtilsLabelEXT &rhs, const VkDebugUtilsLabelEXT &lhs) {
    bool is_equal = (rhs.color[0] == lhs.color[0]) && (rhs.color[1] == lhs.color[1]) && (rhs.color[2] == lhs.color[2]) &&
                    (rhs.color[3] == lhs.color[3]);
    if (is_equal) {
        if (rhs.pLabelName && lhs.pLabelName) {
            is_equal = (0 == strcmp(rhs.pLabelName, lhs.pLabelName));
        } else {
            is_equal = (rhs.pLabelName == nullptr) && (lhs.pLabelName == nullptr);
        }
    }
    return is_equal;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    auto *data = reinterpret_cast<DebugUtilsLabelCheckData *>(pUserData);
    data->callback(pCallbackData, data);
    return VK_FALSE;
}

bool CheckCreateRenderPass2Support(VkRenderFramework *renderFramework, std::vector<const char *> &device_extension_names) {
    if (renderFramework->DeviceExtensionSupported(renderFramework->gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)) {
        device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return true;
    }
    return false;
}

bool CheckDescriptorIndexingSupportAndInitFramework(VkRenderFramework *renderFramework,
                                                    std::vector<const char *> &instance_extension_names,
                                                    std::vector<const char *> &device_extension_names,
                                                    VkValidationFeaturesEXT *features, void *userData) {
    bool descriptor_indexing = renderFramework->InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (descriptor_indexing) {
        instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    renderFramework->InitFramework(userData, features);
    descriptor_indexing = descriptor_indexing && renderFramework->DeviceExtensionSupported(renderFramework->gpu(), nullptr,
                                                                                           VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    descriptor_indexing = descriptor_indexing && renderFramework->DeviceExtensionSupported(
                                                     renderFramework->gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (descriptor_indexing) {
        device_extension_names.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
        device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        return true;
    }
    return false;
}

bool CheckTimelineSemaphoreSupportAndInitState(VkRenderFramework *renderFramework) {
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(renderFramework->instance(),
                                                                     "vkGetPhysicalDeviceFeatures2KHR");
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&timeline_semaphore_features);
    vkGetPhysicalDeviceFeatures2KHR(renderFramework->gpu(), &features2);
    if (!timeline_semaphore_features.timelineSemaphore) {
        return false;
    }
    renderFramework->InitState(nullptr, &features2);
    return true;
}

bool VkExtensionLayerTest::CheckDecompressionSupportAndInitState() {
    bool is_api_version_12_or_above =
        VK_API_VERSION_MAJOR(m_instance_api_version) >= 1 && VK_API_VERSION_MINOR(m_instance_api_version) >= 2;
    if (!is_api_version_12_or_above) {
        // Decompression tests need Vulkan 1.2+
        return false;
    }

    bool decompressionExtensionFound = false;
    if (DeviceExtensionSupported(VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME, 1)) {
        decompressionExtensionFound = true;
        m_device_extension_names.push_back(VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME);
    }

    auto decompress_features = LvlInitStruct<VkPhysicalDeviceMemoryDecompressionFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&decompress_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    VkPhysicalDeviceFeatures2 devFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    VkPhysicalDeviceMemoryDecompressionFeaturesNV decompressionFeature = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV};
    devFeatures.pNext = &decompressionFeature;
    vk::GetPhysicalDeviceFeatures2(gpu(), &devFeatures);

    // Unsupported when neither the driver supports it nor the layer is present
    if (!decompressionFeature.memoryDecompression && !decompressionExtensionFound) {
        return false;
    }

    InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    vk::QueueSubmit2KHR = reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(device(), "vkQueueSubmit2KHR"));

    vk::CmdDecompressMemoryNV =
        reinterpret_cast<PFN_vkCmdDecompressMemoryNV>(vk::GetDeviceProcAddr(device(), "vkCmdDecompressMemoryNV"));
    vk::CmdDecompressMemoryIndirectCountNV = reinterpret_cast<PFN_vkCmdDecompressMemoryIndirectCountNV>(
        vk::GetDeviceProcAddr(device(), "vkCmdDecompressMemoryIndirectCountNV"));

    return true;
}

bool VkExtensionLayerTest::CheckShaderObjectSupportAndInitState() {
    if (!DeviceExtensionSupported(VK_EXT_SHADER_OBJECT_EXTENSION_NAME, 0)) {
        return false;
    }
    if (!DeviceExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, 0)) {
        return false;
    }
    if (!DeviceExtensionSupported(VK_KHR_MAINTENANCE2_EXTENSION_NAME, 0)) {
        return false;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);

    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();

    auto shader_object_features = LvlInitStruct<VkPhysicalDeviceShaderObjectFeaturesEXT>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>(&shader_object_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&dynamic_rendering_features);

    if (DeviceExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, 0)) {
        m_device_extension_names.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        shader_object_features.pNext = &mesh_shader_features;
    }

    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!shader_object_features.shaderObject || !dynamic_rendering_features.dynamicRendering) {
        return false;
    }
    InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk::CmdBindShadersEXT = reinterpret_cast<PFN_vkCmdBindShadersEXT>(vk::GetDeviceProcAddr(device(), "vkCmdBindShadersEXT"));
    vk::CreateShadersEXT = reinterpret_cast<PFN_vkCreateShadersEXT>(vk::GetDeviceProcAddr(device(), "vkCreateShadersEXT"));
    vk::DestroyShaderEXT = reinterpret_cast<PFN_vkDestroyShaderEXT>(vk::GetDeviceProcAddr(device(), "vkDestroyShaderEXT"));
    vk::GetShaderBinaryDataEXT = reinterpret_cast<PFN_vkGetShaderBinaryDataEXT>(vk::GetDeviceProcAddr(device(), "vkGetShaderBinaryDataEXT"));

    return true;
}

bool VkExtensionLayerTest::CheckSynchronization2SupportAndInitState() {
    if (DeviceExtensionSupported(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, 0)) {
        m_device_extension_names.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    }
    if (DeviceExtensionSupported(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, 0)) {
        m_device_extension_names.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    }
    if (DeviceExtensionSupported(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, 0)) {
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        // implicit required extension
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    }

    auto timeline_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&timeline_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&sync2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!sync2_features.synchronization2) {
        return false;
    }
    InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk::GetSemaphoreCounterValueKHR =
        reinterpret_cast<PFN_vkGetSemaphoreCounterValueKHR>(vk::GetDeviceProcAddr(device(), "vkGetSemaphoreCounterValueKHR"));
    vk::WaitSemaphoresKHR = reinterpret_cast<PFN_vkWaitSemaphoresKHR>(vk::GetDeviceProcAddr(device(), "vkWaitSemaphoresKHR"));
    vk::SignalSemaphoreKHR = reinterpret_cast<PFN_vkSignalSemaphoreKHR>(vk::GetDeviceProcAddr(device(), "vkSignalSemaphoreKHR"));
    vk::CmdSetEvent2KHR = reinterpret_cast<PFN_vkCmdSetEvent2KHR>(vk::GetDeviceProcAddr(device(), "vkCmdSetEvent2KHR"));
    vk::CmdResetEvent2KHR = reinterpret_cast<PFN_vkCmdResetEvent2KHR>(vk::GetDeviceProcAddr(device(), "vkCmdResetEvent2KHR"));
    vk::CmdWaitEvents2KHR = reinterpret_cast<PFN_vkCmdWaitEvents2KHR>(vk::GetDeviceProcAddr(device(), "vkCmdWaitEvents2KHR"));
    vk::CmdPipelineBarrier2KHR =
        reinterpret_cast<PFN_vkCmdPipelineBarrier2KHR>(vk::GetDeviceProcAddr(device(), "vkCmdPipelineBarrier2KHR"));
    vk::CmdWriteTimestamp2KHR =
        reinterpret_cast<PFN_vkCmdWriteTimestamp2KHR>(vk::GetDeviceProcAddr(device(), "vkCmdWriteTimestamp2KHR"));
    vk::QueueSubmit2KHR = reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(device(), "vkQueueSubmit2KHR"));
    vk::CmdWriteBufferMarker2AMD =
        reinterpret_cast<PFN_vkCmdWriteBufferMarker2AMD>(vk::GetDeviceProcAddr(device(), "vkCmdWriteBufferMarker2AMD"));
    vk::GetQueueCheckpointData2NV =
        reinterpret_cast<PFN_vkGetQueueCheckpointData2NV>(vk::GetDeviceProcAddr(device(), "vkGetQueueCheckpointData2NV"));

    // Since signature are the same, make simple and override the core version if device in not 1.2

    if (DeviceValidationVersion() <= VK_API_VERSION_1_2) {
        vk::CmdBeginRenderPass2 =
            reinterpret_cast<PFN_vkCmdBeginRenderPass2>(vk::GetDeviceProcAddr(device(), "vkCmdBeginRenderPass2KHR"));
        vk::CmdEndRenderPass2 =
            reinterpret_cast<PFN_vkCmdEndRenderPass2>(vk::GetDeviceProcAddr(device(), "vkCmdEndRenderPass2KHR"));
        vk::CmdNextSubpass2 = reinterpret_cast<PFN_vkCmdNextSubpass2>(vk::GetDeviceProcAddr(device(), "vkCmdNextSubpass2KHR"));
        vk::CreateRenderPass2 =
            reinterpret_cast<PFN_vkCreateRenderPass2>(vk::GetDeviceProcAddr(device(), "vkCreateRenderPass2KHR"));
    }
    vk::CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vk::GetDeviceProcAddr(device(), "vkCreateSwapchainKHR"));
    vk::GetSwapchainImagesKHR =
        reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vk::GetDeviceProcAddr(device(), "vkGetSwapchainImagesKHR"));
    vk::DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vk::GetDeviceProcAddr(device(), "vkDestroySwapchainKHR"));
    vk::AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vk::GetDeviceProcAddr(device(), "vkAcquireNextImageKHR"));

    return true;
}

void VkExtensionLayerTest::Init(VkPhysicalDeviceFeatures *features, VkPhysicalDeviceFeatures2 *features2,
                                const VkCommandPoolCreateFlags flags, void *instance_pnext) {
    InitFramework(m_errorMonitor, instance_pnext);
    InitState(features, features2, flags);
}

VkCommandBufferObj *VkExtensionLayerTest::CommandBuffer() { return m_commandBuffer; }

VkExtensionLayerTest::VkExtensionLayerTest() {
    m_enableWSI = false;

    // Add default instance extensions to the list
    instance_extensions_.push_back(debug_reporter_.debug_extension_name);
    instance_extensions_.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    app_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info_.pNext = NULL;
    app_info_.pApplicationName = "layer_tests";
    app_info_.applicationVersion = 1;
    app_info_.pEngineName = "unittest";
    app_info_.engineVersion = 1;
    app_info_.apiVersion = VK_API_VERSION_1_0;

    // Find out what version the instance supports and record the default target instance
    auto enumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vk::GetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
    if (enumerateInstanceVersion) {
        enumerateInstanceVersion(&m_instance_api_version);
    } else {
        m_instance_api_version = VK_API_VERSION_1_0;
    }
    m_target_api_version = app_info_.apiVersion;
}

bool VkExtensionLayerTest::AddSurfaceInstanceExtension() {
    m_enableWSI = true;
    if (!InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_SURFACE_EXTENSION_NAME);
        return false;
    }
    instance_extensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    bool bSupport = false;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!InstanceExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        return false;
    }
    instance_extensions_.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!InstanceExtensionSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
        return false;
    }
    instance_extensions_.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!InstanceExtensionSupported(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        return false;
    }
    if (XOpenDisplay(NULL)) {
        instance_extensions_.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        bSupport = true;
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (!InstanceExtensionSupported(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        return false;
    }
    if (!bSupport && xcb_connect(NULL, NULL)) {
        instance_extensions_.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        bSupport = true;
    }
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK)
    if (!InstanceExtensionSupported(VK_MVK_MACOS_SURFACE_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
        return false;
    }
    if (InstanceExtensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        instance_extensions_.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    instance_extensions_.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
    bSupport = true;
#endif

    if (bSupport) return true;
    printf("%s No platform's surface extension supported\n", kSkipPrefix);
    return false;
}

bool VkExtensionLayerTest::AddSwapchainDeviceExtension() {
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        printf("%s %s extension not supported\n", kSkipPrefix, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        return false;
    }
    m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    return true;
}

uint32_t VkExtensionLayerTest::SetTargetApiVersion(uint32_t target_api_version) {
    if (target_api_version == 0) target_api_version = VK_API_VERSION_1_0;
    if (target_api_version <= m_instance_api_version) {
        m_target_api_version = target_api_version;
        app_info_.apiVersion = m_target_api_version;
    }
    return m_target_api_version;
}

uint32_t VkExtensionLayerTest::DeviceValidationVersion() {
    // The validation layers assume the version we are validating to is the apiVersion unless the device apiVersion is lower
    return std::min(m_target_api_version, physDevProps().apiVersion);
}

#if defined(ANDROID)
const char *appTag = "VulkanExtensionLayerTests";
static bool initialized = false;
static bool active = false;

// Convert Intents to argv
// Ported from Hologram sample, only difference is flexible key
std::vector<std::string> get_args(android_app &app, const char *intent_extra_data_key) {
    std::vector<std::string> args;
    JavaVM &vm = *app.activity->vm;
    JNIEnv *p_env;
    if (vm.AttachCurrentThread(&p_env, nullptr) != JNI_OK) return args;

    JNIEnv &env = *p_env;
    jobject activity = app.activity->clazz;
    jmethodID get_intent_method = env.GetMethodID(env.GetObjectClass(activity), "getIntent", "()Landroid/content/Intent;");
    jobject intent = env.CallObjectMethod(activity, get_intent_method);
    jmethodID get_string_extra_method =
        env.GetMethodID(env.GetObjectClass(intent), "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
    jvalue get_string_extra_args;
    get_string_extra_args.l = env.NewStringUTF(intent_extra_data_key);
    jstring extra_str = static_cast<jstring>(env.CallObjectMethodA(intent, get_string_extra_method, &get_string_extra_args));

    std::string args_str;
    if (extra_str) {
        const char *extra_utf = env.GetStringUTFChars(extra_str, nullptr);
        args_str = extra_utf;
        env.ReleaseStringUTFChars(extra_str, extra_utf);
        env.DeleteLocalRef(extra_str);
    }

    env.DeleteLocalRef(get_string_extra_args.l);
    env.DeleteLocalRef(intent);
    vm.DetachCurrentThread();

    // split args_str
    std::stringstream ss(args_str);
    std::string arg;
    while (std::getline(ss, arg, ' ')) {
        if (!arg.empty()) args.push_back(arg);
    }

    return args;
}

void addFullTestCommentIfPresent(const ::testing::TestInfo &test_info, std::string &error_message) {
    const char *const type_param = test_info.type_param();
    const char *const value_param = test_info.value_param();

    if (type_param != NULL || value_param != NULL) {
        error_message.append(", where ");
        if (type_param != NULL) {
            error_message.append("TypeParam = ").append(type_param);
            if (value_param != NULL) error_message.append(" and ");
        }
        if (value_param != NULL) {
            error_message.append("GetParam() = ").append(value_param);
        }
    }
}

class LogcatPrinter : public ::testing::EmptyTestEventListener {
    // Called before a test starts.
    virtual void OnTestStart(const ::testing::TestInfo &test_info) {
        __android_log_print(ANDROID_LOG_INFO, appTag, "[ RUN      ] %s.%s", test_info.test_case_name(), test_info.name());
    }

    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(const ::testing::TestPartResult &result) {
        // If the test part succeeded, we don't need to do anything.
        if (result.type() == ::testing::TestPartResult::kSuccess) return;

        __android_log_print(ANDROID_LOG_INFO, appTag, "%s in %s:%d %s", result.failed() ? "*** Failure" : "Success",
                            result.file_name(), result.line_number(), result.summary());
    }

    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo &info) {
        std::string result;
        if (info.result()->Passed()) {
            result.append("[       OK ]");
        } else {
            result.append("[  FAILED  ]");
        }
        result.append(info.test_case_name()).append(".").append(info.name());
        if (info.result()->Failed()) addFullTestCommentIfPresent(info, result);

        if (::testing::GTEST_FLAG(print_time)) {
            std::ostringstream os;
            os << info.result()->elapsed_time();
            result.append(" (").append(os.str()).append(" ms)");
        }

        __android_log_print(ANDROID_LOG_INFO, appTag, "%s", result.c_str());
    };
};

static int32_t processInput(struct android_app *app, AInputEvent *event) { return 0; }

static void processCommand(struct android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            if (app->window) {
                initialized = true;
                VkTestFramework::window = app->window;
            }
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            active = true;
            break;
        }
        case APP_CMD_LOST_FOCUS: {
            active = false;
            break;
        }
    }
}

void android_main(struct android_app *app) {
    app->onAppCmd = processCommand;
    app->onInputEvent = processInput;

    while (1) {
        int events;
        struct android_poll_source *source;
        while (ALooper_pollAll(active ? 0 : -1, NULL, &events, (void **)&source) >= 0) {
            if (source) {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) {
                VkTestFramework::Finish();
                return;
            }
        }

        if (initialized && active) {
            // Use the following key to send arguments to gtest, i.e.
            // --es args "--gtest_filter=-VkExtensionLayerTest.foo"
            const char key[] = "args";
            std::vector<std::string> args = get_args(*app, key);

            std::string filter = "";
            if (args.size() > 0) {
                __android_log_print(ANDROID_LOG_INFO, appTag, "Intent args = %s", args[0].c_str());
                filter += args[0];
            } else {
                __android_log_print(ANDROID_LOG_INFO, appTag, "No Intent args detected");
            }

            int argc = 2;
            char *argv[] = {(char *)"foo", (char *)filter.c_str()};
            __android_log_print(ANDROID_LOG_DEBUG, appTag, "filter = %s", argv[1]);

            // Route output to files until we can override the gtest output
            freopen("/sdcard/Android/data/com.example.VulkanExtensionLayerTests/files/out.txt", "w", stdout);
            freopen("/sdcard/Android/data/com.example.VulkanExtensionLayerTests/files/err.txt", "w", stderr);

            ::testing::InitGoogleTest(&argc, argv);

            ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
            listeners.Append(new LogcatPrinter);

            VkTestFramework::InitArgs(&argc, argv);
            ::testing::AddGlobalTestEnvironment(new TestEnvironment);

            int result = RUN_ALL_TESTS();

            if (result != 0) {
                __android_log_print(ANDROID_LOG_INFO, appTag, "==== Tests FAILED ====");
            } else {
                __android_log_print(ANDROID_LOG_INFO, appTag, "==== Tests PASSED ====");
            }

            VkTestFramework::Finish();

            fclose(stdout);
            fclose(stderr);

            ANativeActivity_finish(app->activity);
            return;
        }
    }
}
#endif

#if defined(_WIN32) && !defined(NDEBUG)
#include <crtdbg.h>
#endif

#if !defined(ANDROID)
int main(int argc, char **argv) {
    int result;

#if defined(_WIN32)
#if !defined(NDEBUG)
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    // Avoid "Abort, Retry, Ignore" dialog boxes
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

    // Set VK_LAYER_PATH so that the loader can find the layers
    SetEnvironment("VK_LAYER_PATH", LAYER_BUILD_LOCATION);

    ::testing::InitGoogleTest(&argc, argv);
    VkTestFramework::InitArgs(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment);

    result = RUN_ALL_TESTS();

    VkTestFramework::Finish();
    return result;
}
#endif
