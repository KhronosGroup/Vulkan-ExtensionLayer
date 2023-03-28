# Vulkan-ExtensionLayer

Layers providing Vulkan features when native support is unavailable

## Default branch changed to 'main' 2023-01-16

As discussed in #158, the default branch of this repository is now 'main'. This change should be largely transparent to repository users, since github rewrites many references to the old 'master' branch to 'main'. However, if you have a checked-out local clone, you may wish to take the following steps as recommended by github:

```sh
git branch -m master main
git fetch origin
git branch -u origin/main main
git remote set-head origin -a
```

## CI Build Status
| Build Status |
|:------------|
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions/workflows/build_windows.yml/badge.svg?branch=main)](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions/workflows/build_linux.yml/badge.svg?branch=main)](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions/workflows/build_macos.yml/badge.svg?branch=main)](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions/workflows/build_android.yml/badge.svg?branch=main)](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/actions) |

## Introduction

There are some extensions and features in Vulkan that are not available everywhere due to various reasons. While not available, some extensions are capable of being done as layer and mapping to any Vulkan implementation regardless of it supporting the desired functionality or not.

## Currently implemented extensions

| Layer                               | Extension provided        | Version | File                        | Status   |
|:-----------------------------------:|:-------------------------:|:-------:|:---------------------------:|:--------:|
| VK_LAYER_KHRONOS_timeline_semaphore | VK_KHR_timeline_semaphore | 1       | layers/timeline_semaphore.c | complete |
| [VK_LAYER_KHRONOS_synchronization2](docs/synchronization2_layer.md)   | VK_KHR_synchronization2   | 1       | layers/synchronization2.cpp | complete |
| VK_LAYER_KHRONOS_shader_object      | VK_EXT_shader_object      | 1       | layers/shader_object.cpp    | complete |

## Information for Developing or Contributing:

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file in this repository for more details.

## How to Build and Run

[BUILD.md](BUILD.md)
Includes directions for building all components and testing them.

## License
This work is released as open source under a Apache-style license from Khronos including a Khronos copyright.

See [LICENSE](LICENSE) for a full list of licenses used in this repository.
