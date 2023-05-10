# Copyright 2015, 2023 The Android Open Source Project
# Copyright (C) 2015, 2023 Valve Corporation

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
SRC_DIR := ../..
LAYER_DIR := ../generated
THIRD_PARTY := ../third_party

VULKAN_INCLUDE := $(LOCAL_PATH)/$(THIRD_PARTY)/Vulkan-Headers/include

include $(CLEAR_VARS)
LOCAL_MODULE := extlayer_utils

LOCAL_SRC_FILES += $(SRC_DIR)/utils/allocator.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/utils/vk_format_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/utils/vk_layer_config.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/utils/generated/vk_safe_struct.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/utils/generated/lvt_function_pointers.cpp

LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils/generated \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -fexceptions
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DVK_PROTOTYPES -fvisibility=hidden
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkLayer_khronos_synchronization2
LOCAL_SRC_FILES += $(SRC_DIR)/layers/synchronization2.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils/generated \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/shaderc/third_party/spirv-tools/external/spirv-headers/include
LOCAL_STATIC_LIBRARIES += extlayer_utils glslang SPIRV-Tools SPIRV-Tools-opt
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -Wno-cast-calling-convention -fexceptions
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DVK_PROTOTYPES -fvisibility=hidden
LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkLayer_khronos_memory_decompression
LOCAL_SRC_FILES += $(SRC_DIR)/layers/decompression/decompression.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils/generated \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/shaderc/third_party/spirv-tools/external/spirv-headers/include
LOCAL_STATIC_LIBRARIES += extlayer_utils glslang SPIRV-Tools SPIRV-Tools-opt
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -Wno-cast-calling-convention -fexceptions
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DVK_PROTOTYPES -fvisibility=hidden
LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkLayer_khronos_timeline_semaphore
LOCAL_SRC_FILES += $(SRC_DIR)/layers/timeline_semaphore.c \
                   $(SRC_DIR)/layers/hash_table.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE)
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -Wno-cast-calling-convention -fexceptions
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DVK_PROTOTYPES -fvisibility=hidden
LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkExtensionLayerTests
LOCAL_SRC_FILES += $(SRC_DIR)/tests/extension_layer_tests.cpp \
                   $(SRC_DIR)/tests/vktestbinding.cpp \
                   $(SRC_DIR)/tests/synchronization2_tests.cpp \
                   $(SRC_DIR)/tests/decompression_tests.cpp \
                   $(SRC_DIR)/tests/vkrenderframework.cpp \
                   $(SRC_DIR)/tests/vktestframeworkandroid.cpp \
                   $(SRC_DIR)/tests/test_environment.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils/generated \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils \
                    $(LOCAL_PATH)/$(SRC_DIR)/libs

LOCAL_STATIC_LIBRARIES := googletest_main extlayer_utils shaderc
LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -Wall -Werror -Wno-unused-function -Wno-unused-const-variable
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DNV_EXTENSIONS -DAMD_EXTENSIONS -fvisibility=hidden
LOCAL_LDLIBS := -llog -landroid -ldl
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_EXECUTABLE)

# Note: The following module is similar in name to the executable, but differs so that loader won't enumerate the resulting .so
include $(CLEAR_VARS)
LOCAL_MODULE := VulkanExtensionLayerTests
LOCAL_SRC_FILES += $(SRC_DIR)/tests/extension_layer_tests.cpp \
                   $(SRC_DIR)/tests/vktestbinding.cpp \
                   $(SRC_DIR)/tests/synchronization2_tests.cpp \
                   $(SRC_DIR)/tests/decompression_tests.cpp \
                   $(SRC_DIR)/tests/vkrenderframework.cpp \
                   $(SRC_DIR)/tests/vktestframeworkandroid.cpp \
                   $(SRC_DIR)/tests/test_environment.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils/generated \
                    $(LOCAL_PATH)/$(SRC_DIR)/utils \
                    $(LOCAL_PATH)/$(SRC_DIR)/libs

LOCAL_STATIC_LIBRARIES := googletest_main extlayer_utils shaderc
LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -Wall -Werror -Wno-unused-function -Wno-unused-const-variable
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DNV_EXTENSIONS -DAMD_EXTENSIONS -fvisibility=hidden -DVALIDATION_APK
LOCAL_WHOLE_STATIC_LIBRARIES += android_native_app_glue
LOCAL_LDLIBS := -llog -landroid -ldl
LOCAL_LDFLAGS := -u ANativeActivity_onCreate
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,third_party/googletest)
$(call import-module,third_party/shaderc)

