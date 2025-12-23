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

public:
enum StateGroup {
    MISC,
    EXTENDED_DYNAMIC_STATE,
    EXTENDED_DYNAMIC_STATE2,
    EXTENDED_DYNAMIC_STATE3,
    VERTEX_INPUT_DYNAMIC_STATE,
    NUM_STATE_GROUPS
};

void SetDepthAttachmentFormat(VkFormat const& element);
VkFormat const& GetDepthAttachmentFormat() const;
void SetStencilAttachmentFormat(VkFormat const& element);
VkFormat const& GetStencilAttachmentFormat() const;
void SetColorAttachmentFormat(uint32_t index, VkFormat const& element);
VkFormat const& GetColorAttachmentFormat(uint32_t index) const;
VkFormat const* GetColorAttachmentFormatPtr() const;
void SetNumColorAttachments(uint32_t const& element);
uint32_t const& GetNumColorAttachments() const;
void SetColorBlendAttachmentState(uint32_t index, VkPipelineColorBlendAttachmentState const& element);
VkPipelineColorBlendAttachmentState const& GetColorBlendAttachmentState(uint32_t index) const;
VkPipelineColorBlendAttachmentState const* GetColorBlendAttachmentStatePtr() const;
void SetComparableShader(uint32_t index, ComparableShader const& element);
ComparableShader const& GetComparableShader(uint32_t index) const;
ComparableShader const* GetComparableShaderPtr() const;
void SetCullMode(VkCullModeFlags const& element);
VkCullModeFlags const& GetCullMode() const;
void SetDepthBoundsTestEnable(VkBool32 const& element);
VkBool32 const& GetDepthBoundsTestEnable() const;
void SetDepthCompareOp(VkCompareOp const& element);
VkCompareOp const& GetDepthCompareOp() const;
void SetDepthTestEnable(VkBool32 const& element);
VkBool32 const& GetDepthTestEnable() const;
void SetDepthWriteEnable(VkBool32 const& element);
VkBool32 const& GetDepthWriteEnable() const;
void SetFrontFace(VkFrontFace const& element);
VkFrontFace const& GetFrontFace() const;
void SetPrimitiveTopology(VkPrimitiveTopology const& element);
VkPrimitiveTopology const& GetPrimitiveTopology() const;
void SetNumScissors(uint32_t const& element);
uint32_t const& GetNumScissors() const;
void SetNumViewports(uint32_t const& element);
uint32_t const& GetNumViewports() const;
void SetStencilFront(VkStencilOpState const& element);
VkStencilOpState const& GetStencilFront() const;
void SetStencilBack(VkStencilOpState const& element);
VkStencilOpState const& GetStencilBack() const;
void SetStencilTestEnable(VkBool32 const& element);
VkBool32 const& GetStencilTestEnable() const;
void SetLogicOp(VkLogicOp const& element);
VkLogicOp const& GetLogicOp() const;
void SetPrimitiveRestartEnable(VkBool32 const& element);
VkBool32 const& GetPrimitiveRestartEnable() const;
void SetRasterizerDiscardEnable(VkBool32 const& element);
VkBool32 const& GetRasterizerDiscardEnable() const;
void SetDepthBiasEnable(VkBool32 const& element);
VkBool32 const& GetDepthBiasEnable() const;
void SetPatchControlPoints(uint32_t const& element);
uint32_t const& GetPatchControlPoints() const;
void SetPolygonMode(VkPolygonMode const& element);
VkPolygonMode const& GetPolygonMode() const;
void SetRasterizationSamples(VkSampleCountFlagBits const& element);
VkSampleCountFlagBits const& GetRasterizationSamples() const;
void SetLogicOpEnable(VkBool32 const& element);
VkBool32 const& GetLogicOpEnable() const;
void SetDepthClampEnable(VkBool32 const& element);
VkBool32 const& GetDepthClampEnable() const;
void SetDomainOrigin(VkTessellationDomainOrigin const& element);
VkTessellationDomainOrigin const& GetDomainOrigin() const;
void SetAlphaToOneEnable(VkBool32 const& element);
VkBool32 const& GetAlphaToOneEnable() const;
void SetAlphaToCoverageEnable(VkBool32 const& element);
VkBool32 const& GetAlphaToCoverageEnable() const;
void SetSampleMask(uint32_t index, VkSampleMask const& element);
VkSampleMask const& GetSampleMask(uint32_t index) const;
VkSampleMask const* GetSampleMaskPtr() const;
void SetRasterizationStream(uint32_t const& element);
uint32_t const& GetRasterizationStream() const;
void SetConservativeRasterizationMode(VkConservativeRasterizationModeEXT const& element);
VkConservativeRasterizationModeEXT const& GetConservativeRasterizationMode() const;
void SetExtraPrimitiveOverestimationSize(float const& element);
float const& GetExtraPrimitiveOverestimationSize() const;
void SetDepthClipEnable(VkBool32 const& element);
VkBool32 const& GetDepthClipEnable() const;
void SetSampleLocationsEnable(VkBool32 const& element);
VkBool32 const& GetSampleLocationsEnable() const;
void SetProvokingVertexMode(VkProvokingVertexModeEXT const& element);
VkProvokingVertexModeEXT const& GetProvokingVertexMode() const;
void SetLineRasterizationMode(VkLineRasterizationModeEXT const& element);
VkLineRasterizationModeEXT const& GetLineRasterizationMode() const;
void SetStippledLineEnable(VkBool32 const& element);
VkBool32 const& GetStippledLineEnable() const;
void SetNegativeOneToOne(VkBool32 const& element);
VkBool32 const& GetNegativeOneToOne() const;
void SetCoverageModulationMode(VkCoverageModulationModeNV const& element);
VkCoverageModulationModeNV const& GetCoverageModulationMode() const;
void SetCoverageModulationTableEnable(VkBool32 const& element);
VkBool32 const& GetCoverageModulationTableEnable() const;
void SetCoverageModulationTableValues(uint32_t index, float const& element);
float const& GetCoverageModulationTableValues(uint32_t index) const;
float const* GetCoverageModulationTableValuesPtr() const;
void SetCoverageModulationTableCount(uint32_t const& element);
uint32_t const& GetCoverageModulationTableCount() const;
void SetCoverageReductionMode(VkCoverageReductionModeNV const& element);
VkCoverageReductionModeNV const& GetCoverageReductionMode() const;
void SetCoverageToColorEnable(VkBool32 const& element);
VkBool32 const& GetCoverageToColorEnable() const;
void SetCoverageToColorLocation(uint32_t const& element);
uint32_t const& GetCoverageToColorLocation() const;
void SetViewportWScalingEnable(VkBool32 const& element);
VkBool32 const& GetViewportWScalingEnable() const;
void SetViewportSwizzleCount(uint32_t const& element);
uint32_t const& GetViewportSwizzleCount() const;
void SetViewportSwizzle(uint32_t index, VkViewportSwizzleNV const& element);
VkViewportSwizzleNV const& GetViewportSwizzle(uint32_t index) const;
VkViewportSwizzleNV const* GetViewportSwizzlePtr() const;
void SetShadingRateImageEnable(VkBool32 const& element);
VkBool32 const& GetShadingRateImageEnable() const;
void SetRepresentativeFragmentTestEnable(VkBool32 const& element);
VkBool32 const& GetRepresentativeFragmentTestEnable() const;
void SetVertexInputAttributeDescription(uint32_t index, VkVertexInputAttributeDescription const& element);
VkVertexInputAttributeDescription const& GetVertexInputAttributeDescription(uint32_t index) const;
VkVertexInputAttributeDescription const* GetVertexInputAttributeDescriptionPtr() const;
void SetVertexInputBindingDescription(uint32_t index, VkVertexInputBindingDescription const& element);
VkVertexInputBindingDescription const& GetVertexInputBindingDescription(uint32_t index) const;
VkVertexInputBindingDescription const* GetVertexInputBindingDescriptionPtr() const;
void SetNumVertexInputAttributeDescriptions(uint32_t const& element);
uint32_t const& GetNumVertexInputAttributeDescriptions() const;
void SetNumVertexInputBindingDescriptions(uint32_t const& element);
uint32_t const& GetNumVertexInputBindingDescriptions() const;
bool operator==(FullDrawStateData const& o) const;
bool CompareStateSubset(FullDrawStateData const& o, VkGraphicsPipelineLibraryFlagBitsEXT flag) const;

