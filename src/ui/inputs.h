// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef INPUTS_H
#define INPUTS_H

#include <config.h>

#include "common/var.h"
#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/shape.h"
#include "ui/image.h"

const uint32_t colors[] = {
  0x000000, // 0 black
  0x000080, // 1 blue
  0x008000, // 2 green
  0x008080, // 3 cyan
  0x800000, // 4 red
  0x800080, // 5 magenta
  0x808000, // 6 yellow
  0xC0C0C0, // 7 white
  0x808080, // 8 gray
  0x0000FF, // 9 light blue
  0x00FF00, // 10 light green
  0x00FFFF, // 11 light cyan
  0xFF0000, // 12 light red
  0xFF00FF, // 13 light magenta
  0xFFFF00, // 14 light yellow
  0xFFFFFF  // 15 bright white
};

using namespace strlib;

#define BN_H 12
#define BN_W 16
#define LN_H 2
#define CHOICE_BN_W 6
#define GRAY_BG_COL    0x4e4c46
#define LABEL_TEXT_COL 0xdfdbd2

#define FORM_VALUE "value"
#define FORM_INPUTS "inputs"
#define FORM_FOCUS "focus"
#define FORM_INPUT_ID "ID"
#define FORM_INPUT_X "x"
#define FORM_INPUT_Y "y"
#define FORM_INPUT_W "width"
#define FORM_INPUT_H "height"
#define FORM_INPUT_VALUE "value"
#define FORM_INPUT_LABEL "label"
#define FORM_INPUT_NAME "name"
#define FORM_INPUT_TYPE "type"
#define FORM_INPUT_BG "backgroundColor"
#define FORM_INPUT_FG "color"
#define FORM_INPUT_IS_EXIT "isExit"
#define FORM_INPUT_RESIZABLE "resizable"
#define FORM_INPUT_VISIBLE "visible"
#define FORM_INPUT_INDEX "selectedIndex"
#define FORM_INPUT_LENGTH "length"
#define FORM_INPUT_NO_FOCUS "noFocus"
#define FORM_INPUT_ONCLICK "onclick"

namespace form_ui {
  bool optionSelected(int index);
};

int get_color(var_p_t value, int def);

struct IFormWidgetListModel {
  virtual ~IFormWidgetListModel() {}
  virtual const char *getTextAt(int index) = 0;
  virtual int getIndex(const char *label) = 0;
  virtual int rows() const = 0;
  virtual int selected() const = 0;
  virtual void selected(int index) = 0;
};

struct ListModel : IFormWidgetListModel {
  ListModel(int selectedIndex, var_t *v);
  virtual ~ListModel() { clear(); }

  void clear();
  void create(var_t *v);
  const char *getTextAt(int index);
  int getIndex(const char *t);
  int rows() const { return _list.size(); }
  int selected() const { return _selectedIndex; }
  void selected(int index) { _selectedIndex = index; }

private:
  void fromArray(var_t *v);
  StringList _list;
  int _selectedIndex;
};

struct FormInput : public Shape {
  FormInput(int x, int y, int w, int h);
  virtual ~FormInput();

  virtual bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  virtual const char *getText() const { return NULL; }
  virtual void setText(const char *text) {}
  virtual bool edit(int key, int screenWidth, int charWidth);
  virtual void updateField(var_p_t form) {};
  virtual void updateForm(var_p_t form);
  virtual bool updateUI(var_p_t form, var_p_t field);
  virtual int padding(bool vert) const { return vert ? BN_H : BN_W; }
  virtual void clicked(int x, int y, bool pressed);
  virtual bool isDrawTop() { return false; }
  virtual bool hasHover() { return false; }
  virtual void setFocus(bool focus);
  virtual void layout(int w, int h) {}

  void construct(var_p_t form, var_p_t field, int id);
  void drawButton(const char *caption, int x, int y, int w, int h, bool pressed);
  void drawHover(int dx, int dy, bool selected);
  void drawLink(const char *caption, int x, int y, int sw, int chw);
  void drawText(const char *text, int x, int y, int sw, int chw);
  void draw(int x, int y, int w, int h, int chw);
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY);
  bool hasFocus() const;
  void hide() { _visible = false; }
  int  getId() { return _id; }
  var_p_t getField(var_p_t form);
  bool isNoFocus() { return _noFocus; }
  bool isResizable() { return _resizable; }
  bool isVisible() { return _visible; }
  void setColor(int bg, int fg) { _bg = bg; _fg = fg; }
  void setTextColor();
  void selected();
  void show() { _visible = true; }
  bool _pressed;

protected:
  int  _id;
  bool _exit;
  bool _visible;
  bool _noFocus;
  bool _resizable;
  int _bg;
  int _fg;
  bcip_t _onclick;
};

struct FormButton : public FormInput {
  FormButton(const char *caption, int x, int y, int w, int h);
  virtual ~FormButton() {}

