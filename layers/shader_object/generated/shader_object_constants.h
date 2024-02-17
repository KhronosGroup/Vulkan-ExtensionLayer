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

#pragma once

#include <cstdint>
#include <cstring>

#include "shader_object/shader_object_util.h"

enum AdditionalExtensionFlagBits {
    DYNAMIC_RENDERING                    = 1u << 0,
    MAINTENANCE_2                        = 1u << 1,
    PRIVATE_DATA                         = 1u << 2,
    EXTENDED_DYNAMIC_STATE_1             = 1u << 3,
    EXTENDED_DYNAMIC_STATE_2             = 1u << 4,
    EXTENDED_DYNAMIC_STATE_3             = 1u << 5,
    VERTEX_INPUT_DYNAMIC                 = 1u << 6,
    GRAPHICS_PIPELINE_LIBRARY            = 1u << 7,
    PIPELINE_LIBRARY                     = 1u << 8,
    MULTIVIEW                            = 1u << 9,
    CREATE_RENDERPASS_2                  = 1u << 10,
    DEPTH_STENCIL_RESOLVE                = 1u << 11,
    DRIVER_PROPERTIES                    = 1u << 12,
    DYNAMIC_RENDERING_UNUSED_ATTACHMENTS = 1u << 13,
    TRANSFORM_FEEDBACK                   = 1u << 14,
    CONSERVATIVE_RASTERIZATION           = 1u << 15,
    DEPTH_CLIP_ENABLE                    = 1u << 16,
    SAMPLE_LOCATIONS                     = 1u << 17,
    PROVOKING_VERTEX                     = 1u << 18,
    LINE_RASTERIZATION                   = 1u << 19,
    DEPTH_CLIP_CONTROL                   = 1u << 20,
    NV_FRAMEBUFFER_MIXED_SAMPLES         = 1u << 21,
    NV_COVERAGE_REDUCTION_MODE           = 1u << 22,
    NV_FRAGMENT_COVERAGE_TO_COLOR        = 1u << 23,
    NV_CLIP_SPACE_W_SCALING              = 1u << 24,
    NV_VIEWPORT_SWIZZLE                  = 1u << 25,
    NV_SHADING_RATE_IMAGE                = 1u << 26,
    NV_REPRESENTATIVE_FRAGMENT_TEST      = 1u << 27,
    SHADER_OBJECT                        = 1u << 28,
};
using AdditionalExtensionFlags = uint32_t;

