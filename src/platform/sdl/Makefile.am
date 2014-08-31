# SmallBASIC for SDL
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I$(top_srcdir)/src -I. @PACKAGE_CFLAGS@

EXTRA_DIST =								\
  fixedfont.xpm							\
  sbasic.nsi							  \
  welcome.bas

bin_PROGRAMS = sbasicg

sbasicg_SOURCES =         	\
  ../../ui/graphics.cpp  		\
  ../../ui/ansiwidget.cpp 	\
  ../../ui/screen.cpp				\
  ../../ui/system.cpp				\
  ../../ui/form_ui.cpp			\
  ../../ui/StringLib.cpp		\
  main.cpp									\
  display.cpp								\
  runtime.cpp								\
  settings.cpp																		

sbasicg_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

sbasicg_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a