  const char *getText() const { return _label.c_str(); }
  void draw(int x, int y, int w, int h, int chw) {
    drawButton(_label.c_str(), x, y, _width, _height, _pressed);
  }
  void setText(const char *text) { _label = text; }

protected:
  String _label;
};

struct FormLabel : public FormInput {
  FormLabel(const char *caption, int x, int y, int w, int h);
  virtual ~FormLabel() {}

  bool updateUI(var_p_t form, var_p_t field);
  const char *getText() const { return _label.c_str(); }
  void draw(int x, int y, int w, int h, int chw);
  void setText(const char *text) { _label = text; }
  int padding(bool vert) const { return 0; }
  void clicked(int x, int y, bool pressed) {}
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) { return false; }

private:
  String _label;
};

struct FormLink : public FormInput {
  FormLink(const char *link, int x, int y, int w, int h);
  virtual ~FormLink() {}

  const char *getText() const { return _link.c_str(); }
  bool hasHover() { return true; }

  void draw(int x, int y, int w, int h, int chw) {
    drawLink(_link.c_str(), x, y, w, chw);
  }
  int padding(bool vert) const { return vert ? LN_H : 0; }

protected:
  String _link;
};

struct FormTab : public FormLink {
  FormTab(const char *link, int x, int y, int w, int h);
  virtual ~FormTab() {}

  void draw(int x, int y, int w, int h, int chw);
  int padding(bool vert) const;
};

struct FormEditInput : public FormInput {
  FormEditInput(int x, int y, int w, int h);
  virtual ~FormEditInput();

  virtual char *copy(bool cut) = 0;
  virtual void paste(const char *text) = 0;
  virtual void selectAll() = 0;
  virtual const char *completeKeyword(int index) = 0;
  virtual int getCompletions(StringList *list, int max) = 0;

  void setFocus(bool focus);
  int  getControlKey(int key);
  bool getControlMode() const { return _controlMode; }
  void setControlMode(bool cursorMode) { _controlMode = cursorMode; }

protected:
  bool _controlMode;
};

struct FormLineInput : public FormEditInput {
  FormLineInput(const char *text, int maxSize, bool grow, int x, int y, int w, int h);
  virtual ~FormLineInput();

  void close();
  void draw(int x, int y, int w, int h, int chw);
  bool edit(int key, int screenWidth, int charWidth);
  const char *getText() const { return _buffer; }
  void setText(const char *text) {}
  void clicked(int x, int y, bool pressed);
  void updateField(var_p_t form);
  bool updateUI(var_p_t form, var_p_t field);
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  int padding(bool) const { return 0; }
  char *copy(bool cut);
  void paste(const char *text);
  void cut();
  void selectAll();
  const char *completeKeyword(int index) { return NULL; }
  int getCompletions(StringList *list, int max) { return 0; }

private:
  char *_buffer;
  int _size;
  int _scroll;
  int _mark;
  int _point;
  bool _grow;
};

struct FormList : public FormInput {
  FormList(ListModel *model, int x, int y, int w, int h);
  virtual ~FormList();

  bool edit(int key, int screenWidth, int charWidth);
  const char *getText() const { return _model->getTextAt(_model->selected()); }
  void optionSelected(int index);
  void updateForm(var_p_t form);
  bool updateUI(var_p_t form, var_p_t field);
  virtual int getListHeight() const { return _height; }

protected:
  void selectIndex(int index);

  ListModel *_model;
  // scroll offset
  int _topIndex;
  // zero is first display item
  int _activeIndex;
};

struct FormDropList : public FormList {
  FormDropList(ListModel *model, int x, int y, int w, int h);
  virtual ~FormDropList() {}

  void clicked(int x, int y, bool pressed);
  void draw(int dx, int dy, int w, int h, int chw);
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  void updateForm(var_p_t form);
  bool isDrawTop() { return _listActive; }
  int getListHeight() const { return _listHeight; }

private:
  void drawList(int dx, int dy, int sh);
  int _listWidth;
  int _listHeight;
  int _visibleRows;
  bool _listActive;
};

struct FormListBox : public FormList {
  FormListBox(ListModel *model, int x, int y, int w, int h);
  void clicked(int x, int y, bool pressed);
  void draw(int x, int y, int w, int h, int chw);
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  virtual ~FormListBox() {}
};

struct FormImage : public FormInput {
  FormImage(ImageDisplay *image, int x, int y);
  virtual ~FormImage();
  void draw(int x, int y, int w, int h, int chw);

private:
  ImageDisplay *_image;
};

struct MenuButton : public FormButton {
  MenuButton(int index, int &selectedIndex,
             const char *caption, int x, int y, int w, int h);
  void clicked(int x, int y, bool pressed);
  void draw(int x, int y, int w, int h, int chw);

  int _index;
  int &_selectedIndex;
};

FormEditInput *get_focus_edit();

#endif
