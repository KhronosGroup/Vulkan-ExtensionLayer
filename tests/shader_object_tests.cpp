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

void ShaderObjectTest::BindDefaultDynamicStates(VkBuffer buffer, bool tessellation) {
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
    vkCmdSetPrimitiveTopologyEXT(cmdBuffer, tessellation ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
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

void ShaderObjectTest::SubmitAndWait() {
    VkCommandBuffer commandBufferHandle = m_commandBuffer->handle();
    VkSubmitInfo submitInfo = LvlInitStruct<VkSubmitInfo>();
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBufferHandle;
    vk::QueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(ShaderObjectTest, VertFragShader) {
    TEST_DESCRIPTION("Test drawing with a vertex and fragment shader");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    m_errorMonitor->ExpectSuccess();

    static const char vertSource[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char fragSource[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<unsigned int> spv[2];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, vertSource, spv[0], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_FRAGMENT_BIT, fragSource, spv[1], false, 0);

    VkShaderStageFlagBits unusedShaderStages[] = {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT};
    VkShaderEXT shaders[2];
    VkShaderCreateInfoEXT createInfos[2];
    for (uint32_t i = 0; i < 2; ++i) {
        createInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfos[i].stage = shaderStages[i];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i].size() * sizeof(unsigned int);
        createInfos[i].pCode = spv[i].data();
        createInfos[i].pName = "main";
    }

    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    VkBufferObj buffer;
    buffer.init(*m_device, sizeof(float) * 4u, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferObj vertexBuffer;
    vertexBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkImageCreateInfo imageInfo = LvlInitStruct<VkImageCreateInfo>();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

    VkRenderingAttachmentInfo color_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = LvlInitStruct<VkRenderingInfo>();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
        (PFN_vkCmdEndRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdEndRenderingKHR");

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vkCmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2u, shaderStages, shaders);
    for (const auto& unusedShader : unusedShaderStages) {
        VkShaderEXT null_shader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &unusedShader, &null_shader);
    }
    BindDefaultDynamicStates(vertexBuffer.handle(), false);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    vkCmdEndRenderingKHR(m_commandBuffer->handle());

    {
        VkImageMemoryBarrier imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);
    }

    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0u;
    copyRegion.imageSubresource.baseArrayLayer = 0u;
    copyRegion.imageSubresource.layerCount = 1u;
    copyRegion.imageOffset.x = static_cast<int32_t>(m_width / 2) + 1;
    copyRegion.imageOffset.y = static_cast<int32_t>(m_height / 2) + 1;
    copyRegion.imageExtent.width = 1u;
    copyRegion.imageExtent.height = 1u;
    copyRegion.imageExtent.depth = 1u;

    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1u, &copyRegion);

    m_commandBuffer->end();

    SubmitAndWait();

    float* data;
    vk::MapMemory(m_device->handle(), buffer.memory().handle(), 0u, sizeof(float) * 4u, 0u, (void**)&data);
    for (uint32_t i = 0; i < 4; ++i) {
        if (data[i] != 0.2f + i * 0.2f) {
            m_errorMonitor->SetError("Wrong pixel value");
        }
    }

    vk::UnmapMemory(m_device->handle(), buffer.memory().handle());

    vk::DestroyShaderEXT(m_device->handle(), shaders[0], nullptr);
    vk::DestroyShaderEXT(m_device->handle(), shaders[1], nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(ShaderObjectTest, LinkedShadersDraw) {
    TEST_DESCRIPTION("Test drawing using linked shaders");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    m_errorMonitor->ExpectSuccess();

    static const char vertSource[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char fragSource[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<unsigned int> spv[2];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, vertSource, spv[0], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_FRAGMENT_BIT, fragSource, spv[1], false, 0);

    VkShaderEXT shaders[2];
    VkShaderCreateInfoEXT createInfos[2];
    for (uint32_t i = 0; i < 2; ++i) {
        createInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfos[i].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
        createInfos[i].stage = shaderStages[i];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i].size() * sizeof(unsigned int);
        createInfos[i].pCode = spv[i].data();
        createInfos[i].pName = "main";
    }

    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    VkBufferObj vertexBuffer;
    vertexBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkImageCreateInfo imageInfo = LvlInitStruct<VkImageCreateInfo>();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

    VkRenderingAttachmentInfo color_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = LvlInitStruct<VkRenderingInfo>();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
        (PFN_vkCmdEndRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdEndRenderingKHR");

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vkCmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2u, shaderStages, shaders);
    BindDefaultDynamicStates(vertexBuffer.handle(), false);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    vkCmdEndRenderingKHR(m_commandBuffer->handle());

    m_commandBuffer->end();

    SubmitAndWait();

    for (uint32_t i = 0; i < 2; ++i) vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(ShaderObjectTest, AllShadersDraw) {
    TEST_DESCRIPTION("Test drawing using all graphics shader");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    m_errorMonitor->ExpectSuccess();

    static const char vertSource[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char tescSource[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char teseSource[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geomSource[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char fragSource[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<unsigned int> spv[5];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, vertSource, spv[0], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tescSource, spv[1], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, teseSource, spv[2], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_GEOMETRY_BIT, geomSource, spv[3], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_FRAGMENT_BIT, fragSource, spv[4], false, 0);

    VkShaderEXT shaders[5];
    VkShaderCreateInfoEXT createInfos[5];
    for (uint32_t i = 0; i < 5; ++i) {
        createInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfos[i].stage = shaderStages[i];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i].size() * sizeof(unsigned int);
        createInfos[i].pCode = spv[i].data();
        createInfos[i].pName = "main";
    }

    vk::CreateShadersEXT(m_device->handle(), 5u, createInfos, nullptr, shaders);

    VkBufferObj vertexBuffer;
    vertexBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkImageCreateInfo imageInfo = LvlInitStruct<VkImageCreateInfo>();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

    VkRenderingAttachmentInfo color_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = LvlInitStruct<VkRenderingInfo>();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
        (PFN_vkCmdEndRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdEndRenderingKHR");

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vkCmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, shaderStages, shaders);
    BindDefaultDynamicStates(vertexBuffer.handle(), true);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    vkCmdEndRenderingKHR(m_commandBuffer->handle());

    m_commandBuffer->end();

    SubmitAndWait();

    for (uint32_t i = 0; i < 5; ++i) vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(ShaderObjectTest, AllShadersDrawBinary) {
    TEST_DESCRIPTION("Test drawing using all graphics binary shader");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    m_errorMonitor->ExpectSuccess();

    static const char vertSource[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char tescSource[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char teseSource[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geomSource[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char fragSource[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<unsigned int> spv[5];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, vertSource, spv[0], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tescSource, spv[1], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, teseSource, spv[2], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_GEOMETRY_BIT, geomSource, spv[3], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_FRAGMENT_BIT, fragSource, spv[4], false, 0);

    VkShaderEXT shaders[5];
    VkShaderCreateInfoEXT createInfos[5];
    for (uint32_t i = 0; i < 5; ++i) {
        createInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfos[i].stage = shaderStages[i];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i].size() * sizeof(unsigned int);
        createInfos[i].pCode = spv[i].data();
        createInfos[i].pName = "main";
    }

    vk::CreateShadersEXT(m_device->handle(), 5u, createInfos, nullptr, shaders);

    VkShaderEXT binaryShaders[5];
    VkShaderCreateInfoEXT binaryCreateInfos[5];
    size_t dataSize[5];
    std::vector<uint8_t> binaryData[5];
    for (uint32_t i = 0; i < 5; ++i) {
        vk::GetShaderBinaryDataEXT(m_device->handle(), shaders[i], &dataSize[i], nullptr);
        binaryData[i].resize(dataSize[i]);
        vk::GetShaderBinaryDataEXT(m_device->handle(), shaders[i], &dataSize[i], binaryData[i].data());

        binaryCreateInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        binaryCreateInfos[i].stage = shaderStages[i];
        binaryCreateInfos[i].codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
        binaryCreateInfos[i].codeSize = dataSize[i];
        binaryCreateInfos[i].pCode = binaryData[i].data();
        binaryCreateInfos[i].pName = "main";
    }

    vk::CreateShadersEXT(m_device->handle(), 5u, binaryCreateInfos, nullptr, binaryShaders);

    VkBufferObj vertexBuffer;
    vertexBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkImageCreateInfo imageInfo = LvlInitStruct<VkImageCreateInfo>();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

    VkRenderingAttachmentInfo color_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = LvlInitStruct<VkRenderingInfo>();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
        (PFN_vkCmdEndRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdEndRenderingKHR");

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vkCmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, shaderStages, binaryShaders);
    BindDefaultDynamicStates(vertexBuffer.handle(), true);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    vkCmdEndRenderingKHR(m_commandBuffer->handle());

    m_commandBuffer->end();

    SubmitAndWait();

    for (uint32_t i = 0; i < 5; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
        vk::DestroyShaderEXT(m_device->handle(), binaryShaders[i], nullptr);
    }

    m_errorMonitor->VerifyNotFound();
}

TEST_F(ShaderObjectTest, ComputeShader) {
    TEST_DESCRIPTION("Test dispatching with compute shader");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    m_errorMonitor->ExpectSuccess();

    static const char compSource[] = R"glsl(
        #version 450
        layout(local_size_x=16, local_size_x=1, local_size_x=1) in;
        layout(binding = 0) buffer Output {
            uint values[16];
        } buffer_out;

        void main() {
            buffer_out.values[gl_LocalInvocationID.x] = gl_LocalInvocationID.x;
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_COMPUTE_BIT};

    std::vector<unsigned int> spv[5];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_COMPUTE_BIT, compSource, spv[0], false, 0);

    VkBufferObj vertexBuffer;
    vertexBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkBufferObj storageBuffer;
    storageBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool ds_pool;
    ds_pool.init(*m_device, ds_pool_ci);
    ASSERT_TRUE(ds_pool.initialized());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);

    VkDescriptorBufferInfo storage_buffer_info = {storageBuffer.handle(), 0, sizeof(uint32_t)};

    VkWriteDescriptorSet descriptorWrite = LvlInitStruct<VkWriteDescriptorSet>();
    descriptorWrite = LvlInitStruct<VkWriteDescriptorSet>();
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.pBufferInfo = &storage_buffer_info;

    vk::UpdateDescriptorSets(m_device->handle(), 1u, &descriptorWrite, 0u, nullptr);

    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {dsl_binding});
    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout});

    VkDescriptorSetLayout descriptorSetLayout = descriptor_set_layout.handle();

    VkShaderEXT shader;
    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = shaderStages[0];
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv[0].size() * sizeof(unsigned int);
    createInfo.pCode = spv[0].data();
    createInfo.pName = "main";
    createInfo.setLayoutCount = 1u;
    createInfo.pSetLayouts = &descriptorSetLayout;

    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_commandBuffer->begin();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                              &descriptorSet, 0u, nullptr);

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, shaderStages, &shader);
    BindDefaultDynamicStates(vertexBuffer.handle(), false);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);

    m_commandBuffer->end();

    SubmitAndWait();

    vk::DestroyShaderEXT(m_device->handle(), shader, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(ShaderObjectTest, TaskMeshShadersDraw) {
    TEST_DESCRIPTION("Test drawing using task and mesh shaders");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mesh_shader_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!mesh_shader_features.taskShader || !mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Task and mesh shaders are required";
    }

    m_errorMonitor->ExpectSuccess();

    static const char taskSource[] = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    static const char meshSource[] = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3, 1);
            gl_MeshVerticesEXT[0].gl_Position = vec4(-1.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[1].gl_Position = vec4( 3.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[2].gl_Position = vec4(-1.0,  3.0, 0.0f, 1.0f);
            gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
        }
    )glsl";

    static const char fragSource[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<unsigned int> spv[3];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TASK_BIT_EXT, taskSource, spv[0], false, 4);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_MESH_BIT_EXT, meshSource, spv[1], false, 4);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_FRAGMENT_BIT, fragSource, spv[2], false, 4);

    VkShaderEXT shaders[3];
    VkShaderCreateInfoEXT createInfos[3];
    for (uint32_t i = 0; i < 3; ++i) {
        createInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfos[i].stage = shaderStages[i];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i].size() * sizeof(unsigned int);
        createInfos[i].pCode = spv[i].data();
        createInfos[i].pName = "main";
    }

    vk::CreateShadersEXT(m_device->handle(), 3u, createInfos, nullptr, shaders);

    VkBufferObj vertexBuffer;
    vertexBuffer.init(*m_device, sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkImageCreateInfo imageInfo = LvlInitStruct<VkImageCreateInfo>();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

    VkRenderingAttachmentInfo color_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = LvlInitStruct<VkRenderingInfo>();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
        (PFN_vkCmdBeginRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
        (PFN_vkCmdEndRenderingKHR)vk::GetDeviceProcAddr(device(), "vkCmdEndRenderingKHR");
    PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT =
        (PFN_vkCmdDrawMeshTasksEXT)vk::GetDeviceProcAddr(device(), "vkCmdDrawMeshTasksEXT");

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = LvlInitStruct<VkImageMemoryBarrier>();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vkCmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 3u, shaderStages, shaders);
    BindDefaultDynamicStates(vertexBuffer.handle(), false);
    vkCmdDrawMeshTasksEXT(m_commandBuffer->handle(), 1, 1, 1);
    vkCmdEndRenderingKHR(m_commandBuffer->handle());

    m_commandBuffer->end();

    SubmitAndWait();

    for (uint32_t i = 0; i < 2; ++i) vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(ShaderObjectTest, FailCreateShaders) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!CheckShaderObjectSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " shader object not supported, skipping test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    TEST_DESCRIPTION("Test drawing using task and mesh shaders");

    m_errorMonitor->ExpectSuccess();

    static const char vertSource[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char tescSource[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char teseSource[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geomSource[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char fragSource[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    constexpr uint32_t stages_count = 5;
    constexpr uint32_t shaders_count = 20;
    constexpr uint32_t fail_index = 15;

    VkShaderStageFlagBits shaderStages[stages_count] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                        VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<unsigned int> spv[stages_count];
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_VERTEX_BIT, vertSource, spv[0], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tescSource, spv[1], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, teseSource, spv[2], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_GEOMETRY_BIT, geomSource, spv[3], false, 0);
    GLSLtoSPV(&m_device->props.limits, VK_SHADER_STAGE_FRAGMENT_BIT, fragSource, spv[4], false, 0);

    VkShaderEXT shaders[shaders_count];

    VkShaderCreateInfoEXT createInfos[shaders_count];
    for (uint32_t i = 0; i < shaders_count; ++i) {
        createInfos[i] = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfos[i].stage = shaderStages[i % stages_count];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i % stages_count].size() * sizeof(unsigned int);
        createInfos[i].pCode = spv[i % stages_count].data();
        createInfos[i].pName = "main";
    }

    createInfos[fail_index].codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;

    VkResult res = vk::CreateShadersEXT(m_device->handle(), 20u, createInfos, nullptr, shaders);
    ASSERT_EQ(res, VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);

    for (uint32_t i = 0; i < shaders_count; ++i) {
        if (i < fail_index) {
            vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
            ASSERT_NE(shaders[i], VK_NULL_HANDLE);
        } else {
            ASSERT_EQ(shaders[i], VK_NULL_HANDLE);
        }
    }

    m_errorMonitor->VerifyNotFound();
}
