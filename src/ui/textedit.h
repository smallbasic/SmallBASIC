// This file is part of SmallBASIC
//
// Copyright(C) 2001-2020 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_UNDOCHARCOUNT 5000
#define MARGIN_CHARS 4
#define MAX_MARKERS 10

#include <config.h>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "lib/stb/stb_textedit.h"
#include "ui/inputs.h"
#include "ui/theme.h"
#include "common/smbas.h"
#include "common/keymap.h"

struct TextEditInput;

struct StackTraceNode {
  StackTraceNode(const char *keyword, int type, int line)
    : _keyword(keyword), _type(type), _line(line) {}
  virtual ~StackTraceNode() = default;
  const char *_keyword;
  int _type;
  int _line;
};

typedef strlib::List<StackTraceNode *> StackTrace;

struct EditBuffer {
  char *_buffer;
  int _len;
  int _size;
  int _lines;
  TextEditInput *_in;

  EditBuffer(TextEditInput *in, const char *text);
  virtual ~EditBuffer();

  void append(const char *text, int len) { insertChars(_len, text, len); }
  void append(const char *text) { insertChars(_len, text, strlen(text)); }
  void clear();
  void convertTabs();
  int  countNewlines(const char *text, int num);
  int  deleteChars(int pos, int num);
  char getChar(int pos) const;
  int  insertChars(int pos, const char *text, int num);
  int  lineCount();
  void removeTrailingSpaces(STB_TexteditState *state);
  char *textRange(int start, int end) const;
};

struct TextEditInput : public FormEditInput {
  TextEditInput(const char *text, int chW, int chH, int x, int y, int w, int h);
  ~TextEditInput() override;

  void append(const char *text, int len) { _buf.append(text, len); }
  void completeWord(const char *word);
  const char *completeKeyword(int index) override;
  void draw(int x, int y, int w, int h, int chw) override;
  bool edit(int key, int screenWidth, int charWidth) override;
  bool find(const char *word, bool next);
  int  getCursorPos() const { return _state.cursor; }
  int  getCol() const { return _cursorCol; }
  int  getRow() const { return _cursorRow + 1; }
  int  getPageRows() const { return _height / _charHeight; }
  int  getLines() { return _buf.lineCount(); }
  int  getMarginWidth() const { return _marginWidth; }
  void getSelectionCounts(int *lines, int *chars);
  int  getSelectionRow();
  int  getSelectionStart() const { return _state.select_start; }
  int  getScroll() const { return _scroll; }
  const char *getText() const override { return _buf._buffer; }
  char *getTextSelection(bool selectAll);
  int  getTextLength() const { return _buf._len; }
  int *getMarkers();
  void gotoLine(const char *buffer);
  void reload(const char *text);
  bool save(const char *filePath);
  void setCursor(int pos);
  void setCursorPos(int pos);
  void setCursorRow(int row);
  void setLineNumbers() { _marginWidth = 1 + (_charWidth * MARGIN_CHARS); }
  void setText(const char *text) override { _buf.clear(); _buf.append(text); }
  void setTheme(EditTheme *theme) { _theme = theme; }
  void clicked(int x, int y, bool pressed) override;
  void updateField(var_p_t form) override;
  bool updateUI(var_p_t form, var_p_t field) override;
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) override;
  int  padding(bool) const override { return 0; }
  void layout(StbTexteditRow *row, int start_i) const;
  int  charWidth(int k, int i) const;
  char *copy(bool cut) override;
  void paste(const char *text) override;
  void selectAll() override;
  bool isDirty() const { return _dirty && _state.undostate.undo_point > 0; }
  void setDirty(bool dirty) { _dirty = dirty; }
  void layout(int w, int h) override;
  const char *getNodeId();
  char *getWordBeforeCursor();
  bool replaceNext(const char *text, bool skip);
  int  getCompletions(StringList *list, int max) override;
  void selectNavigate(bool up);
  EditTheme *getTheme() { return _theme; }

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
  void calcMargin();
  void changeCase();
  void cycleTheme();
  void drawLineNumber(int x, int y, int row, bool selected);
  void editDeleteLine();
  void editEnter();
  void editTab();
  bool endStatement(const char *buf);
  void findMatchingBrace();
  int  getCursorRow();
  uint32_t getHash(const char *str, int offs, int &count);
  int  getIndent(char *spaces, int len, int pos);
  int  getLineChars(StbTexteditRow *row, int pos) const;
  char *getSelection(int *start, int *end);
  void gotoNextMarker();
  void killWord();
  void lineNavigate(bool lineDown);
  char *lineText(int pos);
  int  lineEnd(int pos) { return linePos(pos, true); }
  int  linePos(int pos, bool end, bool excludeBreak=true);
  int  lineStart(int pos) { return linePos(pos, false); }
  bool matchCommand(uint32_t hash);
  bool matchStatement(uint32_t hash);
  void pageNavigate(bool pageDown, bool shift);
  void removeTrailingSpaces();
  void selectWord();
  void setColor(SyntaxState &state);
  void toggleMarker();
  void updateScroll();
  int wordEnd();
  int wordStart();

  EditBuffer _buf;
  STB_TexteditState _state{};
  EditTheme *_theme;
  int _charWidth;
  int _charHeight;
  int _marginWidth;
  int _scroll;
  int _cursorCol;
  int _cursorRow;
  int _cursorLine;
  int _indentLevel;
  int _matchingBrace;
  int _ptY;
  int _pressTick;
  int _xmargin;
  int _ymargin;
  bool _bottom;
  bool _dirty;
  bool _comment;
};

