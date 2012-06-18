LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

common_SRC_FILES := \
	blocksort.c \
	huffman.c \
	crctable.c \
	randtable.c \
	compress.c \
	decompress.c \
	bzlib.c 

common_CFLAGS := -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 

common_C_INCLUDES += $(LOCAL_PATH)/

# For the host
# =====================================================

LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)

LOCAL_MODULE:= bz2

include $(BUILD_STATIC_LIBRARY)

