LOCAL_PATH := $(call my-dir)

#the yuv2rgb library
include $(CLEAR_VARS)
LOCAL_ALLOW_UNDEFINED_SYMBOLS=false
LOCAL_MODULE := yuv2rgb

LOCAL_CFLAGS := -I$(AVPLAYER_PATH)"/jni/yuv2rgb/include" -D__STDC_CONSTANT_MACROS 
LOCAL_CFLAGS += $(CC_OPTIMIZE_FLAG) 

LOCAL_SRC_FILES := src/yuv2rgb16tab.c src/yuv420rgb8888.S src/yuv420rgb565.S 
#LOCAL_SRC_FILES := src/yuv2rgb16tab.c src/yuv420rgb8888.s src/yuv420rgb565.s src/yuv422rgb565.s src/yuv2rgb555.s src/yuv2rgbX.s src/yuv420rgb888.s src/yuv422rgb565.s src/yuv422rgb888.s src/yuv422rgb8888.s src/yuv444rgb565.s src/yuv444rgb888.s src/yuv444rgb8888.s

ifeq ($(TARGET_ARCH_ABI),x86)
   LOCAL_SRC_FILES := src/yuv2rgb16tab.c src/yuv420rgb8888c.c src/yuv420rgb565c.c
endif

ifeq ($(TARGET_ARCH_ABI),mips)
   LOCAL_SRC_FILES := src/yuv2rgb16tab.c src/yuv420rgb8888c.c src/yuv420rgb565c.c
endif

LOCAL_SHARED_LIBRARIES := 
LOCAL_STATIC_LIBRARIES := 
LOCAL_LDLIBS := -ldl -llog 

include $(BUILD_SHARED_LIBRARY)
