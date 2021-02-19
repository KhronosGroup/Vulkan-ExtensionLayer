<!-- markdownlint-disable MD041 -->
<p align="left"><img src="https://vulkan.lunarg.com/img/NewLunarGLogoBlack.png" alt="LunarG" width=263 height=113 /></p>

Copyright &copy; 2015-2021 LunarG, Inc.

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/


# VK\_LAYER\_KHRONOS\_synchronization2
The `VK_LAYER_KHRONOS_synchronization2` extension layer implements the `VK_KHR_synchronization2` extension.
By default, it will disable itself if the underlying driver provides the extension.
It has a force\_enable setting that can be adjusted by either environment variables
or by using the `vk_layer_settings.txt` file.

<br></br>


## Enabling the Layer

### Vkconfig

The synchronization2 layer can be enabled using vkconfig. See the [vkconfig](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) documentation for more information.

### Desktop (Linux/Windows/MacOS)

You must add the location of the generated `VK_LAYER_KHRONOS_synchronization2.json` file and corresponding
`VkLayer_synchronization2` library to your `VK_LAYER_PATH` in order for the Vulkan loader to be able
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

    set VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_synchronization2

**Linux/MacOS**

    export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_synchronization2

<br></br>

### Android

#### Permissions

This layer should not require your application to need any additional permissions.

#### Globally Enabling the Layer

Use ADB to enable the layer for your project by:

    adb shell "setprop debug.vulkan.layers 'VK_LAYER_KHRONOS_synchronization2'"

When done, disable the layer using:

    adb shell "setprop debug.vulkan.layers ''"

<br></br>


## Synchronization2 Layer Options

Setting  | Environment Variable | Settings File Value | Default | Description
-------- | -------------------- | ------------------- | ------- | -----------
Force enable | `VK_SYNC2_FORCE_ENABLE` |`khronos_synchronization2.force_enable` | FALSE | If TRUE, the layer's implementation of the extension will be used even if the underlying driver also implements the extension.

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

To set force enable, which on desktop uses `VK_SYNC2_FORCE_ENABLE`
set the following property:

    debug.sync2_force_enable

Which you can set in the following way:

    adb shell "setprop debug.sync2_force_enable true"

<br></br>