private:
size_t CalculatePartialHash(StateGroup state_group) const;

VkFormat depth_attachment_format_{};
VkFormat stencil_attachment_format_{};
VkFormat* color_attachment_formats_{};
uint32_t num_color_attachments_{};
VkPipelineColorBlendAttachmentState* color_blend_attachment_states_{};
ComparableShader comparable_shaders_[NUM_SHADERS];
VkCullModeFlags cull_mode_{};
VkBool32 depth_bounds_test_enable_{};
VkCompareOp depth_compare_op_{};
VkBool32 depth_test_enable_{};
VkBool32 depth_write_enable_{};
VkFrontFace front_face_{};
VkPrimitiveTopology primitive_topology_{};
uint32_t num_scissors_{};
uint32_t num_viewports_{};
VkStencilOpState stencil_front_{};
VkStencilOpState stencil_back_{};
VkBool32 stencil_test_enable_{};
VkLogicOp logic_op_{};
VkBool32 primitive_restart_enable_{};
VkBool32 rasterizer_discard_enable_{};
VkBool32 depth_bias_enable_{};
uint32_t patch_control_points_ = 1;
VkPolygonMode polygon_mode_{};
VkSampleCountFlagBits rasterization_samples_{};
VkBool32 logic_op_enable_{};
VkBool32 depth_clamp_enable_{};
VkTessellationDomainOrigin domain_origin_{};
VkBool32 alpha_to_one_enable_{};
VkBool32 alpha_to_coverage_enable_{};
VkSampleMask sample_masks_[kMaxSampleMaskLength];
uint32_t rasterization_stream_{};
VkConservativeRasterizationModeEXT conservative_rasterization_mode_{};
float extra_primitive_overestimation_size_{};
VkBool32 depth_clip_enable_{};
VkBool32 sample_locations_enable_{};
VkProvokingVertexModeEXT provoking_vertex_mode_{};
VkLineRasterizationModeEXT line_rasterization_mode_{};
VkBool32 stippled_line_enable_{};
VkBool32 negative_one_to_one_{};
VkCoverageModulationModeNV coverage_modulation_mode_{};
VkBool32 coverage_modulation_table_enable_{};
float coverage_modulation_table_valuess_[VK_SAMPLE_COUNT_64_BIT];
uint32_t coverage_modulation_table_count_{};
VkCoverageReductionModeNV coverage_reduction_mode_{};
VkBool32 coverage_to_color_enable_{};
uint32_t coverage_to_color_location_{};
VkBool32 viewport_w_scaling_enable_{};
uint32_t viewport_swizzle_count_{};
VkViewportSwizzleNV* viewport_swizzles_{};
VkBool32 shading_rate_image_enable_{};
VkBool32 representative_fragment_test_enable_{};
VkVertexInputAttributeDescription* vertex_input_attribute_descriptions_{};
VkVertexInputBindingDescription* vertex_input_binding_descriptions_{};
uint32_t num_vertex_input_attribute_descriptions_{};
uint32_t num_vertex_input_binding_descriptions_{};
