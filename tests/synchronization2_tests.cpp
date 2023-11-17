/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
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
 */

#include <type_traits>

#include "extension_layer_tests.h"
#include "synchronization2_tests.h"

void Sync2Test::SetUp() {
    VkBool32 force_enable = VK_TRUE;

    VkLayerSettingEXT settings[] = {
        {"VK_LAYER_KHRONOS_synchronization2", "force_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &force_enable}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(std::size(settings)), &settings[0]};

    VkExtensionLayerTest::SetUp();
    SetTargetApiVersion(VK_API_VERSION_1_2);
    VkExtensionLayerTest::AddSurfaceInstanceExtension();
    instance_layers_.push_back("VK_LAYER_KHRONOS_synchronization2");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &layer_settings_create_info));

    VkExtensionLayerTest::AddSwapchainDeviceExtension();
}

void Sync2Test::TearDown() {}

void Sync2Test::ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb,
                                         const VkBufferMemoryBarrier2KHR *buf_barrier,
                                         const VkImageMemoryBarrier2KHR *img_barrier) {
    monitor->ExpectSuccess();
    cb->begin();
    auto info = vku::InitStruct<VkDependencyInfoKHR>();
    info.bufferMemoryBarrierCount = buf_barrier ? 1 : 0;
    info.pBufferMemoryBarriers = buf_barrier;
    info.imageMemoryBarrierCount = img_barrier ? 1 : 0;
    info.pImageMemoryBarriers = img_barrier;

    vkCmdPipelineBarrier2KHR(cb->handle(), &info);
    cb->end();

    auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = cb->handle();

    auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vkQueueSubmit2KHR(cb->Queue()->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(cb->Queue()->handle());
    monitor->VerifyNotFound();
}

void Sync2Test::ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                                       const VkBufferMemoryBarrier2KHR *buf_barrier, const VkImageMemoryBarrier2KHR *img_barrier) {
    VkBufferMemoryBarrier2KHR fixup_buf_barrier;
    VkImageMemoryBarrier2KHR fixup_img_barrier;
    if (buf_barrier) {
        fixup_buf_barrier = *buf_barrier;
        fixup_buf_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_buf_barrier.dstAccessMask = 0;
    }
    if (img_barrier) {
        fixup_img_barrier = *img_barrier;
        fixup_img_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_img_barrier.dstAccessMask = 0;
    }

    ValidOwnershipTransferOp(monitor, cb_from, buf_barrier ? &fixup_buf_barrier : nullptr,
                             img_barrier ? &fixup_img_barrier : nullptr);

    if (buf_barrier) {
        fixup_buf_barrier = *buf_barrier;
        fixup_buf_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_buf_barrier.srcAccessMask = 0;
    }
    if (img_barrier) {
        fixup_img_barrier = *img_barrier;
        fixup_img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
        fixup_img_barrier.srcAccessMask = 0;
    }
    ValidOwnershipTransferOp(monitor, cb_to, buf_barrier ? &fixup_buf_barrier : nullptr,
                             img_barrier ? &fixup_img_barrier : nullptr);
}

TEST_F(Sync2Test, OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    uint32_t no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (no_gfx == UINT32_MAX) {
        GTEST_SKIP() << kSkipPrefix << "Required queue families not present (non-graphics capable required)." << std::endl;
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx)[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj no_gfx_cb(m_device, &no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, no_gfx_queue);

    // Create an "exclusive" image owned by the graphics queue.
    VkImageObj image(m_device);
    VkFlags image_use = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, image_use, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    auto image_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
    image_barrier.srcAccessMask = 0;
    image_barrier.dstAccessMask = 0;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.oldLayout = image.Layout();
    image_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    image_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.dstQueueFamilyIndex = no_gfx;

    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, nullptr, &image_barrier);

    m_commandBuffer->reset();
    no_gfx_cb.reset();

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx;
    image_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.oldLayout = image.Layout();
    // Make sure the new layout is different from the old
    if (image_barrier.oldLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
    } else {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    }
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR;

    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, nullptr, &image_barrier);
}

