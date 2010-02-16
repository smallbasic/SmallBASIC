// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <unistd.h>

#include "Profile.h"
#include "MainWindow.h"

const char* configFile = "profile.txt";
const char* pathKey = "path";
const char* indentLevelKey = "indentLevel";
const char* ttyRowsKey = "ttyRows";
const char* logPrintKey = "logPrint";
const char* scrollLockKey = "scrollLock";
const char* hideIdeKey = "hideIde";
const char* gotoLineKey = "gotoLine";

//
// Profile constructor
//
Profile::Profile() {
  // defaults
  indentLevel = 2;
  ttyRows = 1000;
  logPrint = false;
  scrollLock = false;
  hideIde = false;
  gotoLine = false;
}

//
// restore saved settings
//
void Profile::restore(MainWindow* wnd) {
  strlib::String buffer;
  strlib::Properties profile;
  strlib::List paths;
  long len;

  FILE *fp = wnd->openConfig(configFile, "r");
  if (fp) {
    // load the entire file
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    rewind(fp);
    buffer.append(fp, len);
    fclose(fp);

    profile.load(buffer.toString(), buffer.length());
    profile.get(pathKey, &paths);

    restoreTabs(wnd, &paths);
    restoreValue(&profile, indentLevelKey, &indentLevel);
    restoreValue(&profile, ttyRowsKey, &ttyRows);
    restoreValue(&profile, logPrintKey, &logPrint);
    restoreValue(&profile, scrollLockKey, &scrollLock);
    restoreValue(&profile, hideIdeKey, &hideIde);
    restoreValue(&profile, gotoLineKey, &gotoLine);
  }
}

//
// restore the editor tabs
//
void Profile::restoreTabs(MainWindow* wnd, strlib::List* paths) {
  bool usedEditor = false;

  Object** list = paths->getList();
  int len = paths->length();
  for (int i = 0; i < len; i++) {
    const char* path = ((String *) list[i])->toString();
    EditorWidget* editor = 0;
    if (usedEditor) {
      Group* group = wnd->createEditor(path);
      editor = wnd->getEditor(group);
    }
    else {
      // load into the initial buffer
      editor = wnd->getEditor(true);
      usedEditor = true;
    }
    editor->loadFile(path);
  }
}

//
// restore the int value
//
void Profile::restoreValue(strlib::Properties* p, const char* key, int* value) {
  String* s = p->get(indentLevelKey);
  if (s) {
    *value = s->toInteger();
  }
}

//
// persist profile values
//
void Profile::save(MainWindow* wnd) {
  // remember the last edited file
  FILE *fp = wnd->openConfig(configFile);
  if (fp) {
    saveTabs(wnd, fp);

    saveValue(fp, indentLevelKey, indentLevel);
    saveValue(fp, ttyRowsKey, ttyRows);
    saveValue(fp, logPrintKey, logPrint);
    saveValue(fp, scrollLockKey, scrollLock);
    saveValue(fp, hideIdeKey, hideIde);
    saveValue(fp, gotoLineKey, gotoLine);

    fclose(fp);
  }
}

//
// persist the editor tabs
//
void Profile::saveTabs(MainWindow* wnd, FILE* fp) {
  // write tabs
  int n = wnd->tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group* group = (Group*) wnd->tabGroup->child(c);
    if (gw_editor == ((GroupWidget) (int)group->user_data())) {
      EditorWidget* editWidget = (EditorWidget*) group->child(0);
      saveValue(fp, pathKey, editWidget->getFilename());
    }
  }
}

//
// persist a single value
//
void Profile::saveValue(FILE* fp, const char* key, const char* value) {
  int err;
  err = fwrite(key, strlen(key), 1, fp);
  err = fwrite("=", 1, 1, fp);
  err = fwrite(value, strlen(value), 1, fp);
  err = fwrite("\n", 1, 1, fp);
}

//
// persist a single value
//
void Profile::saveValue(FILE* fp, const char* key, int value) {
  String s;
  int err;
  s.append(key).append("=").append(value).append("\n");
  err = fwrite(s, s.length(), 1, fp);
}

// End of "$Id$".
