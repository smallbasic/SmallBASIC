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

// implemented in MainWindow
void statusMsg(const char *filename);
void setRowCol(int row, int col);
void setModified(bool dirty);
void addHistory(const char *fileName);
void fileChanged(bool loadfile);

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

  bool checkSave(bool discard);
  bool findText(const char *find, bool forward);
  int getFontSize();
  void cancelReplace();
  void createFuncList();
  void doChange(int inserted, int deleted);
  void doDelete();
  void doSaveFile(const char *newfile, bool updateUI);
  void findFunc(const char *find);
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  void gotoLine(int line);
  void insertFile();
  void loadFile(const char *newfile, int ipos, bool updateUI);
  void newFile();
  void openFile();
  void replaceAll();
  void replaceNext();
  void saveFile();
  void saveFileAs();
  void setFontSize(int i);
  void setIndentLevel(int level);
  void showFindReplace();
  void undo();
  void getKeywords(strlib::List& keywords);

  // editor callback functions
  static void new_cb(Widget *, void *v) {
    ((EditorWindow *) v)->newFile();
  }
  static void open_cb(Widget *, void *v) {
    ((EditorWindow *) v)->openFile();
  }
  static void insert_cb(Widget *, void *v) {
    ((EditorWindow *) v)->insertFile();
  }
  static void save_cb(Widget *, void *v) {
    ((EditorWindow *) v)->saveFile();
  }
  static void saveas_cb(Widget *, void *v) {
    ((EditorWindow *) v)->saveFileAs();
  }
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
  static void delete_cb(Widget *, void *v) {
    ((EditorWindow *) v)->doDelete();
  }
  static void replace_cb(Widget *, void *v) {
    ((EditorWindow *) v)->showFindReplace();
  }
  static void replace2_cb(Widget *, void *v) {
    ((EditorWindow *) v)->replaceNext();
  }
  static void replall_cb(Widget *, void *v) {
    ((EditorWindow *) v)->replaceAll();
  }
  static void replcan_cb(Widget *, void *v) {
    ((EditorWindow *) v)->cancelReplace();
  }
  static void changed_cb(int, int inserted, int deleted, int, const char *, void *v) {
    ((EditorWindow *) v)->doChange(inserted, deleted);
  }

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
