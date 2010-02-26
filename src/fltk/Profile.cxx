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
#include <fltk/Monitor.h>

#include "Profile.h"
#include "MainWindow.h"

const char* configFile = "config.txt";
const char* pathKey = "path";
const char* indentLevelKey = "indentLevel";
const char* fontNameKey = "fontName";
const char* fontSizeKey = "fontSize";
const char* windowPosKey = "windowPos";
const char* activeTabKey = "activeTab";
const char* createBackupsKey = "createBackups";
const char* lineNumbersKey = "lineNumbers";

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
  color = WHITE;
  loaded = false;
  createBackups = true;
  lineNumbers = true;
}

//
// setup the editor defaults
//
void Profile::loadConfig(EditorWidget* editWidget) {
  editWidget->setIndentLevel(indentLevel);
  editWidget->setFont(font);
  editWidget->setFontSize(fontSize);
  editWidget->setEditorColor(color, false);
  editWidget->editor->linenumber_width(lineNumbers ? 40 : 1);
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
    restoreValue(&profile, createBackupsKey, &createBackups);
    restoreValue(&profile, lineNumbersKey, &lineNumbers);
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
    // prevent overwriting config when not initially used
    FILE *fp = wnd->openConfig(configFile);
    if (fp) {
      saveValue(fp, indentLevelKey, indentLevel);
      saveValue(fp, createBackupsKey, createBackups);
      saveValue(fp, lineNumbersKey, lineNumbers);
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
int Profile::nextInteger(const char* s, int len, int& index) {
  int result = 0;
  while (isdigit(s[index]) && index < len) {
    result = (result * 10) + (s[index] - '0');
    index++;
  }
  if (s[index] == ';') {
    index++;
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
    int len = strlen(buffer);
    int logPrint = nextInteger(buffer, len, index);
    int scrollLock = nextInteger(buffer, len, index);
    int hideIde = nextInteger(buffer, len, index);
    int gotoLine = nextInteger(buffer, len, index);
    int insertPos = nextInteger(buffer, len, index);
    int topLineNo = nextInteger(buffer, len, index);

    const char* path = buffer + index;

    EditorWidget* editWidget = 0;
    if (usedEditor) {
      // constructor will call loadConfig
      Group* group = wnd->createEditor(path);
      editWidget = wnd->getEditor(group);
    }
    else {
      // load into the initial buffer
      editWidget = wnd->getEditor(true);
      loadConfig(editWidget);
      usedEditor = true;
    }

    editWidget->loadFile(path);
    editWidget->setHideIde(hideIde);
    editWidget->setLogPrint(logPrint);
    editWidget->setScrollLock(scrollLock);
    editWidget->setBreakToLine(gotoLine);
    editWidget->editor->insert_position(insertPos);
    editWidget->editor->show_insert_position();
    editWidget->editor->scroll(topLineNo, 0);
  }

  // restore the active tab
  String* activeTab = profile->get(activeTabKey);
  if (activeTab != null) {
    EditorWidget* editWidget = wnd->getEditor(activeTab->toString());
    if (editWidget) {
      wnd->showEditTab(editWidget);
    }
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
  String* windowPos = profile->get(windowPosKey);
  if (windowPos != null) {
    const char* buffer = windowPos->toString();
    int index = 0;
    int len = strlen(buffer);

    int x = nextInteger(buffer, len, index);
    int y = nextInteger(buffer, len, index);
    int w = nextInteger(buffer, len, index);
    int h = nextInteger(buffer, len, index);

    if (x > 0 && y > 0 && w > 100 && h > 100) {
      const Monitor& monitor = Monitor::all();
      if (x < monitor.w() && y < monitor.h()) {
        wnd->resize(x, y, w, h);
      }
    }
  }
}

//
// saves the current font size, face and colour configuration
//
void Profile::saveStyles(FILE *fp) {
  uchar r,g,b;

  saveValue(fp, fontSizeKey, (int) styletable[0].size);
  saveValue(fp, fontNameKey, styletable[0].font->name());
  
  for (int i = 0; i <= st_background; i++) {
    split_color(i == st_background ? color : styletable[i].color, r,g,b);
    fprintf(fp, "%02d=#%02x%02x%02x\n", i, r,g,b);
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
      EditorWidget* editWidget = (EditorWidget*) group->child(0);

      bool logPrint = editWidget->isLogPrint();
      bool scrollLock = editWidget->isScrollLock();
      bool hideIde =  editWidget->isHideIDE();
      bool gotoLine = editWidget->isBreakToLine();
      int insertPos = editWidget->editor->insert_position();
      int topLineNo = editWidget->editor->top_line();

      fprintf(fp, "%s='%d;%d;%d;%d;%d;%d;%s'\n", pathKey, 
              logPrint, scrollLock, hideIde, gotoLine, insertPos, topLineNo,
              editWidget->getFilename());
    }
  }

  // save the active tab
  EditorWidget* editWidget = wnd->getEditor(false);
  if (editWidget) {
    saveValue(fp, activeTabKey, editWidget->getFilename());
  }
}

//
// persist a single value
//
void Profile::saveValue(FILE* fp, const char* key, const char* value) {
  fprintf(fp, "%s='%s'\n", key, value);
}

//
// persist a single value
//
void Profile::saveValue(FILE* fp, const char* key, int value) {
  fprintf(fp, "%s=%d\n", key, value);
}

//
// save the main window position
//
void Profile::saveWindowPos(FILE* fp, MainWindow* wnd) {
  fprintf(fp, "%s=%d;%d;%d;%d\n", windowPosKey, 
          wnd->x(), wnd->y(), wnd->w(), wnd->h());
}

// End of "$Id$".
