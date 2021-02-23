# Build Instructions

1. [Repository Set-Up](#repository-set-up)

## Repository Set-Up

### Display Drivers

This repository does not contain a Vulkan-capable driver. You will need to
obtain and install a Vulkan driver from your graphics hardware vendor or from
some other suitable source if you intend to run Vulkan applications.

### Download the Repository

To create your local git repository:

    git clone https://github.com/KhronosGroup/Vulkan-ExtensionLayer.git

### Repository Dependencies

This repository attempts to resolve some of its dependencies by using
components found from the following places, in this order:

1. CMake or Environment variable overrides (e.g., -DVULKAN_HEADERS_INSTALL_DIR)
1. LunarG Vulkan SDK, located by the `VULKAN_SDK` environment variable
1. System-installed packages, mostly applicable on Linux

#### Vulkan-Headers

This repository has a required dependency on the
[Vulkan Headers repository](https://github.com/KhronosGroup/Vulkan-Headers).
You must clone the headers repository and build its `install` target before
building this repository. The Vulkan-Headers repository is required because it
contains the Vulkan API definition files (registry) that are required to build
the extension layers. You must also take note of the headers' install
directory and pass it on the CMake command line for building this repository,
as described below.

#### Google Test

The extension layer tests depend on the
[Google Test](https://github.com/google/googletest)
framework and do not build unless this framework is downloaded into the
repository's `external` directory.

To obtain the framework, change your current directory to the top of your
Vulkan-ExtensionLayer repository and run:

    git clone https://github.com/google/googletest.git external/googletest
    cd external/googletest
    git checkout tags/release-1.10.0

before configuring your build with CMake.

If you do not need the tests, there is no need to download this
framework.

#### glslang and SPIRV-Headers

The extension layer tests depend on glslang and SPIRV-Headers to build
so these components are only needed if the tests are built and run.

These components can be used from an installed LunarG SDK or an installed Linux package.

If these components are not available from any of these methods and/or it is important
to use the latest code, then you must build
[glslang repository](https://github.com/KhronosGroup/glslang.git)
and
[SPIRV-Headers repository](https://github.com/KhronosGroup/SPIRV-Headers.git)
with their install targets. Take note of the install directory locations and pass
them on the CMake command line by adding the following definitions:

```
-DGLSLANG_INSTALL_DIR=absolute_path_to_install_dir
-DSPIRV_HEADERS_INSTALL_DIR=absolute_path_to_install_dir
```

If you do not intend to run the tests, you do not need these repositories.

### Building Dependent Repositories with Known-Good Revisions

There is a Python utility script, `scripts/update_deps.py`, that you can use to
gather and build the dependent repositories mentioned above. This script uses
information stored in the `scripts/known_good.json` file to check out dependent
repository revisions that are known to be compatible with the revision of this
repository that you currently have checked out. As such, this script is useful
as a quick-start tool for common use cases and default configurations.

For all platforms, start with:

    git clone git@github.com:KhronosGroup/Vulkan-ExtensionLayer.git
    cd Vulkan-ExtensionLayer
    mkdir build
    cd build

For 64-bit Linux and MacOS, continue with:

    ../scripts/update_deps.py
    cmake -C helper.cmake ..
    cmake --build .

For 64-bit Windows, continue with:

    ..\scripts\update_deps.py --arch x64
    cmake -A x64 -C helper.cmake ..
    cmake --build .

For 32-bit Windows, continue with:

    ..\scripts\update_deps.py --arch Win32
    cmake -A Win32 -C helper.cmake ..
    cmake --build .

For Android, continue with:

    cd build-android
    ./build_all.sh

Please see the more detailed build information later in this file if you have
specific requirements for configuring and building these components.

#### Notes

- You may need to adjust some of the CMake options based on your platform. See
  the platform-specific sections later in this document.
- When using update_deps.py to change architectures (x64, Win32...)
  or build configurations (debug, release...) it is strongly recommended to
  add the `--clean-repo` parameter. This ensures compatibility among dependent
  components.
  dependent components will produce consistent build artifacts.
- The `update_deps.py` script fetches and builds the dependent repositories in
  the current directory when it is invoked. In this case, they are built in
  the `build` directory.
- The `build` directory is also being used to build this
  (Vulkan-ExtensionLayer) repository. But there shouldn't be any conflicts
  inside the `build` directory between the dependent repositories and the
  build files for this repository.
- The `--dir` option for `update_deps.py` can be used to relocate the
  dependent repositories to another arbitrary directory using an absolute or
  relative path.
- The `update_deps.py` script generates a file named `helper.cmake` and places
  it in the same directory as the dependent repositories (`build` in this
  case). This file contains CMake commands to set the CMake `*_INSTALL_DIR`
  variables that are used to point to the install artifacts of the dependent
  repositories. You can use this file with the `cmake -C` option to set these
  variables when you generate your build files with CMake. This lets you avoid
  entering several `*_INSTALL_DIR` variable settings on the CMake command line.
- If using "MINGW" (Git For Windows), you may wish to run
  `winpty update_deps.py` in order to avoid buffering all of the script's
  "print" output until the end and to retain the ability to interrupt script
  execution.
- Please use `update_deps.py --help` to list additional options and read the
  internal documentation in `update_deps.py` for further information.

# Testing instructions

Assuming Google Test instructions above were followed, the build should output a
`vk_extension_layer_tests` executable (or APK on Android) to run. The tests should
enable the layer explicitly, so make sure the path if needed.

For 64-bit Linux and MacOS, example of testing Sync2 tests:
```bash
    # In build directory
    ./tests/vk_extension_layer_tests --gtest_filter=*Sync2Test*
```

For Android, example of testing Sync2 tests:
```bash
    # in build-android directory

    # Uninstall if old version on device
    adb uninstall com.example.VulkanExtensionLayerTests
    # Install new version
    adb install -r -g bin/VulkanExtensionLayerTests.apk

    # To run test
    adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.VulkanExtensionLayerTests/android.app.NativeActivity --es args --gtest_filter="*Sync2*"

    # To log info
    adb logcat -c && adb logcat *:S VulkanExtensionLayerTests
    # To see dumpped info
    adb shell cat /sdcard/Android/data/com.example.VulkanExtensionLayerTests/files/out.txt
    adb shell cat /sdcard/Android/data/com.example.VulkanExtensionLayerTests/files/err.txt
```