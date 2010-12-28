# $Id: $
# This file is part of SmallBASIC
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#
# Copyright(C) 2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sbasic

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../

LOCAL_CFLAGS    := -D_UnixOS=1 \
                   -DOS_PREC64=1 \
                   -DUSE_TERM_IO=0 \
                   -DHAVE_SEARCH_H=0 \
                   -DHAVE_MALLOC_USABLE_SIZE=0

LOCAL_SRC_FILES := main.c			\
                   screen.c                     \
                   file.c                       \
                   sound.c                      \
		   ../../bc.c			\
		   ../../blib.c			\
                   ../../blib_db.c		\
		   ../../blib_func.c		\
		   ../../blib_graph.c		\
                   ../../blib_math.c		\
                   ../../matrix.c		\
                   ../../blib_ui.c		\
                   ../../blib_sound.c		\
                   ../../brun.c			\
                   ../../ceval.c		\
                   ../../circle.c		\
                   ../../decomp.c		\
                   ../../device.c		\
                   ../../eval.c			\
                   ../../extlib.c		\
	           ../../file.c			\
                   ../../ffill.c		\
                   ../../fmt.c			\
                   ../../fs_irda.c		\
                   ../../fs_memo.c		\
                   ../../fs_pdoc.c		\
                   ../../fs_serial.c		\
                   ../../fs_socket_client.c	\
                   ../../g_line.c		\
                   ../../geom.c			\
                   ../../inet.c			\
                   ../../kw.c			\
                   ../../match.c		\
                   ../../mem.c			\
                   ../../panic.c		\
                   ../../pfill.c		\
                   ../../plot.c			\
                   ../../proc.c			\
                   ../../sberr.c		\
                   ../../scan.c			\
                   ../../str.c			\
                   ../../tasks.c		\
                   ../../var_hash.c		\
                   ../../var_uds.c		\
                   ../../keymap.c		\
                   ../../units.c		\
                   ../../var.c			\
                   ../../vmt.c

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lc
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
