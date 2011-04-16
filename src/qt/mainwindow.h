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

#include <QLineEdit>
#include <QMainWindow>
#include <QMimeData>

#include "ansiwidget.h"
#include "httpfile.h"
#include "ui_console_view.h"
#include "ui_mainwindow.h"
#include "ui_source_view.h"

namespace Ui {
  class MainWindow;
}

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

extern "C" void trace(const char* format, ...);

enum ExecState {
  init_state,
  run_state,
  restart_state,
  modal_state,
  break_state,
  quit_state
};

struct MainWindow;
extern MainWindow *wnd;

class MainWindow : public QMainWindow, MouseListener, ReadyListener {
  Q_OBJECT
    
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  AnsiWidget* out;

  bool isBreakExec();
  bool isRunning();
  void logWrite(const char* msg);
  void runQuit();
  void setModal(bool modal);

public slots:
  void endModal();
  void fileOpen();
  void helpAbout();
  void helpHomePage();
  void newWindow();
  void runBreak();
  void runRestart();
  void runStart();
  void viewErrorConsole();
  void viewPreferences();
  void viewProgramSource();
  
private:
  // private inherited events
  void mouseMoveEvent(bool down);
  void mousePressEvent();
  void mouseReleaseEvent();

  void closeEvent(QCloseEvent* event);
  void dragEnterEvent(QDragEnterEvent* event); 
  void dropEvent(QDropEvent* event);
  bool event(QEvent* event);
  void keyPressEvent(QKeyEvent* event);

  // private methods
  void basicMain(QString path);
  QString dropFile(const QMimeData* mimeData);
  void loadPath(QString path, bool showPath=true);
  void loadError(QString message);
  void showStatus(bool error);

  // private state variables
  Ui::MainWindow* ui;
  Ui::ErrorConsole* errorUi;
  Ui::SourceDialog* sourceUi;

  QDialog* logDialog;
  QDialog* sourceDialog;
  QLineEdit* textInput;
  QString programPath;
  ExecState runMode;
};

#endif // MAINWINDOW_H

// End of "$Id$".
