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

const char* configFile = "config.txt";
const char* pathKey = "path";
const char* indentLevelKey = "indentLevel";
const char* logPrintKey = "logPrint";
const char* scrollLockKey = "scrollLock";
const char* hideIdeKey = "hideIde";
const char* gotoLineKey = "gotoLine";
const char* fontNameKey = "fontName";
const char* fontSizeKey = "fontSize";

// in BasicEditor.cxx
extern TextDisplay::StyleTableEntry styletable[];

//
// Profile constructor
//
Profile::Profile() {
  // defaults
  indentLevel = 2;
  logPrint = 0;
  scrollLock = 0;
  hideIde = 0;
  gotoLine = 0;
  font = COURIER;
  fontSize = 12;
  color = NO_COLOR;
}

//
// setup the editor defaults
//
void Profile::loadConfig(EditorWidget* editor) {
  editor->setIndentLevel(indentLevel);
  editor->setFont(font);
  editor->setFontSize(fontSize);
  editor->setHideIde(hideIde);
  editor->setLogPrint(logPrint);
  editor->setScrollLock(scrollLock);
  editor->setBreakToLine(gotoLine);
  
  if (color != NO_COLOR) {
    editor->setEditorColor(color, false);
  }
}

//
// restore saved settings
//
void Profile::restore(MainWindow* wnd) {
  strlib::String buffer;
  strlib::Properties profile;
  long len;

  FILE *fp = wnd->openConfig(configFile, "r");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    rewind(fp);
    buffer.append(fp, len);
    fclose(fp);
    profile.load(buffer.toString(), buffer.length());

    restoreValue(&profile, indentLevelKey, &indentLevel);
    restoreValue(&profile, logPrintKey, &logPrint);
    restoreValue(&profile, scrollLockKey, &scrollLock);
    restoreValue(&profile, hideIdeKey, &hideIde);
    restoreValue(&profile, gotoLineKey, &gotoLine);
    restoreStyles(&profile);
    restoreTabs(wnd, &profile);
  }
}

//
// load any stored font or color settings
//
void Profile::restoreStyles(strlib::Properties* profile) {
  // restore size and face
  restoreValue(profile, fontSizeKey, &fontSize);
  String* fontName = profile->get(fontNameKey);
  if (fontName) {
    font = fltk::font(fontName->toString());
  }

  for (int i = 0; i <= st_background; i++) {
    char buffer[4];
    sprintf(buffer, "%02d", i);
    String* color = profile->get(buffer);
    if (color) {
      Color c = fltk::color(color->toString());
      if (c != NO_COLOR) {
        if (i == st_background) {
          this->color = c;
        }
        else {
          styletable[i].color = c;
        }
      }
    }
  }
}

//
// restore the editor tabs
//
void Profile::restoreTabs(MainWindow* wnd, strlib::Properties* profile) {
  bool usedEditor = false;
  strlib::List paths;
  profile->get(pathKey, &paths);
  Object** list = paths.getList();
  int len = paths.length();

  for (int i = 0; i < len; i++) {
    const char* path = ((String *) list[i])->toString();
    EditorWidget* editor = 0;
    if (usedEditor) {
      // constructor will call loadConfig
      Group* group = wnd->createEditor(path);
      editor = wnd->getEditor(group);
    }
    else {
      // load into the initial buffer
      editor = wnd->getEditor(true);
      loadConfig(editor);
      usedEditor = true;
    }
    editor->loadFile(path);
  }
}

//
// restore the int value
//
void Profile::restoreValue(strlib::Properties* p, const char* key, int* value) {
  String* s = p->get(key);
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
    saveValue(fp, indentLevelKey, indentLevel);
    saveValue(fp, logPrintKey, logPrint);
    saveValue(fp, scrollLockKey, scrollLock);
    saveValue(fp, hideIdeKey, hideIde);
    saveValue(fp, gotoLineKey, gotoLine);
    saveStyles(fp);
    saveTabs(wnd, fp);

    fclose(fp);
  }
}

//
// saves the current font size, face and colour configuration
//
void Profile::saveStyles(FILE *fp) {
  char buffer[MAX_PATH];
  int err;
  uchar r,g,b;

  saveValue(fp, fontSizeKey, (int) styletable[0].size);
  saveValue(fp, fontNameKey, styletable[0].font->name());
  
  for (int i = 0; i <= st_background; i++) {
    split_color(i == st_background ? color : styletable[i].color, r,g,b);
    sprintf(buffer, "%02d=#%02x%02x%02x\n", i, r,g,b);
    err = fwrite(buffer, strlen(buffer), 1, fp);
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
  err = fwrite(s.toString(), s.length(), 1, fp);
}

// End of "$Id$".
