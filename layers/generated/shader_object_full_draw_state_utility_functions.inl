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

    static size_t GetSizeInBytes(Limits const& limits) {
        return
            sizeof(VkFormat) * limits.max_color_attachments +
            sizeof(VkPipelineColorBlendAttachmentState) * limits.max_color_attachments +
            sizeof(VkViewportSwizzleNV) * limits.max_viewports +
            sizeof(VkVertexInputAttributeDescription) * limits.max_vertex_input_attributes +
            sizeof(VkVertexInputBindingDescription) * limits.max_vertex_input_bindings +
            sizeof(FullDrawStateData);
    }

    static void SetInternalArrayPointers(FullDrawStateData* state, Limits const& limits) {
        // Set array pointers to beginning of their memory
        char* offset_ptr = (char*)state + sizeof(FullDrawStateData);

        state->color_attachment_formats_ = (VkFormat*)offset_ptr;
        offset_ptr += sizeof(VkFormat) * limits.max_color_attachments;

        state->color_blend_attachment_states_ = (VkPipelineColorBlendAttachmentState*)offset_ptr;
        offset_ptr += sizeof(VkPipelineColorBlendAttachmentState) * limits.max_color_attachments;

        state->viewport_swizzles_ = (VkViewportSwizzleNV*)offset_ptr;
        offset_ptr += sizeof(VkViewportSwizzleNV) * limits.max_viewports;

        state->vertex_input_attribute_descriptions_ = (VkVertexInputAttributeDescription*)offset_ptr;
        offset_ptr += sizeof(VkVertexInputAttributeDescription) * limits.max_vertex_input_attributes;

        state->vertex_input_binding_descriptions_ = (VkVertexInputBindingDescription*)offset_ptr;
        offset_ptr += sizeof(VkVertexInputBindingDescription) * limits.max_vertex_input_bindings;
    }
