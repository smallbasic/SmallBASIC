# SmallBASIC for SDL
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I$(top_srcdir)/src -I. @PACKAGE_CFLAGS@

EXTRA_DIST =																		\
    fixedfont.xpm																\
    sbasic.nsi																	\
    welcome.bas

bin_PROGRAMS = sbasicg

sbasicg_SOURCES =																\
    ../unix/console_main.c											\
    dev_sdl.c																		\
    blib_sdl_ui.cpp

sbasicg_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

sbasicg_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a
