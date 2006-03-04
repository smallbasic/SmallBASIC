# SmallBASIC for GTK
# Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = \
     -I../../ -I. \
	-DPACKAGE_PREFIX=\""$(prefix)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = sbgtk

bin_PROGRAMS : ../../libsb_common.a

sbgtk_SOURCES =									\
	main.c										\
    output.c output.h      						\
    output_model.c output_model.h               \
    output_write.c                              \
	callbacks.c callbacks.h                     \
    interface.c interface.h	

sbgtk_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/sync -lsb_common

#
#servicefiledir=`$(PKG_CONFIG) --variable=libdir dbus-glib-1`/dbus-1.0/services





