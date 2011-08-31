LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := universalchardet

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src -type d))

# Add more subdirs here, like src/subdir1 src/subdir2

LOCAL_CFLAGS := $(foreach D, $(APP_SUBDIRS), -I$(LOCAL_PATH)/$(D)) \
				-I$(AVPLAYER_PATH)"/jni/universalchardet/include" \

LOCAL_CFLAGS += $(CC_OPTIMIZE_FLAG)

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
# Uncomment to also add C sources
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.S))))


LOCAL_SHARED_LIBRARIES := 

LOCAL_STATIC_LIBRARIES := 

LIBS_WITH_LONG_SYMBOLS := $(strip $(shell \
	for f in $(LOCAL_PATH)/../../libs/armeabi/*.so ; do \
		if echo $$f | grep "libuniversalchardet[.]so" > /dev/null ; then \
			continue ; \
		fi ; \
		if [ -e "$$f" ] ; then \
			if nm -g $$f | cut -c 12- | egrep '.{128}' > /dev/null ; then \
				echo $$f | grep -o 'lib[^/]*[.]so' ; \
			fi ; \
		fi ; \
	done \
) )

ifneq "$(LIBS_WITH_LONG_SYMBOLS)" ""
$(foreach F, $(LIBS_WITH_LONG_SYMBOLS), \
$(info Library $(F): abusing symbol names are: \
$(shell nm -g $(LOCAL_PATH)/../../libs/armeabi/$(F) | cut -c 12- | egrep '.{128}' ) ) \
$(info Library $(F) contains symbol names longer than 128 bytes, \
YOUR CODE WILL DEADLOCK WITHOUT ANY WARNING when you'll access such function - \
please make this library static to avoid problems. ) )
$(error Detected libraries with too long symbol names. Remove all files under project/libs/armeabi, make these libs static, and recompile)
endif

LOCAL_LDLIBS := -ldl -llog 

include $(BUILD_SHARED_LIBRARY)
