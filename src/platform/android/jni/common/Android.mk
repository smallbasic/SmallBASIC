# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

COMMON=$(SB_HOME)/src/common

LOCAL_C_INCLUDES := $(SB_HOME) $(SB_HOME)/src
LOCAL_MODULE     := sb_common
LOCAL_CFLAGS     := -DHAVE_CONFIG_H=1
LOCAL_SRC_FILES  :=                             \
    $(COMMON)/bc.c                              \
    $(COMMON)/blib.c                            \
    $(COMMON)/blib_db.c                         \
    $(COMMON)/blib_func.c                       \
    $(COMMON)/blib_graph.c                      \
    $(COMMON)/blib_math.c                       \
    $(COMMON)/matrix.c                          \
    $(COMMON)/blib_sound.c                      \
    $(COMMON)/brun.c                            \
    $(COMMON)/ceval.c                           \
    $(COMMON)/circle.c                          \
    $(COMMON)/decomp.c                          \
    $(COMMON)/device.c                          \
    $(COMMON)/screen.c                          \
    $(COMMON)/system.c                          \
    $(COMMON)/eval.c                            \
    $(COMMON)/extlib.c                          \
    $(COMMON)/file.c                            \
    $(COMMON)/ffill.c                           \
    $(COMMON)/fmt.c                             \
    $(COMMON)/fs_serial.c                       \
    $(COMMON)/fs_socket_client.c                \
    $(COMMON)/fs_stream.c                       \
    $(COMMON)/g_line.c                          \
    $(COMMON)/geom.c                            \
    $(COMMON)/inet.c                            \
    $(COMMON)/kw.c                              \
    $(COMMON)/match.c                           \
    $(COMMON)/mem.c                             \
    $(COMMON)/panic.c                           \
    $(COMMON)/pfill.c                           \
    $(COMMON)/plot.c                            \
    $(COMMON)/proc.c                            \
    $(COMMON)/sberr.c                           \
    $(COMMON)/scan.c                            \
    $(COMMON)/str.c                             \
    $(COMMON)/tasks.c                           \
    $(COMMON)/search.c                          \
    $(COMMON)/var_hash.c                        \
    $(COMMON)/keymap.c                          \
    $(COMMON)/units.c                           \
    $(COMMON)/var.c                             \
    $(COMMON)/vmt.c

include $(BUILD_STATIC_LIBRARY)

