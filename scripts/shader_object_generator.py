# Copyright 2023 Nintendo
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import json
import xml.etree.ElementTree as etree

def create_generated_file(filename):
    out_file = open(filename, 'w')

    out_file.write(
"""// *** THIS FILE IS GENERATED - DO NOT EDIT ***
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

""")

    return out_file

def snake_case_to_upper_camel_case(x):
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

def is_variable_array(variable_data):
    return 'init_time_array_length' in variable_data or 'compile_time_array_length' in variable_data

def get_private_variable_name(variable_data):
    name = variable_data['name']
    is_plural = is_variable_array(variable_data)
    return name + ('s' if is_plural else '') + '_'

def generate_constants_file(data):
    out_file = create_generated_file('../layers/generated/shader_object_constants.h')

    all_important_extensions = data['extensions'] + data['optional_extensions'] + [{ 'name': 'SHADER_OBJECT', 'extension_name_macro': 'VK_EXT_SHADER_OBJECT_EXTENSION_NAME' }]

    longest_name_length = 0
    longest_macro_name_length = 0
    for extension in all_important_extensions:
        longest_name_length = max(longest_name_length, len(extension['name']))
        longest_macro_name_length = max(longest_macro_name_length, len(extension['extension_name_macro']))

    out_file.write('#pragma once\n\n')
    out_file.write('#include <cstdint>\n\n')

    out_file.write('enum AdditionalExtensionFlagBits {\n')
    shift_amount = 0

    for extension in all_important_extensions:
        name = extension['name']
        padding = ' ' * (longest_name_length - len(name))
        out_file.write(f'    {name}{padding} = 1u << {shift_amount},\n')
        shift_amount = shift_amount + 1
    out_file.write('};\n')
    out_file.write(f'using AdditionalExtensionFlags = uint32_t;\n\n')

    out_file.write('inline AdditionalExtensionFlags AdditionalExtensionStringToFlag(const char* pExtensionName) {\n')
    for extension in all_important_extensions:
        name = extension['name']
        extension_macro_name = extension['extension_name_macro']
        padding = ' ' * (longest_macro_name_length - len(extension_macro_name))
        out_file.write(f'    if (strncmp(pExtensionName, {extension_macro_name},{padding} VK_MAX_EXTENSION_NAME_SIZE) == 0) {{ return {name}; }}\n')
    out_file.write('    return {};\n}\n\n')


    out_file.write('struct ExtensionData {\n    const char* extension_name;\n    AdditionalExtensionFlagBits flag;\n};\n\n')

    num_dynamic_states = 7 + 3 # There are 7 dynamic states that are always present from Vulkan 1.0 and 3 potential dynamic states from additional extensions

    out_file.write('constexpr ExtensionData kAdditionalExtensions[] = {\n')
    for extension in data['extensions']:
        name = extension['name']
        extension_macro_name = extension['extension_name_macro']

        padding = ' ' * (longest_macro_name_length - len(extension_macro_name))
        out_file.write(f'    {{ {extension_macro_name},{padding} {name} }},\n')

        if 'dynamic_states' in extension:
            num_dynamic_states = num_dynamic_states + len(extension['dynamic_states'])

    out_file.write('};\n\n')

    out_file.write(f'constexpr uint32_t kMaxDynamicStates = {num_dynamic_states};\n')
    out_file.write(f'constexpr uint32_t kMaxSampleMaskLength = CalculateRequiredGroupSize(VK_SAMPLE_COUNT_64_BIT, 32);\n')

    out_file.close()

def generate_device_data_declare_extension_variables(data):
    out_file = create_generated_file('../layers/generated/shader_object_device_data_declare_extension_variables.inl')

    for extension in (data['extensions'] + data['optional_extensions']):
        if 'feature_struct' not in extension:
            continue

        out_file.writelines([
            extension['feature_struct'],
            ' ',
            extension['name'].lower(),
            ';\n'
        ])

    out_file.close()

