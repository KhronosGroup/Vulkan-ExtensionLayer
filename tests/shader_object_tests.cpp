/* Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Nintendo
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
 * Author: Ziga Markus <ziga@lunarg.com>
 */

#include <type_traits>

#include "extension_layer_tests.h"
#include "shader_object_tests.h"
#include "vk_layer_config.h"

void ShaderObjectTest::SetUp() {
    SetEnvironment("VK_SHADER_OBJECT_FORCE_ENABLE", "1");
    const std::string layer_path = std::string(SHADER_OBJECT_BINARY_PATH);
    SetEnvironment("VK_LAYER_PATH", layer_path.c_str());
    VkExtensionLayerTest::SetUp();
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkExtensionLayerTest::AddSurfaceInstanceExtension();
    instance_layers_.push_back("VK_LAYER_KHRONOS_shader_object");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkExtensionLayerTest::AddSwapchainDeviceExtension();
}

void ShaderObjectTest::TearDown() {}

void ShaderObjectTest::BindDefaultDynamicStates(VkBuffer buffer) {
    PFN_vkCmdSetViewportWithCountEXT vkCmdSetViewportWithCountEXT =
        (PFN_vkCmdSetViewportWithCountEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetViewportWithCountEXT");
    PFN_vkCmdSetScissorWithCountEXT vkCmdSetScissorWithCountEXT =
        (PFN_vkCmdSetScissorWithCountEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetScissorWithCountEXT");
    PFN_vkCmdBindVertexBuffers2EXT vkCmdBindVertexBuffers2EXT =
        (PFN_vkCmdBindVertexBuffers2EXT)vk::GetInstanceProcAddr(instance(), "vkCmdBindVertexBuffers2EXT");
    PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT =
        (PFN_vkCmdSetCullModeEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetCullModeEXT");
    PFN_vkCmdSetDepthBoundsTestEnableEXT vkCmdSetDepthBoundsTestEnableEXT =
        (PFN_vkCmdSetDepthBoundsTestEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetDepthBoundsTestEnableEXT");
    PFN_vkCmdSetDepthCompareOpEXT vkCmdSetDepthCompareOpEXT =
        (PFN_vkCmdSetDepthCompareOpEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetDepthCompareOpEXT");
    PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT =
        (PFN_vkCmdSetDepthTestEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetDepthTestEnableEXT");
    PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT =
        (PFN_vkCmdSetDepthWriteEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetDepthWriteEnableEXT");
    PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT =
        (PFN_vkCmdSetFrontFaceEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetFrontFaceEXT");
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetPrimitiveTopologyEXT");
    PFN_vkCmdSetStencilOpEXT vkCmdSetStencilOpEXT =
        (PFN_vkCmdSetStencilOpEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetStencilOpEXT");
    PFN_vkCmdSetStencilTestEnableEXT vkCmdSetStencilTestEnableEXT =
        (PFN_vkCmdSetStencilTestEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetStencilTestEnableEXT");
    PFN_vkCmdSetDepthBiasEnableEXT vkCmdSetDepthBiasEnableEXT =
        (PFN_vkCmdSetDepthBiasEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetDepthBiasEnableEXT");
    PFN_vkCmdSetPrimitiveRestartEnableEXT vkCmdSetPrimitiveRestartEnableEXT =
        (PFN_vkCmdSetPrimitiveRestartEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetPrimitiveRestartEnableEXT");
    PFN_vkCmdSetRasterizerDiscardEnableEXT vkCmdSetRasterizerDiscardEnableEXT =
        (PFN_vkCmdSetRasterizerDiscardEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetRasterizerDiscardEnableEXT");
    PFN_vkCmdSetVertexInputEXT vkCmdSetVertexInputEXT =
        (PFN_vkCmdSetVertexInputEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetVertexInputEXT");
    PFN_vkCmdSetLogicOpEXT vkCmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetLogicOpEXT");
    PFN_vkCmdSetPatchControlPointsEXT vkCmdSetPatchControlPointsEXT =
        (PFN_vkCmdSetPatchControlPointsEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetPatchControlPointsEXT");
    PFN_vkCmdSetTessellationDomainOriginEXT vkCmdSetTessellationDomainOriginEXT =
        (PFN_vkCmdSetTessellationDomainOriginEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetTessellationDomainOriginEXT");
    PFN_vkCmdSetDepthClampEnableEXT vkCmdSetDepthClampEnableEXT =
        (PFN_vkCmdSetDepthClampEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetDepthClampEnableEXT");
    PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT =
        (PFN_vkCmdSetPolygonModeEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetPolygonModeEXT");
    PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT =
        (PFN_vkCmdSetRasterizationSamplesEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetRasterizationSamplesEXT");
    PFN_vkCmdSetSampleMaskEXT vkCmdSetSampleMaskEXT =
        (PFN_vkCmdSetSampleMaskEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetSampleMaskEXT");
    PFN_vkCmdSetAlphaToCoverageEnableEXT vkCmdSetAlphaToCoverageEnableEXT =
        (PFN_vkCmdSetAlphaToCoverageEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetAlphaToCoverageEnableEXT");
    PFN_vkCmdSetAlphaToOneEnableEXT vkCmdSetAlphaToOneEnableEXT =
        (PFN_vkCmdSetAlphaToOneEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetAlphaToOneEnableEXT");
    PFN_vkCmdSetLogicOpEnableEXT vkCmdSetLogicOpEnableEXT =
        (PFN_vkCmdSetLogicOpEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetLogicOpEnableEXT");
    PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT =
        (PFN_vkCmdSetColorBlendEnableEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetColorBlendEnableEXT");
    PFN_vkCmdSetColorBlendEquationEXT vkCmdSetColorBlendEquationEXT =
        (PFN_vkCmdSetColorBlendEquationEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetColorBlendEquationEXT");
    PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT =
        (PFN_vkCmdSetColorWriteMaskEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetColorWriteMaskEXT");

    VkCommandBuffer cmdBuffer = m_commandBuffer->handle();
    VkViewport viewport = {0, 0, m_width, m_height, 0.0f, 1.0f};
    VkRect2D scissor = {{
                            0,
                            0,
                        },
                        {
                            static_cast<uint32_t>(m_width),
                            static_cast<uint32_t>(m_height),
                        }};
    vkCmdSetViewportWithCountEXT(cmdBuffer, 1u, &viewport);
    vkCmdSetScissorWithCountEXT(cmdBuffer, 1u, &scissor);
    vk::CmdSetLineWidth(cmdBuffer, 1.0f);
    vk::CmdSetDepthBias(cmdBuffer, 1.0f, 1.0f, 1.0f);
    float blendConstants[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vk::CmdSetBlendConstants(cmdBuffer, blendConstants);
    vk::CmdSetDepthBounds(cmdBuffer, 0.0f, 1.0f);
    vk::CmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    VkDeviceSize offset = 0u;
    VkDeviceSize size = sizeof(float);
    vkCmdBindVertexBuffers2EXT(cmdBuffer, 0, 1, &buffer, &offset, &size, &size);
    vkCmdSetCullModeEXT(cmdBuffer, VK_CULL_MODE_NONE);
    vkCmdSetDepthBoundsTestEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetDepthCompareOpEXT(cmdBuffer, VK_COMPARE_OP_NEVER);
    vkCmdSetDepthTestEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetDepthWriteEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetFrontFaceEXT(cmdBuffer, VK_FRONT_FACE_CLOCKWISE);
    vkCmdSetPrimitiveTopologyEXT(cmdBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    vkCmdSetStencilOpEXT(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
                         VK_COMPARE_OP_NEVER);
    vkCmdSetStencilTestEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetDepthBiasEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetPrimitiveRestartEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetRasterizerDiscardEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetVertexInputEXT(cmdBuffer, 0u, nullptr, 0u, nullptr);
    vkCmdSetLogicOpEXT(cmdBuffer, VK_LOGIC_OP_COPY);
    vkCmdSetPatchControlPointsEXT(cmdBuffer, 4u);
    vkCmdSetTessellationDomainOriginEXT(cmdBuffer, VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
    vkCmdSetDepthClampEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    vkCmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 0xFFFFFFFF;
    vkCmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vkCmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetAlphaToOneEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetLogicOpEnableEXT(cmdBuffer, VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    vkCmdSetColorBlendEnableEXT(cmdBuffer, 0u, 1u, &colorBlendEnable);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE,  // VkBlendFactor	srcColorBlendFactor;
        VK_BLEND_FACTOR_ONE,  // VkBlendFactor	dstColorBlendFactor;
        VK_BLEND_OP_ADD,      // VkBlendOp		colorBlendOp;
        VK_BLEND_FACTOR_ONE,  // VkBlendFactor	srcAlphaBlendFactor;
        VK_BLEND_FACTOR_ONE,  // VkBlendFactor	dstAlphaBlendFactor;
        VK_BLEND_OP_ADD,      // VkBlendOp		alphaBlendOp;
    };
    vkCmdSetColorBlendEquationEXT(cmdBuffer, 0u, 1u, &colorBlendEquation);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vkCmdSetColorWriteMaskEXT(cmdBuffer, 0u, 1u, &colorWriteMask);
}
