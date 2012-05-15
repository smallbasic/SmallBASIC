# SmallBASIC for GTK
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES =																			\
	-I../../../common -I.													\
	-DPACKAGE_PREFIX=\""$(prefix)"\"							\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"					\
	@PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

bin_PROGRAMS : ../../../common/libsb_common.a

sbasic_SOURCES =																\
	main.c																				\
	output.c output.h															\
	output_model.c output_model.h									\
	output_write.c																\
	form_ui.c																			\
	callbacks.c callbacks.h												\
	interface.c interface.h	

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/src/common -lsb_common

sbasic_DEPENDENCIES = ../../../common/libsb_common.a

#
#servicefiledir=`$(PKG_CONFIG) --variable=libdir dbus-glib-1`/dbus-1.0/services





