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

#define ENTRY_POINTS_INSTANCE\
    ENTRY_POINT(GetInstanceProcAddr)\
    ENTRY_POINT(CreateInstance)\
    ENTRY_POINT(DestroyInstance)\
    ENTRY_POINT(EnumerateDeviceExtensionProperties)\
    ENTRY_POINT(GetPhysicalDeviceFeatures2)\
    ENTRY_POINT_ALIAS(GetPhysicalDeviceFeatures2KHR, GetPhysicalDeviceFeatures2)\
    ENTRY_POINT(GetPhysicalDeviceProperties2)\
    ENTRY_POINT_ALIAS(GetPhysicalDeviceProperties2KHR, GetPhysicalDeviceProperties2)\
    ENTRY_POINT(CreateDevice)

#define ENTRY_POINTS_DEVICE\
    ENTRY_POINT(GetDeviceProcAddr)\
    ENTRY_POINT(DestroyDevice)\
    ENTRY_POINT(CreateShadersEXT)\
    ENTRY_POINT(DestroyShaderEXT)\
    ENTRY_POINT(CmdBindShadersEXT)\
    ENTRY_POINT(CmdBindPipeline)\
    ENTRY_POINT(CreateImageView)\
    ENTRY_POINT(DestroyImageView)\
    ENTRY_POINT(AllocateCommandBuffers)\
    ENTRY_POINT(FreeCommandBuffers)\
    ENTRY_POINT(DestroyCommandPool)\
    ENTRY_POINT(BeginCommandBuffer)\
    ENTRY_POINT(CmdBeginRendering)\
    ENTRY_POINT_ALIAS(CmdBeginRenderingKHR, CmdBeginRendering)\
    ENTRY_POINT(CmdBeginTransformFeedbackEXT)\
    ENTRY_POINT(GetShaderBinaryDataEXT)\
    ENTRY_POINT(CmdSetViewportWithCount)\
    ENTRY_POINT_ALIAS(CmdSetViewportWithCountEXT, CmdSetViewportWithCount)\
    ENTRY_POINT(CmdSetScissorWithCount)\
    ENTRY_POINT_ALIAS(CmdSetScissorWithCountEXT, CmdSetScissorWithCount)\
    ENTRY_POINT(CmdSetVertexInputEXT)\
    ENTRY_POINT(CmdSetPrimitiveTopology)\
    ENTRY_POINT_ALIAS(CmdSetPrimitiveTopologyEXT, CmdSetPrimitiveTopology)\
    ENTRY_POINT(CmdSetPrimitiveRestartEnable)\
    ENTRY_POINT_ALIAS(CmdSetPrimitiveRestartEnableEXT, CmdSetPrimitiveRestartEnable)\
    ENTRY_POINT(CmdSetRasterizerDiscardEnable)\
    ENTRY_POINT_ALIAS(CmdSetRasterizerDiscardEnableEXT, CmdSetRasterizerDiscardEnable)\
    ENTRY_POINT(CmdSetRasterizationSamplesEXT)\
    ENTRY_POINT(CmdSetPolygonModeEXT)\
    ENTRY_POINT(CmdSetCullMode)\
    ENTRY_POINT_ALIAS(CmdSetCullModeEXT, CmdSetCullMode)\
    ENTRY_POINT(CmdSetFrontFace)\
    ENTRY_POINT_ALIAS(CmdSetFrontFaceEXT, CmdSetFrontFace)\
    ENTRY_POINT(CmdSetDepthTestEnable)\
    ENTRY_POINT_ALIAS(CmdSetDepthTestEnableEXT, CmdSetDepthTestEnable)\
    ENTRY_POINT(CmdSetDepthWriteEnable)\
    ENTRY_POINT_ALIAS(CmdSetDepthWriteEnableEXT, CmdSetDepthWriteEnable)\
    ENTRY_POINT(CmdSetDepthCompareOp)\
    ENTRY_POINT_ALIAS(CmdSetDepthCompareOpEXT, CmdSetDepthCompareOp)\
    ENTRY_POINT(CmdSetStencilTestEnable)\
    ENTRY_POINT_ALIAS(CmdSetStencilTestEnableEXT, CmdSetStencilTestEnable)\
    ENTRY_POINT(CmdSetLogicOpEnableEXT)\
    ENTRY_POINT(CmdSetColorBlendEnableEXT)\
    ENTRY_POINT(CmdSetColorBlendEquationEXT)\
    ENTRY_POINT(CmdSetColorWriteMaskEXT)\
    ENTRY_POINT(CmdSetDepthBoundsTestEnable)\
    ENTRY_POINT_ALIAS(CmdSetDepthBoundsTestEnableEXT, CmdSetDepthBoundsTestEnable)\
    ENTRY_POINT(CmdSetDepthBiasEnable)\
    ENTRY_POINT_ALIAS(CmdSetDepthBiasEnableEXT, CmdSetDepthBiasEnable)\
    ENTRY_POINT(CmdSetDepthClampEnableEXT)\
    ENTRY_POINT(CmdSetStencilOp)\
    ENTRY_POINT_ALIAS(CmdSetStencilOpEXT, CmdSetStencilOp)\
    ENTRY_POINT(CmdDraw)\
    ENTRY_POINT(CmdDrawIndirect)\
    ENTRY_POINT(CmdDrawIndirectCount)\
    ENTRY_POINT(CmdDrawIndexed)\
    ENTRY_POINT(CmdDrawIndexedIndirect)\
    ENTRY_POINT(CmdDrawIndexedIndirectCount)\
    ENTRY_POINT(CmdDrawMeshTasksEXT)\
    ENTRY_POINT(CmdDrawMeshTasksIndirectEXT)\
    ENTRY_POINT(CmdDrawMeshTasksIndirectCountEXT)\
    ENTRY_POINT(CmdDrawMeshTasksNV)\
    ENTRY_POINT(CmdDrawMeshTasksIndirectNV)\
    ENTRY_POINT(CmdDrawMeshTasksIndirectCountNV)\
    ENTRY_POINT(CmdSetLogicOpEXT)\
    ENTRY_POINT(CmdSetPatchControlPointsEXT)\
    ENTRY_POINT(CmdSetTessellationDomainOriginEXT)\
    ENTRY_POINT(CmdSetAlphaToOneEnableEXT)\
    ENTRY_POINT(CmdSetAlphaToCoverageEnableEXT)\
    ENTRY_POINT(CmdSetSampleMaskEXT)\
    ENTRY_POINT(CmdSetRasterizationStreamEXT)\
    ENTRY_POINT(CmdSetConservativeRasterizationModeEXT)\
    ENTRY_POINT(CmdSetExtraPrimitiveOverestimationSizeEXT)\
    ENTRY_POINT(CmdSetDepthClipEnableEXT)\
    ENTRY_POINT(CmdSetSampleLocationsEnableEXT)\
    ENTRY_POINT(CmdSetProvokingVertexModeEXT)\
    ENTRY_POINT(CmdSetLineRasterizationModeEXT)\
    ENTRY_POINT(CmdSetLineStippleEnableEXT)\
    ENTRY_POINT(CmdSetDepthClipNegativeOneToOneEXT)\
    ENTRY_POINT(CmdSetCoverageModulationModeNV)\
    ENTRY_POINT(CmdSetCoverageModulationTableEnableNV)\
    ENTRY_POINT(CmdSetCoverageModulationTableNV)\
    ENTRY_POINT(CmdSetCoverageReductionModeNV)\
    ENTRY_POINT(CmdSetCoverageToColorEnableNV)\
    ENTRY_POINT(CmdSetCoverageToColorLocationNV)\
    ENTRY_POINT(CmdSetViewportWScalingEnableNV)\
    ENTRY_POINT(CmdSetViewportSwizzleNV)\
    ENTRY_POINT(CmdSetShadingRateImageEnableNV)\
    ENTRY_POINT(CmdSetRepresentativeFragmentTestEnableNV)\
    ENTRY_POINT(CmdBindVertexBuffers2)\
    ENTRY_POINT_ALIAS(CmdBindVertexBuffers2EXT, CmdBindVertexBuffers2)\
    ENTRY_POINT(CmdBindDescriptorSets)\
    ENTRY_POINT(CmdPushDescriptorSetKHR)\
    ENTRY_POINT(CmdPushDescriptorSetWithTemplateKHR)\
    ENTRY_POINT(CmdPushConstants)\
    ENTRY_POINT(SetPrivateData)\
    ENTRY_POINT_ALIAS(SetPrivateDataEXT, SetPrivateData)\
    ENTRY_POINT(GetPrivateData)\
    ENTRY_POINT_ALIAS(GetPrivateDataEXT, GetPrivateData)\
    ENTRY_POINT(CreateDescriptorUpdateTemplate)\
    ENTRY_POINT_ALIAS(CreateDescriptorUpdateTemplateKHR, CreateDescriptorUpdateTemplate)\
    ENTRY_POINT(DestroyDescriptorUpdateTemplate)\
    ENTRY_POINT_ALIAS(DestroyDescriptorUpdateTemplateKHR, DestroyDescriptorUpdateTemplate)

