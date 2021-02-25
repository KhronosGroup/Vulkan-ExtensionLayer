/*
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google, Inc.
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
    VkExtensionLayerTest::SetUp();
    SetTargetApiVersion(VK_API_VERSION_1_2);
    instance_layers_.push_back("VK_LAYER_KHRONOS_synchronization2");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!CheckSynchronization2SupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " synchronization2 not supported, skipping test";
    }
}

void Sync2Test::TearDown() {}

void Sync2Test::ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb,
                                         const VkBufferMemoryBarrier2KHR *buf_barrier,
                                         const VkImageMemoryBarrier2KHR *img_barrier) {
    monitor->ExpectSuccess();
    cb->begin();
    VkDependencyInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    info.bufferMemoryBarrierCount = buf_barrier ? 1 : 0;
    info.pBufferMemoryBarriers = buf_barrier;
    info.imageMemoryBarrierCount = img_barrier ? 1 : 0;
    info.pImageMemoryBarriers = img_barrier;

    vk::CmdPipelineBarrier2KHR(cb->handle(), &info);
    cb->end();

    VkCommandBufferSubmitInfoKHR cb_info = {};
    cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info.commandBuffer = cb->handle();

    VkSubmitInfo2KHR submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(cb->Queue()->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(cb->Queue()->handle());
    monitor->VerifyNotFound();
}

void Sync2Test::ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                                       const VkBufferMemoryBarrier2KHR *buf_barrier, const VkImageMemoryBarrier2KHR *img_barrier) {
    ValidOwnershipTransferOp(monitor, cb_from, buf_barrier, img_barrier);
    ValidOwnershipTransferOp(monitor, cb_to, buf_barrier, img_barrier);
}

TEST_F(Sync2Test, OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");

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

    VkImageMemoryBarrier2KHR image_barrier = {};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
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

    VkBufferMemoryBarrier2KHR buffer_barrier = {};
    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR;
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

    m_errorMonitor->ExpectSuccess();

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription2 attach = {};
    attach.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    attach.pNext = nullptr;
    attach.flags = 0;
    attach.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;

    VkAttachmentReference2 ref = {};
    ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    ref.pNext = nullptr;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubpassDescription2 subpass = {};
    subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    subpass.pNext = nullptr;
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

    VkMemoryBarrier2KHR dep_barrier = {};
    dep_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
    dep_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dep_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    dep_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dep_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    VkSubpassDependency2 dep = {};
    dep.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
    dep.pNext = &dep_barrier;
    dep.srcStageMask = 0;
    dep.dstStageMask = 0;
    dep.srcAccessMask = 0;
    dep.dstAccessMask = 0;
    dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dep.viewOffset = 0;

    VkRenderPassCreateInfo2 rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    rpci.pNext = nullptr;
    rpci.flags = 0;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    VkRenderPass rp;

    VkResult err = vk::CreateRenderPass2(m_device->device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.pNext = nullptr;
    rpbi.renderPass = rp;
    rpbi.framebuffer = fb;
    rpbi.renderArea = {{0, 0}, {32, 32}};
    rpbi.clearValueCount = 0;
    rpbi.pClearValues = nullptr;

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                                           nullptr,
                                           rp,
                                           0,
                                           VK_NULL_HANDLE,  // Set to NULL FB handle intentionally to flesh out any errors
                                           VK_FALSE,
                                           0,
                                           0};
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                                     &cbii};
    secondary.begin(&cbbi);
    VkMemoryBarrier2KHR mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
    mem_barrier.pNext = NULL;
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    VkImageMemoryBarrier2KHR img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.oldLayout = image.Layout();
    img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    img_barrier.image = image.handle();
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.levelCount = 1;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.layerCount = 1;

    VkDependencyInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    info.memoryBarrierCount = 1;
    info.pMemoryBarriers = &mem_barrier;
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &img_barrier;

    vk::CmdPipelineBarrier2KHR(secondary.handle(), &info);

    VkImageMemoryBarrier2KHR sec_img_barrier = {};
    sec_img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
    sec_img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    sec_img_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    sec_img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    sec_img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    sec_img_barrier.oldLayout = image.Layout();
    sec_img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    sec_img_barrier.image = image.handle();
    sec_img_barrier.subresourceRange = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    VkDependencyInfoKHR sec_info = {};
    sec_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    sec_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    sec_info.memoryBarrierCount = 1;
    sec_info.pMemoryBarriers = &mem_barrier;
    sec_info.imageMemoryBarrierCount = 1;
    sec_info.pImageMemoryBarriers = &img_barrier;

    vk::CmdPipelineBarrier2KHR(secondary.handle(), &sec_info);
    secondary.end();

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    VkCommandBufferSubmitInfoKHR cb_info = {};
    cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info.commandBuffer = m_commandBuffer->handle();

    VkSubmitInfo2KHR submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;
    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, SecondaryCommandBufferImageLayoutTransitions) {
    TEST_DESCRIPTION("Perform an image layout transition in a secondary command buffer followed by a transition in the primary.");
    VkResult err;
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        GTEST_SKIP() << kSkipPrefix << "  Couldn't find depth stencil format." << std::endl;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->ExpectSuccess();
    // Allocate a secondary and primary cmd buffer
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_VK_SUCCESS(vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &secondary_command_buffer));
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer primary_command_buffer;
    ASSERT_VK_SUCCESS(vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &primary_command_buffer));
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = {};
    command_buffer_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    err = vk::BeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    ASSERT_VK_SUCCESS(err);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    {
        VkImageMemoryBarrier2KHR img_barrier = {};
        img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
        img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        img_barrier.srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
        img_barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        img_barrier.image = image.handle();
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        img_barrier.subresourceRange.baseArrayLayer = 0;
        img_barrier.subresourceRange.baseMipLevel = 0;
        img_barrier.subresourceRange.layerCount = 1;
        img_barrier.subresourceRange.levelCount = 1;

        VkDependencyInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        info.imageMemoryBarrierCount = 1;
        info.pImageMemoryBarriers = &img_barrier;

        vk::CmdPipelineBarrier2KHR(secondary_command_buffer, &info);
    }
    err = vk::EndCommandBuffer(secondary_command_buffer);
    ASSERT_VK_SUCCESS(err);

    // Now update primary cmd buffer to execute secondary and transitions image
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    err = vk::BeginCommandBuffer(primary_command_buffer, &command_buffer_begin_info);
    ASSERT_VK_SUCCESS(err);
    vk::CmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);

    {
        VkImageMemoryBarrier2KHR img_barrier2 = {};
        img_barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
        img_barrier2.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        img_barrier2.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        img_barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        img_barrier2.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        img_barrier2.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        img_barrier2.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
        img_barrier2.image = image.handle();
        img_barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        img_barrier2.subresourceRange.baseArrayLayer = 0;
        img_barrier2.subresourceRange.baseMipLevel = 0;
        img_barrier2.subresourceRange.layerCount = 1;
        img_barrier2.subresourceRange.levelCount = 1;

        VkDependencyInfoKHR info2 = {};
        info2.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        info2.imageMemoryBarrierCount = 1;
        info2.pImageMemoryBarriers = &img_barrier2;

        vk::CmdPipelineBarrier2KHR(primary_command_buffer, &info2);
    }
    err = vk::EndCommandBuffer(primary_command_buffer);
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferSubmitInfoKHR cb_info = {};
    cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info.commandBuffer = primary_command_buffer;

    VkSubmitInfo2KHR submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    err = vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();
    err = vk::DeviceWaitIdle(m_device->device());
    ASSERT_VK_SUCCESS(err);
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &secondary_command_buffer);
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &primary_command_buffer);
}

TEST_F(Sync2Test, QueueSubmitSemaphoresAndLayoutTracking) {
    TEST_DESCRIPTION("Submit multiple command buffers with chained semaphore signals and layout transitions");

    m_errorMonitor->ExpectSuccess();
    VkCommandBuffer cmd_bufs[4];
    VkCommandBufferAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.commandBufferCount = 4;
    alloc_info.commandPool = m_commandPool->handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &alloc_info, cmd_bufs);
    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
               (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkCommandBufferBeginInfo cb_binfo;
    cb_binfo.pNext = NULL;
    cb_binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    // Use 4 command buffers, each with an image layout transition, ColorAO->General->ColorAO->TransferSrc->TransferDst
    vk::BeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkImageMemoryBarrier2KHR img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
    img_barrier.pNext = NULL;
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

    VkDependencyInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &img_barrier;

    vk::CmdPipelineBarrier2KHR(cmd_bufs[0], &info);
    vk::EndCommandBuffer(cmd_bufs[0]);
    vk::BeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    vk::CmdPipelineBarrier2KHR(cmd_bufs[1], &info);
    vk::EndCommandBuffer(cmd_bufs[1]);

    vk::BeginCommandBuffer(cmd_bufs[2], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk::CmdPipelineBarrier2KHR(cmd_bufs[2], &info);
    vk::EndCommandBuffer(cmd_bufs[2]);

    vk::BeginCommandBuffer(cmd_bufs[3], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk::CmdPipelineBarrier2KHR(cmd_bufs[3], &info);
    vk::EndCommandBuffer(cmd_bufs[3]);

    // Submit 4 command buffers in 3 submits, with submits 2 and 3 waiting for semaphores from submits 1 and 2
    VkSemaphore semaphore1, semaphore2;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore1);
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore2);

    VkSubmitInfo2KHR submit_info[3] = {};
    VkCommandBufferSubmitInfoKHR cb_info[4] = {};
    VkSemaphoreSubmitInfoKHR sem_info[2] = {};
    cb_info[0].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info[0].commandBuffer = cmd_bufs[0];

    sem_info[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
    sem_info[0].semaphore = semaphore1;
    sem_info[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;

    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info[0].pNext = nullptr;
    submit_info[0].commandBufferInfoCount = 1;
    submit_info[0].pCommandBufferInfos = &cb_info[0];
    submit_info[0].signalSemaphoreInfoCount = 1;
    submit_info[0].pSignalSemaphoreInfos = &sem_info[0];
    submit_info[0].waitSemaphoreInfoCount = 0;

    cb_info[1].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info[1].commandBuffer = cmd_bufs[1];

    sem_info[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
    sem_info[1].semaphore = semaphore2;
    sem_info[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;

    submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info[1].pNext = nullptr;
    submit_info[1].commandBufferInfoCount = 1;
    submit_info[1].pCommandBufferInfos = &cb_info[1];
    submit_info[1].waitSemaphoreInfoCount = 1;
    submit_info[1].pWaitSemaphoreInfos = &sem_info[0];
    submit_info[1].signalSemaphoreInfoCount = 1;
    submit_info[1].pSignalSemaphoreInfos = &sem_info[1];

    cb_info[2].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info[2].commandBuffer = cmd_bufs[2];
    cb_info[3].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info[3].commandBuffer = cmd_bufs[3];

    submit_info[2].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info[2].pNext = nullptr;
    submit_info[2].commandBufferInfoCount = 2;
    submit_info[2].pCommandBufferInfos = &cb_info[2];
    submit_info[2].waitSemaphoreInfoCount = 1;
    submit_info[2].pWaitSemaphoreInfos = &sem_info[1];
    submit_info[2].signalSemaphoreInfoCount = 0;
    submit_info[2].pSignalSemaphoreInfos = nullptr;
    vk::QueueSubmit2KHR(m_device->m_queue, 3, submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroySemaphore(m_device->device(), semaphore1, NULL);
    vk::DestroySemaphore(m_device->device(), semaphore2, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, CommandBufferSimultaneousUseSync) {
    m_errorMonitor->ExpectSuccess();
    VkResult err;

    // Record (empty!) command buffer that can be submitted multiple times
    // simultaneously.
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
    m_commandBuffer->begin(&cbbi);
    m_commandBuffer->end();

    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence fence;
    err = vk::CreateFence(m_device->device(), &fci, nullptr, &fence);
    ASSERT_VK_SUCCESS(err);

    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    VkSemaphore s1, s2;
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &s1);
    ASSERT_VK_SUCCESS(err);
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &s2);
    ASSERT_VK_SUCCESS(err);

    // Submit CB once signaling s1, with fence so we can roll forward to its retirement.
    VkCommandBufferSubmitInfoKHR cb_info = {};
    cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
    cb_info.commandBuffer = m_commandBuffer->handle();

    VkSemaphoreSubmitInfoKHR sem_info = {};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
    sem_info.semaphore = s1;

    VkSubmitInfo2KHR submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &sem_info;

    err = vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, fence);
    ASSERT_VK_SUCCESS(err);

    // Submit CB again, signaling s2.
    sem_info.semaphore = s2;
    err = vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Wait for fence.
    err = vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    ASSERT_VK_SUCCESS(err);

    // CB is still in flight from second submission, but semaphore s1 is no
    // longer in flight. delete it.
    vk::DestroySemaphore(m_device->device(), s1, nullptr);

    m_errorMonitor->VerifyNotFound();

    // Force device idle and clean up remaining objects
    vk::DeviceWaitIdle(m_device->device());
    vk::DestroySemaphore(m_device->device(), s2, nullptr);
    vk::DestroyFence(m_device->device(), fence, nullptr);
}

TEST_F(Sync2Test, BarrierLayoutToImageUsage) {
    TEST_DESCRIPTION("Ensure barriers' new and old VkImageLayout are compatible with their images' VkImageUsageFlags");
    m_errorMonitor->ExpectSuccess();

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        GTEST_SKIP() << kSkipPrefix << "  No Depth + Stencil format found. Skipped." << std::endl;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageMemoryBarrier2KHR img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
    img_barrier.pNext = NULL;
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
            VkDependencyInfoKHR dep_info = {};
            dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
            dep_info.imageMemoryBarrierCount = 1;
            dep_info.pImageMemoryBarriers = &img_barrier;

            img_barrier.oldLayout = buffer_layouts[i].old_layout;
            img_barrier.newLayout = buffer_layouts[i].new_layout;
            img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT_KHR;
            img_barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);

            img_barrier.oldLayout = buffer_layouts[i].new_layout;
            img_barrier.newLayout = buffer_layouts[i].old_layout;
            img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT_KHR;
            img_barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);
        }
        m_commandBuffer->end();

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, WaitEventThenSet) {
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");

    m_errorMonitor->ExpectSuccess();

    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk::BeginCommandBuffer(command_buffer, &begin_info);

        VkMemoryBarrier2KHR barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT_KHR;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR;

        VkDependencyInfoKHR dep_info = {};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        dep_info.memoryBarrierCount = 1;
        dep_info.pMemoryBarriers = &barrier;

        vk::CmdWaitEvents2KHR(command_buffer, 1, &event, &dep_info);

        vk::CmdResetEvent2KHR(command_buffer, event, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR);
        vk::EndCommandBuffer(command_buffer);
    }
    {
        VkCommandBufferSubmitInfoKHR cb_info = {};
        cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
        cb_info.commandBuffer = command_buffer;

        VkSubmitInfo2KHR submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;

        vk::QueueSubmit2KHR(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    { vk::SetEvent(m_device->device(), event); }

    vk::QueueWaitIdle(queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceTwoWFF) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by two consecutive WaitForFences calls on the same fence.");

    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        GTEST_SKIP() << kSkipPrefix << "  Queue family needs to have multiple queues to run this test." << std::endl;
    }

    m_errorMonitor->ExpectSuccess();

    VkFence fence;
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 1, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        VkMemoryBarrier2KHR barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        VkDependencyInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        info.memoryBarrierCount = 1;
        info.pMemoryBarriers = &barrier;

        vk::CmdPipelineBarrier2KHR(command_buffer[0], &info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkCommandBufferSubmitInfoKHR cb_info = {};
        cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
        cb_info.commandBuffer = command_buffer[0];

        VkSemaphoreSubmitInfoKHR sem_info = {};
        sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
        sem_info.semaphore = semaphore;

        VkSubmitInfo2KHR submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &sem_info;
        vk::QueueSubmit2KHR(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        VkCommandBufferSubmitInfoKHR cb_info = {};
        cb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
        cb_info.commandBuffer = command_buffer[1];

        VkSemaphoreSubmitInfoKHR sem_info = {};
        sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
        sem_info.semaphore = semaphore;
        sem_info.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        VkSubmitInfo2KHR submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cb_info;
        submit_info.waitSemaphoreInfoCount = 1;
        submit_info.pWaitSemaphoreInfos = &sem_info;

        vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, TwoSubmitInfosWithSemaphoreOneQueueSubmitsOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers each in a separate SubmitInfo sent in a single QueueSubmit call followed by a WaitForFences call.");

    m_errorMonitor->ExpectSuccess();

    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence);

    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);

    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer[2] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        VkMemoryBarrier2KHR mem_barrier = {};
        mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
        mem_barrier.srcAccessMask = 0;
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        mem_barrier.dstAccessMask = 0;
        mem_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        VkDependencyInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        info.dependencyFlags = 0;
        info.memoryBarrierCount = 1;
        info.pMemoryBarriers = &mem_barrier;
        vk::CmdPipelineBarrier2KHR(command_buffer[0], &info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[0], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[0]);
    }
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        VkViewport viewport{};
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        viewport.width = 512;
        viewport.height = 512;
        viewport.x = 0;
        viewport.y = 0;
        vk::CmdSetViewport(command_buffer[1], 0, 1, &viewport);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo2KHR submit_info[2] = {};
        VkCommandBufferSubmitInfoKHR cb_info[2] = {};
        VkSemaphoreSubmitInfoKHR sem_info[2] = {};

        cb_info[0].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
        cb_info[0].commandBuffer = command_buffer[0];
        sem_info[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
        sem_info[0].semaphore = semaphore;
        submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
        submit_info[0].commandBufferInfoCount = 1;
        submit_info[0].pCommandBufferInfos = &cb_info[0];
        submit_info[0].signalSemaphoreInfoCount = 1;
        submit_info[0].pSignalSemaphoreInfos = &sem_info[0];

        cb_info[1].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
        cb_info[1].commandBuffer = command_buffer[1];
        sem_info[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
        sem_info[1].semaphore = semaphore;
        sem_info[1].stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
        submit_info[1].commandBufferInfoCount = 1;
        submit_info[1].pCommandBufferInfos = &cb_info[1];
        submit_info[1].waitSemaphoreInfoCount = 1;
        submit_info[1].pWaitSemaphoreInfos = &sem_info[1];

        vk::QueueSubmit2KHR(m_device->m_queue, 2, &submit_info[0], fence);
    }

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 2, &command_buffer[0]);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(Sync2Test, ClearDepthStencilWithValidRange) {
    TEST_DESCRIPTION("Record clear depth with a valid VkImageSubresourceRange");

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
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }

    image.ImageMemoryBarrier(m_commandBuffer, ds_aspect, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Try good case with VK_REMAINING
    {
        m_errorMonitor->ExpectSuccess();
        VkImageSubresourceRange range = {ds_aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyNotFound();
    }
}

TEST_F(Sync2Test, QueueSubmitTimelineSemaphore) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        GTEST_SKIP() << kSkipPrefix << "  Extension " << VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
                     << " is not supported." << std::endl;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    } else {
        GTEST_SKIP() << kSkipPrefix << "  Extension " << VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
                     << " not supported by device; skipped." << std::endl;
    }

    m_errorMonitor->ExpectSuccess();

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSemaphoreSubmitInfoKHR sem_info[2] = {};
    VkSubmitInfo2KHR submit_info[2] = {};

    sem_info[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
    sem_info[0].semaphore = semaphore;
    sem_info[0].value = 1;
    sem_info[0].stageMask = stageFlags;
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info[0].signalSemaphoreInfoCount = 1;
    submit_info[0].pSignalSemaphoreInfos = &sem_info[0];

    sem_info[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
    sem_info[1].semaphore = semaphore;
    sem_info[1].value = 1;
    sem_info[1].stageMask = stageFlags;
    submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
    submit_info[1].waitSemaphoreInfoCount = 1;
    submit_info[1].pWaitSemaphoreInfos = &sem_info[1];

    vk::QueueSubmit2KHR(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE);

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

// TODO: this test crashes in the driver if run without the validation layer present
#if 0
TEST_F(Sync2Test, SubmitInfoDeviceMask) {
    TEST_DESCRIPTION("SubmitInfo2KHR deviceMask.");

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

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
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);

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
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
}
#endif