struct TextEditHelpWidget : public TextEditInput {
  TextEditHelpWidget(TextEditInput *editor, int chW, int chH, bool overlay=true);
  ~TextEditHelpWidget() override;

  enum HelpMode {
    kNone,
    kHelp,
    kHelpKeyword,
    kCompletion,
    kOutline,
    kSearch,
    kEnterReplace,
    kEnterReplaceWith,
    kReplace,
    kReplaceDone,
    kGotoLine,
    kMessage,
    kLineEdit,
    kStacktrace
  };

  void clicked(int x, int y, bool pressed) override;
  void createCompletionHelp();
  void createGotoLine();
  void createHelp();
  void createLineEdit(const char *value);
  void createKeywordIndex();
  void createMessage() { reset(kMessage); }
  void createOutline();
  void createSearch(bool replace);
  void createStackTrace(const char *error, int line, StackTrace &trace);
  void draw(int x, int y, int w, int h, int chw) override;
  bool edit(int key, int screenWidth, int charWidth) override;
  void paste(const char *text) override;
  bool isDrawTop() const { return true; }
  void reset(HelpMode mode);
  void cancelMode() { _mode = kNone; }
  bool closeOnEnter() const;
  bool searchMode() const { return _mode >= kSearch && _mode <= kReplaceDone; }
  void layout(int w, int h) override;
  bool lineEditMode() const { return _mode == kLineEdit; }
  bool messageMode() const { return _mode == kMessage; }
  bool replaceMode() const { return _mode == kReplace; }
  bool replaceModeWith() const { return _mode == kEnterReplaceWith; }
  bool replaceDoneMode() const { return _mode == kReplaceDone; }
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) override;
  void showPopup(int cols, int rows);
  void showSidebar();

private:
  void completeLine(int pos);
  void completeWord(int pos);
  void buildKeywordIndex();
  void toggleKeyword();

  HelpMode _mode;
  strlib::List<int *> _outline;
  TextEditInput *_editor;
  int _keywordIndex;
  int _packageIndex;
  bool _packageOpen;
  enum Layout {
    kLine,
    kSidebar,
    kPopup,
  } _layout;
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
#define STB_TEXTEDIT_GETCHAR(o,i) o->getChar(i)
#define STB_TEXTEDIT_GETWIDTH(o,n,i)      o->_in->charWidth(n, i)
#define STB_TEXTEDIT_LAYOUTROW(r,o,n)     o->_in->layout(r, n)
#define STB_TEXTEDIT_DELETECHARS(o,i,n)   o->deleteChars(i, n)
#define STB_TEXTEDIT_INSERTCHARS(o,i,c,n) o->insertChars(i, c, n)

#endif