#define ADDITIONAL_INSTANCE_FUNCTIONS\
    ENTRY_POINT(GetPhysicalDeviceProperties)\
    ENTRY_POINT(GetPhysicalDeviceImageFormatProperties)

#define ADDITIONAL_DEVICE_FUNCTIONS\
    ENTRY_POINT(CreatePipelineLayout)\
    ENTRY_POINT(CreateGraphicsPipelines)\
    ENTRY_POINT(CreateComputePipelines)\
    ENTRY_POINT(DestroyPipelineLayout)\
    ENTRY_POINT(DestroyPipeline)\
    ENTRY_POINT(CreateShaderModule)\
    ENTRY_POINT(DestroyShaderModule)\
    ENTRY_POINT(CreatePipelineCache)\
    ENTRY_POINT(DestroyPipelineCache)\
    ENTRY_POINT(GetPipelineCacheData)\
    ENTRY_POINT(MergePipelineCaches)\
    ENTRY_POINT(GetDeviceQueue)\
    ENTRY_POINT(QueueSubmit)\
    ENTRY_POINT(QueueSubmit2)\
    ENTRY_POINT_ALIAS(QueueSubmit2KHR, QueueSubmit2)\
    ENTRY_POINT(CmdSetViewport)\
    ENTRY_POINT(CmdSetScissor)\
    ENTRY_POINT(CreatePrivateDataSlotEXT)\
    ENTRY_POINT(DestroyPrivateDataSlotEXT)\
    ENTRY_POINT(CmdBindVertexBuffers)

