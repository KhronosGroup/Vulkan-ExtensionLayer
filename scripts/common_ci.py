#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017, 2019-2022 The Khronos Group Inc.
# Copyright (c) 2015-2017, 2019-2022 Valve Corporation
# Copyright (c) 2015-2017, 2019-2022 LunarG, Inc.
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

import utils.utils as utils

if sys.version_info[0] != 3:
    print("This script requires Python 3. Run script with [-h] option for more details.")
    sys_exit(0)

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
ARCHS = [ 'x64', 'Win32' ]
DEFAULT_ARCH = ARCHS[0]

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
    gen_check_cmd = 'python3 scripts/generate_source.py --verify %s/Vulkan-Headers/registry' % EXTERNAL_DIR
    RunShellCmd(gen_check_cmd)

#
def BuildVEL(args):

    print("Log CMake version")
    cmake_ver_cmd = 'cmake --version'
    RunShellCmd(cmake_ver_cmd)

    GTEST_DIR = RepoRelative("external/googletest")
    if not os.path.exists(GTEST_DIR):
        print("Clone Testing Framework Source Code")
        clone_gtest_cmd = f'git clone https://github.com/google/googletest.git {GTEST_DIR}'
        RunShellCmd(clone_gtest_cmd)

        print("Get Specified Testing Source")
        gtest_checkout_cmd = 'git checkout tags/release-1.10.0'
        RunShellCmd(gtest_checkout_cmd, GTEST_DIR)

    utils.make_dirs(VEL_BUILD_DIR)
    print("Run CMake for Extension Layer")
    cmake_cmd = f'cmake -DUPDATE_DEPS=ON -DCMAKE_BUILD_TYPE={args.configuration.capitalize()} {args.cmake} ..'
    # By default BUILD_WERROR is OFF, CI should always enable it.
    cmake_cmd = cmake_cmd + ' -DBUILD_WERROR=ON'
    if IsWindows(): cmake_cmd = cmake_cmd + f' -A {args.arch}'
    RunShellCmd(cmake_cmd, VEL_BUILD_DIR)

    print("Build Extension Layer and Tests")
    build_cmd = f'cmake --build . --config {args.configuration}'
    if not IsWindows(): build_cmd = build_cmd + f' -- -j{os.cpu_count()}'
    RunShellCmd(build_cmd, VEL_BUILD_DIR)

def GetArgParser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-c', '--config', dest='configuration',
        metavar='CONFIG', action='store',
        choices=CONFIGURATIONS, default=DEFAULT_CONFIGURATION,
        help='Build target configuration. Can be one of: {0}'.format(
            ', '.join(CONFIGURATIONS)))
    parser.add_argument(
        '-a', '--arch', dest='arch',
        metavar='ARCH', action='store',
        choices=ARCHS, default=DEFAULT_ARCH,
        help=f'Target architecture. Can be one of: {ARCHS}')
    parser.add_argument(
        '--cmake', dest='cmake',
        metavar='CMAKE', type=str,
        default='', help='Additional args to pass to cmake')
    return parser