TEST_F(Sync2Test, OwnershipTranfersBuffer) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    uint32_t no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (no_gfx == UINT32_MAX) {
        GTEST_SKIP() << kSkipPrefix << "Required queue families not present (non-graphics capable required)." << std::endl;
    }
    VkQueueObj *no_gfx_queue = m_device->queue_family_queues(no_gfx)[0].get();

    VkCommandPoolObj no_gfx_pool(m_device, no_gfx, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj no_gfx_cb(m_device, &no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, no_gfx_queue);

    // Create a buffer
    const VkDeviceSize buffer_size = 256;
    uint8_t data[buffer_size] = {0xFF};
    VkConstantBufferObj buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized());

    auto buffer_barrier = vku::InitStruct<VkBufferMemoryBarrier2KHR>();
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.offset = 0;
    buffer_barrier.size = VK_WHOLE_SIZE;

    // Let gfx own it.
    buffer_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
    ValidOwnershipTransferOp(m_errorMonitor, m_commandBuffer, &buffer_barrier, nullptr);
    m_commandBuffer->reset();

    // Transfer it to non-gfx
    buffer_barrier.dstQueueFamilyIndex = no_gfx;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR;
    ValidOwnershipTransfer(m_errorMonitor, m_commandBuffer, &no_gfx_cb, &buffer_barrier, nullptr);

    m_commandBuffer->reset();
    no_gfx_cb.reset();

    // Transfer it to gfx
    buffer_barrier.srcQueueFamilyIndex = no_gfx;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
    ValidOwnershipTransfer(m_errorMonitor, &no_gfx_cb, m_commandBuffer, &buffer_barrier, nullptr);
}

TEST_F(Sync2Test, SecondaryCommandBufferBarrier) {
    TEST_DESCRIPTION("Add a pipeline barrier in a secondary command buffer");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    m_errorMonitor->ExpectSuccess();

    // A renderpass with a single subpass that declared a self-dependency
    auto attach = vku::InitStruct<VkAttachmentDescription2>();
    attach.flags = 0;
    attach.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;

    auto ref = vku::InitStruct<VkAttachmentReference2>();
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    auto subpass = vku::InitStruct<VkSubpassDescription2>();
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    auto dep_barrier = vku::InitStruct<VkMemoryBarrier2KHR>();
    dep_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dep_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    dep_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dep_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    auto dep = vku::InitStruct<VkSubpassDependency2>();
    dep.pNext = &dep_barrier;
    dep.srcStageMask = 0;
    dep.dstStageMask = 0;
    dep.srcAccessMask = 0;
    dep.dstAccessMask = 0;
    dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dep.viewOffset = 0;

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo2>();
    rpci.flags = 0;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    VkRenderPass rp;

    VkResult err = vkCreateRenderPass2(m_device->device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fbci = vku::InitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = rp;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &imageView;
    fbci.width = 32;
    fbci.height = 32;
    fbci.layers = 1;

    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();

    auto rpbi = vku::InitStruct<VkRenderPassBeginInfo>();
    rpbi.renderPass = rp;
    rpbi.framebuffer = fb;
    rpbi.renderArea = {{0, 0}, {32, 32}};

    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    auto cbii = vku::InitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = rp;

    auto cbbi = vku::InitStruct<VkCommandBufferBeginInfo>();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;
    secondary.begin(&cbbi);

    auto mem_barrier = vku::InitStruct<VkMemoryBarrier2KHR>();
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    auto img_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.oldLayout = image.Layout();
    img_barrier.newLayout = image.Layout();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.image = image.handle();
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.levelCount = 1;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.layerCount = 1;

    auto info = vku::InitStruct<VkDependencyInfoKHR>();
    info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    info.memoryBarrierCount = 1;
    info.pMemoryBarriers = &mem_barrier;
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &img_barrier;

    vkCmdPipelineBarrier2KHR(secondary.handle(), &info);

    auto sec_img_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
    sec_img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    sec_img_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    sec_img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    sec_img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    sec_img_barrier.oldLayout = image.Layout();
    sec_img_barrier.newLayout = image.Layout();
    sec_img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    sec_img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    sec_img_barrier.image = image.handle();
    sec_img_barrier.subresourceRange = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    auto sec_info = vku::InitStruct<VkDependencyInfoKHR>();
    sec_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    sec_info.memoryBarrierCount = 1;
    sec_info.pMemoryBarriers = &mem_barrier;
    sec_info.imageMemoryBarrierCount = 1;
    sec_info.pImageMemoryBarriers = &img_barrier;

    vkCmdPipelineBarrier2KHR(secondary.handle(), &sec_info);
    secondary.end();

    vkCmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = m_commandBuffer->handle();

    auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;
    vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, SecondaryCommandBufferImageLayoutTransitions) {
    TEST_DESCRIPTION("Perform an image layout transition in a secondary command buffer followed by a transition in the primary.");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    VkResult err;
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        GTEST_SKIP() << kSkipPrefix << "  Couldn't find depth stencil format." << std::endl;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->ExpectSuccess();
    // Allocate a secondary and primary cmd buffer
    auto command_buffer_allocate_info = vku::InitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer));
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer primary_command_buffer;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &primary_command_buffer));

    auto command_buffer_inheritance_info = vku::InitStruct<VkCommandBufferInheritanceInfo>();

    auto command_buffer_begin_info = vku::InitStruct<VkCommandBufferBeginInfo>();
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    err = vkBeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    ASSERT_VK_SUCCESS(err);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    {
        auto img_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
        img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        img_barrier.srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
        img_barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        img_barrier.image = image.handle();
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        img_barrier.subresourceRange.baseArrayLayer = 0;
        img_barrier.subresourceRange.baseMipLevel = 0;
        img_barrier.subresourceRange.layerCount = 1;
        img_barrier.subresourceRange.levelCount = 1;

        auto info = vku::InitStruct<VkDependencyInfoKHR>();
        info.imageMemoryBarrierCount = 1;
        info.pImageMemoryBarriers = &img_barrier;

        vkCmdPipelineBarrier2KHR(secondary_command_buffer, &info);
    }
    err = vkEndCommandBuffer(secondary_command_buffer);
    ASSERT_VK_SUCCESS(err);

    // Now update primary cmd buffer to execute secondary and transitions image
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    err = vkBeginCommandBuffer(primary_command_buffer, &command_buffer_begin_info);
    ASSERT_VK_SUCCESS(err);
    vkCmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);

    {
        auto img_barrier2 = vku::InitStruct<VkImageMemoryBarrier2KHR>();
        img_barrier2.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        img_barrier2.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        img_barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        img_barrier2.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        img_barrier2.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        img_barrier2.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        img_barrier2.image = image.handle();
        img_barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        img_barrier2.subresourceRange.baseArrayLayer = 0;
        img_barrier2.subresourceRange.baseMipLevel = 0;
        img_barrier2.subresourceRange.layerCount = 1;
        img_barrier2.subresourceRange.levelCount = 1;

        auto info2 = vku::InitStruct<VkDependencyInfoKHR>();
        info2.imageMemoryBarrierCount = 1;
        info2.pImageMemoryBarriers = &img_barrier2;

        vkCmdPipelineBarrier2KHR(primary_command_buffer, &info2);
    }
    err = vkEndCommandBuffer(primary_command_buffer);
    ASSERT_VK_SUCCESS(err);

    auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = primary_command_buffer;

    auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    err = vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    err = vkDeviceWaitIdle(m_device->device());
    ASSERT_VK_SUCCESS(err);
    vkFreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &secondary_command_buffer);
    vkFreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &primary_command_buffer);
}

