# Copyright 2023-2024 Nintendo
#
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from base_generator import BaseGenerator

REQUIRED_EXTENSIONS = [
    'VK_KHR_dynamic_rendering',
    'VK_KHR_maintenance2',
    'VK_EXT_private_data',
    'VK_EXT_extended_dynamic_state',
    'VK_EXT_extended_dynamic_state2',
    'VK_EXT_extended_dynamic_state3',
    'VK_EXT_vertex_input_dynamic_state',
    'VK_EXT_graphics_pipeline_library',
    'VK_KHR_pipeline_library',
    'VK_KHR_multiview',
    'VK_KHR_create_renderpass2',
    'VK_KHR_depth_stencil_resolve',
    'VK_KHR_driver_properties',
    'VK_EXT_dynamic_rendering_unused_attachments',
]
OPTIONAL_EXTENSIONS = [
    'VK_EXT_transform_feedback',
    'VK_EXT_conservative_rasterization',
    'VK_EXT_depth_clip_enable',
    'VK_EXT_sample_locations',
    'VK_EXT_provoking_vertex',
    'VK_EXT_line_rasterization',
    'VK_EXT_depth_clip_control',
    'VK_NV_framebuffer_mixed_samples',
    'VK_NV_coverage_reduction_mode',
    'VK_NV_fragment_coverage_to_color',
    'VK_NV_clip_space_w_scaling',
    'VK_NV_viewport_swizzle',
    'VK_NV_shading_rate_image',
    'VK_NV_representative_fragment_test',
]

