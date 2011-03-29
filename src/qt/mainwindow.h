// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ansiwidget.h"

namespace Ui {
  class MainWindow;
}

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

extern "C" void trace(const char* format, ...);

struct MainWindow;
extern MainWindow *wnd;

class MainWindow : public QMainWindow {
  Q_OBJECT
    
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  AnsiWidget* out;

  bool isBreakExec();
  bool getPenMode();
  void setModal(bool flag);
  void resetPen();
  void setPenMode(int flag);
  int getMouseX();
  int getMouseY();

public slots:
  void endModal();
  
private:
  bool event(QEvent* event);
  Ui::MainWindow *ui;

  int mouseX;
  int mouseY;
  int penMode; // PEN ON/OFF
};

#endif // MAINWINDOW_H
