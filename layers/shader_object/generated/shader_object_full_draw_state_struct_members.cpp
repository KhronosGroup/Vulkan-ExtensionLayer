// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See shader_object_generator.py for modifications

/*
 * Copyright 2023 Nintendo
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

#include "shader_object/shader_object_structs.h"

namespace shader_object {

void FullDrawStateData::SetDepthAttachmentFormat(VkFormat const& element) {
    if (element == depth_attachment_format_) {
        return;
    }
    dirty_hash_bits_.set(MISC);
    MarkDirty();
    depth_attachment_format_ = element;
}
VkFormat const& FullDrawStateData::GetDepthAttachmentFormat() const {
    return depth_attachment_format_;
}

void FullDrawStateData::SetStencilAttachmentFormat(VkFormat const& element) {
    if (element == stencil_attachment_format_) {
        return;
    }
    dirty_hash_bits_.set(MISC);
    MarkDirty();
    stencil_attachment_format_ = element;
}
VkFormat const& FullDrawStateData::GetStencilAttachmentFormat() const {
    return stencil_attachment_format_;
}

void FullDrawStateData::SetColorAttachmentFormat(uint32_t index, VkFormat const& element) {
    if (element == color_attachment_formats_[index]) {
        return;
    }
    dirty_hash_bits_.set(MISC);
    MarkDirty();
    color_attachment_formats_[index] = element;
}
VkFormat const& FullDrawStateData::GetColorAttachmentFormat(uint32_t index) const {
    return color_attachment_formats_[index];
}
VkFormat const* FullDrawStateData::GetColorAttachmentFormatPtr() const {
    return color_attachment_formats_;
}

void FullDrawStateData::SetNumColorAttachments(uint32_t const& element) {
    if (element == num_color_attachments_) {
        return;
    }
    dirty_hash_bits_.set(MISC);
    MarkDirty();
    num_color_attachments_ = element;
}
uint32_t const& FullDrawStateData::GetNumColorAttachments() const {
    return num_color_attachments_;
}

void FullDrawStateData::SetColorBlendAttachmentState(uint32_t index, VkPipelineColorBlendAttachmentState const& element) {
    if (element == color_blend_attachment_states_[index]) {
        return;
    }
    dirty_hash_bits_.set(MISC);
    MarkDirty();
    color_blend_attachment_states_[index] = element;
}
VkPipelineColorBlendAttachmentState const& FullDrawStateData::GetColorBlendAttachmentState(uint32_t index) const {
    return color_blend_attachment_states_[index];
}
VkPipelineColorBlendAttachmentState const* FullDrawStateData::GetColorBlendAttachmentStatePtr() const {
    return color_blend_attachment_states_;
}

void FullDrawStateData::SetComparableShader(uint32_t index, ComparableShader const& element) {
    if (element == comparable_shaders_[index]) {
        return;
    }
    dirty_hash_bits_.set(MISC);
    MarkDirty();
    comparable_shaders_[index] = element;
}
ComparableShader const& FullDrawStateData::GetComparableShader(uint32_t index) const {
    return comparable_shaders_[index];
}
ComparableShader const* FullDrawStateData::GetComparableShaderPtr() const {
    return comparable_shaders_;
}

void FullDrawStateData::SetCullMode(VkCullModeFlags const& element) {
    if (element == cull_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    cull_mode_ = element;
}
VkCullModeFlags const& FullDrawStateData::GetCullMode() const {
    return cull_mode_;
}

void FullDrawStateData::SetDepthBoundsTestEnable(VkBool32 const& element) {
    if (element == depth_bounds_test_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    depth_bounds_test_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetDepthBoundsTestEnable() const {
    return depth_bounds_test_enable_;
}

void FullDrawStateData::SetDepthCompareOp(VkCompareOp const& element) {
    if (element == depth_compare_op_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    depth_compare_op_ = element;
}
VkCompareOp const& FullDrawStateData::GetDepthCompareOp() const {
    return depth_compare_op_;
}

void FullDrawStateData::SetDepthTestEnable(VkBool32 const& element) {
    if (element == depth_test_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    depth_test_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetDepthTestEnable() const {
    return depth_test_enable_;
}

void FullDrawStateData::SetDepthWriteEnable(VkBool32 const& element) {
    if (element == depth_write_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    depth_write_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetDepthWriteEnable() const {
    return depth_write_enable_;
}

void FullDrawStateData::SetFrontFace(VkFrontFace const& element) {
    if (element == front_face_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    front_face_ = element;
}
VkFrontFace const& FullDrawStateData::GetFrontFace() const {
    return front_face_;
}

void FullDrawStateData::SetPrimitiveTopology(VkPrimitiveTopology const& element) {
    if (element == primitive_topology_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    primitive_topology_ = element;
}
VkPrimitiveTopology const& FullDrawStateData::GetPrimitiveTopology() const {
    return primitive_topology_;
}

void FullDrawStateData::SetNumScissors(uint32_t const& element) {
    if (element == num_scissors_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    num_scissors_ = element;
}
uint32_t const& FullDrawStateData::GetNumScissors() const {
    return num_scissors_;
}

void FullDrawStateData::SetNumViewports(uint32_t const& element) {
    if (element == num_viewports_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    num_viewports_ = element;
}
uint32_t const& FullDrawStateData::GetNumViewports() const {
    return num_viewports_;
}

void FullDrawStateData::SetStencilFront(VkStencilOpState const& element) {
    if (element == stencil_front_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    stencil_front_ = element;
}
VkStencilOpState const& FullDrawStateData::GetStencilFront() const {
    return stencil_front_;
}

void FullDrawStateData::SetStencilBack(VkStencilOpState const& element) {
    if (element == stencil_back_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    stencil_back_ = element;
}
VkStencilOpState const& FullDrawStateData::GetStencilBack() const {
    return stencil_back_;
}

void FullDrawStateData::SetStencilTestEnable(VkBool32 const& element) {
    if (element == stencil_test_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
    MarkDirty();
    stencil_test_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetStencilTestEnable() const {
    return stencil_test_enable_;
}

void FullDrawStateData::SetLogicOp(VkLogicOp const& element) {
    if (element == logic_op_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
    MarkDirty();
    logic_op_ = element;
}
VkLogicOp const& FullDrawStateData::GetLogicOp() const {
    return logic_op_;
}

void FullDrawStateData::SetPrimitiveRestartEnable(VkBool32 const& element) {
    if (element == primitive_restart_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
    MarkDirty();
    primitive_restart_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetPrimitiveRestartEnable() const {
    return primitive_restart_enable_;
}

void FullDrawStateData::SetRasterizerDiscardEnable(VkBool32 const& element) {
    if (element == rasterizer_discard_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
    MarkDirty();
    rasterizer_discard_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetRasterizerDiscardEnable() const {
    return rasterizer_discard_enable_;
}

void FullDrawStateData::SetDepthBiasEnable(VkBool32 const& element) {
    if (element == depth_bias_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
    MarkDirty();
    depth_bias_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetDepthBiasEnable() const {
    return depth_bias_enable_;
}

void FullDrawStateData::SetPatchControlPoints(uint32_t const& element) {
    if (element == patch_control_points_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
    MarkDirty();
    patch_control_points_ = element;
}
uint32_t const& FullDrawStateData::GetPatchControlPoints() const {
    return patch_control_points_;
}

void FullDrawStateData::SetPolygonMode(VkPolygonMode const& element) {
    if (element == polygon_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    polygon_mode_ = element;
}
VkPolygonMode const& FullDrawStateData::GetPolygonMode() const {
    return polygon_mode_;
}

void FullDrawStateData::SetRasterizationSamples(VkSampleCountFlagBits const& element) {
    if (element == rasterization_samples_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    rasterization_samples_ = element;
}
VkSampleCountFlagBits const& FullDrawStateData::GetRasterizationSamples() const {
    return rasterization_samples_;
}

void FullDrawStateData::SetLogicOpEnable(VkBool32 const& element) {
    if (element == logic_op_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    logic_op_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetLogicOpEnable() const {
    return logic_op_enable_;
}

void FullDrawStateData::SetDepthClampEnable(VkBool32 const& element) {
    if (element == depth_clamp_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    depth_clamp_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetDepthClampEnable() const {
    return depth_clamp_enable_;
}

void FullDrawStateData::SetDomainOrigin(VkTessellationDomainOrigin const& element) {
    if (element == domain_origin_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    domain_origin_ = element;
}
VkTessellationDomainOrigin const& FullDrawStateData::GetDomainOrigin() const {
    return domain_origin_;
}

void FullDrawStateData::SetAlphaToOneEnable(VkBool32 const& element) {
    if (element == alpha_to_one_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    alpha_to_one_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetAlphaToOneEnable() const {
    return alpha_to_one_enable_;
}

void FullDrawStateData::SetAlphaToCoverageEnable(VkBool32 const& element) {
    if (element == alpha_to_coverage_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    alpha_to_coverage_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetAlphaToCoverageEnable() const {
    return alpha_to_coverage_enable_;
}

void FullDrawStateData::SetSampleMask(uint32_t index, VkSampleMask const& element) {
    if (element == sample_masks_[index]) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    sample_masks_[index] = element;
}
VkSampleMask const& FullDrawStateData::GetSampleMask(uint32_t index) const {
    return sample_masks_[index];
}
VkSampleMask const* FullDrawStateData::GetSampleMaskPtr() const {
    return sample_masks_;
}

void FullDrawStateData::SetRasterizationStream(uint32_t const& element) {
    if (element == rasterization_stream_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    rasterization_stream_ = element;
}
uint32_t const& FullDrawStateData::GetRasterizationStream() const {
    return rasterization_stream_;
}

void FullDrawStateData::SetConservativeRasterizationMode(VkConservativeRasterizationModeEXT const& element) {
    if (element == conservative_rasterization_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    conservative_rasterization_mode_ = element;
}
VkConservativeRasterizationModeEXT const& FullDrawStateData::GetConservativeRasterizationMode() const {
    return conservative_rasterization_mode_;
}

void FullDrawStateData::SetExtraPrimitiveOverestimationSize(float const& element) {
    if (element == extra_primitive_overestimation_size_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    extra_primitive_overestimation_size_ = element;
}
float const& FullDrawStateData::GetExtraPrimitiveOverestimationSize() const {
    return extra_primitive_overestimation_size_;
}

void FullDrawStateData::SetDepthClipEnable(VkBool32 const& element) {
    if (element == depth_clip_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    depth_clip_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetDepthClipEnable() const {
    return depth_clip_enable_;
}

void FullDrawStateData::SetSampleLocationsEnable(VkBool32 const& element) {
    if (element == sample_locations_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    sample_locations_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetSampleLocationsEnable() const {
    return sample_locations_enable_;
}

void FullDrawStateData::SetProvokingVertexMode(VkProvokingVertexModeEXT const& element) {
    if (element == provoking_vertex_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    provoking_vertex_mode_ = element;
}
VkProvokingVertexModeEXT const& FullDrawStateData::GetProvokingVertexMode() const {
    return provoking_vertex_mode_;
}

void FullDrawStateData::SetLineRasterizationMode(VkLineRasterizationModeEXT const& element) {
    if (element == line_rasterization_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    line_rasterization_mode_ = element;
}
VkLineRasterizationModeEXT const& FullDrawStateData::GetLineRasterizationMode() const {
    return line_rasterization_mode_;
}

void FullDrawStateData::SetStippledLineEnable(VkBool32 const& element) {
    if (element == stippled_line_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    stippled_line_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetStippledLineEnable() const {
    return stippled_line_enable_;
}

void FullDrawStateData::SetNegativeOneToOne(VkBool32 const& element) {
    if (element == negative_one_to_one_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    negative_one_to_one_ = element;
}
VkBool32 const& FullDrawStateData::GetNegativeOneToOne() const {
    return negative_one_to_one_;
}

void FullDrawStateData::SetCoverageModulationMode(VkCoverageModulationModeNV const& element) {
    if (element == coverage_modulation_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_modulation_mode_ = element;
}
VkCoverageModulationModeNV const& FullDrawStateData::GetCoverageModulationMode() const {
    return coverage_modulation_mode_;
}

void FullDrawStateData::SetCoverageModulationTableEnable(VkBool32 const& element) {
    if (element == coverage_modulation_table_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_modulation_table_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetCoverageModulationTableEnable() const {
    return coverage_modulation_table_enable_;
}

void FullDrawStateData::SetCoverageModulationTableValues(uint32_t index, float const& element) {
    if (element == coverage_modulation_table_valuess_[index]) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_modulation_table_valuess_[index] = element;
}
float const& FullDrawStateData::GetCoverageModulationTableValues(uint32_t index) const {
    return coverage_modulation_table_valuess_[index];
}
float const* FullDrawStateData::GetCoverageModulationTableValuesPtr() const {
    return coverage_modulation_table_valuess_;
}

void FullDrawStateData::SetCoverageModulationTableCount(uint32_t const& element) {
    if (element == coverage_modulation_table_count_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_modulation_table_count_ = element;
}
uint32_t const& FullDrawStateData::GetCoverageModulationTableCount() const {
    return coverage_modulation_table_count_;
}

void FullDrawStateData::SetCoverageReductionMode(VkCoverageReductionModeNV const& element) {
    if (element == coverage_reduction_mode_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_reduction_mode_ = element;
}
VkCoverageReductionModeNV const& FullDrawStateData::GetCoverageReductionMode() const {
    return coverage_reduction_mode_;
}

void FullDrawStateData::SetCoverageToColorEnable(VkBool32 const& element) {
    if (element == coverage_to_color_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_to_color_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetCoverageToColorEnable() const {
    return coverage_to_color_enable_;
}

void FullDrawStateData::SetCoverageToColorLocation(uint32_t const& element) {
    if (element == coverage_to_color_location_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    coverage_to_color_location_ = element;
}
uint32_t const& FullDrawStateData::GetCoverageToColorLocation() const {
    return coverage_to_color_location_;
}

void FullDrawStateData::SetViewportWScalingEnable(VkBool32 const& element) {
    if (element == viewport_w_scaling_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    viewport_w_scaling_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetViewportWScalingEnable() const {
    return viewport_w_scaling_enable_;
}

void FullDrawStateData::SetViewportSwizzleCount(uint32_t const& element) {
    if (element == viewport_swizzle_count_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    viewport_swizzle_count_ = element;
}
uint32_t const& FullDrawStateData::GetViewportSwizzleCount() const {
    return viewport_swizzle_count_;
}

void FullDrawStateData::SetViewportSwizzle(uint32_t index, VkViewportSwizzleNV const& element) {
    if (element == viewport_swizzles_[index]) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    viewport_swizzles_[index] = element;
}
VkViewportSwizzleNV const& FullDrawStateData::GetViewportSwizzle(uint32_t index) const {
    return viewport_swizzles_[index];
}
VkViewportSwizzleNV const* FullDrawStateData::GetViewportSwizzlePtr() const {
    return viewport_swizzles_;
}

void FullDrawStateData::SetShadingRateImageEnable(VkBool32 const& element) {
    if (element == shading_rate_image_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    shading_rate_image_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetShadingRateImageEnable() const {
    return shading_rate_image_enable_;
}

void FullDrawStateData::SetRepresentativeFragmentTestEnable(VkBool32 const& element) {
    if (element == representative_fragment_test_enable_) {
        return;
    }
    dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
    MarkDirty();
    representative_fragment_test_enable_ = element;
}
VkBool32 const& FullDrawStateData::GetRepresentativeFragmentTestEnable() const {
    return representative_fragment_test_enable_;
}

void FullDrawStateData::SetVertexInputAttributeDescription(uint32_t index, VkVertexInputAttributeDescription const& element) {
    if (element == vertex_input_attribute_descriptions_[index]) {
        return;
    }
    dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
    MarkDirty();
    vertex_input_attribute_descriptions_[index] = element;
}
VkVertexInputAttributeDescription const& FullDrawStateData::GetVertexInputAttributeDescription(uint32_t index) const {
    return vertex_input_attribute_descriptions_[index];
}
VkVertexInputAttributeDescription const* FullDrawStateData::GetVertexInputAttributeDescriptionPtr() const {
    return vertex_input_attribute_descriptions_;
}

void FullDrawStateData::SetVertexInputBindingDescription(uint32_t index, VkVertexInputBindingDescription const& element) {
    if (element == vertex_input_binding_descriptions_[index]) {
        return;
    }
    dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
    MarkDirty();
    vertex_input_binding_descriptions_[index] = element;
}
VkVertexInputBindingDescription const& FullDrawStateData::GetVertexInputBindingDescription(uint32_t index) const {
    return vertex_input_binding_descriptions_[index];
}
VkVertexInputBindingDescription const* FullDrawStateData::GetVertexInputBindingDescriptionPtr() const {
    return vertex_input_binding_descriptions_;
}

void FullDrawStateData::SetNumVertexInputAttributeDescriptions(uint32_t const& element) {
    if (element == num_vertex_input_attribute_descriptions_) {
        return;
    }
    dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
    MarkDirty();
    num_vertex_input_attribute_descriptions_ = element;
}
uint32_t const& FullDrawStateData::GetNumVertexInputAttributeDescriptions() const {
    return num_vertex_input_attribute_descriptions_;
}

void FullDrawStateData::SetNumVertexInputBindingDescriptions(uint32_t const& element) {
    if (element == num_vertex_input_binding_descriptions_) {
        return;
    }
    dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
    MarkDirty();
    num_vertex_input_binding_descriptions_ = element;
}
uint32_t const& FullDrawStateData::GetNumVertexInputBindingDescriptions() const {
    return num_vertex_input_binding_descriptions_;
}

bool FullDrawStateData::operator==(FullDrawStateData const& o) const {
    if (!(o.depth_attachment_format_ == depth_attachment_format_) && (!o.dynamic_rendering_unused_attachments_ || o.depth_attachment_format_ != VK_FORMAT_UNDEFINED) && (!dynamic_rendering_unused_attachments_ || depth_attachment_format_ != VK_FORMAT_UNDEFINED)) {
        return false;
    }

    if (!(o.stencil_attachment_format_ == stencil_attachment_format_) && (!o.dynamic_rendering_unused_attachments_ || o.stencil_attachment_format_ != VK_FORMAT_UNDEFINED) && (!dynamic_rendering_unused_attachments_ || stencil_attachment_format_ != VK_FORMAT_UNDEFINED)) {
        return false;
    }

    if (o.limits_.max_color_attachments != limits_.max_color_attachments) {
        return false;
    }
    for (uint32_t i = 0; i < limits_.max_color_attachments; ++i) {
        if (!(o.color_attachment_formats_[i] == color_attachment_formats_[i]) && (!o.dynamic_rendering_unused_attachments_ || o.color_attachment_formats_[i] != VK_FORMAT_UNDEFINED) && (!dynamic_rendering_unused_attachments_ || color_attachment_formats_[i] != VK_FORMAT_UNDEFINED)) {
            return false;
        }
    }

    if (!(o.num_color_attachments_ == num_color_attachments_) && (!o.dynamic_rendering_unused_attachments_ && !dynamic_rendering_unused_attachments_)) {
        return false;
    }

    if (o.limits_.max_color_attachments != limits_.max_color_attachments) {
        return false;
    }
    for (uint32_t i = 0; i < limits_.max_color_attachments; ++i) {
        if (!(o.color_blend_attachment_states_[i] == color_blend_attachment_states_[i])) {
            return false;
        }
    }

    for (uint32_t i = 0; i < NUM_SHADERS; ++i) {
        if (!(o.comparable_shaders_[i] == comparable_shaders_[i])) {
            return false;
        }
    }

    if (!(o.cull_mode_ == cull_mode_)) {
        return false;
    }

    if (!(o.depth_bounds_test_enable_ == depth_bounds_test_enable_)) {
        return false;
    }

    if (!(o.depth_compare_op_ == depth_compare_op_)) {
        return false;
    }

    if (!(o.depth_test_enable_ == depth_test_enable_)) {
        return false;
    }

    if (!(o.depth_write_enable_ == depth_write_enable_)) {
        return false;
    }

    if (!(o.front_face_ == front_face_)) {
        return false;
    }

    if (!(o.primitive_topology_ == primitive_topology_)) {
        return false;
    }

    if (!(o.num_scissors_ == num_scissors_)) {
        return false;
    }

    if (!(o.num_viewports_ == num_viewports_)) {
        return false;
    }

    if (!(o.stencil_front_ == stencil_front_)) {
        return false;
    }

    if (!(o.stencil_back_ == stencil_back_)) {
        return false;
    }

    if (!(o.stencil_test_enable_ == stencil_test_enable_)) {
        return false;
    }

    if (!(o.logic_op_ == logic_op_)) {
        return false;
    }

    if (!(o.primitive_restart_enable_ == primitive_restart_enable_)) {
        return false;
    }

    if (!(o.rasterizer_discard_enable_ == rasterizer_discard_enable_)) {
        return false;
    }

    if (!(o.depth_bias_enable_ == depth_bias_enable_)) {
        return false;
    }

    if (!(o.patch_control_points_ == patch_control_points_)) {
        return false;
    }

    if (!(o.polygon_mode_ == polygon_mode_)) {
        return false;
    }

    if (!(o.rasterization_samples_ == rasterization_samples_)) {
        return false;
    }

    if (!(o.logic_op_enable_ == logic_op_enable_)) {
        return false;
    }

    if (!(o.depth_clamp_enable_ == depth_clamp_enable_)) {
        return false;
    }

    if (!(o.domain_origin_ == domain_origin_)) {
        return false;
    }

    if (!(o.alpha_to_one_enable_ == alpha_to_one_enable_)) {
        return false;
    }

    if (!(o.alpha_to_coverage_enable_ == alpha_to_coverage_enable_)) {
        return false;
    }

    for (uint32_t i = 0; i < kMaxSampleMaskLength; ++i) {
        if (!(o.sample_masks_[i] == sample_masks_[i])) {
            return false;
        }
    }

    if (!(o.rasterization_stream_ == rasterization_stream_)) {
        return false;
    }

    if (!(o.conservative_rasterization_mode_ == conservative_rasterization_mode_)) {
        return false;
    }

    if (!(o.extra_primitive_overestimation_size_ == extra_primitive_overestimation_size_)) {
        return false;
    }

    if (!(o.depth_clip_enable_ == depth_clip_enable_)) {
        return false;
    }

    if (!(o.sample_locations_enable_ == sample_locations_enable_)) {
        return false;
    }

    if (!(o.provoking_vertex_mode_ == provoking_vertex_mode_)) {
        return false;
    }

    if (!(o.line_rasterization_mode_ == line_rasterization_mode_)) {
        return false;
    }

    if (!(o.stippled_line_enable_ == stippled_line_enable_)) {
        return false;
    }

    if (!(o.negative_one_to_one_ == negative_one_to_one_)) {
        return false;
    }

    if (!(o.coverage_modulation_mode_ == coverage_modulation_mode_)) {
        return false;
    }

    if (!(o.coverage_modulation_table_enable_ == coverage_modulation_table_enable_)) {
        return false;
    }

    for (uint32_t i = 0; i < VK_SAMPLE_COUNT_64_BIT; ++i) {
        if (!(o.coverage_modulation_table_valuess_[i] == coverage_modulation_table_valuess_[i])) {
            return false;
        }
    }

    if (!(o.coverage_modulation_table_count_ == coverage_modulation_table_count_)) {
        return false;
    }

    if (!(o.coverage_reduction_mode_ == coverage_reduction_mode_)) {
        return false;
    }

    if (!(o.coverage_to_color_enable_ == coverage_to_color_enable_)) {
        return false;
    }

    if (!(o.coverage_to_color_location_ == coverage_to_color_location_)) {
        return false;
    }

    if (!(o.viewport_w_scaling_enable_ == viewport_w_scaling_enable_)) {
        return false;
    }

    if (!(o.viewport_swizzle_count_ == viewport_swizzle_count_)) {
        return false;
    }

    if (o.limits_.max_viewports != limits_.max_viewports) {
        return false;
    }
    for (uint32_t i = 0; i < limits_.max_viewports; ++i) {
        if (!(o.viewport_swizzles_[i] == viewport_swizzles_[i])) {
            return false;
        }
    }

    if (!(o.shading_rate_image_enable_ == shading_rate_image_enable_)) {
        return false;
    }

    if (!(o.representative_fragment_test_enable_ == representative_fragment_test_enable_)) {
        return false;
    }

    if (o.limits_.max_vertex_input_attributes != limits_.max_vertex_input_attributes) {
        return false;
    }
    for (uint32_t i = 0; i < limits_.max_vertex_input_attributes; ++i) {
        if (!(o.vertex_input_attribute_descriptions_[i] == vertex_input_attribute_descriptions_[i])) {
            return false;
        }
    }

    if (o.limits_.max_vertex_input_bindings != limits_.max_vertex_input_bindings) {
        return false;
    }
    for (uint32_t i = 0; i < limits_.max_vertex_input_bindings; ++i) {
        if (!(o.vertex_input_binding_descriptions_[i] == vertex_input_binding_descriptions_[i])) {
            return false;
        }
    }

    if (!(o.num_vertex_input_attribute_descriptions_ == num_vertex_input_attribute_descriptions_)) {
        return false;
    }

    if (!(o.num_vertex_input_binding_descriptions_ == num_vertex_input_binding_descriptions_)) {
        return false;
    }

    return true;
}

    bool FullDrawStateData::CompareStateSubset(FullDrawStateData const& o, VkGraphicsPipelineLibraryFlagBitsEXT flag) const {
        if (flag == VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
            if (!(o.comparable_shaders_[VERTEX_SHADER] == comparable_shaders_[VERTEX_SHADER])) {
                return false;
            }
            if (!(o.comparable_shaders_[TESSELLATION_CONTROL_SHADER] == comparable_shaders_[TESSELLATION_CONTROL_SHADER])) {
                return false;
            }
            if (!(o.comparable_shaders_[TESSELLATION_EVALUATION_SHADER] == comparable_shaders_[TESSELLATION_EVALUATION_SHADER])) {
                return false;
            }
            if (!(o.comparable_shaders_[GEOMETRY_SHADER] == comparable_shaders_[GEOMETRY_SHADER])) {
                return false;
            }
            if (!(o.comparable_shaders_[TASK_SHADER] == comparable_shaders_[TASK_SHADER])) {
                return false;
            }
            if (!(o.comparable_shaders_[MESH_SHADER] == comparable_shaders_[MESH_SHADER])) {
                return false;
            }
            if (!(o.cull_mode_ == cull_mode_)) {
                return false;
            }

            if (!(o.front_face_ == front_face_)) {
                return false;
            }

            if (!(o.num_scissors_ == num_scissors_)) {
                return false;
            }

            if (!(o.num_viewports_ == num_viewports_)) {
                return false;
            }

            if (!(o.depth_bias_enable_ == depth_bias_enable_)) {
                return false;
            }

            if (!(o.patch_control_points_ == patch_control_points_)) {
                return false;
            }

            if (!(o.polygon_mode_ == polygon_mode_)) {
                return false;
            }

            if (!(o.depth_clamp_enable_ == depth_clamp_enable_)) {
                return false;
            }

        }
        if (flag == VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
            if (!(o.comparable_shaders_[FRAGMENT_SHADER] == comparable_shaders_[FRAGMENT_SHADER])) {
                return false;
            }
            if (!(o.depth_bounds_test_enable_ == depth_bounds_test_enable_)) {
                return false;
            }

            if (!(o.depth_compare_op_ == depth_compare_op_)) {
                return false;
            }

            if (!(o.depth_test_enable_ == depth_test_enable_)) {
                return false;
            }

            if (!(o.depth_write_enable_ == depth_write_enable_)) {
                return false;
            }

            if (!(o.stencil_front_ == stencil_front_)) {
                return false;
            }

            if (!(o.stencil_back_ == stencil_back_)) {
                return false;
            }

            if (!(o.rasterization_samples_ == rasterization_samples_)) {
                return false;
            }

            if (!(o.alpha_to_one_enable_ == alpha_to_one_enable_)) {
                return false;
            }

            if (!(o.alpha_to_coverage_enable_ == alpha_to_coverage_enable_)) {
                return false;
            }

            for (uint32_t i = 0; i < kMaxSampleMaskLength; ++i) {
                if (!(o.sample_masks_[i] == sample_masks_[i])) {
                    return false;
                }
            }

        }
        return true;
    }
    size_t FullDrawStateData::CalculatePartialHash(StateGroup state_group) const {
        switch (state_group) {
            default: assert(false); return 0;
            case EXTENDED_DYNAMIC_STATE_1:
            {
                size_t res = 17;
                res = res * 31 + std::hash<VkCullModeFlags>()(cull_mode_);
                res = res * 31 + std::hash<VkBool32>()(depth_bounds_test_enable_);
                res = res * 31 + std::hash<VkCompareOp>()(depth_compare_op_);
                res = res * 31 + std::hash<VkBool32>()(depth_test_enable_);
                res = res * 31 + std::hash<VkBool32>()(depth_write_enable_);
                res = res * 31 + std::hash<VkFrontFace>()(front_face_);
                res = res * 31 + std::hash<VkPrimitiveTopology>()(primitive_topology_);
                res = res * 31 + std::hash<uint32_t>()(num_scissors_);
                res = res * 31 + std::hash<uint32_t>()(num_viewports_);
                res = res * 31 + std::hash<VkStencilOpState>()(stencil_front_);
                res = res * 31 + std::hash<VkStencilOpState>()(stencil_back_);
                res = res * 31 + std::hash<VkBool32>()(stencil_test_enable_);
                return res;
            }
            case EXTENDED_DYNAMIC_STATE_2:
            {
                size_t res = 17;
                res = res * 31 + std::hash<VkLogicOp>()(logic_op_);
                res = res * 31 + std::hash<VkBool32>()(primitive_restart_enable_);
                res = res * 31 + std::hash<VkBool32>()(rasterizer_discard_enable_);
                res = res * 31 + std::hash<VkBool32>()(depth_bias_enable_);
                res = res * 31 + std::hash<uint32_t>()(patch_control_points_);
                return res;
            }
            case EXTENDED_DYNAMIC_STATE_3:
            {
                size_t res = 17;
                res = res * 31 + std::hash<VkPolygonMode>()(polygon_mode_);
                res = res * 31 + std::hash<VkSampleCountFlagBits>()(rasterization_samples_);
                res = res * 31 + std::hash<VkBool32>()(logic_op_enable_);
                res = res * 31 + std::hash<VkBool32>()(depth_clamp_enable_);
                res = res * 31 + std::hash<VkTessellationDomainOrigin>()(domain_origin_);
                res = res * 31 + std::hash<VkBool32>()(alpha_to_one_enable_);
                res = res * 31 + std::hash<VkBool32>()(alpha_to_coverage_enable_);
                // TODO: array comparison
                res = res * 31 + std::hash<uint32_t>()(rasterization_stream_);
                res = res * 31 + std::hash<VkConservativeRasterizationModeEXT>()(conservative_rasterization_mode_);
                res = res * 31 + std::hash<float>()(extra_primitive_overestimation_size_);
                res = res * 31 + std::hash<VkBool32>()(depth_clip_enable_);
                res = res * 31 + std::hash<VkBool32>()(sample_locations_enable_);
                res = res * 31 + std::hash<VkProvokingVertexModeEXT>()(provoking_vertex_mode_);
                res = res * 31 + std::hash<VkLineRasterizationModeEXT>()(line_rasterization_mode_);
                res = res * 31 + std::hash<VkBool32>()(stippled_line_enable_);
                res = res * 31 + std::hash<VkBool32>()(negative_one_to_one_);
                res = res * 31 + std::hash<VkCoverageModulationModeNV>()(coverage_modulation_mode_);
                res = res * 31 + std::hash<VkBool32>()(coverage_modulation_table_enable_);
                // TODO: array comparison
                res = res * 31 + std::hash<uint32_t>()(coverage_modulation_table_count_);
                res = res * 31 + std::hash<VkCoverageReductionModeNV>()(coverage_reduction_mode_);
                res = res * 31 + std::hash<VkBool32>()(coverage_to_color_enable_);
                res = res * 31 + std::hash<uint32_t>()(coverage_to_color_location_);
                res = res * 31 + std::hash<VkBool32>()(viewport_w_scaling_enable_);
                res = res * 31 + std::hash<uint32_t>()(viewport_swizzle_count_);
                // TODO: array comparison
                res = res * 31 + std::hash<VkBool32>()(shading_rate_image_enable_);
                res = res * 31 + std::hash<VkBool32>()(representative_fragment_test_enable_);
                return res;
            }
            case VERTEX_INPUT_DYNAMIC:
            {
                size_t res = 17;
                // TODO: array comparison
                // TODO: array comparison
                res = res * 31 + std::hash<uint32_t>()(num_vertex_input_attribute_descriptions_);
                res = res * 31 + std::hash<uint32_t>()(num_vertex_input_binding_descriptions_);
                return res;
            }
            case MISC:
            {
                size_t res = 17;
                if (!dynamic_rendering_unused_attachments_) {
                   res = res * 31 + std::hash<VkFormat>()(depth_attachment_format_);
                }
                if (!dynamic_rendering_unused_attachments_) {
                   res = res * 31 + std::hash<VkFormat>()(stencil_attachment_format_);
                }
                // TODO: array comparison
                res = res * 31 + std::hash<uint32_t>()(num_color_attachments_);
                // TODO: array comparison
                // TODO: array comparison
                return res;
            }
        }
    }

}  // namespace shader_object

