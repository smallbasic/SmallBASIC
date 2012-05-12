# SmallBASIC using QT
# Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

EXTRA_DIST =																		\
  sbasic.pro																		\
  sbasic.qrc																		\
  mainwindow.ui																	\
  source_view.ui																\
  console_view.ui																\
  images/next.png																\
  images/previous.png														\
  images/jump.png																\
  images/home.png																\
  images/refresh.png														\
  images/stop.png																\
  bas/settings.bas															\
  bas/list.bas																	\
  bas/bookmarks.bas															\
  bas/home.bas                       

sbasicb_SOURCES =																\
  fixedlayout.cpp																\
  fixedlayout.h																	\
  httpfile.cpp																	\
  httpfile.h																		\
  ansiwidget.cpp																\
  ansiwidget.h																	\
  main.cpp																			\
  mainwindow.cpp																\
  mainwindow.h																	\
  dev_qt.cpp																		\
  form_ui.cpp																		\
  form_ui.h		

bin_PROGRAMS = sbasicb

sbasicb_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

sbasicb_DEPENDENCIES = ../common/libsb_common.a

sbasicb: ${sbasicb_SOURCES}
	$(MAKE) -f Makefile.qt

clean-local:
	$(MAKE) -f Makefile.qt clean

