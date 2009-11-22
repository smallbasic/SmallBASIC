// $Id$
//
// Based on test/editor.cxx - A simple text editor program for the Fast 
// Light Tool Kit (FLTK). This program is described in Chapter 4 of the FLTK 
// Programmer's Guide.
// Copyright 1998-2003 by Bill Spitzak and others.
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include <fltk/Window.h>
#include <fltk/Widget.h>
#include <fltk/Input.h>
#include <fltk/ToggleButton.h>
#include <fltk/Button.h>
#include <fltk/TextEditor.h>
#include <fltk/Choice.h>
#include <limits.h>

#include "StringLib.h"
#include "TtyWidget.h"

using namespace fltk;

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

struct EditorWidget;
EditorWidget* get_editor();

#define CALLBACK_METHOD(FN)                     \
  void FN(Widget* w=0, void *v=0);              \
  static void FN ## _cb(Widget* w, void *v) {   \
    EditorWidget* e = get_editor();             \
    if (e) e->FN(w, v);                         \
  }

enum RunMessage {
  msg_err,
  msg_run,
  msg_none
};

enum StyleField {
  st_text=0,
  st_comments,
  st_strings,
  st_keywords,
  st_funcs,
  st_subs,
  st_findMatches,
  st_icomments,
  st_numbers,
  st_operators,
  st_background,
  st_background_def
};

enum CommandOpt {
  cmd_find=0,
  cmd_find_inc,
  cmd_replace,
  cmd_replace_with,
  cmd_goto,
};

struct CodeEditor : public TextEditor {
  CodeEditor(int x, int y, int w, int h);
  ~CodeEditor();

  bool findText(const char *find, bool forward, bool updatePos);
  int handle(int e);
  unsigned getIndent(char *indent, int len, int pos);
  void draw();
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  char* getSelection(Rectangle* rc);
  void gotoLine(int line);
  void handleTab();
  void showFindText(const char *text);
  void showMatchingBrace();
  void showRowCol();
  void styleChanged();
  void styleParse(const char *text, char *style, int length);
  
  bool readonly;
  int indentLevel;
  int matchingBrace;

  TextBuffer *stylebuf;
  TextBuffer *textbuf;
  char search[256];
};

class EditorWidget : public Group {
public:
  EditorWidget(int x, int y, int w, int h);
   ~EditorWidget();

  CALLBACK_METHOD(change_case);
  CALLBACK_METHOD(command);
  CALLBACK_METHOD(command_opt);
  CALLBACK_METHOD(cut_text);
  CALLBACK_METHOD(do_delete);
  CALLBACK_METHOD(expand_word);
  CALLBACK_METHOD(find);
  CALLBACK_METHOD(font_name);
  CALLBACK_METHOD(func_list);
  CALLBACK_METHOD(goto_line);
  CALLBACK_METHOD(log_print);
  CALLBACK_METHOD(paste_text);
  CALLBACK_METHOD(rename_word);
  CALLBACK_METHOD(replace_next);
  CALLBACK_METHOD(save_file);
  CALLBACK_METHOD(scroll_lock);
  CALLBACK_METHOD(set_color);
  CALLBACK_METHOD(show_replace);
  CALLBACK_METHOD(undo);

  static void changed_cb(int, int inserted, int deleted, int, const char *, void *v) {
    ((EditorWidget *) v)->doChange(inserted, deleted);
  }

  int handle(int e);
  bool isDirty() { return dirty; }
  const char *getFilename() { return filename; }
  bool checkSave(bool discard);
  void copyText();
  void doSaveFile(const char *newfile);
  void fileChanged(bool loadfile);
  void focusWidget();
  int getFontSize();
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  void gotoLine(int line);
  void loadFile(const char *newfile);
  void restoreEdit();
  void runMsg(RunMessage runMessage);
  void saveConfig();
  void saveSelection(const char* path);
  void setHideIde();
  void setFontSize(int i);
  void setIndentLevel(int level);
  void setRowCol(int row, int col);
  void showPath();
  void statusMsg(const char *msg);
  void updateConfig(EditorWidget* current);

  CodeEditor *editor;
  TtyWidget* tty;
  bool readonly();
  void readonly(bool is_readonly);
  bool isBreakToLine() { return breakLineBn->value(); }
  bool isHideIDE() { return hideIdeBn->value(); }
  bool isLoading() { return loading; }
  bool isLogPrint() { return logPrintBn->value(); }

protected:
  void createFuncList();
  void doChange(int inserted, int deleted);
  void findFunc(const char *find);
  char* getSelection(Rectangle* rc);
  const char* getFontName();
  void getKeywords(strlib::List& keywords);
  U32 getModifiedTime();
  void handleFileChange();
  void loadConfig();
  void newFile();
  void reloadFile();
  int  replaceAll(const char* find, const char* replace, bool restorePos, bool matchWord);
  bool searchBackward(const char *text, int startPos,
                      const char *find, int findLen, int *foundPos);
  void setColor(const char* label, StyleField field);
  void setCommand(CommandOpt command);
  void setEditorColor(Color c, bool defColor);
  void setFont(Font* font);
  void setModified(bool dirty);
  void showFindText(const char *text);

private:
  char filename[PATH_MAX];
  bool dirty;
  bool loading;
  U32 modifiedTime;

  // tool-bar
  Input* commandText;
  Widget* rowStatus;
  Widget* colStatus;
  Widget* runStatus;
  Widget* modStatus;
  Choice* funcList;

  ToggleButton* logPrintBn;
  ToggleButton* lockBn;
  ToggleButton* hideIdeBn;
  ToggleButton* breakLineBn;

  // same order as display items
  CommandOpt commandOpt;
  Widget* commandChoice;

  strlib::String commandBuffer;
};

#endif
