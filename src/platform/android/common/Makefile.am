# SmallBASIC
# Copyright(C) 2001-2013 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

COMMON=$(top_srcdir)/src/common

ARFLAGS = Tru

RANLIB=echo

AM_CPPFLAGS=-I$(top_srcdir)/src @PACKAGE_CFLAGS@

noinst_LIBRARIES = libsb_common.a

all-am: Makefile $(LIBRARIES) ant-build

libsb_common_a_SOURCES =                                      \
    $(COMMON)/bc.c $(COMMON)/bc.h                             \
    $(COMMON)/blib.c $(COMMON)/blib.h                         \
    $(COMMON)/blib_db.c                                       \
    $(COMMON)/blib_func.c                                     \
    $(COMMON)/blib_graph.c                                    \
    $(COMMON)/blib_math.c                                     \
    $(COMMON)/matrix.c                                        \
    $(COMMON)/blib_math.h                                     \
    $(COMMON)/blib_sound.c                                    \
    $(COMMON)/brun.c                                          \
    $(COMMON)/ceval.c                                         \
    $(COMMON)/circle.c                                        \
    $(COMMON)/decomp.c                                        \
    $(COMMON)/device.c $(COMMON)/device.h                     \
    $(COMMON)/screen.c                                        \
    $(COMMON)/system.c                                        \
    $(COMMON)/drvmouse.h $(COMMON)/drvsound.h                 \
    $(COMMON)/eval.c                                          \
    $(COMMON)/extlib.c $(COMMON)/extlib.h                     \
    $(COMMON)/file.c                                          \
    $(COMMON)/ffill.c                                         \
    $(COMMON)/fmt.c $(COMMON)/fmt.h                           \
    $(COMMON)/fs_serial.c $(COMMON)/fs_serial.h               \
    $(COMMON)/fs_socket_client.c $(COMMON)/fs_socket_client.h \
    $(COMMON)/fs_stream.c $(COMMON)/fs_stream.h               \
    $(COMMON)/g_line.c                                        \
    $(COMMON)/geom.c $(COMMON)/geom.h $(COMMON)/g_bmp.h       \
    $(COMMON)/inet.c $(COMMON)/inet.h                         \
    $(COMMON)/kw.c $(COMMON)/kw.h                             \
    $(COMMON)/match.c $(COMMON)/match.h                       \
    $(COMMON)/mem.c                                           \
    $(COMMON)/panic.c $(COMMON)/panic.h                       \
    $(COMMON)/pfill.c                                         \
    $(COMMON)/plot.c                                          \
    $(COMMON)/proc.c $(COMMON)/pproc.h                        \
    $(COMMON)/sberr.c $(COMMON)/sberr.h                       \
    $(COMMON)/scan.c $(COMMON)/scan.h                         \
    $(COMMON)/str.c $(COMMON)/str.h                           \
    $(COMMON)/tasks.c $(COMMON)/tasks.h                       \
    $(COMMON)/var_uds.c $(COMMON)/var_uds.h                   \
    $(COMMON)/var_hash.c $(COMMON)/var_hash.h                 \
    $(COMMON)/keymap.c $(COMMON)/keymap.h                     \
    $(COMMON)/units.c $(COMMON)/units.h                       \
    $(COMMON)/var.c $(COMMON)/var.h                           \
    $(COMMON)/vmt.c $(COMMON)/vmt.h                           \
    $(COMMON)/messages.h                                      \
    $(COMMON)/osd.h                                           \
    $(COMMON)/pdb.h                                           \
    $(COMMON)/pmem.h                                          \
    $(COMMON)/sbapp.h                                         \
    $(COMMON)/smbas.h                                         \
    $(COMMON)/sys.h

../libs/armeabi/libsmallbasic.so: libsb_common.a

ndk-build: ../libs/armeabi/libsmallbasic.so
	(cd ../jni && ${NDK}/ndk-build NDK_DEBUG=1)

ant-build: ndk-build
	(cd ../../../../ide/android && ant -quiet debug install)

test: ant-build
	(cd ../../../../ide/android && \
   adb -d logcat -c && \
   adb shell am start net.sourceforge.smallbasic/android.app.NativeActivity && \
   adb -d logcat)


