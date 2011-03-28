// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <QEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include "sbapp.h"
#include <stdio.h>

const char* aboutText =
  "\033[1mAbout SmallBASIC...\033[0m\n"
  "QT Version " VERSION "\n"
  "Copyright (c) 2002-2011 Chris Warren-Smith. \n"
  "Copyright (c) 2000-2006 Nicholas Christopoulos\n"
  "http://smallbasic.sourceforge.net\n"
  "\033[3mSmallBASIC comes with ABSOLUTELY NO WARRANTY.\n"
  "This program is free software; you can use it\n"
  "redistribute it and/or modify it under the terms of the\n"
  "GNU General Public License version 2 as published by\n"
  "the Free Software Foundation.\033[0m\n";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
  ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  delete ui;
}

bool MainWindow::event(QEvent* event) {
  if (event->type() == QEvent::ShowToParent) {
    ui->widget->print(aboutText);
    sbasic_main("s");
  }
  return QMainWindow::event(event);
}

