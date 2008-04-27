// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <fltk/Window.h>
#include <fltk/TabGroup.h>
#include <fltk/ValueInput.h>
#include <fltk/AnsiWidget.h>

#include "EditorWindow.h"
#include "HelpWidget.h"

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

#ifndef max
#define max(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef min
#define min(a,b) ((a>b) ? (b) : (a))
#endif

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

#define CALLBACK_METHOD(FN)                     \
  void FN(void* v);                             \
  static void FN ## _cb(Widget *, void *v) {    \
    ((MainWindow *) v)->FN(v);                  \
  }

extern "C" void trace(const char* format, ...);

enum ExecState {
  init_state,
  edit_state,
  run_state,
  modal_state,
  break_state,
  quit_state
};

enum RunMessage {
  msg_err,
  msg_run,
  msg_none
};

struct BaseWindow : public Window {
  BaseWindow(int w, int h) : Window(w, h, "SmallBASIC") {}
  virtual ~BaseWindow() {};
  int handle(int e);

  int penDownX;
  int penDownY;
  int penState;                 // PUSH/RELEASE events
  int penMode;                  // PEN ON/OFF
};

struct MainWindow : public BaseWindow {
  MainWindow(int w, int h);
  virtual ~MainWindow() {};

  bool isBreakExec(void);
  bool isModal();
  bool isEdit();
  void setModal(bool modal);
  void setBreak();
  void resetPen();
  void execLink(const char* file);

  void statusMsg(const char *filename);
  void setRowCol(int row, int col);
  void setModified(bool dirty);
  void addHistory(const char *fileName);
  void fileChanged(bool loadfile);

  bool isTurbo;
  bool isFullScreen;
  String siteHome;

  // main output
  AnsiWidget* out;
  EditorWindow* editWnd;
  HelpWidget* helpWnd;

  // tabs
  TabGroup* tabGroup;
  Group* editGroup;
  Group* outputGroup;
  Group* helpGroup;

  // tool-bar
  Input* findText;
  Input* gotoLine;
  Choice* funcList;

  // status bar
  Widget* fileStatus;
  Widget* rowStatus;
  Widget* colStatus;
  Widget* runStatus;
  Widget* modStatus;

  CALLBACK_METHOD(change_case);
  CALLBACK_METHOD(copy_text);
  CALLBACK_METHOD(cut_text);
  CALLBACK_METHOD(editor_plugin);
  CALLBACK_METHOD(expand_word);
  CALLBACK_METHOD(find);
  CALLBACK_METHOD(font_size_decr);
  CALLBACK_METHOD(font_size_incr);
  CALLBACK_METHOD(fullscreen);
  CALLBACK_METHOD(func_list);
  CALLBACK_METHOD(goto_line);
  CALLBACK_METHOD(help_about);
  CALLBACK_METHOD(help_app);
  CALLBACK_METHOD(help_contents);
  CALLBACK_METHOD(help_contents_anchor);
  CALLBACK_METHOD(help_home);
  CALLBACK_METHOD(load_file);
  CALLBACK_METHOD(next_tab);
  CALLBACK_METHOD(paste_text);
  CALLBACK_METHOD(quit);
  CALLBACK_METHOD(run);
  CALLBACK_METHOD(run_break);
  CALLBACK_METHOD(set_options);
  CALLBACK_METHOD(tool_plugin);
  CALLBACK_METHOD(turbo);

  bool basicMain(const char *filename, bool toolExec);
  void busyMessage();
  void execHelp();
  void execInit();
  void pathMessage(const char *file);
  void restoreEdit();
  void runMsg(RunMessage runMessage);
  void saveLastEdit(const char *filename);
  void scanPlugIns(Menu* menu);
  void scanRecentFiles(Menu * menu);
  void showEditTab();
  void showHelpPage();
  void showHelpTab();
  void showOutputTab();
  void updatePath(char *filename);
  void focusWidget();
};

extern MainWindow *wnd;
extern ExecState runMode;

#endif
