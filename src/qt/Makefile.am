# SmallBASIC using QT
# Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

EXTRA_DIST =					\
  sbasic.pro					\
  mainwindow.ui

sbasicb_SOURCES =				\
  ansiwidget.cpp				\
  ansiwidget.h					\
  main.cpp					\
  mainwindow.cpp				\
  mainwindow.h					\
  device.cpp
  form_ui.cpp	

bin_PROGRAMS = sbasicb

sbasicb_LDADD = -L$(top_srcdir)/src -lsb_common @PACKAGE_LIBS@

sbasicb_DEPENDENCIES = ../libsb_common.a

sbasicb: ${sbasicb_SOURCES}
	$(MAKE) -f Makefile.qt

clean-local:
	$(MAKE) -f Makefile.qt clean

