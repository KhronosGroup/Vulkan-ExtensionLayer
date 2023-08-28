# Running Test on Android

```bash
cd build-android

# Optional
adb uninstall com.example.VulkanExtensionLayerTests

adb install -r -g --no-incremental bin/VulkanExtensionLayerTests.apk

adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.VulkanExtensionLayerTests/android.app.NativeActivity --es args --gtest_filter="*Decompression*"

adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.VulkanExtensionLayerTests/android.app.NativeActivity --es args --gtest_filter="*Sync2Test*"

adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.VulkanExtensionLayerTests/android.app.NativeActivity --es args --gtest_filter="*ShaderObject*"
```

To view to logging while running in a separate terminal run

```bash
adb logcat -c && adb logcat *:S VulkanExtensionLayerTests
```

Or the files can be pulled off the device with

```bash
adb shell cat /sdcard/Android/data/com.example.VulkanExtensionLayerTests/files/out.txt
adb shell cat /sdcard/Android/data/com.example.VulkanExtensionLayerTests/files/err.txt
```