inline AdditionalExtensionFlags AdditionalExtensionStringToFlag(const char* pExtensionName) {
    if (strncmp(pExtensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,                    VK_MAX_EXTENSION_NAME_SIZE) == 0) { return DYNAMIC_RENDERING; }
    if (strncmp(pExtensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME,                         VK_MAX_EXTENSION_NAME_SIZE) == 0) { return MAINTENANCE_2; }
    if (strncmp(pExtensionName, VK_EXT_PRIVATE_DATA_EXTENSION_NAME,                         VK_MAX_EXTENSION_NAME_SIZE) == 0) { return PRIVATE_DATA; }
    if (strncmp(pExtensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,               VK_MAX_EXTENSION_NAME_SIZE) == 0) { return EXTENDED_DYNAMIC_STATE_1; }
    if (strncmp(pExtensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,             VK_MAX_EXTENSION_NAME_SIZE) == 0) { return EXTENDED_DYNAMIC_STATE_2; }
    if (strncmp(pExtensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,             VK_MAX_EXTENSION_NAME_SIZE) == 0) { return EXTENDED_DYNAMIC_STATE_3; }
    if (strncmp(pExtensionName, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,           VK_MAX_EXTENSION_NAME_SIZE) == 0) { return VERTEX_INPUT_DYNAMIC; }
    if (strncmp(pExtensionName, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,            VK_MAX_EXTENSION_NAME_SIZE) == 0) { return GRAPHICS_PIPELINE_LIBRARY; }
    if (strncmp(pExtensionName, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,                     VK_MAX_EXTENSION_NAME_SIZE) == 0) { return PIPELINE_LIBRARY; }
    if (strncmp(pExtensionName, VK_KHR_MULTIVIEW_EXTENSION_NAME,                            VK_MAX_EXTENSION_NAME_SIZE) == 0) { return MULTIVIEW; }
    if (strncmp(pExtensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,                  VK_MAX_EXTENSION_NAME_SIZE) == 0) { return CREATE_RENDERPASS_2; }
    if (strncmp(pExtensionName, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,                VK_MAX_EXTENSION_NAME_SIZE) == 0) { return DEPTH_STENCIL_RESOLVE; }
    if (strncmp(pExtensionName, VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,                    VK_MAX_EXTENSION_NAME_SIZE) == 0) { return DRIVER_PROPERTIES; }
    if (strncmp(pExtensionName, VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE) == 0) { return DYNAMIC_RENDERING_UNUSED_ATTACHMENTS; }
    if (strncmp(pExtensionName, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,                   VK_MAX_EXTENSION_NAME_SIZE) == 0) { return TRANSFORM_FEEDBACK; }
    if (strncmp(pExtensionName, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,           VK_MAX_EXTENSION_NAME_SIZE) == 0) { return CONSERVATIVE_RASTERIZATION; }
    if (strncmp(pExtensionName, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,                    VK_MAX_EXTENSION_NAME_SIZE) == 0) { return DEPTH_CLIP_ENABLE; }
    if (strncmp(pExtensionName, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME,                     VK_MAX_EXTENSION_NAME_SIZE) == 0) { return SAMPLE_LOCATIONS; }
    if (strncmp(pExtensionName, VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME,                     VK_MAX_EXTENSION_NAME_SIZE) == 0) { return PROVOKING_VERTEX; }
    if (strncmp(pExtensionName, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME,                   VK_MAX_EXTENSION_NAME_SIZE) == 0) { return LINE_RASTERIZATION; }
    if (strncmp(pExtensionName, VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME,                   VK_MAX_EXTENSION_NAME_SIZE) == 0) { return DEPTH_CLIP_CONTROL; }
    if (strncmp(pExtensionName, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME,             VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_FRAMEBUFFER_MIXED_SAMPLES; }
    if (strncmp(pExtensionName, VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME,               VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_COVERAGE_REDUCTION_MODE; }
    if (strncmp(pExtensionName, VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME,            VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_FRAGMENT_COVERAGE_TO_COLOR; }
    if (strncmp(pExtensionName, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME,                  VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_CLIP_SPACE_W_SCALING; }
    if (strncmp(pExtensionName, VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME,                      VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_VIEWPORT_SWIZZLE; }
    if (strncmp(pExtensionName, VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME,                    VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_SHADING_RATE_IMAGE; }
    if (strncmp(pExtensionName, VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME,          VK_MAX_EXTENSION_NAME_SIZE) == 0) { return NV_REPRESENTATIVE_FRAGMENT_TEST; }
    if (strncmp(pExtensionName, VK_EXT_SHADER_OBJECT_EXTENSION_NAME,                        VK_MAX_EXTENSION_NAME_SIZE) == 0) { return SHADER_OBJECT; }
    return {};
}

struct ExtensionData {
    const char* extension_name;
    AdditionalExtensionFlagBits flag;
};

constexpr ExtensionData kAdditionalExtensions[] = {
    { VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,                    DYNAMIC_RENDERING },
    { VK_KHR_MAINTENANCE2_EXTENSION_NAME,                         MAINTENANCE_2 },
    { VK_EXT_PRIVATE_DATA_EXTENSION_NAME,                         PRIVATE_DATA },
    { VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,               EXTENDED_DYNAMIC_STATE_1 },
    { VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,             EXTENDED_DYNAMIC_STATE_2 },
    { VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,             EXTENDED_DYNAMIC_STATE_3 },
    { VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,           VERTEX_INPUT_DYNAMIC },
    { VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,            GRAPHICS_PIPELINE_LIBRARY },
    { VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,                     PIPELINE_LIBRARY },
    { VK_KHR_MULTIVIEW_EXTENSION_NAME,                            MULTIVIEW },
    { VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,                  CREATE_RENDERPASS_2 },
    { VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,                DEPTH_STENCIL_RESOLVE },
    { VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,                    DRIVER_PROPERTIES },
    { VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME, DYNAMIC_RENDERING_UNUSED_ATTACHMENTS },
};

constexpr uint32_t kMaxDynamicStates = 58;
constexpr uint32_t kMaxSampleMaskLength = CalculateRequiredGroupSize(VK_SAMPLE_COUNT_64_BIT, 32);
