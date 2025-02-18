# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

#
# 1. Building oboe for audio support
# modify build_all_android.sh > DBUILD_SHARED_LIBS=false
# export ANDROID_NDK=~/android-sdk/sdk/ndk/27.0.12077973
# ./build_all_android.sh
#
# 2. Building freetype - see README.md
#

FREETYPE_VER=freetype-2.13.3
OBOE_VER=oboe-1.9.3

JNI_PATH := $(call my-dir)
SB_HOME := $(JNI_PATH)/../../../..
SDK := $(HOME)/android-sdk
FREETYPE_HOME := $(SDK)/$(FREETYPE_VER)
OBOE_LIB_DIR := $(SDK)/oboe/${OBOE_VER}/build/$(TARGET_ARCH_ABI)/staging/lib/$(TARGET_ARCH_ABI)
OBOE_INCLUDE_PATH := $(SDK)/oboe/${OBOE_VER}/include

include $(call all-subdir-makefiles)
LOCAL_PATH := $(JNI_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE     := smallbasic
LOCAL_CFLAGS     := -DHAVE_CONFIG_H=1 -DLODEPNG_NO_COMPILE_CPP \
	                  -DPIXELFORMAT_RGBA8888 -Wno-unknown-pragmas
LOCAL_C_INCLUDES := $(SB_HOME) $(SB_HOME)/src $(OBOE_INCLUDE_PATH) \
                    $(FREETYPE_HOME)/freetype/include/freetype2   \
                    $(FREETYPE_HOME)/freetype/include/freetype2/freetype
LOCAL_SRC_FILES  := main.cpp                   \
                    display.cpp                \
                    runtime.cpp                \
                    audio.cpp                  \
                    ../../../ui/screen.cpp     \
                    ../../../ui/ansiwidget.cpp \
                    ../../../ui/window.cpp     \
                    ../../../ui/form.cpp       \
                    ../../../ui/image.cpp      \
                    ../../../ui/inputs.cpp     \
                    ../../../ui/textedit.cpp   \
                    ../../../ui/strlib.cpp     \
                    ../../../ui/graphics.cpp   \
                    ../../../ui/system.cpp
LOCAL_LDLIBS     := -llog -landroid -ljnigraphics -L$(OBOE_LIB_DIR) -loboe
LOCAL_STATIC_LIBRARIES := sb_common freetype android_native_app_glue
include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
