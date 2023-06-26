/* Copyright (c) 2023 The Khronos Group Inc.
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
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
 * Author: Vikram Kushwaha <vkushwaha@nvidia.com>
 */

#include <type_traits>

#include "extension_layer_tests.h"
#include "decompression_tests.h"
#include "decompression_data.h"

void DecompressionTest::SetUp() {
    VkBool32 force_enable = VK_TRUE;

    VkLayerSettingEXT settings[] = {
        {"VK_LAYER_KHRONOS_memory_decompression", "force_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &force_enable}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{
        VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
        static_cast<uint32_t>(std::size(settings)), &settings[0]};

    VkExtensionLayerTest::SetUp();
    SetTargetApiVersion(VK_API_VERSION_1_2);
    VkExtensionLayerTest::AddSurfaceInstanceExtension();
    instance_layers_.push_back("VK_LAYER_KHRONOS_memory_decompression");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &layer_settings_create_info));

    VkExtensionLayerTest::AddSwapchainDeviceExtension();
}

void DecompressionTest::TearDown() {}

TEST_F(DecompressionTest, DecompressMemory) {
    TEST_DESCRIPTION("Test vkCmdDecompressMemoryNV.");
    VkResult result = VK_SUCCESS;

    if (InstanceExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_portability_subset enabled, skipping.\n";
    }

    if (!CheckDecompressionSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " decompression not supported, skipping test";
    }

    VkConstantBufferObj srcBuffer1(m_device, COMPRESSED_SIZE1, compressedData1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    ASSERT_TRUE(srcBuffer1.initialized());

    VkConstantBufferObj srcBuffer2(m_device, COMPRESSED_SIZE2, compressedData2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    ASSERT_TRUE(srcBuffer2.initialized());

    std::vector<uint8_t> decompressData(2 * DECOMPRESSED_SIZE_ALIGNED, 0xFF);
    VkConstantBufferObj dstBuffer(m_device, 2 * DECOMPRESSED_SIZE_ALIGNED, decompressData.data(), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    ASSERT_TRUE(dstBuffer.initialized());

    VkCommandPool command_pool;
    auto pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    ASSERT_TRUE(result == VK_SUCCESS);

    VkCommandBuffer command_buffer;
    auto command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);
    ASSERT_TRUE(result == VK_SUCCESS);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkBufferDeviceAddressInfo srcBufferAddr1 = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, srcBuffer1.handle()};
        VkBufferDeviceAddressInfo srcBufferAddr2 = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, srcBuffer2.handle()};
        VkBufferDeviceAddressInfo dstBufferAddr = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, dstBuffer.handle()};

        VkDecompressMemoryRegionNV region[2] = {};
        region[0].compressedSize = COMPRESSED_SIZE1;
        region[0].decompressedSize = DECOMPRESSED_SIZE;
        region[0].srcAddress = vk::GetBufferDeviceAddress(m_device->device(), &srcBufferAddr1);
        region[0].dstAddress = vk::GetBufferDeviceAddress(m_device->device(), &dstBufferAddr);
        region[0].decompressionMethod = VK_MEMORY_DECOMPRESSION_METHOD_GDEFLATE_1_0_BIT_NV;
        region[1].compressedSize = COMPRESSED_SIZE2;
        region[1].decompressedSize = DECOMPRESSED_SIZE;
        region[1].srcAddress = vk::GetBufferDeviceAddress(m_device->device(), &srcBufferAddr2);
        region[1].dstAddress = vk::GetBufferDeviceAddress(m_device->device(), &dstBufferAddr) + DECOMPRESSED_SIZE_ALIGNED;
        region[1].decompressionMethod = VK_MEMORY_DECOMPRESSION_METHOD_GDEFLATE_1_0_BIT_NV;

        auto begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer, &begin_info);
        vk::CmdDecompressMemoryNV(command_buffer, 2, &region[0]);
        vk::EndCommandBuffer(command_buffer);
    }
    {
        auto submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    vk::QueueWaitIdle(queue);
    {
        void *decompressedDataResult = dstBuffer.memory().map();
        int compareResult = memcmp(decompressedDataResult, decompressedData, DECOMPRESSED_SIZE);
        compareResult |= memcmp((uint8_t *)decompressedDataResult + DECOMPRESSED_SIZE_ALIGNED, decompressedData, DECOMPRESSED_SIZE);
        ASSERT_TRUE(compareResult == 0);
        dstBuffer.memory().unmap();
    }

    vk::FreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);
}

