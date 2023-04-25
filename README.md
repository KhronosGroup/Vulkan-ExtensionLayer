# Vulkan-ExtensionLayer

## Introduction

There are some extensions and features in Vulkan that are not available everywhere due to various reasons. While not available, some extensions are capable of being done as layer and mapping to any Vulkan implementation regardless of it supporting the desired functionality or not.

## Currently implemented extensions

| Layer                               | Extension provided        | Version | File(s)                     | Point of Contact|
|:-----------------------------------:|:-------------------------:|:-------:|:---------------------------:|:--------:|
| VK_LAYER_KHRONOS_timeline_semaphore | VK_KHR_timeline_semaphore | 1       | layers/timeline_semaphore.c | [@llandwerlin-intel](https://github.com/llandwerlin-intel) |
| [VK_LAYER_KHRONOS_synchronization2](docs/synchronization2_layer.md)   | VK_KHR_synchronization2   | 1       | layers/synchronization2.cpp | [@jeremyg-lunarg](https://github.com/jeremyg-lunarg) |
| [VK_LAYER_KHRONOS_shader_object](docs/shader_object_layer.md)      | VK_EXT_shader_object| 1       | layers/shader_object.cpp    | [@daniel-story](https://github.com/daniel-story) |

If you find a problem with one of the layers, please file an Issue and tag the Point of Contact listed in the table above.

NOTE: Not all layers are supported on every platform, please read the documentation for the layer for details.

## Information for Developing or Contributing:

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file in this repository for more details.

## How to Build and Run

[BUILD.md](BUILD.md)
Includes directions for building all components and testing them.

## License
This work is released as open source under a Apache-style license from Khronos including a Khronos copyright.

See [LICENSE](LICENSE) for a full list of licenses used in this repository.
