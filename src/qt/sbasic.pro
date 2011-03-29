# $Id$
# This file is part of SmallBASIC
#
# Copyright(C) 2001-2011 Chris Warren-Smith. [http:#tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

QT       += core gui

TARGET   = sbasicb

TEMPLATE = app

FORMS    = mainwindow.ui

SOURCES += main.cpp \
           mainwindow.cpp \
           ansiwidget.cpp \
           dev_qt.cpp \
           form_ui.cpp

HEADERS += mainwindow.h \
           ansiwidget.h

INCLUDEPATH += ../ ../../

LIBS    += -L../ -lsb_common
