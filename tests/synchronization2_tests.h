#pragma once

#include "extension_layer_tests.h"

class Sync2Test : public VkExtensionLayerTest {
  public:
    void SetUp() override;
    void TearDown() override;

  protected:
    void ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb, const VkBufferMemoryBarrier2KHR *buf_barrier,
                                  const VkImageMemoryBarrier2KHR *img_barrier);
    void ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                                const VkBufferMemoryBarrier2KHR *buf_barrier, const VkImageMemoryBarrier2KHR *img_barrier);
};

class Sync2CompatTest : public VkExtensionLayerTest {};
