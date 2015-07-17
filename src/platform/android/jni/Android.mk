# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

JNI_PATH := $(call my-dir)
SB_HOME := $(JNI_PATH)/../../../..
FREETYPE_HOME := $(HOME)/android-sdk/freetype-2.5.5

include $(call all-subdir-makefiles)
LOCAL_PATH := $(JNI_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE     := smallbasic
LOCAL_CFLAGS     := -DHAVE_CONFIG_H=1 -DLODEPNG_NO_COMPILE_CPP \
	                  -DPIXELFORMAT_RGBA8888
LOCAL_C_INCLUDES := $(SB_HOME) $(SB_HOME)/src                   \
                    $(FREETYPE_HOME)/freetype/include           \
                    $(FREETYPE_HOME)/freetype/include/freetype2
LOCAL_SRC_FILES  := main.cpp                   \
                    display.cpp                \
                    runtime.cpp                \
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
LOCAL_LDLIBS     := -llog -landroid -ljnigraphics
LOCAL_STATIC_LIBRARIES := sb_common freetype android_native_app_glue
include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
