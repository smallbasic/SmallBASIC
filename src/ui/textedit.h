// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_UNDOCHARCOUNT 2000
#define MARGIN_CHARS 4
#define MAX_MARKERS 10

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lib/stb_textedit.h"
#include "ui/inputs.h"
#include "common/smbas.h"
#include "common/keymap.h"

extern unsigned g_themeId;
extern int g_user_theme[];
#define THEME_COLOURS 17

struct TextEditInput;

struct EditTheme {
  EditTheme();
  EditTheme(int fg, int bg);
  void selectTheme(const int theme[]);

  int _color;
  int _background;
  int _selection_color;
  int _selection_background;
  int _number_color;
  int _number_selection_color;
  int _number_selection_background;
  int _cursor_color;
  int _cursor_background;
  int _match_background;
  int _row_cursor;
  int _syntax_comments;
  int _syntax_text;
  int _syntax_command;
  int _syntax_statement;
  int _syntax_digit;
  int _row_marker;
};

struct EditBuffer {
  char *_buffer;
  int _len;
  int _size;
  TextEditInput *_in;

  EditBuffer(TextEditInput *in, const char *text);
  virtual ~EditBuffer();

  void clear();
  void append(const char *text, int len) { insertChars(_len, text, len); }
  void append(const char *text) { insertChars(_len, text, strlen(text)); }
  int deleteChars(int pos, int num);
  int insertChars(int pos, const char *text, int num);
  void removeTrailingSpaces(STB_TexteditState *state);
  char *textRange(int start, int end);
};

struct TextEditInput : public FormEditInput {
  TextEditInput(const char *text, int chW, int chH, int x, int y, int w, int h);
  virtual ~TextEditInput();

  void append(const char *text, int len) { _buf.append(text, len); }
  void completeWord(const char *word);
  const char *completeKeyword(int index);
  void draw(int x, int y, int w, int h, int chw);
  bool edit(int key, int screenWidth, int charWidth);
  bool find(const char *word, bool next);
  int  getCursorPos() const { return _state.cursor; }
  const char *getText() const { return _buf._buffer; }
  int  getTextLength() const { return _buf._len; }
  int *getMarkers();
  void gotoLine(const char *buffer);
  void reload(const char *text);
  bool save(const char *filePath);
  void setCursor(int pos);
  void setCursorPos(int pos) { _state.cursor = pos; }
  void setCursorRow(int row);
  void setLineNumbers() { _marginWidth = 1 + (_charWidth * MARGIN_CHARS); }
  void setText(const char *text) { _buf.clear(); _buf.append(text); }
  void setTheme(EditTheme *theme) { _theme = theme; }
  void clicked(int x, int y, bool pressed);
  void updateField(var_p_t form);
  bool updateUI(var_p_t form, var_p_t field);
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  int  padding(bool) const { return 0; }
  void layout(StbTexteditRow *row, int start_i) const;
  int  charWidth(int k, int i) const;
  char *copy(bool cut);
  void paste(const char *text);
  void selectAll();
  bool isDirty() { return _dirty && _state.undostate.undo_point > 0; }
  void setDirty(bool dirty) { _dirty = dirty; }
  void resize(int w, int h) { _width = w; _height = h; }
  const char *getNodeId();
  char *getWordBeforeCursor();
  bool replaceNext(const char *text);
  int  getCompletions(StringList *list, int max);
  void selectNavigate(bool up);

protected:
  enum SyntaxState {
    kReset = 0,
    kComment,
    kText,
    kCommand,
    kStatement,
    kDigit,
  };

  void dragPage(int y, bool &redraw);
  void drawText(int x, int y, const char *str, int length, SyntaxState &state);
  void changeCase();
  void cycleTheme();
  void drawLineNumber(int x, int y, int row, bool selected);
  void editDeleteLine();
  void editEnter();
  void editTab();
  void findMatchingBrace();
  int  getCursorRow() const;
  uint32_t getHash(const char *str, int offs, int &count);
  int  getIndent(char *spaces, int len, int pos);
  int  getLineChars(StbTexteditRow *row, int pos);
  char *getSelection(int *start, int *end);
  void gotoNextMarker();
  void lineNavigate(bool lineDown);
  char *lineText(int pos);
  int  lineEnd(int pos) { return linePos(pos, true); }
  int  linePos(int pos, bool end, bool excludeBreak=true);
  int  lineStart(int pos) { return linePos(pos, false); }
  bool matchCommand(uint32_t hash);
  bool matchStatement(uint32_t hash);
  void pageNavigate(bool pageDown, bool shift);
  void removeTrailingSpaces();
  void setColor(SyntaxState &state);
  void toggleMarker();
  void updateScroll();
  int wordStart();

