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

sbgtk_SOURCES =									\
	main.c										\
    output.c output.h      						\
    output_model.c output_model.h               \
    output_write.c                              \
	callbacks.c callbacks.h

if WITH_HILDON
sbgtk_SOURCES += hildon_ui.c hildon_ui.h
else
sbgtk_SOURCES += support.c support.h interface.c interface.h	
endif

sbgtk_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/sync -lsb_common

servicefiledir=`$(PKG_CONFIG) --variable=libdir dbus-glib-1`/dbus-1.0/services