TEST_F(Sync2Test, QueueSubmitSemaphoresAndLayoutTracking) {
    TEST_DESCRIPTION("Submit multiple command buffers with chained semaphore signals and layout transitions");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    m_errorMonitor->ExpectSuccess();
    VkCommandBuffer cmd_bufs[4];
    auto alloc_info = vku::InitStruct<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 4;
    alloc_info.commandPool = m_commandPool->handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &alloc_info, cmd_bufs);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
               (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    auto cb_binfo = vku::InitStruct<VkCommandBufferBeginInfo>();

    // Use 4 command buffers, each with an image layout transition, ColorAO->General->ColorAO->TransferSrc->TransferDst
    vkBeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    auto img_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    auto info = vku::InitStruct<VkDependencyInfoKHR>();
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &img_barrier;

    vkCmdPipelineBarrier2KHR(cmd_bufs[0], &info);
    vkEndCommandBuffer(cmd_bufs[0]);
    vkBeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    vkCmdPipelineBarrier2KHR(cmd_bufs[1], &info);
    vkEndCommandBuffer(cmd_bufs[1]);

    vkBeginCommandBuffer(cmd_bufs[2], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vkCmdPipelineBarrier2KHR(cmd_bufs[2], &info);
    vkEndCommandBuffer(cmd_bufs[2]);

    vkBeginCommandBuffer(cmd_bufs[3], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkCmdPipelineBarrier2KHR(cmd_bufs[3], &info);
    vkEndCommandBuffer(cmd_bufs[3]);

    // Submit 4 command buffers in 3 submits, with submits 2 and 3 waiting for semaphores from submits 1 and 2
    VkSemaphore semaphore1, semaphore2;
    auto semaphore_create_info = vku::InitStruct<VkSemaphoreCreateInfo>();
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore1);
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore2);

    std::vector<VkSubmitInfo2KHR> submit_info(3, vku::InitStruct<VkSubmitInfo2KHR>());
    std::vector<VkCommandBufferSubmitInfoKHR> cb_info(4, vku::InitStruct<VkCommandBufferSubmitInfoKHR>());
    std::vector<VkSemaphoreSubmitInfoKHR> sem_info(2, vku::InitStruct<VkSemaphoreSubmitInfoKHR>());

    cb_info[0].commandBuffer = cmd_bufs[0];

    sem_info[0].semaphore = semaphore1;
    sem_info[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;

    submit_info[0].commandBufferInfoCount = 1;
    submit_info[0].pCommandBufferInfos = &cb_info[0];
    submit_info[0].signalSemaphoreInfoCount = 1;
    submit_info[0].pSignalSemaphoreInfos = &sem_info[0];
    submit_info[0].waitSemaphoreInfoCount = 0;

    cb_info[1].commandBuffer = cmd_bufs[1];

    sem_info[1].semaphore = semaphore2;
    sem_info[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;

    submit_info[1].commandBufferInfoCount = 1;
    submit_info[1].pCommandBufferInfos = &cb_info[1];
    submit_info[1].waitSemaphoreInfoCount = 1;
    submit_info[1].pWaitSemaphoreInfos = &sem_info[0];
    submit_info[1].signalSemaphoreInfoCount = 1;
    submit_info[1].pSignalSemaphoreInfos = &sem_info[1];

    cb_info[2].commandBuffer = cmd_bufs[2];
    cb_info[3].commandBuffer = cmd_bufs[3];

    submit_info[2].commandBufferInfoCount = 2;
    submit_info[2].pCommandBufferInfos = &cb_info[2];
    submit_info[2].waitSemaphoreInfoCount = 1;
    submit_info[2].pWaitSemaphoreInfos = &sem_info[1];
    submit_info[2].signalSemaphoreInfoCount = 0;
    submit_info[2].pSignalSemaphoreInfos = nullptr;
    vkQueueSubmit2KHR(m_device->m_queue, submit_info.size(), submit_info.data(), VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);

    vkDestroySemaphore(m_device->device(), semaphore1, NULL);
    vkDestroySemaphore(m_device->device(), semaphore2, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, CommandBufferSimultaneousUseSync) {
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }
    m_errorMonitor->ExpectSuccess();
    VkResult err;

    // Record (empty!) command buffer that can be submitted multiple times
    // simultaneously.
    auto cbbi = vku::InitStruct<VkCommandBufferBeginInfo>();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    m_commandBuffer->begin(&cbbi);
    m_commandBuffer->end();

    auto fci = vku::InitStruct<VkFenceCreateInfo>();
    VkFence fence;
    err = vkCreateFence(m_device->device(), &fci, nullptr, &fence);
    ASSERT_VK_SUCCESS(err);

    auto sci = vku::InitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore s1, s2;
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &s1);
    ASSERT_VK_SUCCESS(err);
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &s2);
    ASSERT_VK_SUCCESS(err);

    // Submit CB once signaling s1, with fence so we can roll forward to its retirement.
    auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = m_commandBuffer->handle();

    auto sem_info = vku::InitStruct<VkSemaphoreSubmitInfoKHR>();
    sem_info.semaphore = s1;

    auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &sem_info;

    err = vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, fence);
    ASSERT_VK_SUCCESS(err);

    // Submit CB again, signaling s2.
    sem_info.semaphore = s2;
    err = vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Wait for fence.
    err = vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    ASSERT_VK_SUCCESS(err);

    // CB is still in flight from second submission, but semaphore s1 is no
    // longer in flight. delete it.
    vkDestroySemaphore(m_device->device(), s1, nullptr);

    m_errorMonitor->VerifyNotFound();

    // Force device idle and clean up remaining objects
    vkDeviceWaitIdle(m_device->device());
    vkDestroySemaphore(m_device->device(), s2, nullptr);
    vkDestroyFence(m_device->device(), fence, nullptr);
}

