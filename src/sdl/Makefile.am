# SmallBASIC for SDL
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = -I../ -I. @PACKAGE_CFLAGS@

EXTRA_DIST =					\
    fixedfont.xpm				\
    sbasic.nsi					\
    welcome.bas

bin_PROGRAMS = sbasicg

sbasicg_SOURCES =      \
    ../console_main.c  \
    dev_sdl.c          \
    blib_sdl_ui.cpp

sbasicg_LDADD = -L$(top_srcdir)/src -lsb_common @PACKAGE_LIBS@

sbasicg_DEPENDENCIES = ../libsb_common.a
