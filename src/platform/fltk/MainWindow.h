// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <fltk/Window.h>
#include <fltk/TabGroup.h>
#include <fltk/ValueInput.h>

#include <stdint.h>

#include "platform/fltk/display.h"
#include "platform/fltk/EditorWidget.h"
#include "platform/fltk/HelpWidget.h"
#include "platform/fltk/Profile.h"
#include "platform/fltk/utils.h"

#define MNU_HEIGHT 22
#define DEF_FONT_SIZE 12
#define NUM_RECENT_ITEMS 9
#define MAX_DESCENT 8           // filter to avoid non-text fonts

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

enum ExecState {
  init_state,
  edit_state,
  run_state,
  modal_state,
  break_state,
  quit_state
};

enum GroupWidget {
  gw_editor,
  gw_output,
  gw_help,
  gw_file
};

struct LineInput : public fltk::Input {
  LineInput(int x, int y, int w, int h);
  bool replace(int b, int e, const char *text, int ilen);
  void layout();
  int handle(int event);

private:
  int orig_x, orig_y, orig_w, orig_h;
};

struct MainWindow;
extern MainWindow *wnd;
extern ExecState runMode;

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

#define CALLBACK_METHOD(FN)                     \
  void FN(Widget *w=0, void *v=0);              \
  static void FN ## _cb(Widget *w, void *v) {   \
    wnd->FN(w, v);                              \
  }

struct BaseWindow : public Window {
  BaseWindow(int w, int h) : Window(w, h, "SmallBASIC") {} 
  virtual ~BaseWindow() {};
  int handle(int e);
  bool handleKeyEvent();

  int _penDownX;
  int _penDownY;
  int _penMode;   // PEN ON/OFF
};

struct MainWindow : public BaseWindow {
  MainWindow(int w, int h);
  virtual ~MainWindow() {};

  bool basicMain(EditorWidget *editWidget, const char *filename, bool toolExec);
  bool isBreakExec(void);       // whether BREAK mode has been entered
  bool isRunning(void);         // whether a program is running
  bool isEdit();                // whether a program is currently being edited 
  bool isIdeHidden();           // whether to run without the IDE displayed
  bool isInteractive();         // whether to run without an interface
  bool isModal();               // whether a modal gui loop is active
  void addPlugin(Menu *menu, const char *label, const char *filename);
  void busyMessage();
  bool execHelp();
  void execLink(strlib::String &file);
  void loadIcon(const char *prefix, int resourceId);
  void pathMessage(const char *file);
  void resetPen();
  void saveEditConfig(EditorWidget *editWidget);
  void scanPlugIns(Menu *menu);
  void scanRecentFiles(Menu *menu);
  void setBreak();
  void setModal(bool modal);
  void setTitle(Window *widget, const char *filename);
  void showEditTab(EditorWidget *editWidget);
  void showHelpPage();
  void statusMsg(RunMessage runMessage, const char *filename);
  void updateConfig(EditorWidget *current);
  void updateEditTabName(EditorWidget *editWidget);

  Group *createEditor(const char *title);
  EditorWidget *getEditor(Group *group);
  EditorWidget *getEditor(const char *fullPath);
  EditorWidget *getEditor(bool select = false);
  void editFile(const char *filePath);
  Group *getSelectedTab();
  Group *getNextTab(Group *current);
  Group *getPrevTab(Group *current);
  Group *selectTab(const char *label);
  Group *findTab(const char *label);
  Group *findTab(GroupWidget groupWidget);
  GroupWidget getGroupWidget(Group *group) {
    return (GroupWidget) (intptr_t) group->user_data();
  }
  bool logPrint();
  FILE *openConfig(const char *fileName, const char *flags = "w");
  TtyWidget *tty();

  CALLBACK_METHOD(close_tab);
  CALLBACK_METHOD(copy_text);
  CALLBACK_METHOD(cut_text);
  CALLBACK_METHOD(editor_plugin);
  CALLBACK_METHOD(export_file);
  CALLBACK_METHOD(font_size_decr);
  CALLBACK_METHOD(font_size_incr);
  CALLBACK_METHOD(font_cache_clear);
  CALLBACK_METHOD(help_about);
  CALLBACK_METHOD(help_app);
  CALLBACK_METHOD(help_contents);
  CALLBACK_METHOD(help_contents_anchor);
  CALLBACK_METHOD(help_home);
  CALLBACK_METHOD(hide_ide);
  CALLBACK_METHOD(load_file);
  CALLBACK_METHOD(new_file);
  CALLBACK_METHOD(next_tab);
  CALLBACK_METHOD(open_file);
  CALLBACK_METHOD(paste_text);
  CALLBACK_METHOD(prev_tab);
  CALLBACK_METHOD(quit);
  CALLBACK_METHOD(restart_run);
  CALLBACK_METHOD(run);
  CALLBACK_METHOD(run_break);
  CALLBACK_METHOD(run_selection);
  CALLBACK_METHOD(save_file_as);
  CALLBACK_METHOD(set_flag);
  CALLBACK_METHOD(set_options);
  CALLBACK_METHOD(tool_plugin);

  HelpWidget *getHelp();

  strlib::String _siteHome;
  strlib::String _exportFile;

  // main output
  DisplayWidget *_out;
  Group *_outputGroup;

  EditorWidget *_runEditWidget;

  // tab parent
  TabGroup *_tabGroup;

  // configuration
  Profile *_profile;
};

#endif
