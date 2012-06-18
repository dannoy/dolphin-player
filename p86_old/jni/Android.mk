#BASE PATH FOR JNI
AVPLAYER_PATH := "c:/p86"

# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := com_broov_playerx86

# Path to shared libraries - Android 1.6 cannot load them properly, thus we have to specify absolute path here
SDL_SHARED_LIBRARIES_PATH := /data/data/com.broov.playerx86/lib

# Your application will just set current directory there
SDL_CURDIR_PATH := /data/data/com.broov.playerx86/files

# Android Dev Phone G1 has trackball instead of cursor keys, and 
# sends trackball movement events as rapid KeyDown/KeyUp events,
# this will make Up/Down/Left/Right key up events with X frames delay,
# so if application expects you to press and hold button it will process the event correctly.
# TODO: create a libsdl config file for that option and for key mapping/on-screen keyboard
SDL_TRACKBALL_KEYUP_DELAY := 1

# If the application designed for higher screen resolution enable this to get the screen
# resized in HW-accelerated way, however it eats a tiny bit of CPU
SDL_VIDEO_RENDER_RESIZE := 1

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
   LOCAL_CFLAGS += -DHAVE_NEON=1
endif

include $(call all-subdir-makefiles)
