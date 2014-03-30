// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <unistd.h>
#include <ctype.h>
#include <fltk/Monitor.h>

#include "Profile.h"
#include "MainWindow.h"

const char *configFile = "config.txt";
const char *pathKey = "path";
const char *indentLevelKey = "_indentLevel";
const char *fontNameKey = "fontName";
const char *fontSizeKey = "_fontSize";
const char *windowPosKey = "windowPos";
const char *activeTabKey = "activeTab";
const char *createBackupsKey = "_createBackups";
const char *lineNumbersKey = "_lineNumbers";
const char *appPositionKey = "_appPosition";

// in BasicEditor.cxx
extern TextDisplay::StyleTableEntry styletable[];

//
// Profile constructor
//
Profile::Profile() : 
  _color(WHITE),
  _font(COURIER),
  _appPosition(0, 0, 640, 480),
  _fontSize(12),
  _indentLevel(2),
  _createBackups(true),
  _lineNumbers(true),
  _loaded(false) {
}

//
// setup the editor defaults
//
void Profile::loadConfig(EditorWidget *editWidget) {
  editWidget->setIndentLevel(_indentLevel);
  editWidget->setFont(_font);
  editWidget->setFontSize(_fontSize);
  editWidget->setEditorColor(_color, false);
  editWidget->editor->linenumber_width(_lineNumbers ? 40 : 1);
}

//
// restore saved settings
//
void Profile::restore(MainWindow *wnd) {
  strlib::String buffer;
  Properties profile;
  long len;

  FILE *fp = wnd->openConfig(configFile, "r");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    rewind(fp);
    buffer.append(fp, len);
    fclose(fp);
    profile.load(buffer.toString(), buffer.length());

    restoreValue(&profile, indentLevelKey, &_indentLevel);
    restoreValue(&profile, createBackupsKey, &_createBackups);
    restoreValue(&profile, lineNumbersKey, &_lineNumbers);
    restoreStyles(&profile);

    Rectangle rc;
    rc = restoreRect(&profile, appPositionKey);
    if (!rc.empty()) {
      _appPosition = rc;
    }

    rc = restoreRect(&profile, windowPosKey);
    if (!rc.empty()) {
      restoreWindowPos(wnd, rc);
    }

    restoreTabs(wnd, &profile);
  }
  _loaded = true;
}

//
// restore the standalone window position
//
void Profile::restoreAppPosition(Rectangle *wnd) {
  if (!_appPosition.empty()) {
    wnd->w(_appPosition.w());
    wnd->h(_appPosition.h());
    if (_appPosition.x() != 0) {
      wnd->x(_appPosition.x());
      wnd->y(_appPosition.y());
    }
  }
}

//
// persist profile values
//
void Profile::save(MainWindow *wnd) {
  if (_loaded) {
    // prevent overwriting config when not initially used
    FILE *fp = wnd->openConfig(configFile);
    if (fp) {
      saveValue(fp, indentLevelKey, _indentLevel);
      saveValue(fp, createBackupsKey, _createBackups);
      saveValue(fp, lineNumbersKey, _lineNumbers);
      saveStyles(fp);
      saveTabs(fp, wnd);
      saveRect(fp, appPositionKey, &_appPosition);
      saveRect(fp, windowPosKey, wnd);
      fclose(fp);
    }
  }
}

