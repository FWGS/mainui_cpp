#Xash3d mainui port for android
#Copyright (c) nicknekit

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
	LOCAL_MODULE_FILENAME = libmenu_hardfp
endif

LOCAL_MODULE := menu
LOCAL_CPPFLAGS := -std=gnu++11 -DMAINUI_USE_STB -DMAINUI_USE_CUSTOM_FONT_RENDER -DNO_STL -fno-rtti -fno-exceptions -DMAINUI_RENDER_PICBUTTON_TEXT -DCS16CLIENT

LOCAL_C_INCLUDES := \
	$(SDL_PATH)/include				\
	$(LOCAL_PATH)/. 				\
	$(LOCAL_PATH)/../common 			\
	$(LOCAL_PATH)/../pm_shared 			\
	$(LOCAL_PATH)/../engine 			\
	$(LOCAL_PATH)/../engine/common 			\
	$(LOCAL_PATH)/../utils/vgui/include 		\
	$(LOCAL_PATH)/../public/			\
	$(LOCAL_PATH)/../dlls	 			\
	$(LOCAL_PATH)/menus 				\
	$(LOCAL_PATH)/controls 				\
	$(LOCAL_PATH)/font 				\
	$(LOCAL_PATH)/utl 				\
	$(LOCAL_PATH)/model

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH) -name *.cpp)) \
	$(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH)/controls -name *.cpp)) \
	$(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH)/menus -name *.cpp)) \
	$(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH)/utl -name *.cpp)) \
	$(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH)/font -name *.cpp)) \
	../common/interface.cpp

include $(BUILD_SHARED_LIBRARY)
