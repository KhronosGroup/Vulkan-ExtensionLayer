#!/usr/bin/env python3
# Copyright (c) 2019-2025 The Khronos Group Inc.
# Copyright (c) 2019-2025 Valve Corporation
# Copyright (c) 2019-2025 LunarG, Inc.
# Copyright (c) 2019 Google Inc.
# Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# Copyright (c) 2023-2023 RasterGrid Kft.
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
#
# Author: Mike Schuchardt <mikes@lunarg.com>

import argparse
import filecmp
import os
import subprocess
import shutil
import sys
import tempfile
import re

from xml.etree import ElementTree


def repo_relative(path):
    ''' helper to define paths relative to the repo root'''
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))


# Points to the directory containing the top level CMakeLists.txt
PROJECT_SRC_DIR = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))
if not os.path.isfile(f'{PROJECT_SRC_DIR}/CMakeLists.txt'):
    print(f'PROJECT_SRC_DIR invalid! {PROJECT_SRC_DIR}')
    sys.exit(1)


def IsGHA():
    '''Returns true if we are running in GitHub actions
    https://docs.github.com/en/actions/learn-github-actions/variables#default-environment-variables'''
    if 'GITHUB_ACTION' in os.environ:
        return True
    return False


def RunShellCmd(command, start_dir=PROJECT_SRC_DIR, env=None, verbose=False):
    '''Runs a command in a directory and returns its return code.
    Directory is project root by default, or a relative path from project root'''

    # Flush stdout here. Helps when debugging on CI.
    sys.stdout.flush()

    if start_dir != PROJECT_SRC_DIR:
        start_dir = repo_relative(start_dir)
    cmd_list = command.split(" ")

    # Helps a lot when debugging CI issues
    if IsGHA():
        verbose = True

    if verbose:
        print(f'CICMD({cmd_list}, env={env})')
    subprocess.check_call(cmd_list, cwd=start_dir, env=env)


GENERATED_FILENAMES = [
    'shader_object_constants.h',
    'shader_object_device_data_declare_extension_variables.inl',
    'shader_object_device_data_set_extension_variables.inl',
    'shader_object_device_data_dynamic_state_adding.inl',
    'shader_object_create_device_feature_structs.inl',
    'shader_object_find_intercepted_dynamic_state_function_by_name.inl',
    'shader_object_full_draw_state_struct_members.inl',
    'shader_object_full_draw_state_struct_members.cpp',
    'shader_object_full_draw_state_utility_functions.inl',
    'shader_object_entry_points_x_macros.inl',
]

# Because we have special logic to import the Registry from input arguments and the BaseGenerator
# comes from the registry, we have to delay defining it until *after* the Registry has been
# imported. Yes this is awkward, but it was the least awkward way to make --verify work.
generators = {}


def RunGenerators(api: str, registry: str, directory: str, styleFile: str, targetFilter: str, flatOutput: bool):

    try:
        RunShellCmd(f'clang-format --version')
        has_clang_format = True
    except:
        has_clang_format = False
    if not has_clang_format:
        print("WARNING: Unable to find clang-format!")

    # These live in the Vulkan-Docs repo, but are pulled in via the
    # Vulkan-Headers/registry folder
    # At runtime we inject python path to find these helper scripts
    scripts = os.path.dirname(registry)
    scripts_directory_path = os.path.dirname(os.path.abspath(__file__))
    registry_headers_path = os.path.join(scripts_directory_path, scripts)
    sys.path.insert(0, registry_headers_path)
    try:
        from reg import Registry
    except:
        # normal python error message
        print("ModuleNotFoundError: No module named 'reg'")
        print(
            f'{registry_headers_path} is not pointing to the Vulkan-Headers registry directory.')
        print("Inside Vulkan-Headers there is a registry/reg.py file that is used.")
        sys.exit(1)  # Return without call stack so easy to spot error

    from base_generator import BaseGeneratorOptions
    from shader_object_generator import ShaderObjectGenerator

    # These set fields that are needed by both OutputGenerator and BaseGenerator,
    # but are uniform and don't need to be set at a per-generated file level
    from base_generator import (SetTargetApiName, SetMergedApiNames)
    SetTargetApiName(api)

    for filename in GENERATED_FILENAMES:
        generators.update({filename: {'generator': ShaderObjectGenerator, 'genCombined': False, 'directory': 'layers/shader_object/generated/'}})

    unknownTargets = [x for x in (
        targetFilter if targetFilter else []) if x not in generators.keys()]
    if unknownTargets:
        print(
            f'ERROR: No generator options for unknown target(s): {", ".join(unknownTargets)}', file=sys.stderr)
        return 1

    # Filter if --target is passed in
    targets = [x for x in generators.keys(
    ) if not targetFilter or x in targetFilter]

    for index, target in enumerate(targets, start=1):
        print(f'[{index}|{len(targets)}] Generating {target}')

        # First grab a class constructor object and create an instance
        generator = generators[target]['generator']
        gen = generator()

        # This code and the 'genCombined' generator metadata is used by downstream
        # users to generate code with all Vulkan APIs merged into the target API variant
        # (e.g. Vulkan SC) when needed. The constructed apiList is also used to filter
        # out non-applicable extensions later below.
        apiList = [api]
        if api != 'vulkan' and generators[target]['genCombined']:
            SetMergedApiNames('vulkan')
            apiList.append('vulkan')
        else:
            SetMergedApiNames(None)

        # For people who want to generate all the files in a single director
        if flatOutput:
            outDirectory = os.path.abspath(os.path.join(directory))
        else:
            outDirectory = os.path.abspath(os.path.join(
                directory, generators[target]['directory']))

        options = BaseGeneratorOptions(
            customFileName=target,
            customDirectory=outDirectory)

        if not os.path.exists(outDirectory):
            os.makedirs(outDirectory)

        # Create the registry object with the specified generator and generator
        # options. The options are set before XML loading as they may affect it.
        reg = Registry(gen, options)

        # Parse the specified registry XML into an ElementTree object
        tree = ElementTree.parse(registry)

        # Load the XML tree into the registry object
        reg.loadElementTree(tree)

        # Finally, use the output generator to create the requested target
        reg.apiGen()

        # Run clang-format on the file
        if has_clang_format and styleFile:
            RunShellCmd(f'clang-format -i --style=file:{styleFile} {os.path.join(outDirectory, target)}')