def generate_device_data_set_extension_variables(data):
    out_file = create_generated_file('../layers/generated/shader_object_device_data_set_extension_variables.inl')

    for extension in (data['extensions'] + data['optional_extensions']):
        if 'feature_struct' not in extension:
            continue

        var_name = extension['name'].lower()
        out_file.write(f'device_data->{var_name} = {var_name}_ptr ? *{var_name}_ptr : {var_name}_local;\n')

    out_file.close()

def generate_device_data_dynamic_state_adding(data):
    out_file = create_generated_file('../layers/generated/shader_object_device_data_dynamic_state_adding.inl')

    for extension in data['extensions']:
        if 'dynamic_states' not in extension:
            continue

        struct_name = extension['name'].lower()
        for dynamic_state in extension['dynamic_states']:
            member = dynamic_state['required_feature_struct_member']
            enum = dynamic_state['dynamic_state_enum']

            conditional = f'{struct_name}_ptr && {struct_name}_ptr->{member} == VK_TRUE'
            if 'required_additional_extensions' in dynamic_state:
                for additional_extension in dynamic_state['required_additional_extensions']:
                    additional_struct_name = additional_extension['name'].lower()
                    conditional += ' && (enabled_additional_extensions & ' + additional_extension['name'] + ') != 0'
                    if 'member' in additional_extension:
                        additional_struct_member = additional_extension['member']
                        conditional += f' && {additional_struct_name}_ptr && {additional_struct_name}_ptr->{additional_struct_member} == VK_TRUE'

            out_file.write(f'if ({conditional}) {{\n')
            out_file.write(f'    device_data->AddDynamicState({enum});\n')
            out_file.write('}\n')

    out_file.close()

def generate_create_device_feature_structs(data):
    out_file = create_generated_file('../layers/generated/shader_object_create_device_feature_structs.inl')

    if (len(data['extensions']) == 0):
        return

    out_file.write('VkBaseOutStructure* appended_features_chain = nullptr;\n')
    out_file.write('VkBaseOutStructure* appended_features_chain_last = nullptr;\n\n')

    # Performance could be improved, this is a naive implementation

    for extension in data['extensions']:
        if 'feature_struct' not in extension:
            continue

        struct_name = extension['feature_struct']
        sType = extension['feature_struct_stype']
        enum_name = extension['name']
        var_name = extension['name'].lower()
        initializer = '{' + extension['feature_struct_stype'] + '}'
        out_file.write(f'auto {var_name}_ptr = reinterpret_cast<{struct_name}*>(FindStructureInChain(device_next_chain, {sType}));\n')
        out_file.write(f'{struct_name} {var_name}_local{initializer};\n')
        out_file.write(f'if ({var_name}_ptr == nullptr && (physical_device_data->supported_additional_extensions & {enum_name}) != 0) {{\n')
        out_file.write(f'    {var_name}_ptr = &{var_name}_local;\n')
        out_file.write(f'    if (appended_features_chain_last == nullptr) {{\n')
        out_file.write(f'        appended_features_chain = (VkBaseOutStructure*){var_name}_ptr;\n')
        out_file.write(f'        appended_features_chain_last = appended_features_chain;\n')
        out_file.write(f'    }} else {{\n')
        out_file.write(f'        appended_features_chain_last->pNext = (VkBaseOutStructure*){var_name}_ptr;\n')
        out_file.write(f'        appended_features_chain_last = appended_features_chain_last->pNext;\n')
        out_file.write(f'    }}\n')
        out_file.write(f'}}\n')

    for extension in data['optional_extensions']:
        if 'feature_struct' not in extension:
            continue

        struct_name = extension['feature_struct']
        sType = extension['feature_struct_stype']
        enum_name = extension['name']
        var_name = extension['name'].lower()
        initializer = '{' + extension['feature_struct_stype'] + '}'
        out_file.write(f'auto {var_name}_ptr = reinterpret_cast<{struct_name}*>(FindStructureInChain(device_next_chain, {sType}));\n')
        out_file.write(f'{struct_name} {var_name}_local{initializer};\n')
        out_file.write(f'if ({var_name}_ptr == nullptr) {{\n')
        out_file.write(f'    {var_name}_ptr = &{var_name}_local;\n')
        out_file.write(f'}}\n')

    out_file.write('\n')

    out_file.close()