TEST_F(Sync2Test, BarrierLayoutToImageUsage) {
    TEST_DESCRIPTION("Ensure barriers' new and old VkImageLayout are compatible with their images' VkImageUsageFlags");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }
    m_errorMonitor->ExpectSuccess();

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        GTEST_SKIP() << kSkipPrefix << "  No Depth + Stencil format found. Skipped." << std::endl;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto img_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    {
        VkImageObj img_color(m_device);
        img_color.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_color.initialized());

        VkImageObj img_ds1(m_device);
        img_ds1.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds1.initialized());

        VkImageObj img_ds2(m_device);
        img_ds2.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds2.initialized());

        VkImageObj img_xfer_src(m_device);
        img_xfer_src.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_src.initialized());

        VkImageObj img_xfer_dst(m_device);
        img_xfer_dst.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_dst.initialized());

        VkImageObj img_sampled(m_device);
        img_sampled.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_sampled.initialized());

        VkImageObj img_input(m_device);
        img_input.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_input.initialized());

        const struct {
            VkImageObj &image_obj;
            VkImageLayout old_layout;
            VkImageLayout new_layout;
        } buffer_layouts[] = {
            // clang-format off
            {img_color,    VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,          VK_IMAGE_LAYOUT_GENERAL},
            {img_ds1,      VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,          VK_IMAGE_LAYOUT_GENERAL},
            {img_ds2,      VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,           VK_IMAGE_LAYOUT_GENERAL},
            {img_sampled,  VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,           VK_IMAGE_LAYOUT_GENERAL},
            {img_input,    VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,           VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,            VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,            VK_IMAGE_LAYOUT_GENERAL},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(buffer_layouts) / sizeof(buffer_layouts[0]);

        m_commandBuffer->begin();
        for (uint32_t i = 0; i < layout_count; ++i) {
            img_barrier.image = buffer_layouts[i].image_obj.handle();
            const VkImageUsageFlags usage = buffer_layouts[i].image_obj.usage();
            img_barrier.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                          ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                          : VK_IMAGE_ASPECT_COLOR_BIT;
            auto dep_info = vku::InitStruct<VkDependencyInfoKHR>();
            dep_info.imageMemoryBarrierCount = 1;
            dep_info.pImageMemoryBarriers = &img_barrier;

            img_barrier.oldLayout = buffer_layouts[i].old_layout;
            img_barrier.newLayout = buffer_layouts[i].new_layout;
            img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT_KHR;
            img_barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);

            img_barrier.oldLayout = buffer_layouts[i].new_layout;
            img_barrier.newLayout = buffer_layouts[i].old_layout;
            img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT_KHR;
            img_barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);
        }
        m_commandBuffer->end();

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, WaitEventThenSet) {
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    m_errorMonitor->ExpectSuccess();

    VkEvent event;
    auto event_create_info = vku::InitStruct<VkEventCreateInfo>();
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    auto pool_create_info = vku::InitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    auto command_buffer_allocate_info = vku::InitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        auto begin_info = vku::InitStruct<VkCommandBufferBeginInfo>();
        vkBeginCommandBuffer(command_buffer, &begin_info);

        auto barrier = vku::InitStruct<VkMemoryBarrier2KHR>();
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT_KHR;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR;

        auto dep_info = vku::InitStruct<VkDependencyInfoKHR>();
        dep_info.memoryBarrierCount = 1;
        dep_info.pMemoryBarriers = &barrier;

        vkCmdWaitEvents2KHR(command_buffer, 1, &event, &dep_info);

        vkCmdResetEvent2KHR(command_buffer, event, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR);
        vkEndCommandBuffer(command_buffer);
    }
    {
        auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
        cb_info.commandBuffer = command_buffer;

        auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;

        vkQueueSubmit2KHR(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    { vkSetEvent(m_device->device(), event); }

    vkQueueWaitIdle(queue);

    vkDestroyEvent(m_device->device(), event, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceTwoWFF) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by two consecutive WaitForFences calls on the same fence.");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << kSkipPrefix << "  Queue family needs to have multiple queues to run this test." << std::endl;
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    auto fence_create_info = vku::InitStruct<VkFenceCreateInfo>();
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    auto semaphore_create_info = vku::InitStruct<VkSemaphoreCreateInfo>();
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    auto pool_create_info = vku::InitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    auto command_buffer_allocate_info = vku::InitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    auto begin_info = vku::InitStruct<VkCommandBufferBeginInfo>();
    {
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        auto barrier = vku::InitStruct<VkMemoryBarrier2KHR>();
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        auto info = vku::InitStruct<VkDependencyInfoKHR>();
        info.memoryBarrierCount = 1;
        info.pMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2KHR(command_buffer[0], &info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
        cb_info.commandBuffer = command_buffer[0];

        auto sem_info = vku::InitStruct<VkSemaphoreSubmitInfoKHR>();
        sem_info.semaphore = semaphore;

        auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &sem_info;
        vkQueueSubmit2KHR(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
        cb_info.commandBuffer = command_buffer[1];

        auto sem_info = vku::InitStruct<VkSemaphoreSubmitInfoKHR>();
        sem_info.semaphore = semaphore;
        sem_info.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;
        submit_info.waitSemaphoreInfoCount = 1;
        submit_info.pWaitSemaphoreInfos = &sem_info;

        vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, TwoSubmitInfosWithSemaphoreOneQueueSubmitsOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers each in a separate SubmitInfo sent in a single QueueSubmit call followed by a WaitForFences call.");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence = VK_NULL_HANDLE;
    auto fence_create_info = vku::InitStruct<VkFenceCreateInfo>();
    vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore = VK_NULL_HANDLE;
    auto semaphore_create_info = vku::InitStruct<VkSemaphoreCreateInfo>();
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool = VK_NULL_HANDLE;
    auto pool_create_info = vku::InitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2] = {};
    auto command_buffer_allocate_info = vku::InitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        auto begin_info = vku::InitStruct<VkCommandBufferBeginInfo>();
        vkBeginCommandBuffer(command_buffer[0], &begin_info);

        auto mem_barrier = vku::InitStruct<VkMemoryBarrier2KHR>();
        mem_barrier.srcAccessMask = 0;
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        mem_barrier.dstAccessMask = 0;
        mem_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        auto info = vku::InitStruct<VkDependencyInfoKHR>();
        info.dependencyFlags = 0;
        info.memoryBarrierCount = 1;
        info.pMemoryBarriers = &mem_barrier;
        vkCmdPipelineBarrier2KHR(command_buffer[0], &info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[0]);
    }
    {
        auto begin_info = vku::InitStruct<VkCommandBufferBeginInfo>();
        vkBeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vkCmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vkEndCommandBuffer(command_buffer[1]);
    }
    {
        std::vector<VkSubmitInfo2KHR> submit_info(2, vku::InitStruct<VkSubmitInfo2KHR>());
        std::vector<VkCommandBufferSubmitInfoKHR> cb_info(2, vku::InitStruct<VkCommandBufferSubmitInfoKHR>());
        std::vector<VkSemaphoreSubmitInfoKHR> sem_info(2, vku::InitStruct<VkSemaphoreSubmitInfoKHR>());

        cb_info[0].commandBuffer = command_buffer[0];
        sem_info[0].semaphore = semaphore;
        submit_info[0].commandBufferInfoCount = 1;
        submit_info[0].pCommandBufferInfos = &cb_info[0];
        submit_info[0].signalSemaphoreInfoCount = 1;
        submit_info[0].pSignalSemaphoreInfos = &sem_info[0];

        cb_info[1].commandBuffer = command_buffer[1];
        sem_info[1].semaphore = semaphore;
        sem_info[1].stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        submit_info[1].commandBufferInfoCount = 1;
        submit_info[1].pCommandBufferInfos = &cb_info[1];
        submit_info[1].waitSemaphoreInfoCount = 1;
        submit_info[1].pWaitSemaphoreInfos = &sem_info[1];

        vkQueueSubmit2KHR(m_device->m_queue, 2, &submit_info[0], fence);
    }

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(m_device->device(), fence, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, ClearDepthStencilWithValidRange) {
    TEST_DESCRIPTION("Record clear depth with a valid VkImageSubresourceRange");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        GTEST_SKIP() << kSkipPrefix << "  No Depth + Stencil format found. Skipped." << std::endl;
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());
    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearDepthStencilValue clear_value = {};

    m_commandBuffer->begin();
    const auto cb_handle = m_commandBuffer->handle();

    // Try good case
    {
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }

    image.ImageMemoryBarrier(m_commandBuffer, ds_aspect, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Try good case with VK_REMAINING
    {
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {ds_aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }
}

TEST_F(Sync2Test, QueueSubmitTimelineSemaphore) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore.");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    auto timeline_features = vku::InitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    auto features2 = vku::InitStruct<VkPhysicalDeviceFeatures2KHR>(&timeline_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!timeline_features.timelineSemaphore) {
        GTEST_SKIP() << "Timeline semaphores not supported.";
    }

    m_errorMonitor->ExpectSuccess();

    auto semaphore_type_create_info = vku::InitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto semaphore_create_info = vku::InitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    std::vector<VkSemaphoreSubmitInfoKHR> sem_info(2, vku::InitStruct<VkSemaphoreSubmitInfoKHR>());
    std::vector<VkSubmitInfo2KHR> submit_info(3, vku::InitStruct<VkSubmitInfo2KHR>());

    sem_info[0].semaphore = semaphore;
    sem_info[0].value = 1;
    sem_info[0].stageMask = stageFlags;
    submit_info[0].signalSemaphoreInfoCount = 1;
    submit_info[0].pSignalSemaphoreInfos = &sem_info[0];

    sem_info[1].semaphore = semaphore;
    sem_info[1].value = 1;
    sem_info[1].stageMask = stageFlags;
    submit_info[1].waitSemaphoreInfoCount = 1;
    submit_info[1].pWaitSemaphoreInfos = &sem_info[1];

    vkQueueSubmit2KHR(m_device->m_queue, submit_info.size(), submit_info.data(), VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
}

// TODO: this test crashes in the driver if run without the validation layer present
#if 0
TEST_F(Sync2Test, SubmitInfoDeviceMask) {
    TEST_DESCRIPTION("SubmitInfo2KHR deviceMask.");

    uint32_t physical_device_group_count = 0;
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << kSkipPrefix << " physical_device_group_count is 0, skipping test" << std::endl;
    }
    // Test VkDeviceGroupCommandBufferBeginInfo
    VkDeviceGroupCommandBufferBeginInfo dev_grp_cmd_buf_info = {};
    dev_grp_cmd_buf_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
    VkCommandBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext = &dev_grp_cmd_buf_info;

    dev_grp_cmd_buf_info.deviceMask = 0x00000001;
    m_commandBuffer->reset();
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);

    // Test CommandBufferSubmitInfoKHR.deviceMask
    VkCommandBufferSubmitInfoKHR cb_info = {};
    cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info.commandBuffer = m_commandBuffer->handle();
    cb_info.deviceMask = 0xFFFFFFFF;

    VkSubmitInfo2KHR submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    m_commandBuffer->reset();
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    vkEndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
    vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);
}
#endif

