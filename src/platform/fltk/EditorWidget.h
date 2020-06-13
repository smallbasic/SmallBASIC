// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include <stdint.h>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Choice.H>
#include <limits.h>
#include "ui/strlib.h"
#include "platform/fltk/TtyWidget.h"
#include "platform/fltk/BasicEditor.h"
#include "platform/fltk/utils.h"

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

struct EditorWidget;
EditorWidget *get_editor();

#define CALLBACK_METHOD(FN)                      \
  void FN(Fl_Widget*w=0, void *v=0);             \
  static void FN ## _cb(Fl_Widget *w, void *v) { \
    EditorWidget *e = get_editor();              \
    if (e) e->FN(w, v);                          \
  }

enum RunMessage {
  rs_err,
  rs_run,
  rs_ready
};

enum StyleField {
  st_text = 0,
  st_comments,
  st_strings,
  st_keywords,
  st_funcs,
  st_subs,
  st_findMatches,
  st_icomments,
  st_numbers,
  st_operators,
  st_selection,
  st_background,
  st_lineNumbers
};

enum CommandOpt {
  cmd_find = 0,
  cmd_find_inc,
  cmd_replace,
  cmd_replace_with,
  cmd_goto,
  cmd_input_text,
};

struct EditorWidget : public Fl_Group, StatusBar {
  EditorWidget(Fl_Widget *rect, Fl_Menu_Bar *menu);
  virtual ~EditorWidget();

  CALLBACK_METHOD(change_case);
  CALLBACK_METHOD(command);
  CALLBACK_METHOD(command_opt);
  CALLBACK_METHOD(cut_text);
  CALLBACK_METHOD(do_delete);
  CALLBACK_METHOD(expand_word);
  CALLBACK_METHOD(find);
  CALLBACK_METHOD(func_list);
  CALLBACK_METHOD(goto_line);
  CALLBACK_METHOD(paste_text);
  CALLBACK_METHOD(rename_word);
  CALLBACK_METHOD(replace_next);
  CALLBACK_METHOD(save_file);
  CALLBACK_METHOD(scroll_lock);
  CALLBACK_METHOD(select_all);
  CALLBACK_METHOD(set_color);
  CALLBACK_METHOD(show_replace);
  CALLBACK_METHOD(undo);
  CALLBACK_METHOD(un_select);
  CALLBACK_METHOD(clear_console);

  static void changed_cb(int, int inserted, int deleted, int, const char *, void *v) {
    ((EditorWidget *) v)->doChange(inserted, deleted);
  }

  bool checkSave(bool discard);
  void copyText();
  void doSaveFile(const char *newfile, bool force=false);
  void fileChanged(bool loadfile);
  bool focusWidget();
  const char *getFilename() { return _filename; }
  int getFontSize();
  void getInput(char *result, int size);
  void getRowCol(int *row, int *col);
  char *getSelection(int *start, int *end);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  void gotoLine(int line);
  int handle(int e);
  bool isDirty() { return _dirty; }
  void loadFile(const char *newfile);
  bool readonly();
  void readonly(bool is_readonly);
  void runState(RunMessage runMessage);
  void saveSelection(const char *path);
  void setBreakToLine(bool b) { _gotoLineBn->value(b); }
  void setTheme(EditTheme *theme);
  void setFont(Fl_Font font);
  void setFontSize(int i);
  void setHideIde(bool b) { _hideIdeBn->value(b); if (b) setLogPrint(!b); }
  void setIndentLevel(int level);
  void setLogPrint(bool b) { _logPrintBn->value(b); if (b) setHideIde(!b); }
  void setRowCol(int row, int col);
  void setScrollLock(bool b) { _lockBn->value(b); _tty->setScrollLock(b); }
  void showPath();
  void statusMsg(const char *msg);
  void updateConfig(EditorWidget *current);
  bool isBreakToLine() { return _gotoLineBn->value(); }
  bool isHideIDE() { return _hideIdeBn->value(); }
  bool isLoading() { return _loading; }
  bool isLogPrint() { return _logPrintBn->value(); }
  bool isScrollLock() { return _lockBn->value(); }
  void selectAll() { _editor->_textbuf->select(0, _editor->_textbuf->length()); }
  const char *data() { return _editor->_textbuf->text(); }
  int dataLength() { return _editor->_textbuf->length(); }
  int top_line() { return _editor->top_line(); }
  Fl_Text_Editor *getEditor() { return _editor; }
  TtyWidget *getTty() { return _tty; }

protected:
  void addHistory(const char *filename);
  void createFuncList();
  void doChange(int inserted, int deleted);
  void findFunc(const char *find);
  char *getSelection(Fl_Rect *rc);
  void getKeywords(strlib::List<String *> &keywords);
  uint32_t getModifiedTime();
  void handleFileChange();
  void resetList();
  void resize(int x, int y, int w, int h);
  void newFile();
  void reloadFile();
  int replaceAll(const char *find, const char *replace, bool restorePos, bool matchWord);
  bool searchBackward(const char *text, int startPos, const char *find, int findLen, int *foundPos);
  void selectRowInBrowser(int row);
  void setCommand(CommandOpt command);
  void setModified(bool dirty);
  void showFindText(const char *text);

private:
  BasicEditor *_editor;
  TtyWidget *_tty;

  char _filename[PATH_MAX];
  bool _dirty;
  bool _loading;
  uint32_t _modifiedTime;

  // tool-bar
  Fl_Group *_statusBar;
  Fl_Input *_commandText;
  Fl_Widget *_rowStatus;
  Fl_Widget *_colStatus;
  Fl_Button *_runStatus;
  Fl_Button *_modStatus;
  Fl_Tree *_funcList;
  bool _funcListEvent;

  Fl_Toggle_Button *_logPrintBn;
  Fl_Toggle_Button *_lockBn;
  Fl_Toggle_Button *_hideIdeBn;
  Fl_Toggle_Button *_gotoLineBn;

  // same order as display items
  CommandOpt _commandOpt;
  Fl_Button *_commandChoice;

  strlib::String _commandBuffer;

  Fl_Menu_Bar *_menuBar;
};

#endif
