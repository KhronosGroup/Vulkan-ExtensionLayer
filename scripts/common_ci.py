#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017, 2019-2022 The Khronos Group Inc.
# Copyright (c) 2015-2017, 2019-2023 Valve Corporation
# Copyright (c) 2015-2017, 2019-2023 LunarG, Inc.
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
# Author: Mark Lobodzinski <mark@lunarg.com>
# Author: Jeremy Gebben <jeremyg@lunarg.com>

import os
import sys
import subprocess
import platform
import shutil
import argparse

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))

# TODO: Pass this in as arg, may be useful for running locally
EXTERNAL_DIR_NAME = "external"
BUILD_DIR_NAME = "build"
EXTERNAL_DIR = RepoRelative(EXTERNAL_DIR_NAME)
VEL_BUILD_DIR = RepoRelative(BUILD_DIR_NAME)
CONFIGURATIONS = ['release', 'debug']
DEFAULT_CONFIGURATION = CONFIGURATIONS[0]

# Runs a command in a directory and returns its return code.
# Directory is project root by default, or a relative path from project root
def RunShellCmd(command, start_dir = PROJECT_ROOT, env=None, verbose=False):
    if start_dir != PROJECT_ROOT:
        start_dir = RepoRelative(start_dir)
    cmd_list = command.split(" ")
    if verbose or ('VEL_CI_VERBOSE' in os.environ and os.environ['VEL_CI_VERBOSE'] != '0'):
        print(f'CICMD({cmd_list}, env={env})')
    subprocess.check_call(cmd_list, cwd=start_dir, env=env)

#
# Check if the system is Windows
def IsWindows(): return 'windows' == platform.system().lower()

#
# Verify consistency of generated source code
def CheckVELCodegenConsistency():
    print("Check Generated Source Code Consistency")
    gen_check_cmd = f'python scripts/generate_source.py --verify {EXTERNAL_DIR}/Vulkan-Headers/registry'
    RunShellCmd(gen_check_cmd)

def BuildVEL(args):
    print("Log CMake version")
    cmake_ver_cmd = 'cmake --version'
    RunShellCmd(cmake_ver_cmd)

    print("Run CMake for Extension Layer")
    cmake_cmd = f'cmake -S . -B {VEL_BUILD_DIR} -DUPDATE_DEPS_DIR={EXTERNAL_DIR} -DUPDATE_DEPS=ON -DCMAKE_BUILD_TYPE={args.configuration.capitalize()}'
    # By default BUILD_WERROR is OFF, CI should always enable it.
    cmake_cmd = cmake_cmd + ' -DBUILD_WERROR=ON'
    cmake_cmd = cmake_cmd + ' -DBUILD_TESTS=ON'

    if args.cmake:
        cmake_cmd += f' {args.cmake}'

    RunShellCmd(cmake_cmd)

    print("Build Extension Layer and Tests")
    build_cmd = f'cmake --build {VEL_BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Extension Layers")
    install_cmd = f'cmake --install {VEL_BUILD_DIR} --prefix {VEL_BUILD_DIR}/install/'
    RunShellCmd(install_cmd)

def GetArgParser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-c', '--config', dest='configuration',
        metavar='CONFIG', action='store',
        choices=CONFIGURATIONS, default=DEFAULT_CONFIGURATION,
        help='Build target configuration. Can be one of: {0}'.format(
            ', '.join(CONFIGURATIONS)))
    parser.add_argument(
        '--cmake', dest='cmake',
        metavar='CMAKE', type=str,
        default='', help='Additional args to pass to cmake')
    return parser