TEST_F(DecompressionTest, DecompressMemoryIndirect) {
    TEST_DESCRIPTION("Test vkCmdDecompressMemoryIndirectCountNV.");

    if (InstanceExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_portability_subset enabled, skipping.\n";
    }

    if (!CheckDecompressionSupportAndInitState()) {
        GTEST_SKIP() << kSkipPrefix << " decompression not supported, skipping test";
    }
    VkResult result = VK_SUCCESS;

    VkConstantBufferObj srcBuffer1(m_device, COMPRESSED_SIZE1, compressedData1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    ASSERT_TRUE(srcBuffer1.initialized());

    VkConstantBufferObj srcBuffer2(m_device, COMPRESSED_SIZE2, compressedData2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    ASSERT_TRUE(srcBuffer2.initialized());

    std::vector<uint8_t> decompressData(2 * DECOMPRESSED_SIZE_ALIGNED, 0xFF);
    VkConstantBufferObj dstBuffer(m_device, 2 * DECOMPRESSED_SIZE_ALIGNED, decompressData.data(), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    ASSERT_TRUE(dstBuffer.initialized());

    VkBufferDeviceAddressInfo srcBufferAddr1 = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, srcBuffer1.handle()};
    VkBufferDeviceAddressInfo srcBufferAddr2 = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, srcBuffer2.handle()};
    VkBufferDeviceAddressInfo dstBufferAddr = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, dstBuffer.handle()};

    const uint32_t count = 2;
    VkDecompressMemoryRegionNV regions[count];
    regions[0].compressedSize = COMPRESSED_SIZE1;
    regions[0].decompressedSize = DECOMPRESSED_SIZE;
    regions[0].srcAddress = vk::GetBufferDeviceAddress(m_device->device(), &srcBufferAddr1);
    regions[0].dstAddress = vk::GetBufferDeviceAddress(m_device->device(), &dstBufferAddr);
    regions[0].decompressionMethod = VK_MEMORY_DECOMPRESSION_METHOD_GDEFLATE_1_0_BIT_NV;
    regions[1].compressedSize = COMPRESSED_SIZE2;
    regions[1].decompressedSize = DECOMPRESSED_SIZE;
    regions[1].srcAddress = vk::GetBufferDeviceAddress(m_device->device(), &srcBufferAddr2);
    regions[1].dstAddress = vk::GetBufferDeviceAddress(m_device->device(), &dstBufferAddr) + DECOMPRESSED_SIZE_ALIGNED;
    regions[1].decompressionMethod = VK_MEMORY_DECOMPRESSION_METHOD_GDEFLATE_1_0_BIT_NV;

    VkConstantBufferObj indirectBufferCount(m_device, sizeof(uint32_t), &count, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    ASSERT_TRUE(dstBuffer.initialized());
    VkConstantBufferObj indirectBufferDecompress(m_device, count * sizeof(VkDecompressMemoryRegionNV), &regions[0],
                                                 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    ASSERT_TRUE(dstBuffer.initialized());

    VkBufferDeviceAddressInfo indirectCountAddrInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr,
                                                       indirectBufferCount.handle()};
    VkDeviceAddress indirectCountAddr = vk::GetBufferDeviceAddress(m_device->device(), &indirectCountAddrInfo);
    ASSERT_TRUE(indirectCountAddr != 0);

    VkBufferDeviceAddressInfo decompressParamAddrInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr,
                                                         indirectBufferDecompress.handle()};
    VkDeviceAddress decompressParamAddr = vk::GetBufferDeviceAddress(m_device->device(), &decompressParamAddrInfo);
    ASSERT_TRUE(decompressParamAddr != 0);

    VkCommandPool command_pool;
    auto pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    ASSERT_TRUE(result == VK_SUCCESS);

    VkCommandBuffer command_buffer;
    auto command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);
    ASSERT_TRUE(result == VK_SUCCESS);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        auto begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer, &begin_info);
        vk::CmdDecompressMemoryIndirectCountNV(command_buffer, decompressParamAddr, indirectCountAddr,
                                               sizeof(VkDecompressMemoryRegionNV));
        vk::EndCommandBuffer(command_buffer);
    }
    {
        auto submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    vk::QueueWaitIdle(queue);
    {
        void *decompressedDataResult = dstBuffer.memory().map();
        int compareResult = memcmp(decompressedDataResult, decompressedData, DECOMPRESSED_SIZE);
        compareResult |= memcmp((uint8_t *)decompressedDataResult + DECOMPRESSED_SIZE_ALIGNED, decompressedData, DECOMPRESSED_SIZE);
        ASSERT_TRUE(compareResult == 0);
        dstBuffer.memory().unmap();
    }

    vk::FreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);
}
