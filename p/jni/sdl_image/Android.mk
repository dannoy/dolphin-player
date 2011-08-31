LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_image

LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH) -I$(AVPLAYER_PATH)"/jni/jpeg/include" -I$(AVPLAYER_PATH)"/jni/png/include" -I$(AVPLAYER_PATH)"/jni/sdl/include" -I$(AVPLAYER_PATH)"/jni/sdl_image/include" \
				-DLOAD_PNG -DLOAD_JPG -DLOAD_GIF -DLOAD_BMP

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := png jpeg

LOCAL_SHARED_LIBRARIES := sdl

LOCAL_LDLIBS := -lz

include $(BUILD_SHARED_LIBRARY)

