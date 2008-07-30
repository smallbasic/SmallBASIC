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
#include <fltk/ReturnButton.h>
#include <fltk/Button.h>
#include <fltk/TextEditor.h>
#include <fltk/Choice.h>
#include <limits.h>

#include "StringLib.h"

using namespace fltk;

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

struct EditorWidget;
EditorWidget* get_editor();

#define CALLBACK_METHOD(FN)                     \
  void FN(void *v=0);                           \
  static void FN ## _cb(Widget* w, void *v) {   \
    EditorWidget* e = get_editor();             \
    if (e) e->FN(v);                            \
  }

enum RunMessage {
  msg_err,
  msg_run,
  msg_none
};

struct CodeEditor : public TextEditor {
  CodeEditor(int x, int y, int w, int h);
  ~CodeEditor();

  bool findText(const char *find, bool forward);
  int handle(int e);
  unsigned getIndent(char *indent, int len, int pos);
  void draw();
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  void gotoLine(int line);
  void handleTab();
  void showMatchingBrace();
  void showRowCol();
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

  int handle(int e);
  bool isDirty() {
    return dirty;
  }
  const char *getFilename() {
    return filename;
  }

  bool checkSave(bool discard);
  int getFontSize();
  void createFuncList();
  void doChange(int inserted, int deleted);
  void doSaveFile(const char *newfile);
  void fileChanged(bool loadfile);
  void findFunc(const char *find);
  void focusWidget();
  void getKeywords(strlib::List& keywords);
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  void gotoLine(int line);
  void loadFile(const char *newfile);
  void newFile();
  void restoreEdit();
  void runMsg(RunMessage runMessage);
  void setFontSize(int i);
  void setIndentLevel(int level);
  void setModified(bool dirty);
  void setRowCol(int row, int col);
  void statusMsg(const char *filename);
  void undo();

  static void undo_cb(Widget *, void *v) {
    TextEditor::kf_undo(0, ((EditorWidget *) v)->editor);
  }

  static void cut_cb(Widget *, void *v) {
    TextEditor::kf_cut(0, ((EditorWidget *) v)->editor);
  }

  static void paste_cb(Widget *, void *v) {
    TextEditor::kf_paste(0, ((EditorWidget *) v)->editor);
  }

  static void copy_cb(Widget *, void *v) {
    TextEditor::kf_copy(0, ((EditorWidget *) v)->editor);
  }

  static void changed_cb(int, int inserted, int deleted, int, const char *, void *v) {
    ((EditorWidget *) v)->doChange(inserted, deleted);
  }

  CALLBACK_METHOD(cancelReplace);
  CALLBACK_METHOD(doDelete);
  CALLBACK_METHOD(find);
  CALLBACK_METHOD(func_list);
  CALLBACK_METHOD(goto_line);
  CALLBACK_METHOD(replaceAll);
  CALLBACK_METHOD(replaceNext);
  CALLBACK_METHOD(saveFile);
  CALLBACK_METHOD(saveFileAs);
  CALLBACK_METHOD(showFindReplace);

  CodeEditor *editor;
  bool readonly();
  void readonly(bool is_readonly);
  bool isLoading() {
    return loading;
  }

protected:
  void handleFileChange();
  U32 getModifiedTime();
  void reloadFile();

private:
  char filename[PATH_MAX];
  bool dirty;
  bool loading;
  U32 modifiedTime;

  // tool-bar
  Input* findTextInput;
  Input* gotoLineInput;
  Choice* funcList;

  // status bar
  Widget* fileStatus;
  Widget* rowStatus;
  Widget* colStatus;
  Widget* runStatus;
  Widget* modStatus;
};

#endif