DYNAMIC_STATE_MAP = {
    'VK_EXT_extended_dynamic_state': {
        'VK_DYNAMIC_STATE_CULL_MODE_EXT': {
            'commands': ['vkCmdSetCullMode', 'vkCmdSetCullModeEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkCullModeFlags', 'name': 'cull_mode'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT': {
            'commands': [
                'vkCmdSetDepthBoundsTestEnable',
                'vkCmdSetDepthBoundsTestEnableEXT',
            ],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkBool32', 'name': 'depth_bounds_test_enable'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT': {
            'commands': ['vkCmdSetDepthCompareOp', 'vkCmdSetDepthCompareOpEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkCompareOp', 'name': 'depth_compare_op'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT': {
            'commands': ['vkCmdSetDepthTestEnable', 'vkCmdSetDepthTestEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkBool32', 'name': 'depth_test_enable'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT': {
            'commands': ['vkCmdSetDepthWriteEnable', 'vkCmdSetDepthWriteEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkBool32', 'name': 'depth_write_enable'}],
        },
        'VK_DYNAMIC_STATE_FRONT_FACE_EXT': {
            'commands': ['vkCmdSetFrontFace', 'vkCmdSetFrontFaceEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkFrontFace', 'name': 'front_face'}],
        },
        'VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT': {
            'commands': ['vkCmdSetPrimitiveTopology', 'vkCmdSetPrimitiveTopologyEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [
                {'type': 'VkPrimitiveTopology', 'name': 'primitive_topology'}
            ],
            'additional_interception_requirement': '(flags & DeviceData::HAS_PRIMITIVE_TOPLOGY_UNRESTRICTED) != 0',
        },
        'VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT': {
            'commands': ['vkCmdSetScissorWithCount', 'vkCmdSetScissorWithCountEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'uint32_t', 'name': 'num_scissors'}],
        },
        'VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT': {
            'commands': ['vkCmdSetViewportWithCount', 'vkCmdSetViewportWithCountEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'uint32_t', 'name': 'num_viewports'}],
        },
        'VK_DYNAMIC_STATE_STENCIL_OP_EXT': {
            'commands': ['vkCmdSetStencilOp', 'vkCmdSetStencilOpEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [
                {'type': 'VkStencilOpState', 'name': 'stencil_front'},
                {'type': 'VkStencilOpState', 'name': 'stencil_back'},
            ],
        },
        'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT': {
            'commands': ['vkCmdSetStencilTestEnable', 'vkCmdSetStencilTestEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState',
            'variables': [{'type': 'VkBool32', 'name': 'stencil_test_enable'}],
        },
        'VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT': {
            'commands': ['vkCmdBindVertexBuffers2', 'vkCmdBindVertexBuffers2EXT'],
            'required_feature_struct_member': 'extendedDynamicState',
        },
    },
    'VK_EXT_extended_dynamic_state2': {
        'VK_DYNAMIC_STATE_LOGIC_OP_EXT': {
            'commands': ['vkCmdSetLogicOpEXT'],
            'required_feature_struct_member': 'extendedDynamicState2LogicOp',
            'variables': [{'type': 'VkLogicOp', 'name': 'logic_op'}],
        },
        'VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE': {
            'commands': [
                'vkCmdSetPrimitiveRestartEnable',
                'vkCmdSetPrimitiveRestartEnableEXT',
            ],
            'required_feature_struct_member': 'extendedDynamicState2',
            'variables': [{'type': 'VkBool32', 'name': 'primitive_restart_enable'}],
        },
        'VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE': {
            'commands': [
                'vkCmdSetRasterizerDiscardEnable',
                'vkCmdSetRasterizerDiscardEnableEXT',
            ],
            'required_feature_struct_member': 'extendedDynamicState2',
            'variables': [{'type': 'VkBool32', 'name': 'rasterizer_discard_enable'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT': {
            'commands': ['vkCmdSetDepthBiasEnable', 'vkCmdSetDepthBiasEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState2',
            'variables': [{'type': 'VkBool32', 'name': 'depth_bias_enable'}],
        },
        'VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT': {
            'commands': ['vkCmdSetPatchControlPointsEXT'],
            'required_feature_struct_member': 'extendedDynamicState2PatchControlPoints',
            'variables': [{'type': 'uint32_t', 'name': 'patch_control_points'}],
        },
    },
    'VK_EXT_extended_dynamic_state3': {
        'VK_DYNAMIC_STATE_POLYGON_MODE_EXT': {
            'commands': ['vkCmdSetPolygonModeEXT'],
            'required_feature_struct_member': 'extendedDynamicState3PolygonMode',
            'variables': [{'type': 'VkPolygonMode', 'name': 'polygon_mode'}],
        },
        'VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT': {
            'commands': ['vkCmdSetRasterizationSamplesEXT'],
            'required_feature_struct_member': 'extendedDynamicState3RasterizationSamples',
            'variables': [
                {'type': 'VkSampleCountFlagBits', 'name': 'rasterization_samples'}
            ],
        },
        'VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT': {
            'commands': ['vkCmdSetLogicOpEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3LogicOpEnable',
            'variables': [{'type': 'VkBool32', 'name': 'logic_op_enable'}],
        },
        'VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT': {
            'commands': ['vkCmdSetColorWriteMaskEXT'],
            'required_feature_struct_member': 'extendedDynamicState3ColorWriteMask',
            'variables': [],
        },
        'VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT': {
            'commands': ['vkCmdSetColorBlendEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3ColorBlendEnable',
            'variables': [],
        },
        'VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT': {
            'commands': ['vkCmdSetColorBlendEquationEXT'],
            'required_feature_struct_member': 'extendedDynamicState3ColorBlendEquation',
            'variables': [],
        },
        'VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT': {
            'commands': ['vkCmdSetDepthClampEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3DepthClampEnable',
            'variables': [{'type': 'VkBool32', 'name': 'depth_clamp_enable'}],
        },
        'VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT': {
            'commands': ['vkCmdSetTessellationDomainOriginEXT'],
            'required_feature_struct_member': 'extendedDynamicState3TessellationDomainOrigin',
            'variables': [
                {'type': 'VkTessellationDomainOrigin', 'name': 'domain_origin'}
            ],
        },
        'VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT': {
            'commands': ['vkCmdSetAlphaToOneEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3AlphaToOneEnable',
            'variables': [{'type': 'VkBool32', 'name': 'alpha_to_one_enable'}],
        },
        'VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT': {
            'commands': ['vkCmdSetAlphaToCoverageEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3AlphaToCoverageEnable',
            'variables': [{'type': 'VkBool32', 'name': 'alpha_to_coverage_enable'}],
        },
        'VK_DYNAMIC_STATE_SAMPLE_MASK_EXT': {
            'commands': ['vkCmdSetSampleMaskEXT'],
            'required_feature_struct_member': 'extendedDynamicState3SampleMask',
            'variables': [
                {
                    'type': 'VkSampleMask',
                    'name': 'sample_mask',
                    'compile_time_array_length': 'kMaxSampleMaskLength',
                }
            ],
        },
        'VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT': {
            'commands': ['vkCmdSetRasterizationStreamEXT'],
            'required_feature_struct_member': 'extendedDynamicState3RasterizationStream',
            'variables': [{'type': 'uint32_t', 'name': 'rasterization_stream'}],
            'required_additional_extensions': [
                {'name': 'VK_EXT_TRANSFORM_FEEDBACK',
                    'member': 'transformFeedback'}
            ],
        },
        'VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT': {
            'commands': ['vkCmdSetConservativeRasterizationModeEXT'],
            'required_feature_struct_member': 'extendedDynamicState3ConservativeRasterizationMode',
            'variables': [
                {
                    'type': 'VkConservativeRasterizationModeEXT',
                    'name': 'conservative_rasterization_mode',
                }
            ],
            'required_additional_extensions': [{'name': 'VK_EXT_CONSERVATIVE_RASTERIZATION'}],
        },
        'VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT': {
            'commands': ['vkCmdSetExtraPrimitiveOverestimationSizeEXT'],
            'required_feature_struct_member': 'extendedDynamicState3ExtraPrimitiveOverestimationSize',
            'variables': [
                {'type': 'float', 'name': 'extra_primitive_overestimation_size'}
            ],
            'required_additional_extensions': [{'name': 'VK_EXT_CONSERVATIVE_RASTERIZATION'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT': {
            'commands': ['vkCmdSetDepthClipEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3DepthClipEnable',
            'variables': [{'type': 'VkBool32', 'name': 'depth_clip_enable'}],
            'required_additional_extensions': [
                {'name': 'VK_EXT_DEPTH_CLIP_ENABLE', 'member': 'depthClipEnable'}
            ],
        },
        'VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT': {
            'commands': ['vkCmdSetSampleLocationsEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3SampleLocationsEnable',
            'variables': [{'type': 'VkBool32', 'name': 'sample_locations_enable'}],
            'required_additional_extensions': [{'name': 'VK_EXT_SAMPLE_LOCATIONS'}],
        },
        'VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT': {
            'commands': ['vkCmdSetProvokingVertexModeEXT'],
            'required_feature_struct_member': 'extendedDynamicState3ProvokingVertexMode',
            'variables': [
                {'type': 'VkProvokingVertexModeEXT',
                    'name': 'provoking_vertex_mode'}
            ],
            'required_additional_extensions': [{'name': 'VK_EXT_PROVOKING_VERTEX'}],
        },
        'VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT': {
            'commands': ['vkCmdSetLineRasterizationModeEXT'],
            'required_feature_struct_member': 'extendedDynamicState3LineRasterizationMode',
            'variables': [
                {
                    'type': 'VkLineRasterizationModeEXT',
                    'name': 'line_rasterization_mode',
                }
            ],
            'required_additional_extensions': [{'name': 'VK_EXT_LINE_RASTERIZATION'}],
        },
        'VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT': {
            'commands': ['vkCmdSetLineStippleEnableEXT'],
            'required_feature_struct_member': 'extendedDynamicState3LineStippleEnable',
            'variables': [{'type': 'VkBool32', 'name': 'stippled_line_enable'}],
            'required_additional_extensions': [{'name': 'VK_EXT_LINE_RASTERIZATION'}],
        },
        'VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT': {
            'commands': ['vkCmdSetDepthClipNegativeOneToOneEXT'],
            'required_feature_struct_member': 'extendedDynamicState3DepthClipNegativeOneToOne',
            'variables': [{'type': 'VkBool32', 'name': 'negative_one_to_one'}],
            'required_additional_extensions': [{'name': 'VK_EXT_DEPTH_CLIP_CONTROL'}],
        },
        'VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV': {
            'commands': ['vkCmdSetCoverageModulationModeNV'],
            'required_feature_struct_member': 'extendedDynamicState3CoverageModulationMode',
            'variables': [
                {
                    'type': 'VkCoverageModulationModeNV',
                    'name': 'coverage_modulation_mode',
                }
            ],
            'required_additional_extensions': [
                {'name': 'VK_NV_FRAMEBUFFER_MIXED_SAMPLES'}
            ],
        },
        'VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV': {
            'commands': ['vkCmdSetCoverageModulationTableEnableNV'],
            'required_feature_struct_member': 'extendedDynamicState3CoverageModulationTableEnable',
            'variables': [
                {'type': 'VkBool32', 'name': 'coverage_modulation_table_enable'}
            ],
            'required_additional_extensions': [
                {'name': 'VK_NV_FRAMEBUFFER_MIXED_SAMPLES'}
            ],
        },
        'VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV': {
            'commands': ['vkCmdSetCoverageModulationTableNV'],
            'required_feature_struct_member': 'extendedDynamicState3CoverageModulationTable',
            'variables': [
                {
                    'type': 'float',
                    'name': 'coverage_modulation_table_values',
                    'compile_time_array_length': 'VK_SAMPLE_COUNT_64_BIT',
                },
                {'type': 'uint32_t', 'name': 'coverage_modulation_table_count'},
            ],
            'required_additional_extensions': [
                {'name': 'VK_NV_FRAMEBUFFER_MIXED_SAMPLES'}
            ],
        },
        'VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV': {
            'commands': ['vkCmdSetCoverageReductionModeNV'],
            'required_feature_struct_member': 'extendedDynamicState3CoverageReductionMode',
            'variables': [
                {'type': 'VkCoverageReductionModeNV',
                    'name': 'coverage_reduction_mode'}
            ],
            'required_additional_extensions': [{'name': 'VK_NV_COVERAGE_REDUCTION_MODE'}],
        },
        'VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV': {
            'commands': ['vkCmdSetCoverageToColorEnableNV'],
            'required_feature_struct_member': 'extendedDynamicState3CoverageToColorEnable',
            'variables': [{'type': 'VkBool32', 'name': 'coverage_to_color_enable'}],
            'required_additional_extensions': [
                {'name': 'VK_NV_FRAGMENT_COVERAGE_TO_COLOR'}
            ],
        },
        'VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV': {
            'commands': ['vkCmdSetCoverageToColorLocationNV'],
            'required_feature_struct_member': 'extendedDynamicState3CoverageToColorLocation',
            'variables': [{'type': 'uint32_t', 'name': 'coverage_to_color_location'}],
            'required_additional_extensions': [
                {'name': 'VK_NV_FRAGMENT_COVERAGE_TO_COLOR'}
            ],
        },
        'VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV': {
            'commands': ['vkCmdSetViewportWScalingEnableNV'],
            'required_feature_struct_member': 'extendedDynamicState3ViewportWScalingEnable',
            'variables': [{'type': 'VkBool32', 'name': 'viewport_w_scaling_enable'}],
            'required_additional_extensions': [{'name': 'VK_NV_CLIP_SPACE_W_SCALING'}],
        },
        'VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV': {
            'commands': ['vkCmdSetViewportSwizzleNV'],
            'required_feature_struct_member': 'extendedDynamicState3ViewportSwizzle',
            'variables': [
                {'type': 'uint32_t', 'name': 'viewport_swizzle_count'},
                {
                    'type': 'VkViewportSwizzleNV',
                    'name': 'viewport_swizzle',
                    'init_time_array_length': 'max_viewports',
                },
            ],
            'required_additional_extensions': [{'name': 'VK_NV_VIEWPORT_SWIZZLE'}],
        },
        'VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV': {
            'commands': ['vkCmdSetShadingRateImageEnableNV'],
            'required_feature_struct_member': 'extendedDynamicState3ShadingRateImageEnable',
            'variables': [{'type': 'VkBool32', 'name': 'shading_rate_image_enable'}],
            'required_additional_extensions': [{'name': 'VK_NV_SHADING_RATE_IMAGE'}],
        },
        'VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV': {
            'commands': ['vkCmdSetRepresentativeFragmentTestEnableNV'],
            'required_feature_struct_member': 'extendedDynamicState3RepresentativeFragmentTestEnable',
            'variables': [
                {'type': 'VkBool32', 'name': 'representative_fragment_test_enable'}
            ],
            'required_additional_extensions': [
                {'name': 'VK_NV_REPRESENTATIVE_FRAGMENT_TEST'}
            ],
        },
    },
    'VK_EXT_vertex_input_dynamic_state': {
        'VK_DYNAMIC_STATE_VERTEX_INPUT_EXT': {
            'commands': ['vkCmdSetVertexInputEXT'],
            'required_feature_struct_member': 'vertexInputDynamicState',
            'variables': [
                {
                    'type': 'VkVertexInputAttributeDescription',
                    'name': 'vertex_input_attribute_description',
                    'init_time_array_length': 'max_vertex_input_attributes'
                },
                {
                    'type': 'VkVertexInputBindingDescription',
                    'name': 'vertex_input_binding_description',
                    'init_time_array_length': 'max_vertex_input_bindings'
                },
                {'type': 'uint32_t', 'name': 'num_vertex_input_attribute_descriptions'},
                {'type': 'uint32_t', 'name': 'num_vertex_input_binding_descriptions'}
            ],
        },
    },
}

STATIC_DRAW_STATES = [
    {
        'type': 'VkFormat',
        'name': 'depth_attachment_format'
    },
    {
        'type': 'VkFormat',
        'name': 'stencil_attachment_format'
    },
    {
        'type': 'VkFormat',
        'name': 'color_attachment_format',
        'init_time_array_length': 'max_color_attachments'
    },
    {
        'type': 'uint32_t',
        'name': 'num_color_attachments'
    },
    {
        'type': 'VkPipelineColorBlendAttachmentState',
        'name': 'color_blend_attachment_state',
        'init_time_array_length': 'max_color_attachments'
    },
    {
        'type': 'ComparableShader',
        'name': 'comparable_shader',
        'compile_time_array_length': 'NUM_SHADERS'
    }
]

ENTRY_POINTS = {
    'intercept': {
        'instance': [
            ['GetInstanceProcAddr'],
            ['_layerGetPhysicalDeviceProcAddr'],
            ['CreateInstance'],
            ['DestroyInstance'],
            ['EnumerateInstanceExtensionProperties'],
            ['EnumerateInstanceLayerProperties']
        ],
        'physical_device': [
            ['EnumerateDeviceExtensionProperties'],
            ['GetPhysicalDeviceFeatures2', 'GetPhysicalDeviceFeatures2KHR'],
            ['GetPhysicalDeviceProperties2', 'GetPhysicalDeviceProperties2KHR'],
            ['CreateDevice']
        ],
        'device': [
            ['GetDeviceProcAddr'],
            ['DestroyDevice'],
            ['CreateShadersEXT'],
            ['DestroyShaderEXT'],
            ['CmdBindShadersEXT'],
            ['CmdBindPipeline'],
            ['CreateImageView'],
            ['DestroyImageView'],
            ['AllocateCommandBuffers'],
            ['FreeCommandBuffers'],
            ['DestroyCommandPool'],
            ['BeginCommandBuffer'],
            ['CmdBeginRendering', 'CmdBeginRenderingKHR'],
            ['GetShaderBinaryDataEXT'],
            ['CmdSetViewportWithCount', 'CmdSetViewportWithCountEXT'],
            ['CmdSetScissorWithCount', 'CmdSetScissorWithCountEXT'],
            ['CmdSetVertexInputEXT'],
            ['CmdSetPrimitiveTopology', 'CmdSetPrimitiveTopologyEXT'],
            ['CmdSetPrimitiveRestartEnable', 'CmdSetPrimitiveRestartEnableEXT'],
            ['CmdSetRasterizerDiscardEnable', 'CmdSetRasterizerDiscardEnableEXT'],
            ['CmdSetRasterizationSamplesEXT'],
            ['CmdSetPolygonModeEXT'],
            ['CmdSetCullMode', 'CmdSetCullModeEXT'],
            ['CmdSetFrontFace', 'CmdSetFrontFaceEXT'],
            ['CmdSetDepthTestEnable', 'CmdSetDepthTestEnableEXT'],
            ['CmdSetDepthWriteEnable', 'CmdSetDepthWriteEnableEXT'],
            ['CmdSetDepthCompareOp', 'CmdSetDepthCompareOpEXT'],
            ['CmdSetStencilTestEnable', 'CmdSetStencilTestEnableEXT'],
            ['CmdSetLogicOpEnableEXT'],
            ['CmdSetColorBlendEnableEXT'],
            ['CmdSetColorBlendEquationEXT'],
            ['CmdSetColorWriteMaskEXT'],
            ['CmdSetDepthBoundsTestEnable', 'CmdSetDepthBoundsTestEnableEXT'],
            ['CmdSetDepthBiasEnable', 'CmdSetDepthBiasEnableEXT'],
            ['CmdSetDepthClampEnableEXT'],
            ['CmdSetStencilOp', 'CmdSetStencilOpEXT'],
            ['CmdDraw'],
            ['CmdDrawIndirect'],
            ['CmdDrawIndirectCount'],
            ['CmdDrawIndexed'],
            ['CmdDrawIndexedIndirect'],
            ['CmdDrawIndexedIndirectCount'],
            ['CmdDrawMeshTasksEXT'],
            ['CmdDrawMeshTasksIndirectEXT'],
            ['CmdDrawMeshTasksIndirectCountEXT'],
            ['CmdDrawMeshTasksNV'],
            ['CmdDrawMeshTasksIndirectNV'],
            ['CmdDrawMeshTasksIndirectCountNV'],
            ['CmdSetLogicOpEXT'],
            ['CmdSetPatchControlPointsEXT'],
            ['CmdSetTessellationDomainOriginEXT'],
            ['CmdSetAlphaToOneEnableEXT'],
            ['CmdSetAlphaToCoverageEnableEXT'],
            ['CmdSetSampleMaskEXT'],
            ['CmdSetRasterizationStreamEXT'],
            ['CmdSetConservativeRasterizationModeEXT'],
            ['CmdSetExtraPrimitiveOverestimationSizeEXT'],
            ['CmdSetDepthClipEnableEXT'],
            ['CmdSetSampleLocationsEnableEXT'],
            ['CmdSetProvokingVertexModeEXT'],
            ['CmdSetLineRasterizationModeEXT'],
            ['CmdSetLineStippleEnableEXT'],
            ['CmdSetDepthClipNegativeOneToOneEXT'],
            ['CmdSetCoverageModulationModeNV'],
            ['CmdSetCoverageModulationTableEnableNV'],
            ['CmdSetCoverageModulationTableNV'],
            ['CmdSetCoverageReductionModeNV'],
            ['CmdSetCoverageToColorEnableNV'],
            ['CmdSetCoverageToColorLocationNV'],
            ['CmdSetViewportWScalingEnableNV'],
            ['CmdSetViewportSwizzleNV'],
            ['CmdSetShadingRateImageEnableNV'],
            ['CmdSetRepresentativeFragmentTestEnableNV'],
            ['CmdBindVertexBuffers2', 'CmdBindVertexBuffers2EXT'],
            ['CmdBindDescriptorSets'],
            ['CmdPushDescriptorSetKHR'],
            ['CmdPushDescriptorSetWithTemplateKHR'],
            ['CmdPushConstants'],
            ['SetPrivateData', 'SetPrivateDataEXT'],
            ['GetPrivateData', 'GetPrivateDataEXT'],
            ['CreateDescriptorUpdateTemplate', 'CreateDescriptorUpdateTemplateKHR'],
            ['DestroyDescriptorUpdateTemplate', 'DestroyDescriptorUpdateTemplateKHR'],
            ['SetDebugUtilsObjectNameEXT'],
            ['SetDebugUtilsObjectTagEXT']
        ]
    },
    'forward': {
        'instance': [],
        'physical_device': [
            ['GetPhysicalDeviceProperties'],
            ['GetPhysicalDeviceImageFormatProperties']
        ],
        'device': [
            ['CreatePipelineLayout'],
            ['CreateGraphicsPipelines'],
            ['CreateComputePipelines'],
            ['DestroyPipelineLayout'],
            ['DestroyPipeline'],
            ['CreateShaderModule'],
            ['DestroyShaderModule'],
            ['CreatePipelineCache'],
            ['DestroyPipelineCache'],
            ['GetPipelineCacheData'],
            ['MergePipelineCaches'],
            ['GetDeviceQueue'],
            ['QueueSubmit'],
            ['QueueSubmit2', 'QueueSubmit2KHR'],
            ['CmdSetViewport'],
            ['CmdSetScissor'],
            ['CreatePrivateDataSlotEXT'],
            ['DestroyPrivateDataSlotEXT'],
            ['CmdBindVertexBuffers']
        ]
    }
}

PIPELINE_SUBSETS = {
    'VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT': {
        'shaders': [
            'VERTEX_SHADER',
            'TESSELLATION_CONTROL_SHADER',
            'TESSELLATION_EVALUATION_SHADER',
            'GEOMETRY_SHADER',
            'TASK_SHADER',
            'MESH_SHADER'
        ],
        'dynamic_state_enums': [
            'VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT',
            'VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT',
            'VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT',
            'VK_DYNAMIC_STATE_POLYGON_MODE_EXT',
            'VK_DYNAMIC_STATE_CULL_MODE_EXT',
            'VK_DYNAMIC_STATE_FRONT_FACE_EXT',
            'VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT',
            'VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT'
        ]
    },
    'VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT': {
        'shaders': [
            'FRAGMENT_SHADER'
        ],
        'dynamic_state_enums': [
            'VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT',
            'VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT',
            'VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT',
            'VK_DYNAMIC_STATE_SAMPLE_MASK_EXT',
            'VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT',
            'VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT',
            'VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT',
            'VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT',
            'VK_DYNAMIC_STATE_STENCIL_OP_EXT'
        ]
    }
}


class ShaderObjectGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write('''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See shader_object_generator.py for modifications

/*
 * Copyright 2023-2025 Nintendo
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
    ''')

        if self.filename == 'shader_object_constants.h':
            self.generate_constants_file()
        elif self.filename == 'shader_object_device_data_declare_extension_variables.inl':
            self.generate_device_data_declare_extension_variables()
        elif self.filename == 'shader_object_device_data_set_extension_variables.inl':
            self.generate_device_data_set_extension_variables()
        elif self.filename == 'shader_object_device_data_dynamic_state_adding.inl':
            self.generate_device_data_dynamic_state_adding()
        elif self.filename == 'shader_object_create_device_feature_structs.inl':
            self.generate_create_device_feature_structs()
        elif self.filename == 'shader_object_find_intercepted_dynamic_state_function_by_name.inl':
            self.generate_find_intercepted_dynamic_state_function_by_name()
        elif self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.generate_full_draw_state_struct_members()
        elif self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.generate_full_draw_state_struct_members()
        elif self.filename == 'shader_object_full_draw_state_utility_functions.inl':
            self.generate_full_draw_state_utility_functions()
        elif self.filename == 'shader_object_entry_points_x_macros.inl':
            self.generate_entry_points()

    def snake_case_to_upper_camel_case(self, x):
        uppercase_next = True

        out = ''
        for char in x:
            is_underscore = char == '_'

            if not is_underscore:
                if uppercase_next:
                    out += char.upper()
                else:
                    out += char

            uppercase_next = is_underscore

        return out

    def is_variable_array(self, variable_data):
        return 'init_time_array_length' in variable_data or 'compile_time_array_length' in variable_data

    def get_private_variable_name(self, variable_data):
        name = variable_data['name']
        is_plural = self.is_variable_array(variable_data)
        return name + ('s' if is_plural else '') + '_'

    def get_feature_struct(self, extension):
        for struct in extension.structs:
            if 'VkPhysicalDevice' in struct.name and 'Features' in struct.name:
                return struct
        return None

    def get_extension_flag_name(self, extension_name):
        cut_prefix_amount = 1 if 'NV' in extension_name else 2
        return '_'.join(extension_name.split('_')[cut_prefix_amount:]).upper()

    def get_extension_variable_name(self, extension_name):
        return '_'.join(extension_name.split('_')[2:]).lower()

    def generate_constants_file(self):

        all_important_extensions = []
        all_important_extensions.extend(REQUIRED_EXTENSIONS)
        all_important_extensions.extend(OPTIONAL_EXTENSIONS)
        all_important_extensions.append('VK_EXT_shader_object')

        self.write('#pragma once\n')
        self.write('#include <cstdint>')
        self.write('#include <cstring>\n')

        self.write('#include "shader_object/shader_object_util.h"\n')

        self.write('enum AdditionalExtensionFlagBits {')
        shift_amount = 0

        for extension in all_important_extensions:
            name = self.get_extension_flag_name(extension)
            self.write(f'    {name}= 1u << {shift_amount},')
            shift_amount = shift_amount + 1
        self.write('};')
        self.write('using AdditionalExtensionFlags = uint32_t;\n')

        self.write('inline AdditionalExtensionFlags AdditionalExtensionStringToFlag(const char* pExtensionName) {')

        for extension in all_important_extensions:
            name = self.get_extension_flag_name(extension)
            extension_macro_name = self.vk.extensions[extension].nameString
            self.write(
                f'if (strncmp(pExtensionName, {extension_macro_name}, VK_MAX_EXTENSION_NAME_SIZE) == 0) {{ return {name}; }}')

        self.write('return {};}')

        self.write('struct ExtensionData { const char* extension_name; AdditionalExtensionFlagBits flag;};')

        self.write('constexpr ExtensionData kAdditionalExtensions[] = {')
        for extension in REQUIRED_EXTENSIONS:
            name = self.get_extension_flag_name(extension)
            extension_macro_name = self.vk.extensions[extension].nameString
            self.write(f'    {{ {extension_macro_name}, {name} }},')

        num_dynamic_states = 0
        for dynamic_state_field in self.vk.enums['VkDynamicState'].fields:
            if len(dynamic_state_field.extensions) == 0 or set(dynamic_state_field.extensions).issubset(REQUIRED_EXTENSIONS):
                num_dynamic_states += 1

        self.write('};\n')

        self.write(f'constexpr uint32_t kMaxDynamicStates = {num_dynamic_states};')
        self.write('constexpr uint32_t kMaxSampleMaskLength = CalculateRequiredGroupSize(VK_SAMPLE_COUNT_64_BIT, 32);')

    def generate_device_data_declare_extension_variables(self):
        for extension_name in REQUIRED_EXTENSIONS + OPTIONAL_EXTENSIONS:
            extension = self.vk.extensions[extension_name]
            feature_struct = self.get_feature_struct(extension)
            if feature_struct is not None:
                self.write(f'{feature_struct.name} {self.get_extension_variable_name(extension.name)};')

    def generate_device_data_set_extension_variables(self):
        for extension_name in REQUIRED_EXTENSIONS + OPTIONAL_EXTENSIONS:
            extension = self.vk.extensions[extension_name]
            if self.get_feature_struct(extension) is None:
                continue
            var_name = self.get_extension_variable_name(extension.name)
            self.write(f'device_data->{var_name} = {var_name}_ptr ? *{var_name}_ptr : {var_name}_local;')

    def generate_device_data_dynamic_state_adding(self):
        for extension, extension_details in DYNAMIC_STATE_MAP.items():
            for dynamic_state, dynamic_state_details in extension_details.items():
                if self.get_feature_struct(self.vk.extensions[extension]) is None:
                    continue

                member = dynamic_state_details['required_feature_struct_member']

                struct_name = self.get_extension_variable_name(extension)
                conditional = f'{struct_name}_ptr && {struct_name}_ptr->{member} == VK_TRUE'
                if 'required_additional_extensions' in dynamic_state_details:
                    for additional_extension in dynamic_state_details['required_additional_extensions']:
                        additional_struct_name = self.get_extension_variable_name(additional_extension['name'])

                        conditional += ' && (enabled_additional_extensions & ' + self.get_extension_flag_name(additional_extension['name']) + ') != 0'

                        if 'member' in additional_extension:
                            additional_struct_member = additional_extension['member']
                            conditional += f' && {additional_struct_name}_ptr && {additional_struct_name}_ptr->{additional_struct_member} == VK_TRUE'

                self.write(f'if ({conditional}) {{')
                self.write(f'device_data->AddDynamicState({dynamic_state});')
                self.write('}')

    def generate_create_device_feature_structs(self):
        self.write('VkBaseOutStructure* appended_features_chain = nullptr;')
        self.write('VkBaseOutStructure* appended_features_chain_last = nullptr;')
        self.write('auto VK_VERSION_1_1_ptr = reinterpret_cast<VkPhysicalDeviceVulkan11Features*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES));')
        self.write('auto VK_VERSION_1_3_ptr = reinterpret_cast<VkPhysicalDeviceVulkan13Features*>(FindStructureInChain(device_next_chain, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES));')

        # Performance could be improved, this is a naive implementation

        for extension_name in REQUIRED_EXTENSIONS:
            extension = self.vk.extensions[extension_name]
            feature_struct = self.get_feature_struct(extension)
            if feature_struct is None:
                continue

            struct_name = feature_struct.name
            sType = feature_struct.sType
            enum_name = extension_name
            var_name = self.get_extension_variable_name(extension.name)
            initializer = '{' + sType + '}'
            promoted_struct = f'{extension.promotedTo}_ptr == nullptr && ' if extension.promotedTo is not None else ''
            self.write(f'''auto {var_name}_ptr = reinterpret_cast<{struct_name}*>(FindStructureInChain(device_next_chain, {sType}));
            {struct_name} {var_name}_local{initializer};
            if ({promoted_struct}{var_name}_ptr == nullptr && (physical_device_data->supported_additional_extensions & {enum_name}) != 0) {{
                {var_name}_ptr = &{var_name}_local;
                if (appended_features_chain_last == nullptr) {{
                    appended_features_chain = (VkBaseOutStructure*){var_name}_ptr;
                    appended_features_chain_last = appended_features_chain;
                }} else {{
                    appended_features_chain_last->pNext = (VkBaseOutStructure*){var_name}_ptr;
                    appended_features_chain_last = appended_features_chain_last->pNext;
                }}
            }}''')

        for extension_name in OPTIONAL_EXTENSIONS:
            extension = self.vk.extensions[extension_name]
            feature_struct = self.get_feature_struct(extension)
            if feature_struct is None:
                continue

            struct_name = feature_struct.name
            sType = feature_struct.sType
            enum_name = extension_name
            var_name = self.get_extension_variable_name(extension.name)
            initializer = '{' + sType + '}'
            self.write(f'''auto {var_name}_ptr = reinterpret_cast<{struct_name}*>(FindStructureInChain(device_next_chain, {sType}));
            {struct_name} {var_name}_local{initializer};
            if ({var_name}_ptr == nullptr) {{
                {var_name}_ptr = &{var_name}_local;
            }}''')

    def generate_find_intercepted_dynamic_state_function_by_name(self):
        for extension, extension_details in DYNAMIC_STATE_MAP.items():
            struct_name = self.get_extension_variable_name(extension)

            for dynamic_state_details in extension_details.values():

                member = dynamic_state_details['required_feature_struct_member']

                conditional = f'{struct_name}.{member} == VK_TRUE && ('
                for idx, command_name in enumerate(dynamic_state_details['commands']):
                    conditional = (
                        conditional
                        + ('' if idx == 0 else ' || ')
                        + f'strcmp("{command_name}", pName) == 0'
                    )
                conditional = conditional + ')'

                if 'additional_interception_requirement' in dynamic_state_details:
                    conditional = conditional + ' && ' + dynamic_state_details['additional_interception_requirement']

                self.write(f'''if ({conditional}) {{
                DEBUG_LOG("not intercepting %s because real dynamic state exists ({struct_name}.{member} == VK_TRUE)\\n", pName);
                return nullptr;
                }}''')

    def generate_full_draw_state_struct_members(self):
        getter_setter_section = ['']
        getter_setter_section_impl = ['']
        operator_equals_section = ['']
        member_variables_section = ['']
        subset_compare_section = dict()

        state_to_pipeline_subset = dict()
        for subset, subset_details in PIPELINE_SUBSETS.items():
            for dynamic_state_enum in subset_details['dynamic_state_enums']:
                state_to_pipeline_subset[dynamic_state_enum] = subset

        def generate_getter_and_setter(state_group, name, variable_name, type, array_length=None):
            is_singular = array_length == None
            is_singular_sep_decl_str = '' if is_singular else 'uint32_t index, '
            is_singular_decl_str = '' if is_singular else 'uint32_t index'
            is_singular_str = '' if is_singular else '[index]'
            upper_camel_case_name = self.snake_case_to_upper_camel_case(name)
            getter_setter_section.append(f'void Set{upper_camel_case_name}({is_singular_sep_decl_str}{type} const& element);')
            getter_setter_section_impl.append(
                f'''void FullDrawStateData::Set{upper_camel_case_name}({is_singular_sep_decl_str}{type} const& element) {{
                    if (element == {variable_name}{is_singular_str}) {{
                        return;
                    }}
                    dirty_hash_bits_.set({state_group});
                    MarkDirty();
                    {variable_name}{is_singular_str} = element;
                }}''')

            getter_setter_section.append(f'{type} const& Get{upper_camel_case_name}({is_singular_decl_str}) const;')
            getter_setter_section_impl.append(f'''
                {type} const& FullDrawStateData::Get{upper_camel_case_name}({is_singular_decl_str}) const {{
                return {variable_name}{is_singular_str};
            }}''')

            if not is_singular:
                getter_setter_section.append(f'{type} const* Get{upper_camel_case_name}Ptr() const;')
                getter_setter_section_impl.append(f'{type} const* FullDrawStateData::Get{upper_camel_case_name}Ptr() const {{')
                getter_setter_section_impl.append(f'    return {variable_name};')
                getter_setter_section_impl.append('}')

        def process_variable(variable_state_group, dynamic_state_enum, variable_data):
            var_type = variable_data['type']
            var_name_private = self.get_private_variable_name(variable_data)

            comparison_code = []

            if 'init_time_array_length' in variable_data:
                # dynamic array member, we know the length at init time but not compile time
                length = 'limits_.' + variable_data['init_time_array_length']
                member_variables_section.append(f'{var_type}* {var_name_private}{{}};')

                comparison_code.append(f'if (o.{length} != {length}) {{')
                comparison_code.append('    return false;')
                comparison_code.append('}')
                comparison_code.append(f'for (uint32_t i = 0; i < {length}; ++i) {{')
                if var_type == 'VkFormat':
                    comparison_code.append(f'if (!(o.{var_name_private}[i] == {var_name_private}[i]) && (!o.dynamic_rendering_unused_attachments_ || o.{var_name_private}[i] != VK_FORMAT_UNDEFINED) && (!dynamic_rendering_unused_attachments_ || {var_name_private}[i] != VK_FORMAT_UNDEFINED)) {{')
                else:
                    comparison_code.append(f'    if (!(o.{var_name_private}[i] == {var_name_private}[i])) {{')
                comparison_code.append('        return false;')
                comparison_code.append('    }')
                comparison_code.append('}\n')

                generate_getter_and_setter(variable_state_group, variable_data['name'], var_name_private, var_type, length)
            elif 'compile_time_array_length' in variable_data:
                # static array member
                length = variable_data['compile_time_array_length']
                member_variables_section.append(f'{var_type} {var_name_private}[{length}];')

                comparison_code.append(
                    f'''for (uint32_t i = 0; i < {length}; ++i) {{
                        if (!(o.{var_name_private}[i] == {var_name_private}[i])) {{
                            return false;
                        }}
                    }}\n''')

                generate_getter_and_setter(variable_state_group, variable_data['name'], var_name_private, var_type, length)
            else:
                # value member
                if var_name_private == 'patch_control_points_':
                    member_variables_section.append(f'{var_type} {var_name_private} = 1;')
                else:
                    member_variables_section.append(f'{var_type} {var_name_private}{{}};')

                if var_type == 'VkFormat':
                    comparison_code.append(f'if (!(o.{var_name_private} == {var_name_private}) && (!o.dynamic_rendering_unused_attachments_ || o.{var_name_private} != VK_FORMAT_UNDEFINED) && (!dynamic_rendering_unused_attachments_ || {var_name_private} != VK_FORMAT_UNDEFINED)) {{')
                elif var_name_private == 'num_color_attachments_':
                    comparison_code.append(f'if (!(o.{var_name_private} == {var_name_private}) && (!o.dynamic_rendering_unused_attachments_ && !dynamic_rendering_unused_attachments_)) {{')
                else:
                    comparison_code.append(f'if (!(o.{var_name_private} == {var_name_private})) {{')
                comparison_code.append('    return false;')
                comparison_code.append('}\n')

                generate_getter_and_setter(variable_state_group, variable_data['name'], var_name_private, var_type)

            operator_equals_section.extend(comparison_code)

            if dynamic_state_enum in state_to_pipeline_subset:
                subset = state_to_pipeline_subset[dynamic_state_enum]
                if subset not in subset_compare_section:
                    subset_compare_section[subset] = []
                subset_compare_section[subset].append(comparison_code)

        for static_state in STATIC_DRAW_STATES:
            process_variable('MISC', None, static_state)

        for extension, extension_details in DYNAMIC_STATE_MAP.items():
            for dynamic_state, dynamic_state_details in extension_details.items():
                if 'variables' not in dynamic_state_details:
                    continue
                for variable in dynamic_state_details['variables']:
                    process_variable(extension, dynamic_state, variable)

        if self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.write('#include "shader_object/shader_object_structs.h"\n')
            self.write('namespace shader_object {\n')

        # public section
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('public:')

        # generate state group enum
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('    enum StateGroup { MISC')
            for extension in REQUIRED_EXTENSIONS:
                if extension in DYNAMIC_STATE_MAP:
                    self.write(', ' + self.get_extension_flag_name(extension))
            self.write(', NUM_STATE_GROUPS };\n')

        # getters and setters
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('\n    '.join(getter_setter_section))
        elif self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.write('\n'.join(getter_setter_section_impl))

        # generate operator== function
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('    bool operator==(FullDrawStateData const& o) const;')
        elif self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.write('bool FullDrawStateData::operator==(FullDrawStateData const& o) const {')
            self.write('\n    '.join(operator_equals_section))
            self.write('    return true;')
            self.write('}\n')

        # generate compare state subset
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('bool CompareStateSubset(FullDrawStateData const& o, VkGraphicsPipelineLibraryFlagBitsEXT flag) const;')
        elif self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.write('bool FullDrawStateData::CompareStateSubset(FullDrawStateData const& o, VkGraphicsPipelineLibraryFlagBitsEXT flag) const {')

            for subset_flag, details in subset_compare_section.items():
                self.write(f'if (flag == {subset_flag}) {{')

                for shader in PIPELINE_SUBSETS[subset_flag]['shaders']:
                    self.write(f'if (!(o.comparable_shaders_[{shader}] == comparable_shaders_[{shader}])) {{')
                    self.write('return false;')
                    self.write('}')

                for comparison_lines in details:
                    lines = ['']
                    lines.extend(comparison_lines)
                    self.write('\n            '.join(lines))
                self.write('}')

            self.write('return true;')
            self.write('}')

        # private section
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('\nprivate:')

        # generate partial hash functions
        variables_by_state_group = dict()
        for extension, extension_details in DYNAMIC_STATE_MAP.items():
            state_group = self.get_extension_flag_name(extension)

            variables_by_state_group[state_group] = []
            for dynamic_state_details in extension_details.values():
                if 'variables' not in dynamic_state_details:
                    continue
                variables_by_state_group[state_group].extend(
                    dynamic_state_details['variables'])

        variables_by_state_group['MISC'] = STATIC_DRAW_STATES
        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('size_t CalculatePartialHash(StateGroup state_group) const;')
        elif self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.write('size_t FullDrawStateData::CalculatePartialHash(StateGroup state_group) const {')
            self.write('switch (state_group) {')
            self.write('default: assert(false); return 0;')
            for state_group, variables in variables_by_state_group.items():
                self.write(f'case {state_group}:')
                self.write('{')
                self.write('size_t res = 17;')

                for variable in variables:
                    var_type = variable['type']
                    var_name_private = self.get_private_variable_name(variable)
                    if self.is_variable_array(variable):
                        self.write('// TODO: array comparison')
                    else:
                        if var_type == 'VkFormat':
                            self.write('if (!dynamic_rendering_unused_attachments_) {')
                            self.write(f'res = res * 31 + std::hash<{var_type}>()({var_name_private});')
                            self.write('}')
                        else:
                            self.write(f'res = res * 31 + std::hash<{var_type}>()({var_name_private});')

                self.write('return res;')
                self.write('}')
            self.write('}')
            self.write('}\n')

        if self.filename == 'shader_object_full_draw_state_struct_members.inl':
            self.write('\n    '.join(member_variables_section))
        elif self.filename == 'shader_object_full_draw_state_struct_members.cpp':
            self.write('}  // namespace shader_object\n')

    def generate_full_draw_state_utility_functions(self):
        # gather init time array variables

        init_time_array_variables = []
        for variable in STATIC_DRAW_STATES:
            if 'init_time_array_length' in variable:
                init_time_array_variables.append(variable)

        for extension_details in DYNAMIC_STATE_MAP.values():
            for dynamic_state_details in extension_details.values():
                if 'variables' not in dynamic_state_details:
                    continue

                for variable in dynamic_state_details['variables']:
                    if 'init_time_array_length' in variable:
                        init_time_array_variables.append(variable)

        # generate GetSizeInBytes

        self.write('static constexpr void ReserveMemory(AlignedMemory& aligned_memory, Limits const& limits) {')
        self.write('aligned_memory.Add<FullDrawStateData>();')

        for var_data in init_time_array_variables:
            var_type = var_data['type']
            length = var_data['init_time_array_length']
            self.write(f'aligned_memory.Add<{var_type}>(limits.{length});')

        self.write('}\n')

        # generate SetInternalArrayPointers

        self.write('static void SetInternalArrayPointers(FullDrawStateData* state, Limits const& limits) {')
        self.write('// Set array pointers to beginning of their memory')
        self.write('AlignedMemory aligned_memory;')
        self.write('aligned_memory.SetMemoryWritePtr((char*)state + sizeof(FullDrawStateData));')

        for var_data in init_time_array_variables:
            var_name_private = self.get_private_variable_name(var_data)
            var_type = var_data['type']
            length = var_data['init_time_array_length']

            self.write(f'state->{var_name_private} = aligned_memory.GetNextAlignedPtr<{var_type}>(limits.{length});')

        self.write('}')

    def generate_entry_points(self):
        # generate all x macros

        def generate_x_macro(name, function_names_list):
            out = []
            for function_names in function_names_list:
                canon = function_names[0]
                alias = function_names[1] if len(function_names) > 1 else None
                out.append(f'ENTRY_POINT({canon})')
                if alias is not None:
                    out.append(f'ENTRY_POINT_ALIAS({alias}, {canon})')
            if len(out) > 0:
                self.write(f'#define {name} \\')
                self.write('\\\n    '.join(out))
            else:
                self.write(f'#define {name}')
            self.write('\n')

        generate_x_macro('ENTRY_POINTS_INSTANCE', ENTRY_POINTS['intercept']['instance'])
        generate_x_macro('ENTRY_POINTS_PHYSICAL_DEVICE', ENTRY_POINTS['intercept']['physical_device'])
        generate_x_macro('ENTRY_POINTS_DEVICE', ENTRY_POINTS['intercept']['device'])
        generate_x_macro('ADDITIONAL_INSTANCE_FUNCTIONS', ENTRY_POINTS['forward']['instance'])
        generate_x_macro('ADDITIONAL_PHYSICAL_DEVICE_FUNCTIONS', ENTRY_POINTS['forward']['physical_device'])
        generate_x_macro('ADDITIONAL_DEVICE_FUNCTIONS', ENTRY_POINTS['forward']['device'])
