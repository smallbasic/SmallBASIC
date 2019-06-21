// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <stdint.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tabs.H>
#include "platform/fltk/display.h"
#include "platform/fltk/runtime.h"
#include "platform/fltk/EditorWidget.h"
#include "platform/fltk/HelpView.h"
#include "platform/fltk/Profile.h"
#include "platform/fltk/utils.h"

enum ExecState {
  init_state,
  edit_state,
  run_state,
  modal_state,
  break_state,
  quit_state
};

enum GroupWidgetEnum {
  gw_editor,
  gw_output,
  gw_help,
  gw_file
};

struct MainWindow;
extern MainWindow *wnd;
extern ExecState runMode;

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

#define CALLBACK_METHOD(FN)                      \
  void FN(Fl_Widget *w=0, void *v=0);            \
  static void FN ## _cb(Fl_Widget *w, void *v) { \
    wnd->FN(w, v);                               \
  }

struct BaseWindow : public Fl_Double_Window {
  BaseWindow(int w, int h) : Fl_Double_Window(w, h, "SmallBASIC") {}
  virtual ~BaseWindow() {};
  int handle(int e);
  bool handleKeyEvent();
};

struct MainWindow : public BaseWindow {
  MainWindow(int w, int h);
  virtual ~MainWindow();

  bool basicMain(EditorWidget *editWidget, const char *filename, bool toolExec);
  bool isBreakExec(void);       // whether BREAK mode has been entered
  bool isRunning(void);         // whether a program is running
  bool isEdit();                // whether a program is currently being edited
  bool isIdeHidden();           // whether to run without the IDE displayed
  bool isInteractive();         // whether to run without an interface
  bool isModal();               // whether a modal gui loop is active
  void addPlugin(Fl_Menu_Bar *menu, const char *label, const char *filename);
  void busyMessage();
  int  handle(int e);
  void loadHelp(const char *path);
  void loadIcon();
  void pathMessage(const char *file);
  void resize(int x, int y, int w, int h);
  void resizeDisplay(int w, int h);
  void saveEditConfig(EditorWidget *editWidget);
  void scanPlugIns(Fl_Menu_Bar *menu);
  void scanRecentFiles(Fl_Menu_Bar *menu);
  void setBreak();
  void setModal(bool modal);
  void setTitle(Fl_Window *widget, const char *filename);
  void showEditTab(EditorWidget *editWidget);
  void statusMsg(RunMessage runMessage, const char *filename);
  void updateConfig(EditorWidget *current);
  void updateEditTabName(EditorWidget *editWidget);

  Fl_Group *createEditor(const char *title);
  EditorWidget *getEditor(Fl_Group *group);
  EditorWidget *getEditor(const char *fullPath);
  EditorWidget *getEditor(bool select = false);
  void editFile(const char *filePath);
  Fl_Group *getSelectedTab();
  Fl_Group *getNextTab(Fl_Group *current);
  Fl_Group *getPrevTab(Fl_Group *current);
  Fl_Group *selectTab(const char *label);
  Fl_Group *findTab(const char *label);
  Fl_Group *findTab(GroupWidgetEnum groupWidget);
  GroupWidgetEnum getGroupWidget(Fl_Group *group) {
    return (GroupWidgetEnum) (intptr_t) group->user_data();
  }
  bool logPrint();
  FILE *openConfig(const char *fileName, const char *flags = "w");
  TtyWidget *tty();

  CALLBACK_METHOD(close_tab);
  CALLBACK_METHOD(close_other_tabs);
  CALLBACK_METHOD(copy_text);
  CALLBACK_METHOD(cut_text);
  CALLBACK_METHOD(editor_plugin);
  CALLBACK_METHOD(export_file);
  CALLBACK_METHOD(font_size_decr);
  CALLBACK_METHOD(font_size_incr);
  CALLBACK_METHOD(help_about);
  CALLBACK_METHOD(help_app);
  CALLBACK_METHOD(help_contents);
  CALLBACK_METHOD(help_contents_brief);
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
  CALLBACK_METHOD(set_options);
  CALLBACK_METHOD(set_theme);
  CALLBACK_METHOD(tool_plugin);

  HelpView *getHelp();
  Fl_Group *createTab(GroupWidgetEnum groupWidgetEnum, const char *label = NULL);

  strlib::String _siteHome;
  strlib::String _exportFile;

  // display system
  GraphicsWidget *_out;
  Runtime *_system;

  // main output
  Fl_Group *_outputGroup;

  EditorWidget *_runEditWidget;

  // tab parent
  Fl_Tabs *_tabGroup;

  // configuration
  Profile *_profile;

  // the system menu
  Fl_Menu_Bar *_menuBar;
};

#endif
