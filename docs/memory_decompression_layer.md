<!-- markdownlint-disable MD041 -->

Copyright &copy; 2023 The Khronos Group Inc.

Copyright &copy; 2023 Nvidia Corporation.

# VK\_LAYER\_KHRONOS\_memory_decompression
The `VK_LAYER_KHRONOS_memory_decompression` extension layer implements the `VK_NV_memory_decompression` extension.
By default, it will disable itself if the underlying driver provides the extension.

## Requirements for the layer

VK_LAYER_KHRONOS_memory_decompression requires an implementation to support the following:

* VkPhysicalDeviceSubgroupProperties::supportedStages must include VK_SHADER_STAGE_COMPUTE_BIT

* VkPhysicalDeviceSubgroupProperties::supportedOperations must include VK_SUBGROUP_FEATURE_BASIC_BIT

* VkPhysicalDeviceVulkan12Features::shaderInt8 feature must be supported and enabled

* VkPhysicalDeviceVulkan12Features::bufferDeviceAddress feature must be supported and enabled


## Configuring the memory_decompression Layer

For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

The memory_decompression Layer settings are documented below.

The memory_decompression Layer can also be enabled and configured using vkconfig. See the [vkconfig](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) documentation for more information.


## Enabling the Layer

### Desktop (Linux/Windows/MacOS)

You must add the location of the generated `VK_LAYER_KHRONOS_memory_decompression.json` file and corresponding
`VK_LAYER_KHRONOS_memory_decompression` library to your `VK_LAYER_PATH` in order for the Vulkan loader to be able
to find the layer.

Then, you must also enable the layer in one of two ways:

 * Directly in your application using the layer's name during vkCreateInstance
 * Indirectly by using the `VK_INSTANCE_LAYERS` environment variable.

#### Setting the `VK_LAYER_PATH`

**Windows**

If your source was located in: `C:\my_folder\ExtensionLayer` and your build folder was `build64`, then you would add it to the layer path in the following way:

    set VK_LAYER_PATH=C:\my_folder\ExtensionLayer\build64\layers\Debug;%VK_LAYER_PATH%

**Linux/MacOS**

If your source was located in: `/my_folder/ExtensionLayer` and your build folder was `build`, then you would add it to the layer path in the following way:

    export VK_LAYER_PATH=/my_folder/ExtensionLayer/build/layers:$VK_LAYER_PATH

Forcing the layer with `VK_INSTANCE_LAYERS`

To force the layer to be enabled for Vulkan applications, you can set the `VK_INSTANCE_LAYERS` environment variable in the following way:

**Windows**

    set VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_memory_decompression

**Linux/MacOS**

    export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_memory_decompression

To force the layer to be enabled for Vulkan applications, even though the Vulkan implementation supports `VK_NV_memory_decompression` extension, you can
set the `VK_MEMORY_DECOMPRESSION_FORCE_ENABLE` environment variable in the following way:

**Windows**

    set VK_MEMORY_DECOMPRESSION_FORCE_ENABLE=true

**Linux/MacOS**

    export VK_MEMORY_DECOMPRESSION_FORCE_ENABLE=true

<br></br>

### Android

#### Packaging layer with application

This layer should not require your application to need any additional permissions.

At `vkCreateInstance` time add the layer

```c++
// std::vector<const char *> instance_layers
instance_layers.push_back("VK_LAYER_KHRONOS_memory_decompression");

VkInstanceCreateInfo info;
info.enabledLayerCount = static_cast<uint32_t>(instance_layers_.size());
info.ppEnabledLayerNames = instance_layers_.data();
```

For this to work `libVkLayer_khronos_memory_decompression.so` needs to be packaged inside the APK as Android will be able to read in the layer from there. One can open the APK in Android Studio to verify the binary is there. Make sure to match up the correct ABI as well (`armeabi-v7a`, `arm64-v8a`, etc).

#### Globally Enabling the Layer

This will require a debugable application or additional permissions. For more information please read the [Android Developer page](https://developer.android.com/ndk/guides/graphics/validation-layer#enable-layers-outside-app)

Use ADB to enable the layer for your project by:

    adb shell "setprop debug.vulkan.layers 'VK_LAYER_KHRONOS_memory_decompression'"

When done, disable the layer using:

    adb shell "setprop debug.vulkan.layers ''"

<br></br>

### Settings Priority

If you have a setting defined in both the Settings File as well as an Environment
Variable, the Environment Variable value will **always** override the Settings File
value.
This is intended to let you dynamically change settings without having to adjust
the Settings File.

<br></br>


### Applying Environment Settings on Android

On Android, you must use properties to set the environment variables.
The format of the properties to set takes the following form:

    debug. + (lower-case environment variable with 'vk_' stripped)

The easiest way to set a property is from the ADB shell:

    adb shell "setprop <property_name> '<property_value>'"

**For example:**

To set force enable, which on desktop uses `VK_MEMORY_DECOMPRESSION_FORCE_ENABLE`
set the following property:

    debug.vulkan.decompression

Which you can set in the following way:

    adb shell "setprop debug.vulkan.decompression true"

<br></br>

## Layer Options

The options for this layer are specified in VkLayer_khronos_memory_decompression.json.

