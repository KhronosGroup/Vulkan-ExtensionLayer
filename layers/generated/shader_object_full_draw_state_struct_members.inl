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

public:
    enum StateGroup { MISC, EXTENDED_DYNAMIC_STATE_1, EXTENDED_DYNAMIC_STATE_2, EXTENDED_DYNAMIC_STATE_3, VERTEX_INPUT_DYNAMIC, NUM_STATE_GROUPS };

    void SetDepthAttachmentFormat(VkFormat const& element) {
        if (element == depth_attachment_format_) {
            return;
        }
        dirty_hash_bits_.set(MISC);
        MarkDirty();
        depth_attachment_format_ = element;
    }
    VkFormat const& GetDepthAttachmentFormat() const {
        return depth_attachment_format_;
    }
    
    void SetStencilAttachmentFormat(VkFormat const& element) {
        if (element == stencil_attachment_format_) {
            return;
        }
        dirty_hash_bits_.set(MISC);
        MarkDirty();
        stencil_attachment_format_ = element;
    }
    VkFormat const& GetStencilAttachmentFormat() const {
        return stencil_attachment_format_;
    }
    
    void SetColorAttachmentFormat(uint32_t index, VkFormat const& element) {
        if (element == color_attachment_formats_[index]) {
            return;
        }
        dirty_hash_bits_.set(MISC);
        MarkDirty();
        color_attachment_formats_[index] = element;
    }
    VkFormat const& GetColorAttachmentFormat(uint32_t index) const {
        return color_attachment_formats_[index];
    }
    VkFormat const* GetColorAttachmentFormatPtr() const {
        return color_attachment_formats_;
    }
    
    void SetNumColorAttachments(uint32_t const& element) {
        if (element == num_color_attachments_) {
            return;
        }
        dirty_hash_bits_.set(MISC);
        MarkDirty();
        num_color_attachments_ = element;
    }
    uint32_t const& GetNumColorAttachments() const {
        return num_color_attachments_;
    }
    
    void SetColorBlendAttachmentState(uint32_t index, VkPipelineColorBlendAttachmentState const& element) {
        if (element == color_blend_attachment_states_[index]) {
            return;
        }
        dirty_hash_bits_.set(MISC);
        MarkDirty();
        color_blend_attachment_states_[index] = element;
    }
    VkPipelineColorBlendAttachmentState const& GetColorBlendAttachmentState(uint32_t index) const {
        return color_blend_attachment_states_[index];
    }
    VkPipelineColorBlendAttachmentState const* GetColorBlendAttachmentStatePtr() const {
        return color_blend_attachment_states_;
    }
    
    void SetComparableShader(uint32_t index, ComparableShader const& element) {
        if (element == comparable_shaders_[index]) {
            return;
        }
        dirty_hash_bits_.set(MISC);
        MarkDirty();
        comparable_shaders_[index] = element;
    }
    ComparableShader const& GetComparableShader(uint32_t index) const {
        return comparable_shaders_[index];
    }
    ComparableShader const* GetComparableShaderPtr() const {
        return comparable_shaders_;
    }
    
    void SetCullMode(VkCullModeFlags const& element) {
        if (element == cull_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        cull_mode_ = element;
    }
    VkCullModeFlags const& GetCullMode() const {
        return cull_mode_;
    }
    
    void SetDepthBoundsTestEnable(VkBool32 const& element) {
        if (element == depth_bounds_test_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        depth_bounds_test_enable_ = element;
    }
    VkBool32 const& GetDepthBoundsTestEnable() const {
        return depth_bounds_test_enable_;
    }
    
    void SetDepthCompareOp(VkCompareOp const& element) {
        if (element == depth_compare_op_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        depth_compare_op_ = element;
    }
    VkCompareOp const& GetDepthCompareOp() const {
        return depth_compare_op_;
    }
    
    void SetDepthTestEnable(VkBool32 const& element) {
        if (element == depth_test_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        depth_test_enable_ = element;
    }
    VkBool32 const& GetDepthTestEnable() const {
        return depth_test_enable_;
    }
    
    void SetDepthWriteEnable(VkBool32 const& element) {
        if (element == depth_write_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        depth_write_enable_ = element;
    }
    VkBool32 const& GetDepthWriteEnable() const {
        return depth_write_enable_;
    }
    
    void SetFrontFace(VkFrontFace const& element) {
        if (element == front_face_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        front_face_ = element;
    }
    VkFrontFace const& GetFrontFace() const {
        return front_face_;
    }
    
    void SetPrimitiveTopology(VkPrimitiveTopology const& element) {
        if (element == primitive_topology_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        primitive_topology_ = element;
    }
    VkPrimitiveTopology const& GetPrimitiveTopology() const {
        return primitive_topology_;
    }
    
    void SetNumScissors(uint32_t const& element) {
        if (element == num_scissors_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        num_scissors_ = element;
    }
    uint32_t const& GetNumScissors() const {
        return num_scissors_;
    }
    
    void SetNumViewports(uint32_t const& element) {
        if (element == num_viewports_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        num_viewports_ = element;
    }
    uint32_t const& GetNumViewports() const {
        return num_viewports_;
    }
    
    void SetStencilFront(VkStencilOpState const& element) {
        if (element == stencil_front_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        stencil_front_ = element;
    }
    VkStencilOpState const& GetStencilFront() const {
        return stencil_front_;
    }
    
    void SetStencilBack(VkStencilOpState const& element) {
        if (element == stencil_back_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        stencil_back_ = element;
    }
    VkStencilOpState const& GetStencilBack() const {
        return stencil_back_;
    }
    
    void SetStencilTestEnable(VkBool32 const& element) {
        if (element == stencil_test_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_1);
        MarkDirty();
        stencil_test_enable_ = element;
    }
    VkBool32 const& GetStencilTestEnable() const {
        return stencil_test_enable_;
    }
    
    void SetLogicOp(VkLogicOp const& element) {
        if (element == logic_op_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
        MarkDirty();
        logic_op_ = element;
    }
    VkLogicOp const& GetLogicOp() const {
        return logic_op_;
    }
    
    void SetPrimitiveRestartEnable(VkBool32 const& element) {
        if (element == primitive_restart_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
        MarkDirty();
        primitive_restart_enable_ = element;
    }
    VkBool32 const& GetPrimitiveRestartEnable() const {
        return primitive_restart_enable_;
    }
    
    void SetRasterizerDiscardEnable(VkBool32 const& element) {
        if (element == rasterizer_discard_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
        MarkDirty();
        rasterizer_discard_enable_ = element;
    }
    VkBool32 const& GetRasterizerDiscardEnable() const {
        return rasterizer_discard_enable_;
    }
    
    void SetDepthBiasEnable(VkBool32 const& element) {
        if (element == depth_bias_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
        MarkDirty();
        depth_bias_enable_ = element;
    }
    VkBool32 const& GetDepthBiasEnable() const {
        return depth_bias_enable_;
    }
    
    void SetPatchControlPoints(uint32_t const& element) {
        if (element == patch_control_points_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_2);
        MarkDirty();
        patch_control_points_ = element;
    }
    uint32_t const& GetPatchControlPoints() const {
        return patch_control_points_;
    }
    
    void SetPolygonMode(VkPolygonMode const& element) {
        if (element == polygon_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        polygon_mode_ = element;
    }
    VkPolygonMode const& GetPolygonMode() const {
        return polygon_mode_;
    }
    
    void SetRasterizationSamples(VkSampleCountFlagBits const& element) {
        if (element == rasterization_samples_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        rasterization_samples_ = element;
    }
    VkSampleCountFlagBits const& GetRasterizationSamples() const {
        return rasterization_samples_;
    }
    
    void SetLogicOpEnable(VkBool32 const& element) {
        if (element == logic_op_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        logic_op_enable_ = element;
    }
    VkBool32 const& GetLogicOpEnable() const {
        return logic_op_enable_;
    }
    
    void SetDepthClampEnable(VkBool32 const& element) {
        if (element == depth_clamp_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        depth_clamp_enable_ = element;
    }
    VkBool32 const& GetDepthClampEnable() const {
        return depth_clamp_enable_;
    }
    
    void SetDomainOrigin(VkTessellationDomainOrigin const& element) {
        if (element == domain_origin_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        domain_origin_ = element;
    }
    VkTessellationDomainOrigin const& GetDomainOrigin() const {
        return domain_origin_;
    }
    
    void SetAlphaToOneEnable(VkBool32 const& element) {
        if (element == alpha_to_one_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        alpha_to_one_enable_ = element;
    }
    VkBool32 const& GetAlphaToOneEnable() const {
        return alpha_to_one_enable_;
    }
    
    void SetAlphaToCoverageEnable(VkBool32 const& element) {
        if (element == alpha_to_coverage_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        alpha_to_coverage_enable_ = element;
    }
    VkBool32 const& GetAlphaToCoverageEnable() const {
        return alpha_to_coverage_enable_;
    }
    
    void SetSampleMask(uint32_t index, VkSampleMask const& element) {
        if (element == sample_masks_[index]) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        sample_masks_[index] = element;
    }
    VkSampleMask const& GetSampleMask(uint32_t index) const {
        return sample_masks_[index];
    }
    VkSampleMask const* GetSampleMaskPtr() const {
        return sample_masks_;
    }
    
    void SetRasterizationStream(uint32_t const& element) {
        if (element == rasterization_stream_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        rasterization_stream_ = element;
    }
    uint32_t const& GetRasterizationStream() const {
        return rasterization_stream_;
    }
    
    void SetConservativeRasterizationMode(VkConservativeRasterizationModeEXT const& element) {
        if (element == conservative_rasterization_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        conservative_rasterization_mode_ = element;
    }
    VkConservativeRasterizationModeEXT const& GetConservativeRasterizationMode() const {
        return conservative_rasterization_mode_;
    }
    
    void SetExtraPrimitiveOverestimationSize(float const& element) {
        if (element == extra_primitive_overestimation_size_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        extra_primitive_overestimation_size_ = element;
    }
    float const& GetExtraPrimitiveOverestimationSize() const {
        return extra_primitive_overestimation_size_;
    }
    
    void SetDepthClipEnable(VkBool32 const& element) {
        if (element == depth_clip_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        depth_clip_enable_ = element;
    }
    VkBool32 const& GetDepthClipEnable() const {
        return depth_clip_enable_;
    }
    
    void SetSampleLocationsEnable(VkBool32 const& element) {
        if (element == sample_locations_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        sample_locations_enable_ = element;
    }
    VkBool32 const& GetSampleLocationsEnable() const {
        return sample_locations_enable_;
    }
    
    void SetProvokingVertexMode(VkProvokingVertexModeEXT const& element) {
        if (element == provoking_vertex_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        provoking_vertex_mode_ = element;
    }
    VkProvokingVertexModeEXT const& GetProvokingVertexMode() const {
        return provoking_vertex_mode_;
    }
    
    void SetLineRasterizationMode(VkLineRasterizationModeEXT const& element) {
        if (element == line_rasterization_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        line_rasterization_mode_ = element;
    }
    VkLineRasterizationModeEXT const& GetLineRasterizationMode() const {
        return line_rasterization_mode_;
    }
    
    void SetStippledLineEnable(VkBool32 const& element) {
        if (element == stippled_line_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        stippled_line_enable_ = element;
    }
    VkBool32 const& GetStippledLineEnable() const {
        return stippled_line_enable_;
    }
    
    void SetNegativeOneToOne(VkBool32 const& element) {
        if (element == negative_one_to_one_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        negative_one_to_one_ = element;
    }
    VkBool32 const& GetNegativeOneToOne() const {
        return negative_one_to_one_;
    }
    
    void SetCoverageModulationMode(VkCoverageModulationModeNV const& element) {
        if (element == coverage_modulation_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_modulation_mode_ = element;
    }
    VkCoverageModulationModeNV const& GetCoverageModulationMode() const {
        return coverage_modulation_mode_;
    }
    
    void SetCoverageModulationTableEnable(VkBool32 const& element) {
        if (element == coverage_modulation_table_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_modulation_table_enable_ = element;
    }
    VkBool32 const& GetCoverageModulationTableEnable() const {
        return coverage_modulation_table_enable_;
    }
    
    void SetCoverageModulationTableValues(uint32_t index, float const& element) {
        if (element == coverage_modulation_table_valuess_[index]) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_modulation_table_valuess_[index] = element;
    }
    float const& GetCoverageModulationTableValues(uint32_t index) const {
        return coverage_modulation_table_valuess_[index];
    }
    float const* GetCoverageModulationTableValuesPtr() const {
        return coverage_modulation_table_valuess_;
    }
    
    void SetCoverageModulationTableCount(uint32_t const& element) {
        if (element == coverage_modulation_table_count_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_modulation_table_count_ = element;
    }
    uint32_t const& GetCoverageModulationTableCount() const {
        return coverage_modulation_table_count_;
    }
    
    void SetCoverageReductionMode(VkCoverageReductionModeNV const& element) {
        if (element == coverage_reduction_mode_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_reduction_mode_ = element;
    }
    VkCoverageReductionModeNV const& GetCoverageReductionMode() const {
        return coverage_reduction_mode_;
    }
    
    void SetCoverageToColorEnable(VkBool32 const& element) {
        if (element == coverage_to_color_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_to_color_enable_ = element;
    }
    VkBool32 const& GetCoverageToColorEnable() const {
        return coverage_to_color_enable_;
    }
    
    void SetCoverageToColorLocation(uint32_t const& element) {
        if (element == coverage_to_color_location_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        coverage_to_color_location_ = element;
    }
    uint32_t const& GetCoverageToColorLocation() const {
        return coverage_to_color_location_;
    }
    
    void SetViewportWScalingEnable(VkBool32 const& element) {
        if (element == viewport_w_scaling_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        viewport_w_scaling_enable_ = element;
    }
    VkBool32 const& GetViewportWScalingEnable() const {
        return viewport_w_scaling_enable_;
    }
    
    void SetViewportSwizzleCount(uint32_t const& element) {
        if (element == viewport_swizzle_count_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        viewport_swizzle_count_ = element;
    }
    uint32_t const& GetViewportSwizzleCount() const {
        return viewport_swizzle_count_;
    }
    
    void SetViewportSwizzle(uint32_t index, VkViewportSwizzleNV const& element) {
        if (element == viewport_swizzles_[index]) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        viewport_swizzles_[index] = element;
    }
    VkViewportSwizzleNV const& GetViewportSwizzle(uint32_t index) const {
        return viewport_swizzles_[index];
    }
    VkViewportSwizzleNV const* GetViewportSwizzlePtr() const {
        return viewport_swizzles_;
    }
    
    void SetShadingRateImageEnable(VkBool32 const& element) {
        if (element == shading_rate_image_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        shading_rate_image_enable_ = element;
    }
    VkBool32 const& GetShadingRateImageEnable() const {
        return shading_rate_image_enable_;
    }
    
    void SetRepresentativeFragmentTestEnable(VkBool32 const& element) {
        if (element == representative_fragment_test_enable_) {
            return;
        }
        dirty_hash_bits_.set(EXTENDED_DYNAMIC_STATE_3);
        MarkDirty();
        representative_fragment_test_enable_ = element;
    }
    VkBool32 const& GetRepresentativeFragmentTestEnable() const {
        return representative_fragment_test_enable_;
    }
    
    void SetVertexInputAttributeDescription(uint32_t index, VkVertexInputAttributeDescription const& element) {
        if (element == vertex_input_attribute_descriptions_[index]) {
            return;
        }
        dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
        MarkDirty();
        vertex_input_attribute_descriptions_[index] = element;
    }
    VkVertexInputAttributeDescription const& GetVertexInputAttributeDescription(uint32_t index) const {
        return vertex_input_attribute_descriptions_[index];
    }
    VkVertexInputAttributeDescription const* GetVertexInputAttributeDescriptionPtr() const {
        return vertex_input_attribute_descriptions_;
    }
    
    void SetVertexInputBindingDescription(uint32_t index, VkVertexInputBindingDescription const& element) {
        if (element == vertex_input_binding_descriptions_[index]) {
            return;
        }
        dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
        MarkDirty();
        vertex_input_binding_descriptions_[index] = element;
    }
    VkVertexInputBindingDescription const& GetVertexInputBindingDescription(uint32_t index) const {
        return vertex_input_binding_descriptions_[index];
    }
    VkVertexInputBindingDescription const* GetVertexInputBindingDescriptionPtr() const {
        return vertex_input_binding_descriptions_;
    }
    
    void SetNumVertexInputAttributeDescriptions(uint32_t const& element) {
        if (element == num_vertex_input_attribute_descriptions_) {
            return;
        }
        dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
        MarkDirty();
        num_vertex_input_attribute_descriptions_ = element;
    }
    uint32_t const& GetNumVertexInputAttributeDescriptions() const {
        return num_vertex_input_attribute_descriptions_;
    }
    
    void SetNumVertexInputBindingDescriptions(uint32_t const& element) {
        if (element == num_vertex_input_binding_descriptions_) {
            return;
        }
        dirty_hash_bits_.set(VERTEX_INPUT_DYNAMIC);
        MarkDirty();
        num_vertex_input_binding_descriptions_ = element;
    }
    uint32_t const& GetNumVertexInputBindingDescriptions() const {
        return num_vertex_input_binding_descriptions_;
    }
    
    bool operator==(FullDrawStateData const& o) const {
        if (!(o.depth_attachment_format_ == depth_attachment_format_)) {
            return false;
        }

        if (!(o.stencil_attachment_format_ == stencil_attachment_format_)) {
            return false;
        }

        if (o.limits_.max_color_attachments != limits_.max_color_attachments) {
            return false;
        }
        for (uint32_t i = 0; i < limits_.max_color_attachments; ++i) {
            if (!(o.color_attachment_formats_[i] == color_attachment_formats_[i])) {
                return false;
            }
        }

        if (!(o.num_color_attachments_ == num_color_attachments_)) {
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

    bool CompareStateSubset(FullDrawStateData const& o, VkGraphicsPipelineLibraryFlagBitsEXT flag) const {
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

private:
    size_t CalculatePartialHash(StateGroup state_group) const {
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
                res = res * 31 + std::hash<VkFormat>()(depth_attachment_format_);
                res = res * 31 + std::hash<VkFormat>()(stencil_attachment_format_);
                // TODO: array comparison
                res = res * 31 + std::hash<uint32_t>()(num_color_attachments_);
                // TODO: array comparison
                // TODO: array comparison
                return res;
            }
        }
    }

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
    uint32_t patch_control_points_{};
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
