// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <QDesktopServices>
#include <QDialog>
#include <QEvent>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QUrl>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_source_view.h"
#include "config.h"
#include "sbapp.h"
#include <stdio.h>

const char* aboutText =
 "QT Version " VERSION "\n\n"
 "Copyright (c) 2002-2011 Chris Warren-Smith. \n"
 "Copyright (c) 2000-2006 Nicholas Christopoulos\n\n"
 "http://smallbasic.sourceforge.net\n\n"
 "SmallBASIC comes with ABSOLUTELY NO WARRANTY.\n"
 "This program is free software; you can use it\n"
 "redistribute it and/or modify it under the terms of the\n"
 "GNU General Public License version 2 as published by\n"
 "the Free Software Foundation.";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QLineEdit* urlInput = new QLineEdit();
  ui->toolBar->addWidget(urlInput);
  ui->toolBar->addAction(ui->actionStart);

  // connect signals and slots
  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
  connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
  connect(ui->actionCopy, SIGNAL(triggered()), ui->ansiWidget, SLOT(copySelection()));
  connect(ui->actionFind, SIGNAL(triggered()), ui->ansiWidget, SLOT(findText()));
  connect(ui->actionFindAgain, SIGNAL(triggered()), ui->ansiWidget, SLOT(findNextText()));
  connect(ui->actionSelectAll, SIGNAL(triggered()), ui->ansiWidget, SLOT(selectAll()));
  connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(viewPreferences()));
  connect(ui->actionToolbar, SIGNAL(triggered()), this, SLOT(viewToolbar()));
  connect(ui->actionHomePage, SIGNAL(triggered()), this, SLOT(helpHomePage()));
  connect(ui->actionNewWindow, SIGNAL(triggered()), this, SLOT(newWindow()));
  connect(ui->actionBreak, SIGNAL(triggered()), this, SLOT(runBreak()));
  connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(runRestart()));
  connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(runRefresh()));
  connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(runStart()));
  connect(ui->actionProgramSource, SIGNAL(triggered()), this, SLOT(viewProgramSource()));
  connect(ui->actionErrorConsole, SIGNAL(triggered()), this, SLOT(viewErrorConsole()));

  wnd = this;
  out = ui->ansiWidget;
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

void MainWindow::runQuit() {
}

bool MainWindow::event(QEvent* event) {
  if (event->type() == QEvent::ShowToParent) {
    // launch home page program
  }
  return QMainWindow::event(event);
}

void MainWindow::fileOpen() {
  QString fileName =
  QFileDialog::getOpenFileName(this, 
                               tr("Open Program"), 
                               QString(), 
                               tr("BASIC Files (*.bas)"));
}

void MainWindow::helpAbout() {
  int ret = QMessageBox::information(this, tr("SmallBASIC"),
                                     tr(aboutText), 
                                     QMessageBox::Ok);
}

void MainWindow::helpHomePage() {
  QDesktopServices::openUrl(QUrl("http://smallbasic.sourceforge.net"));
}

void MainWindow::newWindow() {
  QProcess::startDetached(QCoreApplication::applicationFilePath());
}

void MainWindow::runBreak() {
  if (runMode == run_state || runMode == modal_state) {
    brun_break();
    runMode = break_state;
  }
}

void MainWindow::runRefresh() {

}

void MainWindow::runRestart() {

}

void MainWindow::runStart() {

}

void MainWindow::viewErrorConsole() {

}

void MainWindow::viewPreferences() {

}

void MainWindow::viewProgramSource() {
  Ui::SourceDialog sourceDialog;
  QDialog dlg;
  sourceDialog.setupUi(&dlg);
  dlg.exec();
}

void MainWindow::viewToolbar() {

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
