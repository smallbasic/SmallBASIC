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

enum ExecState {
  init_state,
  run_state,
  modal_state,
  break_state,
  quit_state
};

struct MainWindow;
extern MainWindow *wnd;

class MainWindow : public QMainWindow {
  Q_OBJECT
    
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  AnsiWidget* out;

  bool isBreakExec();
  bool isRunning();
  bool getPenMode() {return penMode;}
  void setModal(bool modal);
  void resetPen();
  void runBreak();
  void runQuit();
  void setPenMode(bool mode);
  int getMouseX(bool current) {return current ? mouseX : prevMouseX;}
  int getMouseY(bool current) {return current ? mouseY : prevMouseY;}

public slots:
  void endModal();
  
private:
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void keyPressEvent(QKeyEvent* event);

  bool event(QEvent* event);
  Ui::MainWindow *ui;

  int mouseX;
  int mouseY;
  int prevMouseX;
  int prevMouseY;
  bool penMode; // PEN ON/OFF
  ExecState runMode;
};

#endif // MAINWINDOW_H

// End of "$Id$".
