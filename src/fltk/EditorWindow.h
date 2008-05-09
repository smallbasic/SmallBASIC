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

#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

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

#define CALLBACK_METHOD(FN)                     \
  void FN();                                    \
  static void FN ## _cb(Widget* w, void *v) {   \
    ((EditorWindow *) v)->FN();                 \
  }

class EditorWindow : public Group {
public:
  EditorWindow(int x, int y, int w, int h);
   ~EditorWindow();

  int handle(int e);
  bool isDirty() {
    return dirty;
  }
  const char *getFilename() {
    return filename;
  }

  int getFontSize();
  bool checkSave(bool discard);
  bool findText(const char *find, bool forward);
  void createFuncList();
  void doChange(int inserted, int deleted);
  void doSaveFile(const char *newfile, bool updateUI);
  void findFunc(const char *find);
  void getKeywords(strlib::List& keywords);
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  void gotoLine(int line);
  void loadFile(const char *newfile, int ipos, bool updateUI);
  void setFontSize(int i);
  void setIndentLevel(int level);
  void undo();

  static void undo_cb(Widget *, void *v) {
    TextEditor::kf_undo(0, ((EditorWindow *) v)->editor);
  }

  static void cut_cb(Widget *, void *v) {
    TextEditor::kf_cut(0, ((EditorWindow *) v)->editor);
  }

  static void paste_cb(Widget *, void *v) {
    TextEditor::kf_paste(0, ((EditorWindow *) v)->editor);
  }

  static void copy_cb(Widget *, void *v) {
    TextEditor::kf_copy(0, ((EditorWindow *) v)->editor);
  }

  static void changed_cb(int, int inserted, int deleted, int, const char *, void *v) {
    ((EditorWindow *) v)->doChange(inserted, deleted);
  }

  CALLBACK_METHOD(cancelReplace);
  CALLBACK_METHOD(doDelete);
  CALLBACK_METHOD(insertFile);
  CALLBACK_METHOD(newFile);
  CALLBACK_METHOD(openFile);
  CALLBACK_METHOD(replaceAll);
  CALLBACK_METHOD(replaceNext);
  CALLBACK_METHOD(saveFile);
  CALLBACK_METHOD(saveFileAs);
  CALLBACK_METHOD(showFindReplace);

  TextEditor *editor;
  bool readonly();
  void readonly(bool is_readonly);
  bool isLoading() {
    return loading;
  }

protected:
  void handleFileChange();
  ulong getModifiedTime();
  void reloadFile();

private:
  char filename[PATH_MAX];
  bool dirty;
  bool loading;
  Window *replaceDlg;
  Input *replaceFind;
  Input *replaceWith;
  ulong modifiedTime;
};

#endif
