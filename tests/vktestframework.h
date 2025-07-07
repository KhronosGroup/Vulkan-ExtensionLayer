/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Charles Giessen <charles@LunarG.com>
 */

#pragma once

// SPVRemapper.h includes spirv.hpp11 which uses "None" in the enum class SamplerAddressingMode. Need to undefine the macro so
#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef None
#endif

#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/SPVRemapper.h"
#include "spirv-tools/libspirv.h"
#include "glslang/Public/ShaderLang.h"
#include "test_common.h"
#include "test_environment.h"

#include <fstream>
#include <iostream>
#include <list>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// Can be used by tests to record additional details / description of test
#define TEST_DESCRIPTION(desc) RecordProperty("description", desc)

using namespace std;

class VkImageObj;

class VkTestFramework : public ::testing::Test {
  public:
    VkFormat GetFormat(VkInstance instance, vk_testing::Device *device);
    static bool optionMatch(const char *option, char *optionLine);
    static void InitArgs(int *argc, char *argv[]);
    static void Finish();

    bool GLSLtoSPV(VkPhysicalDeviceLimits const *const device_limits, const VkShaderStageFlagBits shader_type, const char *pshader,
                   std::vector<unsigned int> &spv, bool debug = false, uint32_t spirv_minor_version = 0);
    bool ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *pasm, std::vector<unsigned int> &spv);
    static bool m_canonicalize_spv;
    static bool m_strip_spv;
    static bool m_do_everything_spv;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    static inline ANativeWindow *window = nullptr;
#endif

    char **ReadFileData(const char *fileName);
    void FreeFileData(char **data);

    static void setEnvironmentSetting(std::string setting, const char *val);

  protected:
    VkTestFramework();
    virtual ~VkTestFramework() = 0;

  private:
    int m_compile_options;
    int m_num_shader_strings;
    TBuiltInResource Resources;
    void SetMessageOptions(EShMessages &messages);
    void ProcessConfigFile(VkPhysicalDeviceLimits const *const device_limits);
    EShLanguage FindLanguage(const std::string &name);
    EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);
    std::string ConfigFile;
    bool SetConfigFile(const std::string &name);
    static int m_width;
    static int m_height;
    string m_testName;
};

class TestEnvironment : public ::testing::Environment {
  public:
    void SetUp();

    void TearDown();
};
