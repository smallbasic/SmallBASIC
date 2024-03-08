# SmallBASIC
# Copyright(C) 2024 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

JNI_PATH := $(call my-dir)
SB_HOME := $(JNI_PATH)/../../../..

include $(call all-subdir-makefiles)
LOCAL_PATH := $(JNI_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE     := ioio
LOCAL_CFLAGS     := -DHAVE_CONFIG_H=1 -Wno-unknown-pragmas
LOCAL_C_INCLUDES := $(SB_HOME) $(SB_HOME)/src
LOCAL_SRC_FILES  := src/main/cpp/ioio.cpp
LOCAL_LDLIBS     := -llog -landroid
include $(BUILD_SHARED_LIBRARY)

