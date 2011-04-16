# $Id$
# This file is part of SmallBASIC
#
# Copyright(C) 2001-2011 Chris Warren-Smith. [http:#tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

# CONFIG   += qt debug

QT       += core gui network

TARGET   = sbasicb

TEMPLATE = app

FORMS    = mainwindow.ui \
           source_view.ui \
           console_view.ui

SOURCES += main.cpp \
           mainwindow.cpp \
           ansiwidget.cpp \
           dev_qt.cpp \
           form_ui.cpp \
           httpfile.cpp

HEADERS += mainwindow.h \
           ansiwidget.h \
           form_ui.h \
           httpfile.h

INCLUDEPATH += ../ ../../

LIBS    += -L../ -lsb_common

RESOURCES += \
    sbasic.qrc
