<!-- markdownlint-disable MD041 -->
<p align="left"><img src="https://vulkan.lunarg.com/img/NewLunarGLogoBlack.png" alt="LunarG" width=263 height=113 /></p>

Copyright &copy; 2015-2024 LunarG, Inc.

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/


# VK\_LAYER\_KHRONOS\_shader_object
The `Shader object` extension Layer implements the `VK_EXT_shader_object` extension.
By default, it will disable itself if the underlying driver provides the extension.

The `VK_EXT_shader_object` extension requires the `VK_KHR_maintenance2` and `VK_KHR_dynamic_rendering` extensions, this layer will not work on devices that do not implement them.

Because the `VK_KHR_dynamic_rendering` extension is not supported on most Android devices, this layer is not implemented for Android.

## Configuring the Shader object layer

For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

The Shader object Layer settings are documented in the [Layer Details](https://vulkan.lunarg.com/doc/sdk/latest/windows/shader_object_layer.html#user-content-layer-details) section below.

The Shader object Layer can also be enabled and configured using vkconfig. See the [vkconfig](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) documentation for more information.


## Enabling the Layer

### Desktop (Linux/Windows/MacOS)

You must add the location of the generated `VK_LAYER_KHRONOS_shader_object.json` file and corresponding
`VkLayer_shader_object` library to your `VK_LAYER_PATH` in order for the Vulkan loader to be able
to find the layer.

Then, you must also enable the layer in one of two ways:

 * Directly in your application using the layer's name during vkCreateInstance
 * Indirectly by using the `VK_INSTANCE_LAYERS` environment variable.

#### Setting the `VK_LAYER_PATH`

**Windows**

If your source was located in: `C:\my_folder\ExtensionLayer` and your build folder was build64, then you would add it to the layer path in the following way:

    set VK_LAYER_PATH=C:\my_folder\ExtensionLayer\build64\layers\Debug;%VK_LAYER_PATH%

**Linux/MacOS**

If your source was located in: `/my_folder/ExtensionLayer` and your build folder was build, then you would add it to the layer path in the following way:

    export VK_LAYER_PATH=/my_folder/ExtensionLayer/build/layers:$VK_LAYER_PATH

Forcing the layer with `VK_INSTANCE_LAYERS`

To force the layer to be enabled for Vulkan applications, you can set the `VK_INSTANCE_LAYERS` environment variable in the following way:

**Windows**

    set VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_shader_object

**Linux/MacOS**

    export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_shader_object

To force the layer to be enabled for Vulkan applications, even though the Vulkan implementation supports `VK_EXT_shader_object` extension, you can set the `VK_SHADER_OBJECT_FORCE_ENABLE` environment variable in the following way:

**Windows**

    set VK_SHADER_OBJECT_FORCE_ENABLE=true

**Linux/MacOS**

    export VK_SHADER_OBJECT_FORCE_ENABLE=true

To disable pipeline pre-caching to reduce memory overhead, you can set the `VK_SHADER_OBJECT_DISABLE_PIPELINE_PRE_CACHING` environment variable in the following way:

**Windows**

    set VK_SHADER_OBJECT_DISABLE_PIPELINE_PRE_CACHING=true

**Linux/MacOS**

    export VK_SHADER_OBJECT_DISABLE_PIPELINE_PRE_CACHING=true

<br>

### Settings Priority

If you have a setting defined in both the Settings File as well as an Environment
Variable, the Environment Variable value will **always** override the Settings File
value.
This is intended to let you dynamically change settings without having to adjust
the Settings File.

<br>


## Layer Options

The options for this layer are specified in VkLayer_khronos_shader_object.json. The details of the layer options are documented in the [Shader object layer documentation](https://vulkan.lunarg.com/doc/sdk/latest/windows/shader_object_layer.html).
