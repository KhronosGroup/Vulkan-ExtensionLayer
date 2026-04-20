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

if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetCullMode", pName) == 0 || strcmp("vkCmdSetCullModeEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetDepthBoundsTestEnable", pName) == 0 || strcmp("vkCmdSetDepthBoundsTestEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetDepthCompareOp", pName) == 0 || strcmp("vkCmdSetDepthCompareOpEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetDepthTestEnable", pName) == 0 || strcmp("vkCmdSetDepthTestEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetDepthWriteEnable", pName) == 0 || strcmp("vkCmdSetDepthWriteEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetFrontFace", pName) == 0 || strcmp("vkCmdSetFrontFaceEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetPrimitiveTopology", pName) == 0 || strcmp("vkCmdSetPrimitiveTopologyEXT", pName) == 0) &&
    (flags & DeviceData::HAS_PRIMITIVE_TOPLOGY_UNRESTRICTED) != 0) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetScissorWithCount", pName) == 0 || strcmp("vkCmdSetScissorWithCountEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetViewportWithCount", pName) == 0 || strcmp("vkCmdSetViewportWithCountEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetStencilOp", pName) == 0 || strcmp("vkCmdSetStencilOpEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdSetStencilTestEnable", pName) == 0 || strcmp("vkCmdSetStencilTestEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state.extendedDynamicState == VK_TRUE &&
    (strcmp("vkCmdBindVertexBuffers2", pName) == 0 || strcmp("vkCmdBindVertexBuffers2EXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state.extendedDynamicState == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state2.extendedDynamicState2LogicOp == VK_TRUE && (strcmp("vkCmdSetLogicOpEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state2.extendedDynamicState2LogicOp == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state2.extendedDynamicState2 == VK_TRUE &&
    (strcmp("vkCmdSetPrimitiveRestartEnable", pName) == 0 || strcmp("vkCmdSetPrimitiveRestartEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state2.extendedDynamicState2 == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state2.extendedDynamicState2 == VK_TRUE &&
    (strcmp("vkCmdSetRasterizerDiscardEnable", pName) == 0 || strcmp("vkCmdSetRasterizerDiscardEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state2.extendedDynamicState2 == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state2.extendedDynamicState2 == VK_TRUE &&
    (strcmp("vkCmdSetDepthBiasEnable", pName) == 0 || strcmp("vkCmdSetDepthBiasEnableEXT", pName) == 0)) {
    DEBUG_LOG("not intercepting %s because real dynamic state exists (extended_dynamic_state2.extendedDynamicState2 == VK_TRUE)\n",
              pName);
    return nullptr;
}
if (extended_dynamic_state2.extendedDynamicState2PatchControlPoints == VK_TRUE &&
    (strcmp("vkCmdSetPatchControlPointsEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state2.extendedDynamicState2PatchControlPoints == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3PolygonMode == VK_TRUE && (strcmp("vkCmdSetPolygonModeEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3PolygonMode == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3RasterizationSamples == VK_TRUE &&
    (strcmp("vkCmdSetRasterizationSamplesEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3RasterizationSamples "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3LogicOpEnable == VK_TRUE && (strcmp("vkCmdSetLogicOpEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3LogicOpEnable == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ColorWriteMask == VK_TRUE && (strcmp("vkCmdSetColorWriteMaskEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3ColorWriteMask == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ColorBlendEnable == VK_TRUE &&
    (strcmp("vkCmdSetColorBlendEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3ColorBlendEnable == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ColorBlendEquation == VK_TRUE &&
    (strcmp("vkCmdSetColorBlendEquationEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3ColorBlendEquation == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3DepthClampEnable == VK_TRUE &&
    (strcmp("vkCmdSetDepthClampEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3DepthClampEnable == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3TessellationDomainOrigin == VK_TRUE &&
    (strcmp("vkCmdSetTessellationDomainOriginEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3TessellationDomainOrigin == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3AlphaToOneEnable == VK_TRUE &&
    (strcmp("vkCmdSetAlphaToOneEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3AlphaToOneEnable == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3AlphaToCoverageEnable == VK_TRUE &&
    (strcmp("vkCmdSetAlphaToCoverageEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3AlphaToCoverageEnable "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3SampleMask == VK_TRUE && (strcmp("vkCmdSetSampleMaskEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3SampleMask == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3RasterizationStream == VK_TRUE &&
    (strcmp("vkCmdSetRasterizationStreamEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3RasterizationStream "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ConservativeRasterizationMode == VK_TRUE &&
    (strcmp("vkCmdSetConservativeRasterizationModeEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3ConservativeRasterizationMode == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ExtraPrimitiveOverestimationSize == VK_TRUE &&
    (strcmp("vkCmdSetExtraPrimitiveOverestimationSizeEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3ExtraPrimitiveOverestimationSize == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3DepthClipEnable == VK_TRUE && (strcmp("vkCmdSetDepthClipEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3DepthClipEnable == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3SampleLocationsEnable == VK_TRUE &&
    (strcmp("vkCmdSetSampleLocationsEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3SampleLocationsEnable "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ProvokingVertexMode == VK_TRUE &&
    (strcmp("vkCmdSetProvokingVertexModeEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3ProvokingVertexMode "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3LineRasterizationMode == VK_TRUE &&
    (strcmp("vkCmdSetLineRasterizationModeEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3LineRasterizationMode "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3LineStippleEnable == VK_TRUE &&
    (strcmp("vkCmdSetLineStippleEnableEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3LineStippleEnable == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3DepthClipNegativeOneToOne == VK_TRUE &&
    (strcmp("vkCmdSetDepthClipNegativeOneToOneEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3DepthClipNegativeOneToOne == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3CoverageModulationMode == VK_TRUE &&
    (strcmp("vkCmdSetCoverageModulationModeNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3CoverageModulationMode == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3CoverageModulationTableEnable == VK_TRUE &&
    (strcmp("vkCmdSetCoverageModulationTableEnableNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3CoverageModulationTableEnable == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3CoverageModulationTable == VK_TRUE &&
    (strcmp("vkCmdSetCoverageModulationTableNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3CoverageModulationTable == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3CoverageReductionMode == VK_TRUE &&
    (strcmp("vkCmdSetCoverageReductionModeNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3CoverageReductionMode "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3CoverageToColorEnable == VK_TRUE &&
    (strcmp("vkCmdSetCoverageToColorEnableNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3CoverageToColorEnable "
        "== VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3CoverageToColorLocation == VK_TRUE &&
    (strcmp("vkCmdSetCoverageToColorLocationNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3CoverageToColorLocation == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ViewportWScalingEnable == VK_TRUE &&
    (strcmp("vkCmdSetViewportWScalingEnableNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3ViewportWScalingEnable == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ViewportSwizzle == VK_TRUE && (strcmp("vkCmdSetViewportSwizzleNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (extended_dynamic_state3.extendedDynamicState3ViewportSwizzle == "
        "VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3ShadingRateImageEnable == VK_TRUE &&
    (strcmp("vkCmdSetShadingRateImageEnableNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3ShadingRateImageEnable == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (extended_dynamic_state3.extendedDynamicState3RepresentativeFragmentTestEnable == VK_TRUE &&
    (strcmp("vkCmdSetRepresentativeFragmentTestEnableNV", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists "
        "(extended_dynamic_state3.extendedDynamicState3RepresentativeFragmentTestEnable == VK_TRUE)\n",
        pName);
    return nullptr;
}
if (vertex_input_dynamic_state.vertexInputDynamicState == VK_TRUE && (strcmp("vkCmdSetVertexInputEXT", pName) == 0)) {
    DEBUG_LOG(
        "not intercepting %s because real dynamic state exists (vertex_input_dynamic_state.vertexInputDynamicState == VK_TRUE)\n",
        pName);
    return nullptr;
}