//
// returns the next integer from the given string
//
int Profile::nextInteger(const char *s, int len, int &index) {
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
// restore a rectangle value with the given key
//
Rectangle Profile::restoreRect(Properties *profile, const char *key) {
  Rectangle result(0, 0, 0, 0);
  String *value = profile->get(key);
  if (value != null) {
    const char *buffer = value->toString();
    int index = 0;
    int len = strlen(buffer);

    result.x(nextInteger(buffer, len, index));
    result.y(nextInteger(buffer, len, index));
    result.w(nextInteger(buffer, len, index));
    result.h(nextInteger(buffer, len, index));
  }
  return result;
}

//
// load any stored font or color settings
//
void Profile::restoreStyles(Properties *profile) {
  // restore size and face
  restoreValue(profile, fontSizeKey, &_fontSize);
  String *fontName = profile->get(fontNameKey);
  if (fontName) {
    _font = fltk::font(fontName->toString());
  }

  for (int i = 0; i <= st_background; i++) {
    char buffer[4];
    sprintf(buffer, "%02d", i);
    String *color = profile->get(buffer);
    if (color) {
      Color c = fltk::color(color->toString());
      if (c != NO_COLOR) {
        if (i == st_background) {
          this->_color = c;
        } else {
          styletable[i].color = c;
        }
      }
    }
  }
}

//
// restore the editor tabs
//
void Profile::restoreTabs(MainWindow *wnd, Properties *profile) {
  bool usedEditor = false;
  strlib::List<String *> paths;
  profile->get(pathKey, &paths);

  List_each(String*, it, paths) {
    String *nextString = (*it);
    const char *buffer = nextString->toString();
    int index = 0;
    int len = strlen(buffer);
    int logPrint = nextInteger(buffer, len, index);
    int scrollLock = nextInteger(buffer, len, index);
    int hideIde = nextInteger(buffer, len, index);
    int gotoLine = nextInteger(buffer, len, index);
    int insertPos = nextInteger(buffer, len, index);
    int topLineNo = nextInteger(buffer, len, index);

    const char *path = buffer + index;

    EditorWidget *editWidget = 0;
    if (usedEditor) {
      // constructor will call loadConfig
      Group *group = wnd->createEditor(path);
      editWidget = wnd->getEditor(group);
    } else {
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
  String *activeTab = profile->get(activeTabKey);
  if (activeTab != null) {
    EditorWidget *editWidget = wnd->getEditor(activeTab->toString());
    if (editWidget) {
      wnd->showEditTab(editWidget);
    }
  }
}

//
// restore the int value
//
void Profile::restoreValue(Properties *p, const char *key, int *value) {
  String *s = p->get(key);
  if (s) {
    *value = s->toInteger();
  }
}

//
// restore the main window position
//
void Profile::restoreWindowPos(MainWindow *wnd, Rectangle &rc) {
  int x = rc.x();
  int y = rc.y();
  int w = rc.w();
  int h = rc.h();

  if (x > 0 && y > 0 && w > 100 && h > 100) {
    const Monitor & monitor = Monitor::all();
    if (x < monitor.w() && y < monitor.h()) {
      wnd->resize(x, y, w, h);
    }
  }
}

//
// save the window position
//
void Profile::saveRect(FILE *fp, const char *key, Rectangle *rc) {
  fprintf(fp, "%s=%d;%d;%d;%d\n", key, rc->x(), rc->y(), rc->w(), rc->h());
}

//
// saves the current font size, face and colour configuration
//
void Profile::saveStyles(FILE *fp) {
  uchar r, g, b;

  saveValue(fp, fontSizeKey, (int)styletable[0].size);
  saveValue(fp, fontNameKey, styletable[0].font->name());

  for (int i = 0; i <= st_background; i++) {
    split_color(i == st_background ? _color : styletable[i].color, r, g, b);
    fprintf(fp, "%02d=#%02x%02x%02x\n", i, r, g, b);
  }
}

//
// persist the editor tabs
//
void Profile::saveTabs(FILE *fp, MainWindow *wnd) {
  int n = wnd->_tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group *group = (Group *) wnd->_tabGroup->child(c);
    if (gw_editor == ((GroupWidget) (intptr_t) group->user_data())) {
      EditorWidget *editWidget = (EditorWidget *) group->child(0);

      bool logPrint = editWidget->isLogPrint();
      bool scrollLock = editWidget->isScrollLock();
      bool hideIde = editWidget->isHideIDE();
      bool gotoLine = editWidget->isBreakToLine();
      int insertPos = editWidget->editor->insert_position();
      int topLineNo = editWidget->editor->top_line();

      fprintf(fp, "%s='%d;%d;%d;%d;%d;%d;%s'\n", pathKey,
              logPrint, scrollLock, hideIde, gotoLine, insertPos, topLineNo, editWidget->getFilename());
    }
  }

  // save the active tab
  EditorWidget *editWidget = wnd->getEditor(false);
  if (editWidget) {
    saveValue(fp, activeTabKey, editWidget->getFilename());
  }
}

//
// persist a single value
//
void Profile::saveValue(FILE *fp, const char *key, const char *value) {
  fprintf(fp, "%s='%s'\n", key, value);
}

//
// persist a single value
//
void Profile::saveValue(FILE *fp, const char *key, int value) {
  fprintf(fp, "%s=%d\n", key, value);
}
