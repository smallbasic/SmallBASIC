// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>
#include <unistd.h>
#include <ctype.h>
#include "platform/fltk/Profile.h"
#include "platform/fltk/MainWindow.h"
#include "platform/fltk/utils.h"

const char *configFile = "fl_config.txt";
const char *pathKey = "path";
const char *indentLevelKey = "indentLevel";
const char *fontNameKey = "fontName";
const char *fontSizeKey = "fontSize";
const char *windowPosKey = "windowPos";
const char *activeTabKey = "activeTab";
const char *createBackupsKey = "createBackups";
const char *lineNumbersKey = "lineNumbers";
const char *appPositionKey = "appPosition";
const char *themeIdKey = "themeId";
const char *helpThemeIdKey = "helpThemeId";

// in BasicEditor.cxx
extern Fl_Text_Display::Style_Table_Entry styletable[];

static StyleField lastStyle = st_lineNumbers;

//
// Profile constructor
//
Profile::Profile() :
  _font(FL_COURIER),
  _appPosition(0, 0, 640, 480),
  _loaded(false),
  _createBackups(true),
  _lineNumbers(true),
  _fontSize(DEF_FONT_SIZE),
  _indentLevel(2),
  _helpThemeId(0) {
  loadEditTheme(0);
  _helpTheme.setId(_helpThemeId);
}

//
// setup the editor defaults
//
void Profile::loadConfig(EditorWidget *editWidget) {
  editWidget->setIndentLevel(_indentLevel);
  editWidget->setFont(_font);
  editWidget->setFontSize(_fontSize);
  editWidget->setTheme(&_theme);
  editWidget->getEditor()->linenumber_width(_lineNumbers ? LINE_NUMBER_WIDTH : 1);
}

//
// select the given theme
//
void Profile::loadEditTheme(int themeId) {
  _theme.setId(themeId);
  _themeId = themeId;
  styletable[0].color = get_color(_theme._color); // A - plain
  styletable[1].color = get_color(_theme._syntax_comments); // B - comments
  styletable[2].color = get_color(_theme._syntax_text); // C - string
  styletable[3].color = get_color(_theme._syntax_statement); // D - keywords
  styletable[4].color = get_color(_theme._syntax_command); // E - functions
  styletable[5].color = get_color(_theme._syntax_command); // F - procedures
  styletable[6].color = get_color(_theme._match_background); // G - find matches
  styletable[7].color = get_color(_theme._syntax_comments); // H - comments
  styletable[8].color = get_color(_theme._syntax_digit); // I - numbers
  styletable[9].color = get_color(_theme._syntax_command); // J - operators
  styletable[10].color = get_color(_theme._selection_background); // K Selection Background
  styletable[11].color = get_color(_theme._background); // L Background
  styletable[12].color = get_color(_theme._number_color); // M Line numbers
}

//
// restore saved settings
//
void Profile::restore(MainWindow *wnd) {
  strlib::String buffer;
  Properties<String *> profile;

  FILE *fp = wnd->openConfig(configFile, "r");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);
    buffer.append(fp, len);
    fclose(fp);
    profile.load(buffer.c_str(), buffer.length());

    restoreValue(&profile, indentLevelKey, &_indentLevel);
    restoreValue(&profile, createBackupsKey, &_createBackups);
    restoreValue(&profile, lineNumbersKey, &_lineNumbers);
    restoreValue(&profile, themeIdKey, &_themeId);
    restoreValue(&profile, helpThemeIdKey, &_helpThemeId);
    restoreStyles(&profile);

    Fl_Rect rc;
    rc = restoreRect(&profile, appPositionKey);
    if (rc.w() && rc.h()) {
      _appPosition = rc;
    }

    rc = restoreRect(&profile, windowPosKey);
    if (rc.w() && rc.h()) {
      restoreWindowPos(wnd, rc);
    }

    restoreTabs(wnd, &profile);
    wnd->_runtime->setFontSize(_fontSize);
  }
  _loaded = true;
}

//
// restore the standalone window position
//
void Profile::restoreAppPosition(Fl_Window *wnd) {
  if (_appPosition.w() && _appPosition.h()) {
    int x = _appPosition.x() != 0 ? _appPosition.x() : wnd->x();
    int y = _appPosition.y() != 0 ? _appPosition.y() : wnd->y();
    wnd->resize(x, y, _appPosition.w(), _appPosition.h());
  }
}

//
// set editor theme colors
//
void Profile::setEditTheme(EditorWidget *editWidget) {
  editWidget->setTheme(&_theme);
  editWidget->redraw();
}

//
// set help theme colors
//
void Profile::setHelpTheme(HelpWidget *helpWidget, int themeId) {
  if (themeId != -1) {
    _helpTheme.setId(themeId);
    _helpThemeId = themeId;
  }
  helpWidget->setTheme(&_helpTheme);
  helpWidget->damage(FL_DAMAGE_ALL);
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
      saveValue(fp, themeIdKey, _themeId);
      saveValue(fp, helpThemeIdKey, _helpThemeId);
      saveStyles(fp);
      saveTabs(fp, wnd);
      saveRect(fp, appPositionKey, &_appPosition);
      Fl_Rect wndRect(wnd->x(), wnd->y(), wnd->w(), wnd->h());
      saveRect(fp, windowPosKey, &wndRect);
      fclose(fp);
    }
  }
}