// in a separate group to avoid the convience stuff in SetUp() above
TEST_F(Sync2CompatTest, Vulkan10) {
    TEST_DESCRIPTION("Minimal Vulkan 1.0 compatibility test.");
    static const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "sync2_test", 1, NULL, 0, VK_API_VERSION_1_0,
    };
    static const char *layer_name = "VK_LAYER_KHRONOS_synchronization2";
    static const char *ext_name = "VK_KHR_synchronization2";
    static VkInstanceCreateInfo inst_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &app_info, 1, &layer_name, 0, NULL,
    };

#ifdef __APPLE__
    // Must account for portability on Apple platforms
    std::vector<const char *> requiredExtensions;
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    inst_info.enabledExtensionCount = (int)requiredExtensions.size();
    inst_info.ppEnabledExtensionNames = requiredExtensions.data();
    inst_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkInstance instance;

    ASSERT_VK_SUCCESS(vkCreateInstance(&inst_info, NULL, &instance));

    volkLoadInstance(instance);

    uint32_t gpu_count = 0;
    ASSERT_VK_SUCCESS(vkEnumeratePhysicalDevices(instance, &gpu_count, NULL));

    std::vector<VkPhysicalDevice> gpus{gpu_count};
    ASSERT_VK_SUCCESS(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data()));

    const float priority = 1.0f;

    auto queue_info = vku::InitStruct<VkDeviceQueueCreateInfo>();
    queue_info.queueFamilyIndex = UINT32_MAX;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props{queue_count};
    ASSERT_NE(queue_count, 0);
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_count, queue_props.data());

    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;

    auto sync2_features = vku::InitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    sync2_features.synchronization2 = true;
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(gpus[0], &features);

    auto dev_info = vku::InitStruct<VkDeviceCreateInfo>(&sync2_features);
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledExtensionCount = 1;
    dev_info.ppEnabledExtensionNames = &ext_name;
    dev_info.pEnabledFeatures = &features;

    VkDevice device;
    ASSERT_VK_SUCCESS(vkCreateDevice(gpus[0], &dev_info, NULL, &device));

    volkLoadDevice(device);

    VkQueue queue;
    vkGetDeviceQueue(device, queue_info.queueFamilyIndex, 0, &queue);


    auto semaphore_create_info = vku::InitStruct<VkSemaphoreCreateInfo>();

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSemaphoreSubmitInfoKHR sem_info[2] = {};
    std::vector<VkSubmitInfo2KHR> submit_info(2, vku::InitStruct<VkSubmitInfo2KHR>());

    sem_info[0].semaphore = semaphore;
    sem_info[0].value = 1;
    sem_info[0].stageMask = stageFlags;
    submit_info[0].signalSemaphoreInfoCount = 1;
    submit_info[0].pSignalSemaphoreInfos = &sem_info[0];

    sem_info[1].semaphore = semaphore;
    sem_info[1].value = 1;
    sem_info[1].stageMask = stageFlags;
    submit_info[1].waitSemaphoreInfoCount = 1;
    submit_info[1].pWaitSemaphoreInfos = &sem_info[1];

    vkQueueSubmit2KHR(queue, submit_info.size(), submit_info.data(), VK_NULL_HANDLE);

    ASSERT_VK_SUCCESS(vkQueueWaitIdle(queue));

    vkDestroySemaphore(device, semaphore, nullptr);

    vkDestroyDevice(device, NULL);

    vkDestroyInstance(instance, NULL);
}