def main(argv):

    # files to exclude from --verify check
    verify_exclude = []  # None currently

    parser = argparse.ArgumentParser(
        description='Generate source code for this repository')
    parser.add_argument('registry', metavar='REGISTRY_PATH',
                        help='path to the Vulkan-Headers registry directory')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('--generated-version',
                        help='sets the header version used to generate the repo')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--target', nargs='+',
                       help='only generate file names passed in')
    group.add_argument('-i', '--incremental', action='store_true',
                       help='only update repo files that change')
    group.add_argument('-v', '--verify', action='store_true',
                       help='verify repo files match generator output')
    group.add_argument('-o', action='store', dest='directory',
                       help='Create target and related files in specified directory')
    args = parser.parse_args(argv)

    repo_dir = repo_relative('.')

    registry = os.path.abspath(os.path.join(args.registry,  'vk.xml'))
    if not os.path.isfile(registry):
        registry = os.path.abspath(os.path.join(
            args.registry, 'Vulkan-Headers/registry/vk.xml'))
        if not os.path.isfile(registry):
            print(f'cannot find vk.xml in {args.registry}')
            return -1

    # Need pass style file in case running with --verify and it can't find the file automatically in the temp directory
    style_file = os.path.join(repo_dir, '.clang-format')

    # get directory where generators will run
    if args.verify or args.incremental:
        # generate in temp directory so we can compare or copy later
        temp_obj = tempfile.TemporaryDirectory(
            prefix='extension_layer_codegen_')
        temp_dir = temp_obj.name
        gen_dir = temp_dir
    elif args.directory:
        gen_dir = args.directory
    else:
        # generate directly in the repo
        gen_dir = repo_dir

    RunGenerators(api=args.api, registry=registry, directory=gen_dir,
                  styleFile=style_file, targetFilter=args.target, flatOutput=False)

    # optional post-generation steps
    if args.verify:

        # compare contents of temp dir and repo
        temp_files = {}
        repo_files = {}
        for details in generators.values():
            if details['directory'] not in temp_files:
                temp_files[details['directory']] = set()
            temp_files[details['directory']].update(
                set(os.listdir(os.path.join(temp_dir, details['directory']))))
            if details['directory'] not in repo_files:
                repo_files[details['directory']] = set()
            repo_files[details['directory']].update(set(os.listdir(
                os.path.join(repo_dir, details['directory']))) - set(verify_exclude))

        # compare contents of temp dir and repo
        files_match = True
        for filename, details in generators.items():
            if filename not in repo_files[details['directory']]:
                print('ERROR: Missing repo file', filename)
                files_match = False
            elif filename not in temp_files[details['directory']]:
                print('ERROR: Missing generator for', filename)
                files_match = False
            elif not filecmp.cmp(os.path.join(temp_dir, details['directory'], filename),
                                 os.path.join(
                                     repo_dir, details['directory'], filename),
                                 shallow=False):
                print('ERROR: Repo files do not match generator output for', filename)
                files_match = False

        # return code for test scripts
        if files_match:
            print('SUCCESS: Repo files match generator output')
            return 0
        return 1

    elif args.incremental:
        # copy missing or differing files from temp directory to repo
        for filename, details in generators.items():
            temp_filename = os.path.join(
                temp_dir, details['directory'], filename)
            repo_filename = os.path.join(
                repo_dir, details['directory'], filename)
            if not os.path.exists(repo_filename) or \
                    not filecmp.cmp(temp_filename, repo_filename, shallow=False):
                print('update', repo_filename)
                shutil.copyfile(temp_filename, repo_filename)

    # write out the header version used to generate the code to a checked in CMake file
    if args.generated_version:
        # Update the CMake project version
        with open(repo_relative('CMakeLists.txt'), "r+") as f:
            data = f.read()
            f.seek(0)
            f.write(re.sub("project.*VERSION.*",
                    f"project(VEL VERSION {args.generated_version} LANGUAGES CXX)", data))
            f.truncate()

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
