#pragma once

#include "extension_layer_tests.h"

class ShaderObjectTest : public VkExtensionLayerTest {
  public:
    void SetUp() override;
    void TearDown() override;

  protected:
    void BindDefaultDynamicStates();
};
