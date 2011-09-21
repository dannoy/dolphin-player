LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := cpudetect
LOCAL_SRC_FILES := cpudetect.c

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -ldl -llog 

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
#include $(BUILD_STATIC_LIBRARY)
