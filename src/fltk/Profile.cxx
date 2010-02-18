// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <unistd.h>
#include <ctype.h>

#include "Profile.h"
#include "MainWindow.h"

const char* configFile = "config.txt";
const char* pathKey = "path";
const char* indentLevelKey = "indentLevel";
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
  font = COURIER;
  fontSize = 12;
  color = NO_COLOR;
  loaded = false;
}

//
// setup the editor defaults
//
void Profile::loadConfig(EditorWidget* editor) {
  editor->setIndentLevel(indentLevel);
  editor->setFont(font);
  editor->setFontSize(fontSize);
  
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
    restoreStyles(&profile);
    restoreWindowPos(wnd, &profile);
    restoreTabs(wnd, &profile);
  }
  loaded = true;
}

//
// persist profile values
//
void Profile::save(MainWindow* wnd) {
  if (loaded) {
    FILE *fp = wnd->openConfig(configFile);
    if (fp) {
      saveValue(fp, indentLevelKey, indentLevel);
      saveStyles(fp);
      saveTabs(fp, wnd);
      saveWindowPos(fp, wnd);
      fclose(fp);
    }
  }
}

//
// returns the next integer from the given string
//
int Profile::nextInteger(const char* s, int* index) {
  int result = 0;
  while (isdigit(s[*index])) {
    result = (result * 10) + (s[*index] - '0');
    (*index)++;
  }
  if (s[*index] == ';') {
    (*index)++;
  }
  return result;
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
    const char* buffer = ((String *) list[i])->toString();
    int index = 0;
    int logPrint = nextInteger(buffer, &index);
    int scrollLock = nextInteger(buffer, &index);
    int hideIde = nextInteger(buffer, &index);
    int gotoLine = nextInteger(buffer, &index);
    int insertPos = nextInteger(buffer, &index);

    const char* path = buffer + index;

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
    editor->setHideIde(hideIde);
    editor->setLogPrint(logPrint);
    editor->setScrollLock(scrollLock);
    editor->setBreakToLine(gotoLine);
    editor->editor->insert_position(insertPos);
    editor->editor->show_insert_position();
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
// restore the main window position
//
void Profile::restoreWindowPos(MainWindow* wnd, strlib::Properties* profile) {
  int x,y,w,h;

  x = y = w = h = -1;

  restoreValue(profile, "x", &x);
  restoreValue(profile, "y", &y);
  restoreValue(profile, "w", &w);
  restoreValue(profile, "h", &h);

  if (x > 1 && y > 1 && w > 100 && h > 100) {
    wnd->resize(x, y, w, h);
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
void Profile::saveTabs(FILE* fp, MainWindow* wnd) {
  int n = wnd->tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group* group = (Group*) wnd->tabGroup->child(c);
    if (gw_editor == ((GroupWidget) (int)group->user_data())) {
      EditorWidget* editor = (EditorWidget*) group->child(0);

      bool logPrint = editor->isLogPrint();
      bool scrollLock = editor->isScrollLock();
      bool hideIde =  editor->isHideIDE();
      bool gotoLine = editor->isBreakToLine();
      int insertPos = editor->editor->insert_position();
      
      String s;
      s.append(logPrint).append(";")
      .append(scrollLock).append(";")
      .append(hideIde).append(";")
      .append(gotoLine).append(";")
      .append(insertPos).append(";")
      .append(editor->getFilename());
      saveValue(fp, pathKey, s.toString());
    }
  }
}

//
// persist a single value
//
void Profile::saveValue(FILE* fp, const char* key, const char* value) {
  int err;
  err = fwrite(key, strlen(key), 1, fp);
  err = fwrite("='", 2, 1, fp);
  err = fwrite(value, strlen(value), 1, fp);
  err = fwrite("'\n", 2, 1, fp);
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

//
// save the main window position
//
void Profile::saveWindowPos(FILE* fp, MainWindow* wnd) {
  saveValue(fp, "x", wnd->x());
  saveValue(fp, "y", wnd->y());
  saveValue(fp, "w", wnd->w());
  saveValue(fp, "h", wnd->h());
}

// End of "$Id$".
