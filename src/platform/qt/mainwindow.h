// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMimeData>

#include "ansiwidget.h"
#include "httpfile.h"
#include "fixedlayout.h"
#include "ui_console_view.h"
#include "ui_mainwindow.h"
#include "ui_source_view.h"

namespace Ui {
  class MainWindow;
}

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
extern "C" void trace(const char *format, ...);

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

class MainWindow : public QMainWindow, AnsiWidgetListener, HttpFileListener {
  Q_OBJECT
    
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  AnsiWidget *out;

  bool isBreakExec();
  bool isResourceApp() { return resourceApp; }
  bool isRunning();
  bool isRunModal();
  void logWrite(const char *msg);
  void setRunModal(bool modal);
  void addWidget(QWidget *widget);
  void removeWidget(QWidget *widget);

public slots:
  void bookmarkProgram();
  void endModal();
  void fileOpen();
  void helpAbout();
  void helpHomePage();
  void historyBackward();
  void historyForward();
  void newWindow();
  void runBreak();
  void runHome();
  void runRestart();
  void runStart();
  void viewBookmarks();
  void viewErrorConsole();
  void viewPreferences();
  void viewProgramSource();

private:
  // private inherited events
  void mouseMoveEvent(bool down);
  void mousePressEvent();
  void mouseReleaseEvent();

  void closeEvent(QCloseEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dropEvent(QDropEvent *event);
  bool event(QEvent *event);
  void keyPressEvent(QKeyEvent *event);

  // private methods
  void basicMain(QString path);
  bool deferExec(QString path, int message);
  QString dropFile(const QMimeData *mimeData);
  void loadResource(QString path);
  void loadPath(QString path, bool showPath = true, bool setHistory = true);
  void loadError(QString message);
  void showStatus(bool error);
  void updateHistory(QString path, bool setHistory);

  // private state variables
  Ui::MainWindow *ui;
  Ui::ErrorConsole *errorUi;
  Ui::SourceDialog *sourceUi;

  FixedLayout *fixedLayout;     // for positioning child widgets
  QDialog *logDialog;           // log window widget
  QDialog *sourceDialog;        // source dialog widget
  QLineEdit *textInput;         // text input control
  QString programPath;          // path to the current program
  QString deferPath;            // path to the deferred program 
  QList <QString> history;      // history buffer
  int historyIndex;             // history index
  ExecState runMode;            // current run state
  QLabel status;                // the status bar widget
  bool resourceApp;             // permission to use settings
};

#endif // MAINWINDOW_H