def generate_find_intercepted_dynamic_state_function_by_name(data):
    out_file = create_generated_file('../layers/generated/shader_object_find_intercepted_dynamic_state_function_by_name.inl')

    for extension in data['extensions']:
        if 'dynamic_states' not in extension:
            continue

        struct_name = extension['name'].lower()
        for dynamic_state in extension['dynamic_states']:
            member = dynamic_state['required_feature_struct_member']

            conditional = f'{struct_name}.{member} == VK_TRUE && ('
            for idx, function_name in enumerate(dynamic_state['function_names']):
                conditional = conditional + ('' if idx == 0 else ' || ') + f'strcmp("vk{function_name}", pName) == 0'
            conditional = conditional + ')'

            if 'additional_interception_requirement' in dynamic_state:
                conditional = conditional + ' && ' + dynamic_state['additional_interception_requirement']

            out_file.write(f'if ({conditional}) {{\n')
            out_file.write(f'    DEBUG_LOG("not intercepting %s because real dynamic state exists ({struct_name}.{member} == VK_TRUE)\\n", pName);\n')
            out_file.write(f'    return nullptr;\n')
            out_file.write('}\n')

    out_file.close()

def generate_full_draw_state_struct_members(data):
    getter_setter_section = ['']
    operator_equals_section = ['']
    member_variables_section = ['']
    subset_compare_section = dict()

    state_to_pipeline_subset = dict()
    for subset in data['pipeline_subsets']:
        for dynamic_state_enum in data['pipeline_subsets'][subset]['dynamic_state_enums']:
            state_to_pipeline_subset[dynamic_state_enum] = subset

    def generate_getter_and_setter(state_group, name, variable_name, type, array_length = None):
        is_singular = array_length == None
        upper_camel_case_name = snake_case_to_upper_camel_case(name)
        getter_setter_section.append(f'void Set{upper_camel_case_name}(' + ('' if is_singular else 'uint32_t index, ') + f'{type} const& element) {{\n')
        getter_setter_section.append(f'    if (element == {variable_name}' + ('' if is_singular else '[index]') + ') {\n')
        getter_setter_section.append('        return;\n')
        getter_setter_section.append('    }\n')
        getter_setter_section.append(f'    dirty_hash_bits_.set({state_group});\n')
        getter_setter_section.append('    MarkDirty();\n')
        getter_setter_section.append(f'    {variable_name}' + ('' if is_singular else '[index]') + ' = element;\n')
        getter_setter_section.append('}\n')
        getter_setter_section.append(f'{type} const& Get{upper_camel_case_name}(' + ('' if is_singular else 'uint32_t index') + ') const {\n')
        getter_setter_section.append(f'    return {variable_name}' + ('' if is_singular else '[index]') + ';\n')
        getter_setter_section.append('}\n')
        if not is_singular:
            getter_setter_section.append(f'{type} const* Get{upper_camel_case_name}Ptr() const {{\n')
            getter_setter_section.append(f'    return {variable_name};\n')
            getter_setter_section.append('}\n')
        getter_setter_section.append('\n')

    def process_variable(variable_state_group, dynamic_state_enum, variable_data):
        var_type = variable_data['type']
        var_name_private = get_private_variable_name(variable_data)

        comparison_code = []

        if 'init_time_array_length' in variable_data:
            # dynamic array member, we know the length at init time but not compile time
            length = 'limits_.' + variable_data['init_time_array_length']
            member_variables_section.append(f'{var_type}* {var_name_private}{{}};\n')

            comparison_code.append(f'if (o.{length} != {length}) {{\n')
            comparison_code.append(f'    return false;\n')
            comparison_code.append('}\n')
            comparison_code.append(f'for (uint32_t i = 0; i < {length}; ++i) {{\n')
            comparison_code.append(f'    if (!(o.{var_name_private}[i] == {var_name_private}[i])) {{\n')
            comparison_code.append(f'        return false;\n')
            comparison_code.append(f'    }}\n')
            comparison_code.append(f'}}\n\n')

            generate_getter_and_setter(variable_state_group, variable_data['name'], var_name_private, var_type, length)
        elif 'compile_time_array_length' in variable_data:
            # static array member
            length = variable_data['compile_time_array_length']
            member_variables_section.append(f'{var_type} {var_name_private}[{length}];\n')

            comparison_code.append(f'for (uint32_t i = 0; i < {length}; ++i) {{\n')
            comparison_code.append(f'    if (!(o.{var_name_private}[i] == {var_name_private}[i])) {{\n')
            comparison_code.append(f'        return false;\n')
            comparison_code.append(f'    }}\n')
            comparison_code.append(f'}}\n\n')

            generate_getter_and_setter(variable_state_group, variable_data['name'], var_name_private, var_type, length)
        else:
            # value member
            member_variables_section.append(f'{var_type} {var_name_private}{{}};\n')
            comparison_code.append(f'if (!(o.{var_name_private} == {var_name_private})) {{\n')
            comparison_code.append('    return false;\n')
            comparison_code.append('}\n\n')

            generate_getter_and_setter(variable_state_group, variable_data['name'], var_name_private, var_type)

        operator_equals_section.extend(comparison_code)

        if dynamic_state_enum in state_to_pipeline_subset:
            subset = state_to_pipeline_subset[dynamic_state_enum]
            if subset not in subset_compare_section:
                subset_compare_section[subset] = []
            subset_compare_section[subset].append(comparison_code)

    for static_state in data['static_draw_states']:
        process_variable('MISC', None, static_state)

    for extension in data['extensions']:
        if 'dynamic_states' not in extension:
            continue

        state_group = extension['name']
        for dynamic_state in extension['dynamic_states']:
            if 'variables' not in dynamic_state:
                continue
            enum = dynamic_state['dynamic_state_enum']
            for variable in dynamic_state['variables']:
                process_variable(state_group, enum, variable)

    out_file = create_generated_file('../layers/generated/shader_object_full_draw_state_struct_members.inl')

    # public section
    out_file.write('public:\n')

    # generate state group enum
    out_file.write('    enum StateGroup { MISC')
    for extension in data['extensions']:
        if 'dynamic_states' in extension and len(extension['dynamic_states']) > 0:
            out_file.write(', ' + extension['name'])
    out_file.write(', NUM_STATE_GROUPS };\n\n')

    # getters and setters
    out_file.write('    '.join(getter_setter_section))

    # generate operator== function
    out_file.write('    bool operator==(FullDrawStateData const& o) const {\n')
    out_file.write('        '.join(operator_equals_section))
    out_file.write('        return true;\n')
    out_file.write('    }\n\n')

    # generate compare state subset
    out_file.write('    bool CompareStateSubset(FullDrawStateData const& o, VkGraphicsPipelineLibraryFlagBitsEXT flag) const {\n');

    for subset_flag in subset_compare_section:
        out_file.write(f'        if (flag == {subset_flag}) {{\n')

        for shader in data['pipeline_subsets'][subset_flag]['shaders']:
            out_file.write(f'            if (!(o.comparable_shaders_[{shader}] == comparable_shaders_[{shader}])) {{\n')
            out_file.write(f'                return false;\n')
            out_file.write(f'            }}\n')

        for comparison_lines in subset_compare_section[subset_flag]:
            lines = ['']
            lines.extend(comparison_lines)
            out_file.write('            '.join(lines))
        out_file.write('        }\n')

    out_file.write('        return true;\n')
    out_file.write('    }\n')

    # private section
    out_file.write('\nprivate:\n')

    # generate partial hash functions
    variables_by_state_group = dict()
    for extension in data['extensions']:
        if 'dynamic_states' not in extension or len(extension['dynamic_states']) == 0:
            continue
        state_group = extension['name']

        variables_by_state_group[state_group] = []
        for dynamic_state in extension['dynamic_states']:
            if 'variables' not in dynamic_state:
                continue
            variables_by_state_group[state_group].extend(dynamic_state['variables'])

    variables_by_state_group['MISC'] = data['static_draw_states']

    out_file.write('    size_t CalculatePartialHash(StateGroup state_group) const {\n')
    out_file.write('        switch (state_group) {\n')
    out_file.write('            default: assert(false); return 0;\n')
    for state_group, variables in variables_by_state_group.items():
        out_file.write(f'            case {state_group}:\n')
        out_file.write('            {\n')
        out_file.write('                size_t res = 17;\n')

        for variable in variables:
            var_type = variable['type']
            var_name_private = get_private_variable_name(variable)
            if is_variable_array(variable):
                out_file.write('                // TODO: array comparison\n')
            else:
                out_file.write(f'                res = res * 31 + std::hash<{var_type}>()({var_name_private});\n')

        out_file.write('                return res;\n')
        out_file.write('            }\n')
    out_file.write('        }\n')
    out_file.write('    }\n\n')

    out_file.write('    '.join(member_variables_section))

    out_file.close()