//
// update the theme from the style table
//
void Profile::updateTheme() {
  _theme._color = styletable[0].color >> 8;
  _theme._syntax_comments = styletable[1].color >> 8;
  _theme._syntax_text = styletable[2].color >> 8;
  _theme._syntax_statement = styletable[3].color >> 8;
  _theme._syntax_command = styletable[4].color >> 8;
  _theme._syntax_command = styletable[5].color >> 8;
  _theme._match_background = styletable[6].color >> 8;
  _theme._syntax_comments = styletable[7].color >> 8;
  _theme._syntax_digit = styletable[8].color >> 8;
  _theme._syntax_command = styletable[9].color >> 8;
  _theme._selection_background = styletable[10].color >> 8;
  _theme._background = styletable[11].color >> 8;
  _theme._number_color = styletable[12].color >> 8;
}

//
// returns the next integer from the given string
//
int Profile::nextInteger(const char *s, int len, int &index) {
  int result = 0;
  while (index < len && isdigit(s[index])) {
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
Fl_Rect Profile::restoreRect(Properties<String *> *profile, const char *key) {
  Fl_Rect result(0, 0, 0, 0);
  String *value = profile->get(key);
  if (value != NULL) {
    const char *buffer = value->c_str();
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
void Profile::restoreStyles(Properties<String *> *profile) {
  // restore size and face
  loadEditTheme(_themeId);
  _helpTheme.setId(_helpThemeId);

  restoreValue(profile, fontSizeKey, &_fontSize);
  String *fontName = profile->get(fontNameKey);
  if (fontName) {
    _font = get_font(fontName->c_str());
  }

  for (int i = 0; i <= lastStyle; i++) {
    char buffer[4];
    sprintf(buffer, "%02d", i);
    String *color = profile->get(buffer);
    if (color) {
      Fl_Color c = get_color(color->c_str(), NO_COLOR);
      if (c != (Fl_Color)NO_COLOR) {
        styletable[i].color = c;
      }
    }
  }
  updateTheme();
}

//
// restore the editor tabs
//
void Profile::restoreTabs(MainWindow *wnd, Properties<String *> *profile) {
  bool usedEditor = false;
  strlib::List<String *> paths;
  profile->get(pathKey, &paths);

  List_each(String*, it, paths) {
    String *nextString = (*it);
    const char *buffer = nextString->c_str();
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
      Fl_Group *group = wnd->createEditor(path);
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
    editWidget->getEditor()->insert_position(insertPos);
    editWidget->getEditor()->show_insert_position();
    editWidget->getEditor()->scroll(topLineNo, 0);
  }

  // restore the active tab
  String *activeTab = profile->get(activeTabKey);
  if (activeTab != NULL) {
    EditorWidget *editWidget = wnd->getEditor(activeTab->c_str());
    if (editWidget) {
      wnd->showEditTab(editWidget);
    }
  }
}

//
// restore the int value
//
void Profile::restoreValue(Properties<String *> *p, const char *key, int *value) {
  String *s = p->get(key);
  if (s) {
    *value = s->toInteger();
  }
}

//
// restore the main window position
//
void Profile::restoreWindowPos(MainWindow *wnd, Fl_Rect &rc) {
  int x = rc.x();
  int y = rc.y();
  int w = rc.w();
  int h = rc.h();
  if (x > 0 && y > 0 && w > 100 && h > 100) {
    if (x < Fl::w() && y < Fl::h()) {
      wnd->resize(x, y, w, h);
    }
  }
}

//
// save the window position
//
void Profile::saveRect(FILE *fp, const char *key, Fl_Rect *rc) {
  fprintf(fp, "%s=%d;%d;%d;%d\n", key, rc->x(), rc->y(), rc->w(), rc->h());
}

//
// saves the current font size, face and colour configuration
//
void Profile::saveStyles(FILE *fp) {
  uint8_t r, g, b;

  saveValue(fp, fontSizeKey, (int)styletable[0].size);
  saveValue(fp, fontNameKey, styletable[0].font);

  for (int i = 0; i <= lastStyle; i++) {
    Fl::get_color(styletable[i].color, r, g, b);
    fprintf(fp, "%02d=#%02x%02x%02x\n", i, r, g, b);
  }
}

//
// persist the editor tabs
//
void Profile::saveTabs(FILE *fp, MainWindow *wnd) {
  int n = wnd->_tabGroup->children();
  for (int c = 0; c < n; c++) {
    Fl_Group *group = (Fl_Group *) wnd->_tabGroup->child(c);
    if (gw_editor == ((GroupWidgetEnum) (intptr_t) group->user_data())) {
      EditorWidget *editWidget = (EditorWidget *) group->child(0);

      bool logPrint = editWidget->isLogPrint();
      bool scrollLock = editWidget->isScrollLock();
      bool hideIde = editWidget->isHideIDE();
      bool gotoLine = editWidget->isBreakToLine();
      int insertPos = editWidget->getEditor()->insert_position();
      int topLineNo = editWidget->top_line();

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
