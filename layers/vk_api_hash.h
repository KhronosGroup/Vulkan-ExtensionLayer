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

#include <functional>

#include <vulkan/vulkan.h>

namespace std {

#define APPEND_HASH(var) res = res * 31 + std::hash<std::decay<decltype(var)>::type>()(var)

#define APPEND_HASH_ARRAY(array_member, current_length_member, max_length) \
    APPEND_HASH(current_length_member);                                    \
    for (size_t i = 0; i < current_length_member; ++i) {                   \
        APPEND_HASH((array_member)[i]);                                    \
    }                                                                      \
    for (size_t i = current_length_member; i < max_length; ++i) {          \
        APPEND_HASH(0u);                                                   \
    }

template <>
struct hash<VkStencilOpState> {
    std::size_t operator()(VkStencilOpState const& o) const {
        size_t res = 17;
        APPEND_HASH(o.failOp);
        APPEND_HASH(o.passOp);
        APPEND_HASH(o.depthFailOp);
        APPEND_HASH(o.compareOp);
        APPEND_HASH(o.compareMask);
        APPEND_HASH(o.writeMask);
        APPEND_HASH(o.reference);
        return res;
    }
};

template <>
struct hash<VkRect2D> {
    std::size_t operator()(VkRect2D const& o) const {
        size_t res = 17;
        APPEND_HASH(o.offset.x);
        APPEND_HASH(o.offset.y);
        APPEND_HASH(o.extent.width);
        APPEND_HASH(o.extent.height);
        return res;
    }
};

template <>
struct hash<VkViewport> {
    std::size_t operator()(VkViewport const& o) const {
        size_t res = 17;
        APPEND_HASH(o.x);
        APPEND_HASH(o.y);
        APPEND_HASH(o.width);
        APPEND_HASH(o.height);
        APPEND_HASH(o.minDepth);
        APPEND_HASH(o.maxDepth);
        return res;
    }
};

template <>
struct hash<VkVertexInputAttributeDescription> {
    std::size_t operator()(VkVertexInputAttributeDescription const& o) const {
        size_t res = 17;
        APPEND_HASH(o.location);
        APPEND_HASH(o.binding);
        APPEND_HASH(o.format);
        APPEND_HASH(o.offset);
        return res;
    }
};

template <>
struct hash<VkVertexInputBindingDescription> {
    std::size_t operator()(VkVertexInputBindingDescription const& o) const {
        size_t res = 17;
        APPEND_HASH(o.binding);
        APPEND_HASH(o.stride);
        APPEND_HASH(o.inputRate);
        return res;
    }
};

template <>
struct hash<VkPipelineColorBlendAttachmentState> {
    std::size_t operator()(VkPipelineColorBlendAttachmentState const& o) const {
        size_t res = 17;
        APPEND_HASH(o.blendEnable);
        APPEND_HASH(o.srcColorBlendFactor);
        APPEND_HASH(o.dstColorBlendFactor);
        APPEND_HASH(o.colorBlendOp);
        APPEND_HASH(o.srcAlphaBlendFactor);
        APPEND_HASH(o.dstAlphaBlendFactor);
        APPEND_HASH(o.alphaBlendOp);
        APPEND_HASH(o.colorWriteMask);
        return res;
    }
};

template <>
struct hash<VkPushConstantRange> {
    std::size_t operator()(VkPushConstantRange const& o) const {
        size_t res = 17;
        APPEND_HASH(o.offset);
        APPEND_HASH(o.size);
        APPEND_HASH(o.stageFlags);
        return res;
    }
};

template <>
struct hash<VkSpecializationMapEntry> {
    std::size_t operator()(VkSpecializationMapEntry const& o) const {
        size_t res = 17;
        APPEND_HASH(o.constantID);
        APPEND_HASH(o.offset);
        APPEND_HASH(o.size);
        return res;
    }
};

template <>
struct hash<VkSpecializationInfo> {
    std::size_t operator()(VkSpecializationInfo const& o) const {
        size_t res = 17;
        APPEND_HASH_ARRAY((uint8_t*)o.pData, o.dataSize, o.dataSize);
        APPEND_HASH_ARRAY(o.pMapEntries, o.mapEntryCount, o.mapEntryCount);
        return res;
    }
};

}  // namespace std
