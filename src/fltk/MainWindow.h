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

#define MNU_HEIGHT 22
#define DEF_FONT_SIZE 12
#define SCAN_LABEL "(Refresh)"
#define NUM_RECENT_ITEMS 9

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define UNTITLED_FILE "untitled.bas"
#define LASTEDIT_FILE "lastedit.txt"

extern "C" void trace(const char* format, ...);

enum ExecState {
  init_state,
  edit_state,
  run_state,
  modal_state,
  break_state,
  quit_state
};

struct MainWindow;
extern MainWindow *wnd;
extern ExecState runMode;

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

#define CALLBACK_METHOD(FN)                     \
  void FN(Widget* w=0, void* v=0);              \
  static void FN ## _cb(Widget* w, void *v) {   \
    wnd->FN(w, v);                              \
  }

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

  bool basicMain(const char *filename, bool toolExec);
  bool isBreakExec(void);
  bool isEdit();
  bool isEditHidden() {return isHideEditor;}
  bool isInteractive();
  bool isModal();
  void addHistory(const char *fileName);
  void execHelp();
  void execInit();
  void execLink(const char* file);
  void resetPen();
  void saveLastEdit(const char *filename);
  void scanPlugIns(Menu* menu);
  void scanRecentFiles(Menu * menu);
  void setBreak();
  void setHideEditor();
  void setModal(bool modal);
  void showEditTab();
  void showHelpPage();
  void showHelpTab();
  void showOutputTab();
  void updatePath(char *filename);

  CALLBACK_METHOD(change_case);
  CALLBACK_METHOD(copy_text);
  CALLBACK_METHOD(cut_text);
  CALLBACK_METHOD(editor_plugin);
  CALLBACK_METHOD(expand_word);
  CALLBACK_METHOD(font_size_decr);
  CALLBACK_METHOD(font_size_incr);
  CALLBACK_METHOD(help_about);
  CALLBACK_METHOD(help_app);
  CALLBACK_METHOD(help_contents);
  CALLBACK_METHOD(help_contents_anchor);
  CALLBACK_METHOD(help_home);
  CALLBACK_METHOD(hide_editor);
  CALLBACK_METHOD(load_file);
  CALLBACK_METHOD(next_tab);
  CALLBACK_METHOD(paste_text);
  CALLBACK_METHOD(quit);
  CALLBACK_METHOD(restart_run);
  CALLBACK_METHOD(run);
  CALLBACK_METHOD(run_break);
  CALLBACK_METHOD(set_options);
  CALLBACK_METHOD(tool_plugin);
  CALLBACK_METHOD(turbo);

  EditorWindow* getEditor();

  bool isTurbo;
  bool isHideEditor;
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

  // common editing widgets
  Window* replaceDlg;
  Input* replaceFind;
  Input* replaceWith;
};

#endif
