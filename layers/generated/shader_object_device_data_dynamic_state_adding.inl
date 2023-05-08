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

if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_CULL_MODE_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_FRONT_FACE_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_STENCIL_OP_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT);
}
if (extended_dynamic_state_1_ptr && extended_dynamic_state_1_ptr->extendedDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT);
}
if (extended_dynamic_state_2_ptr && extended_dynamic_state_2_ptr->extendedDynamicState2LogicOp == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
}
if (extended_dynamic_state_2_ptr && extended_dynamic_state_2_ptr->extendedDynamicState2 == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
}
if (extended_dynamic_state_2_ptr && extended_dynamic_state_2_ptr->extendedDynamicState2 == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
}
if (extended_dynamic_state_2_ptr && extended_dynamic_state_2_ptr->extendedDynamicState2 == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT);
}
if (extended_dynamic_state_2_ptr && extended_dynamic_state_2_ptr->extendedDynamicState2PatchControlPoints == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3PolygonMode == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_POLYGON_MODE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3RasterizationSamples == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3LogicOpEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ColorWriteMask == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ColorBlendEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ColorBlendEquation == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3DepthClampEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3TessellationDomainOrigin == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3AlphaToOneEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3AlphaToCoverageEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3SampleMask == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3RasterizationStream == VK_TRUE && (enabled_additional_extensions & TRANSFORM_FEEDBACK) != 0 && transform_feedback_ptr && transform_feedback_ptr->transformFeedback == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ConservativeRasterizationMode == VK_TRUE && (enabled_additional_extensions & CONSERVATIVE_RASTERIZATION) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ExtraPrimitiveOverestimationSize == VK_TRUE && (enabled_additional_extensions & CONSERVATIVE_RASTERIZATION) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3DepthClipEnable == VK_TRUE && (enabled_additional_extensions & DEPTH_CLIP_ENABLE) != 0 && depth_clip_enable_ptr && depth_clip_enable_ptr->depthClipEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3SampleLocationsEnable == VK_TRUE && (enabled_additional_extensions & SAMPLE_LOCATIONS) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ProvokingVertexMode == VK_TRUE && (enabled_additional_extensions & PROVOKING_VERTEX) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3LineRasterizationMode == VK_TRUE && (enabled_additional_extensions & LINE_RASTERIZATION) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3LineStippleEnable == VK_TRUE && (enabled_additional_extensions & LINE_RASTERIZATION) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3DepthClipNegativeOneToOne == VK_TRUE && (enabled_additional_extensions & DEPTH_CLIP_CONTROL) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3CoverageModulationMode == VK_TRUE && (enabled_additional_extensions & NV_FRAMEBUFFER_MIXED_SAMPLES) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3CoverageModulationTableEnable == VK_TRUE && (enabled_additional_extensions & NV_FRAMEBUFFER_MIXED_SAMPLES) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3CoverageModulationTable == VK_TRUE && (enabled_additional_extensions & NV_FRAMEBUFFER_MIXED_SAMPLES) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3CoverageReductionMode == VK_TRUE && (enabled_additional_extensions & NV_COVERAGE_REDUCTION_MODE) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3CoverageToColorEnable == VK_TRUE && (enabled_additional_extensions & NV_FRAGMENT_COVERAGE_TO_COLOR) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3CoverageToColorLocation == VK_TRUE && (enabled_additional_extensions & NV_FRAGMENT_COVERAGE_TO_COLOR) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ViewportWScalingEnable == VK_TRUE && (enabled_additional_extensions & NV_CLIP_SPACE_W_SCALING) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ViewportSwizzle == VK_TRUE && (enabled_additional_extensions & NV_VIEWPORT_SWIZZLE) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3ShadingRateImageEnable == VK_TRUE && (enabled_additional_extensions & NV_SHADING_RATE_IMAGE) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV);
}
if (extended_dynamic_state_3_ptr && extended_dynamic_state_3_ptr->extendedDynamicState3RepresentativeFragmentTestEnable == VK_TRUE && (enabled_additional_extensions & NV_REPRESENTATIVE_FRAGMENT_TEST) != 0) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV);
}
if (vertex_input_dynamic_ptr && vertex_input_dynamic_ptr->vertexInputDynamicState == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
}
if (color_write_enable_ptr && color_write_enable_ptr->colorWriteEnable == VK_TRUE) {
    device_data->AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
}
