// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include <QtGui/QApplication>
#include "mainwindow.h"
#include "form_ui.h"

MainWindow* wnd;

int main(int argc, char *argv[]) {
  // register WidgetInfo to enable invoked() slot
  qRegisterMetaType<WidgetInfo>("WidgetInfo");

  QApplication a(argc, argv);
  MainWindow w;
  wnd = &w;
  w.show();
  
  return a.exec();
}
