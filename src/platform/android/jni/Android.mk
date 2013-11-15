# SmallBASIC
# Copyright(C) 2001-2013 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

# see: ~/android-sdk/android-ndk-r9b/docs/ANDROID-MK.html

LOCAL_PATH := $(call my-dir)
SB_HOME := $(LOCAL_PATH)/../../../..
FREETYPE_HOME := $(HOME)/android-sdk/freetype-2.5.0.1/freetype

include $(CLEAR_VARS)
LOCAL_MODULE := sb_common
LOCAL_SRC_FILES := ../common/libsb_common.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := freetype
LOCAL_SRC_FILES := $(FREETYPE_HOME)/lib/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE     := smallbasic
LOCAL_CFLAGS     := -DHAVE_CONFIG_H=1
LOCAL_C_INCLUDES := $(SB_HOME) $(SB_HOME)/src \
                    $(FREETYPE_HOME)/include \
                    $(FREETYPE_HOME)/include/freetype2
LOCAL_SRC_FILES  := main.cpp \
                    display.cpp \
                    runtime.cpp \
                    ../../common/screen.cpp \
                    ../../common/ansiwidget.cpp \
                    ../../common/form_ui.cpp \
                    ../../common/StringLib.cpp \
                    ../../common/system.cpp
LOCAL_LDLIBS     := -llog -landroid -ljnigraphics
LOCAL_STATIC_LIBRARIES := sb_common freetype android_native_app_glue 
include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