  EditBuffer _buf;
  STB_TexteditState _state;
  EditTheme *_theme;
  int _charWidth;
  int _charHeight;
  int _marginWidth;
  int _scroll;
  int _cursorRow;
  int _cursorLine;
  int _indentLevel;
  int _matchingBrace;
  int _ptY;
  bool _dirty;
};

struct TextEditHelpWidget : public TextEditInput {
  TextEditHelpWidget(TextEditInput *editor, int chW, int chH, bool overlay=true);
  virtual ~TextEditHelpWidget();

  enum HelpMode {
    kNone,
    kHelp,
    kHelpKeyword,
    kCompletion,
    kOutline,
    kSearch,
    kSearchReplace,
    kReplace,
    kReplaceDone,
    kGotoLine,
    kMessage,
    kLineEdit
  };

  void clicked(int x, int y, bool pressed);
  void createCompletionHelp();
  void createGotoLine();
  void createHelp();
  void createLineEdit(const char *value);
  void createKeywordIndex();
  void createMessage() { reset(kMessage); }
  void createOutline();
  void createSearch(bool replace);
  bool edit(int key, int screenWidth, int charWidth);
  void paste(const char *text) {}
  bool isDrawTop() { return true; }
  void resize(int w, int h) { _x = w - _width; _height = h; }
  void reset(HelpMode mode);
  bool closeOnEnter() const;
  bool lineEditMode() const { return _mode == kLineEdit; }
  bool messageMode() const { return _mode == kMessage; }
  bool replaceMode() const { return _mode == kReplace; }
  bool replaceDoneMode() const { return _mode == kReplaceDone; }
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  void toggleKeyword();

private:
  void completeLine(int pos);
  void completeWord(int pos);
  void createPackageIndex();

  HelpMode _mode;
  strlib::List<int *> _outline;
  TextEditInput *_editor;
  const char *_openPackage;
  int _openKeyword;
};

#define STB_TEXTEDIT_STRING       EditBuffer
#define STB_TEXTEDIT_K_LEFT       SB_KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT      SB_KEY_RIGHT
#define STB_TEXTEDIT_K_UP         SB_KEY_UP
#define STB_TEXTEDIT_K_DOWN       SB_KEY_DOWN
#define STB_TEXTEDIT_K_LINESTART  SB_KEY_HOME
#define STB_TEXTEDIT_K_LINEEND    SB_KEY_END
#define STB_TEXTEDIT_K_DELETE     SB_KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE  SB_KEY_BACKSPACE
#define STB_TEXTEDIT_K_TEXTSTART  SB_KEY_CTRL(SB_KEY_HOME)
#define STB_TEXTEDIT_K_TEXTEND    SB_KEY_CTRL(SB_KEY_END)
#define STB_TEXTEDIT_K_UNDO       SB_KEY_CTRL('z')
#define STB_TEXTEDIT_K_REDO       SB_KEY_CTRL('y')
#define STB_TEXTEDIT_K_INSERT     SB_KEY_INSERT
#define STB_TEXTEDIT_K_WORDLEFT   SB_KEY_CTRL(SB_KEY_LEFT)
#define STB_TEXTEDIT_K_WORDRIGHT  SB_KEY_CTRL(SB_KEY_RIGHT)
#define STB_TEXTEDIT_K_PGUP       SB_KEY_PGUP
#define STB_TEXTEDIT_K_PGDOWN     SB_KEY_PGDN
#define STB_TEXTEDIT_NEWLINE      '\n'
#define STB_TEXTEDIT_K_CONTROL    SB_KEY_CTRL(0)
#define STB_TEXTEDIT_K_SHIFT      SB_KEY_SHIFT(0)
#define STB_TEXTEDIT_GETWIDTH_NEWLINE 0.0f
#define STB_TEXTEDIT_KEYTOTEXT(k) k
#define STB_TEXTEDIT_STRINGLEN(o) o->_len
#define STB_TEXTEDIT_GETCHAR(o,i) o->_buffer[i]
#define STB_TEXTEDIT_GETWIDTH(o,n,i)      o->_in->charWidth(n, i)
#define STB_TEXTEDIT_LAYOUTROW(r,o,n)     o->_in->layout(r, n)
#define STB_TEXTEDIT_DELETECHARS(o,i,n)   o->deleteChars(i, n)
#define STB_TEXTEDIT_INSERTCHARS(o,i,c,n) o->insertChars(i, c, n)

#endif

