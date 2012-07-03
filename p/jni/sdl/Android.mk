# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl

#	-DSDL_VIDEO_RENDER_RESIZE=1 \

LOCAL_CFLAGS := -I$(AVPLAYER_PATH)"/jni/sdl/include" \
	-DSDL_JAVA_PACKAGE_PATH="com_broov_player" \
	-DBROOV_PLAYER_NO_DEBUG_LOG \
	$(SDL_ADDITIONAL_CFLAGS)
LOCAL_CFLAGS += $(CC_OPTIMIZE_FLAG)

#ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#   LOCAL_CFLAGS += -DHAVE_NEON=1
#endif

SDL_SRCS := \
	src/*.c \
	src/audio/*.c \
	src/cdrom/*.c \
	src/cpuinfo/*.c \
	src/events/*.c \
	src/file/*.c \
	src/haptic/*.c \
	src/joystick/*.c \
	src/stdlib/*.c \
	src/thread/*.c \
	src/timer/*.c \
	src/video/*.c \
	src/main/*.c \
	src/power/*.c \
	src/thread/pthread/*.c \
	src/timer/unix/*.c \
	src/audio/android/*.c \
	src/cdrom/dummy/*.c \
	src/video/android/*.c \
	src/haptic/dummy/*.c \
	src/loadso/dlopen/*.c \
	src/atomic/dummy/*.c 

# TODO: use libcutils for atomic operations, but it's not included in NDK

#	src/atomic/linux/*.c \
#	src/power/linux/*.c \
#	src/joystick/android/*.c \
#	src/haptic/android/*.c \
#	src/libm/*.c \

LOCAL_CPP_EXTENSION := .cpp

# Note this "simple" makefile var substitution, you can find even more complex examples in different Android projects
LOCAL_SRC_FILES := $(foreach F, $(SDL_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))

#LOCAL_SHARED_LIBRARIES := andprof
#LOCAL_STATIC_LIBRARIES := andprof

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)


