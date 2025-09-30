// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See shader_object_generator.py for modifications

/*
 * Copyright 2023-2025 Nintendo
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

VkBaseOutStructure* appended_features_chain = nullptr;
VkBaseOutStructure* appended_features_chain_last = nullptr;

auto vulkan_1_3_ptr = reinterpret_cast<VkPhysicalDeviceVulkan13Features*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES));

auto dynamic_rendering_ptr = reinterpret_cast<VkPhysicalDeviceDynamicRenderingFeatures*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES));
VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
if (vulkan_1_3_ptr == nullptr && dynamic_rendering_ptr == nullptr && (physical_device_data->supported_additional_extensions & DYNAMIC_RENDERING) != 0) {
    dynamic_rendering_ptr = &dynamic_rendering_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)dynamic_rendering_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)dynamic_rendering_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto private_data_ptr = reinterpret_cast<VkPhysicalDevicePrivateDataFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES));
VkPhysicalDevicePrivateDataFeaturesEXT private_data_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES};
if (vulkan_1_3_ptr == nullptr && private_data_ptr == nullptr && (physical_device_data->supported_additional_extensions & PRIVATE_DATA) != 0) {
    private_data_ptr = &private_data_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)private_data_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)private_data_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto extended_dynamic_state_1_ptr = reinterpret_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT));
VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_1_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
if (extended_dynamic_state_1_ptr == nullptr && (physical_device_data->supported_additional_extensions & EXTENDED_DYNAMIC_STATE_1) != 0) {
    extended_dynamic_state_1_ptr = &extended_dynamic_state_1_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)extended_dynamic_state_1_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)extended_dynamic_state_1_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto extended_dynamic_state_2_ptr = reinterpret_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT));
VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state_2_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT};
if (extended_dynamic_state_2_ptr == nullptr && (physical_device_data->supported_additional_extensions & EXTENDED_DYNAMIC_STATE_2) != 0) {
    extended_dynamic_state_2_ptr = &extended_dynamic_state_2_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)extended_dynamic_state_2_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)extended_dynamic_state_2_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto extended_dynamic_state_3_ptr = reinterpret_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT));
VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_3_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT};
if (extended_dynamic_state_3_ptr == nullptr && (physical_device_data->supported_additional_extensions & EXTENDED_DYNAMIC_STATE_3) != 0) {
    extended_dynamic_state_3_ptr = &extended_dynamic_state_3_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)extended_dynamic_state_3_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)extended_dynamic_state_3_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto vertex_input_dynamic_ptr = reinterpret_cast<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT));
VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT};
if (vertex_input_dynamic_ptr == nullptr && (physical_device_data->supported_additional_extensions & VERTEX_INPUT_DYNAMIC) != 0) {
    vertex_input_dynamic_ptr = &vertex_input_dynamic_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)vertex_input_dynamic_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)vertex_input_dynamic_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto graphics_pipeline_library_ptr = reinterpret_cast<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT));
VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphics_pipeline_library_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT};
if (graphics_pipeline_library_ptr == nullptr && (physical_device_data->supported_additional_extensions & GRAPHICS_PIPELINE_LIBRARY) != 0) {
    graphics_pipeline_library_ptr = &graphics_pipeline_library_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)graphics_pipeline_library_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)graphics_pipeline_library_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto dynamic_rendering_unused_attachments_ptr = reinterpret_cast<VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT));
VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamic_rendering_unused_attachments_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT};
if (dynamic_rendering_unused_attachments_ptr == nullptr && (physical_device_data->supported_additional_extensions & DYNAMIC_RENDERING_UNUSED_ATTACHMENTS) != 0) {
    dynamic_rendering_unused_attachments_ptr = &dynamic_rendering_unused_attachments_local;
    if (appended_features_chain_last == nullptr) {
        appended_features_chain = (VkBaseOutStructure*)dynamic_rendering_unused_attachments_ptr;
        appended_features_chain_last = appended_features_chain;
    } else {
        appended_features_chain_last->pNext = (VkBaseOutStructure*)dynamic_rendering_unused_attachments_ptr;
        appended_features_chain_last = appended_features_chain_last->pNext;
    }
}
auto transform_feedback_ptr = reinterpret_cast<VkPhysicalDeviceTransformFeedbackFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT));
VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT};
if (transform_feedback_ptr == nullptr) {
    transform_feedback_ptr = &transform_feedback_local;
}
auto depth_clip_enable_ptr = reinterpret_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT));
VkPhysicalDeviceDepthClipEnableFeaturesEXT depth_clip_enable_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT};
if (depth_clip_enable_ptr == nullptr) {
    depth_clip_enable_ptr = &depth_clip_enable_local;
}
auto provoking_vertex_ptr = reinterpret_cast<VkPhysicalDeviceProvokingVertexFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT));
VkPhysicalDeviceProvokingVertexFeaturesEXT provoking_vertex_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT};
if (provoking_vertex_ptr == nullptr) {
    provoking_vertex_ptr = &provoking_vertex_local;
}
auto line_rasterization_ptr = reinterpret_cast<VkPhysicalDeviceLineRasterizationFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT));
VkPhysicalDeviceLineRasterizationFeaturesEXT line_rasterization_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT};
if (line_rasterization_ptr == nullptr) {
    line_rasterization_ptr = &line_rasterization_local;
}
auto depth_clip_control_ptr = reinterpret_cast<VkPhysicalDeviceDepthClipControlFeaturesEXT*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT));
VkPhysicalDeviceDepthClipControlFeaturesEXT depth_clip_control_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT};
if (depth_clip_control_ptr == nullptr) {
    depth_clip_control_ptr = &depth_clip_control_local;
}
auto nv_coverage_reduction_mode_ptr = reinterpret_cast<VkPhysicalDeviceCoverageReductionModeFeaturesNV*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV));
VkPhysicalDeviceCoverageReductionModeFeaturesNV nv_coverage_reduction_mode_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV};
if (nv_coverage_reduction_mode_ptr == nullptr) {
    nv_coverage_reduction_mode_ptr = &nv_coverage_reduction_mode_local;
}
auto nv_shading_rate_image_ptr = reinterpret_cast<VkPhysicalDeviceShadingRateImageFeaturesNV*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV));
VkPhysicalDeviceShadingRateImageFeaturesNV nv_shading_rate_image_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV};
if (nv_shading_rate_image_ptr == nullptr) {
    nv_shading_rate_image_ptr = &nv_shading_rate_image_local;
}
auto nv_representative_fragment_test_ptr = reinterpret_cast<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV));
VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV nv_representative_fragment_test_local{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV};
if (nv_representative_fragment_test_ptr == nullptr) {
    nv_representative_fragment_test_ptr = &nv_representative_fragment_test_local;
}

