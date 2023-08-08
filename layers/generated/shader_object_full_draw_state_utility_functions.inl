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

    static constexpr void ReserveMemory(AlignedMemory& aligned_memory, Limits const& limits) {
        aligned_memory.Add<FullDrawStateData>();
        aligned_memory.Add<VkFormat>(limits.max_color_attachments);
        aligned_memory.Add<VkPipelineColorBlendAttachmentState>(limits.max_color_attachments);
        aligned_memory.Add<VkViewportSwizzleNV>(limits.max_viewports);
        aligned_memory.Add<VkVertexInputAttributeDescription>(limits.max_vertex_input_attributes);
        aligned_memory.Add<VkVertexInputBindingDescription>(limits.max_vertex_input_bindings);
    }

    static void SetInternalArrayPointers(FullDrawStateData* state, Limits const& limits) {
        // Set array pointers to beginning of their memory
        AlignedMemory aligned_memory;
        aligned_memory.SetMemoryWritePtr((char*)state + sizeof(FullDrawStateData));
        state->color_attachment_formats_ = aligned_memory.GetNextAlignedPtr<VkFormat>(limits.max_color_attachments);
        state->color_blend_attachment_states_ = aligned_memory.GetNextAlignedPtr<VkPipelineColorBlendAttachmentState>(limits.max_color_attachments);
        state->viewport_swizzles_ = aligned_memory.GetNextAlignedPtr<VkViewportSwizzleNV>(limits.max_viewports);
        state->vertex_input_attribute_descriptions_ = aligned_memory.GetNextAlignedPtr<VkVertexInputAttributeDescription>(limits.max_vertex_input_attributes);
        state->vertex_input_binding_descriptions_ = aligned_memory.GetNextAlignedPtr<VkVertexInputBindingDescription>(limits.max_vertex_input_bindings);
    }