def generate_full_draw_state_utility_functions(data):
    out_file = create_generated_file('../layers/generated/shader_object_full_draw_state_utility_functions.inl')

    # gather init time array variables

    init_time_array_variables = []
    for variable in data['static_draw_states']:
        if 'init_time_array_length' in variable:
            init_time_array_variables.append(variable)

    for extension in data['extensions']:
        if 'dynamic_states' not in extension:
            continue

        for dynamic_state in extension['dynamic_states']:
            if 'variables' not in dynamic_state:
                continue

            for variable in dynamic_state['variables']:
                if 'init_time_array_length' in variable:
                    init_time_array_variables.append(variable)

    # generate GetSizeInBytes

    out_file.write('    static constexpr void ReserveMemory(AlignedMemory& aligned_memory, Limits const& limits) {\n')

    for var_data in init_time_array_variables:
        var_type = var_data['type']
        length = var_data['init_time_array_length']
        out_file.write(f'        aligned_memory.Add<{var_type}>(limits.{length});\n')

    out_file.write(f'        aligned_memory.Add<FullDrawStateData>();\n')
    out_file.write('    }\n\n')

    # generate SetInternalArrayPointers

    out_file.write('    static void SetInternalArrayPointers(FullDrawStateData* state, Limits const& limits) {\n')
    out_file.write('        // Set array pointers to beginning of their memory\n')
    out_file.write('        char* offset_ptr = (char*)state + sizeof(FullDrawStateData);\n')

    for var_data in init_time_array_variables:
        var_name_private = get_private_variable_name(var_data)
        var_type = var_data['type']
        length = var_data['init_time_array_length']

        out_file.write(f'\n        state->{var_name_private} = ({var_type}*)offset_ptr;\n')
        out_file.write(f'        offset_ptr += sizeof({var_type}) * limits.{length};\n')

    out_file.write('    }\n')

    out_file.close()

