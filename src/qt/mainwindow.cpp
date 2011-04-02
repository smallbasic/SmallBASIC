// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include "sbapp.h"
#include <stdio.h>

const char* aboutText =
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
  wnd = this;
  out = ui->widget;
}

MainWindow::~MainWindow() {
  delete ui;
}

bool MainWindow::isBreakExec() {
  return (runMode == break_state || runMode == quit_state);
}

bool MainWindow::isRunning() {
  return (runMode == run_state || runMode == modal_state);
}

void MainWindow::setModal(bool modal) {
  runMode = modal ? modal_state : run_state;  
}

void MainWindow::endModal() {
  runMode = run_state;
}

void MainWindow::setPenMode(bool flag) {
  penMode = flag;
  setMouseTracking(flag);
}

void MainWindow::resetPen() {
  mouseX = 0;
  mouseY = 0;
  penMode = false;
  setMouseTracking(false);
}

void MainWindow::runBreak() {
  if (runMode == run_state || runMode == modal_state) {
    brun_break();
    runMode = break_state;
  }
}

void MainWindow::runQuit() {
}

bool MainWindow::event(QEvent* event) {
  if (event->type() == QEvent::ShowToParent) {
    out->print(aboutText);
  }
  return QMainWindow::event(event);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
  if (isRunning()) {
    mouseX = event->x();
    mouseY = event->y();
    keymap_invoke(SB_KEY_MK_PUSH);
  }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
  if (isRunning()) {
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    mouseX = -1;
    mouseY = -1;
    keymap_invoke(SB_KEY_MK_RELEASE);
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
  if (isRunning()) {
    mouseX = event->x();
    mouseY = event->y();
    keymap_invoke(penMode && mouseX == -1 ? SB_KEY_MK_MOVE : SB_KEY_MK_DRAG);
  }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (isRunning()) {
    switch (event->key()) {
    case Qt::Key_Tab:
      dev_pushkey(SB_KEY_TAB);
      break;
    case Qt::Key_Home:
      dev_pushkey(SB_KEY_KP_HOME);
      break;
    case Qt::Key_End:
      dev_pushkey(SB_KEY_END);
      break;
    case Qt::Key_Insert:
      dev_pushkey(SB_KEY_INSERT);
      break;
    case Qt::Key_Menu:
      dev_pushkey(SB_KEY_MENU);
      break;
    case Qt::Key_multiply:
      dev_pushkey(SB_KEY_KP_MUL);
      break;
    case Qt::Key_Plus:
      dev_pushkey(SB_KEY_KP_PLUS);
      break;
    case Qt::Key_Minus:
      dev_pushkey(SB_KEY_KP_MINUS);
      break;
    case Qt::Key_Slash:
      dev_pushkey(SB_KEY_KP_DIV);
      break;
    case Qt::Key_F1:       
      dev_pushkey(SB_KEY_F(1));
      break;
    case Qt::Key_F2:
      dev_pushkey(SB_KEY_F(2));
      break;
    case Qt::Key_F3:
      dev_pushkey(SB_KEY_F(3));
      break;
    case Qt::Key_F4:
      dev_pushkey(SB_KEY_F(4));
      break;
    case Qt::Key_F5:
      dev_pushkey(SB_KEY_F(5));
      break;
    case Qt::Key_F6:
      dev_pushkey(SB_KEY_F(6));
      break;
    case Qt::Key_F7:
      dev_pushkey(SB_KEY_F(7));
      break;
    case Qt::Key_F8:
      dev_pushkey(SB_KEY_F(8));
      break;
    case Qt::Key_F9:
      dev_pushkey(SB_KEY_F(9));
      break;
    case Qt::Key_F10:
      dev_pushkey(SB_KEY_F(10));
      break;
    case Qt::Key_F11:
      dev_pushkey(SB_KEY_F(11));
      break;
    case Qt::Key_F12:
      dev_pushkey(SB_KEY_F(12));
      break;
    case Qt::Key_PageUp:
      dev_pushkey(SB_KEY_PGUP);
      break;
    case Qt::Key_PageDown:
      dev_pushkey(SB_KEY_PGDN);
      break;
    case Qt::Key_Up:
      dev_pushkey(SB_KEY_UP);
      break;
    case Qt::Key_Down:
      dev_pushkey(SB_KEY_DN);
      break;
    case Qt::Key_Left:
      dev_pushkey(SB_KEY_LEFT);
      break;
    case Qt::Key_Right:
      dev_pushkey(SB_KEY_RIGHT);
      break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
      dev_pushkey(SB_KEY_BACKSPACE);
      break;
    case Qt::Key_Return:
      dev_pushkey(13);
      break;
    case 'b':
      if (event->modifiers() & Qt::ControlModifier) {
        runBreak();
        break;
      }
      dev_pushkey(event->key());
      break;
    case 'q':
      if (event->modifiers() & Qt::ControlModifier) {
        runQuit();
        break;
      }
      dev_pushkey(event->key());
      break;
    
    default:
      dev_pushkey(event->key());
      break;
    }
  }
}

// End of "$Id$".