TEST_F(Sync2Test, SwapchainImage) {
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }
    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping CmdCopySwapchainImage test\n", kSkipPrefix);
        return;
    }
    uint32_t image_index, image_count;
    vkGetSwapchainImagesKHR(m_device->handle(), m_swapchain, &image_count, NULL);
    std::vector<VkImage> swapchain_images(image_count);
    vkGetSwapchainImagesKHR(m_device->handle(), m_swapchain, &image_count, swapchain_images.data());

    VkSemaphore semaphore;
    auto semaphore_create_info = vku::InitStruct<VkSemaphoreCreateInfo>();
    auto ret = vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    ASSERT_VK_SUCCESS(ret);

    ret = vkAcquireNextImageKHR(m_device->handle(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &image_index);
    ASSERT_VK_SUCCESS(ret);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats;
    if (format_count != 0) {
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, formats.data());
    }

    VkAttachmentDescription attach[] = {
        {0, formats[0].format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>();
    rpci.flags = 0;
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    VkRenderPass rp1, rp2;

    ret = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp1);
    ASSERT_VK_SUCCESS(ret);
    attach[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ret = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp2);

    auto ivci = vku::InitStruct<VkImageViewCreateInfo>();
    ivci.image = swapchain_images[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = formats[0].format;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageView view;
    ret = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(ret);
    auto fci = vku::InitStruct<VkFramebufferCreateInfo>();
    fci.renderPass = rp1;
    fci.attachmentCount = 1;
    fci.pAttachments = &view;
    fci.width = 32;
    fci.height = 32;
    fci.layers = 1;

    VkFramebuffer fb1, fb2;
    ret = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb1);
    fci.renderPass = rp2;
    ret = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb2);
    ASSERT_VK_SUCCESS(ret);

    auto rpbi = vku::InitStruct<VkRenderPassBeginInfo>();
    rpbi.renderPass = rp1;
    rpbi.framebuffer = fb1;
    rpbi.renderArea = {{0, 0}, {32, 32}};

    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer->handle());

    rpbi.framebuffer = fb2;
    rpbi.renderPass = rp2;
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer->handle());

    auto img_barrier = vku::InitStruct<VkImageMemoryBarrier2KHR>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.image = swapchain_images[image_index];
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    auto info = vku::InitStruct<VkDependencyInfoKHR>();
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &img_barrier;
    vkCmdPipelineBarrier2KHR(m_commandBuffer->handle(), &info);
    m_commandBuffer->end();

    {
        auto sem_info = vku::InitStruct<VkSemaphoreSubmitInfoKHR>();
        sem_info.semaphore = semaphore;
        sem_info.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR;

        auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
        submit_info.waitSemaphoreInfoCount = 1;
        submit_info.pWaitSemaphoreInfos = &sem_info;

        vkQueueSubmit2KHR(m_commandBuffer->Queue()->handle(), 1, &submit_info, VK_NULL_HANDLE);
    }

    {
        auto cb_info = vku::InitStruct<VkCommandBufferSubmitInfoKHR>();
        cb_info.commandBuffer = m_commandBuffer->handle();

        auto submit_info = vku::InitStruct<VkSubmitInfo2KHR>();
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;
        vkQueueSubmit2KHR(m_commandBuffer->Queue()->handle(), 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(vkQueueWaitIdle(m_commandBuffer->Queue()->handle()));
        m_errorMonitor->ExpectSuccess();
    }

    vkDestroyFramebuffer(m_device->device(), fb1, NULL);
    vkDestroyRenderPass(m_device->device(), rp1, NULL);
    vkDestroyFramebuffer(m_device->device(), fb2, NULL);
    vkDestroyRenderPass(m_device->device(), rp2, NULL);
    vkDestroySemaphore(m_device->device(), semaphore, NULL);
    vkDestroyImageView(m_device->device(), view, NULL);
    DestroySwapchain();
}

TEST_F(Sync2Test, EnumerateDeviceExtensionProperties) {
    TEST_DESCRIPTION("Verify the extension is found only once");
    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }

    uint32_t propertyCount;
    vkEnumerateDeviceExtensionProperties(m_device->phy().handle(), nullptr, &propertyCount, nullptr);
    std::vector<VkExtensionProperties> props(propertyCount);
    vkEnumerateDeviceExtensionProperties(m_device->phy().handle(), nullptr, &propertyCount, props.data());

    uint32_t count = 0;
    for (const auto &p : props) {
        if (strcmp(p.extensionName, "VK_KHR_synchronization2") == 0) {
            ++count;
        }
    }

    ASSERT_EQ(count, 1);
}