def generate_entry_points(data):
    # generate all x macros

    out_file = create_generated_file('../layers/generated/shader_object_entry_points_x_macros.inl')

    def generate_x_macro(name, function_names_list):
        out_file.write(f'#define {name}')

        for function_names in function_names_list:
            canon = function_names[0]
            alias = function_names[1] if len(function_names) > 1 else None
            out_file.write(f'\\\n    ENTRY_POINT({canon})')
            if alias is not None:
                out_file.write(f'\\\n    ENTRY_POINT_ALIAS({alias}, {canon})')

        out_file.write('\n\n')

    generate_x_macro('ENTRY_POINTS_INSTANCE', data['entry_points']['intercept']['instance'])
    generate_x_macro('ENTRY_POINTS_DEVICE', data['entry_points']['intercept']['device'])
    generate_x_macro('ADDITIONAL_INSTANCE_FUNCTIONS', data['entry_points']['forward']['instance'])
    generate_x_macro('ADDITIONAL_DEVICE_FUNCTIONS', data['entry_points']['forward']['device'])

    out_file.close()

# Load data

f = open('shader_object_data.json', 'r')
data = json.load(f)
f.close()

# Generate files

generate_constants_file(data)
generate_device_data_declare_extension_variables(data)
generate_device_data_set_extension_variables(data)
generate_device_data_dynamic_state_adding(data)
generate_create_device_feature_structs(data)
generate_find_intercepted_dynamic_state_function_by_name(data)
generate_full_draw_state_struct_members(data)
generate_full_draw_state_utility_functions(data)
generate_entry_points(data)
