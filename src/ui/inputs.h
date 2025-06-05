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
#include "ui/utils.h"

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
#define FOCUS_COLOR    0x444999

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
#define FORM_INPUT_HELP "help"
#define FORM_INPUT_BG "backgroundColor"
#define FORM_INPUT_FG "color"
#define FORM_INPUT_IS_EXIT "isExit"
#define FORM_INPUT_IS_EXTERNAL "isExternal"
#define FORM_INPUT_RESIZABLE "resizable"
#define FORM_INPUT_VISIBLE "visible"
#define FORM_INPUT_INDEX "selectedIndex"
#define FORM_INPUT_LENGTH "length"
#define FORM_INPUT_NO_FOCUS "noFocus"
#define FORM_INPUT_ONCLICK "onclick"

namespace form_ui {
  bool optionSelected(int index);
}

void set_input_defaults(int fg, int bg);
int get_color(var_p_t value, int def);

struct IFormWidgetListModel {
  virtual ~IFormWidgetListModel() = default;
  virtual const char *getTextAt(int index) = 0;
  virtual int getIndex(const char *label) = 0;
  virtual int rows() const = 0;
  virtual int selected() const = 0;
  virtual void selected(int index) = 0;
};

struct ListModel : IFormWidgetListModel {
  ListModel(int selectedIndex, var_t *v);
  ~ListModel() override { clear(); }

  void clear();
  void create(var_t *v);
  const char *getTextAt(int index) override;
  int getIndex(const char *t) override;
  int rows() const override { return _list.size(); }
  int selected() const override { return _selectedIndex; }
  void selected(int index) override { _selectedIndex = index; }

private:
  void fromArray(var_t *v);
  StringList _list;
  int _selectedIndex;
};

struct FormInput : public Shape {
  FormInput(int x, int y, int w, int h);
  ~FormInput() override;

  virtual bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  virtual const char *getText() const { return nullptr; }
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
  virtual void layout(int x, int y, int w, int h) {}
  virtual bool floatBottom() { return false; }
  virtual bool floatTop() { return false; }

  void construct(var_p_t form, var_p_t field, int id);
  void drawButton(const char *caption, int x, int y, int w, int h, bool pressed);
  void drawHover(int dx, int dy, bool selected);
  void drawLink(const char *caption, int x, int y, int sw, int chw);
  void drawText(const char *text, int x, int y, int sw, int chw);
  void draw(int x, int y, int w, int h, int chw) override;
  const char *getValue();
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY);
  bool hasFocus() const;
  void hide() { _visible = false; }
  int  getId() const { return _id; }
  var_p_t getField(var_p_t form) const;
  bool isNoFocus() const { return _noFocus; }
  bool isResizable() const { return _resizable; }
  bool isVisible() const { return _visible; }
  void setColor(int bg, int fg) { _bg = bg; _fg = fg; }
  void setTextColor() const;
  void setHelpTextColor() const;
  void selected();
  void show() { _visible = true; }
  bool _pressed;

protected:
  bool __padding[3]{};
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
  ~FormButton() override = default;

  const char *getText() const override { return _label.c_str(); }
  void draw(int x, int y, int w, int h, int chw) override {
    drawButton(_label.c_str(), x, y, _width, _height, _pressed);
  }
  void setText(const char *text) override { _label = text; }

protected:
  String _label;
};

struct FormLabel : public FormInput {
  FormLabel(const char *caption, int x, int y, int w, int h);
  ~FormLabel() override = default;

  bool updateUI(var_p_t form, var_p_t field) override;
  const char *getText() const override { return _label.c_str(); }
  void draw(int x, int y, int w, int h, int chw) override;
  void setText(const char *text) override { _label = text; }
  int padding(bool vert) const override { return 0; }
  void clicked(int x, int y, bool pressed) override {}
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) override { return false; }

private:
  String _label;
};

struct FormLink : public FormInput {
  FormLink(const char *link, bool external, int x, int y, int w, int h);
  ~FormLink() override = default;

  void clicked(int x, int y, bool pressed) override;
  const char *getText() const override { return _link.c_str(); }
  bool hasHover() override { return true; }

  void draw(int x, int y, int w, int h, int chw) override ;
  int padding(bool vert) const override { return vert ? LN_H : 0; }

protected:
  String _link;
  bool _external;
};

struct FormEditInput : public FormInput {
  FormEditInput(int x, int y, int w, int h);
  ~FormEditInput() override;

  virtual char *copy(bool cut) = 0;
  virtual void paste(const char *text) = 0;
  virtual void selectAll() = 0;
  virtual const char *completeKeyword(int index) = 0;
  virtual int getCompletions(StringList *list, int max) = 0;

  void clicked(int x, int y, bool pressed) override;
  void setFocus(bool focus) override;
  int  getControlKey(int key);
  bool getControlMode() const { return _controlMode; }
  void setControlMode(bool cursorMode) { _controlMode = cursorMode; }

protected:
  bool _controlMode;
};

struct FormLineInput : public FormEditInput {
  FormLineInput(const char *text, const char *help,
                int maxSize, bool grow, int x, int y, int w, int h);
  ~FormLineInput() override;

  void draw(int x, int y, int w, int h, int chw) override;
  bool edit(int key, int screenWidth, int charWidth) override;
  const char *getText() const override { return _buffer; }
  void setText(const char *text) override {}
  void clicked(int x, int y, bool pressed) override;
  void updateField(var_p_t form)  override;
  bool updateUI(var_p_t form, var_p_t field)  override;
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw)  override;
  int padding(bool) const override { return 0; }
  char *copy(bool cut) override;
  void paste(const char *text) override;
  void cut();
  void selectAll() override;
  const char *completeKeyword(int index) override { return nullptr; }
  int getCompletions(StringList *list, int max) override { return 0; }

private:
  String _help;
  char *_buffer;
  int _size;
  int _scroll;
  int _mark;
  int _point;
  bool _grow;
};

struct FormList : public FormInput {
  FormList(ListModel *model, int x, int y, int w, int h);
  ~FormList() override;

  bool edit(int key, int screenWidth, int charWidth) override;
  const char *getText() const override { return _model->getTextAt(_model->selected()); }
  void optionSelected(int index) ;
  void updateForm(var_p_t form) override;
  bool updateUI(var_p_t form, var_p_t field) override;
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
  ~FormDropList() override = default;

  void clicked(int x, int y, bool pressed) override;
  void draw(int dx, int dy, int w, int h, int chw) override;
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) override;
  void updateForm(var_p_t form) override;
  bool isDrawTop() override { return _listActive; }
  int getListHeight() const override { return _listHeight; }

private:
  void drawList(int dx, int dy, int sh);
  int _listWidth;
  int _listHeight;
  int _visibleRows;
  bool _listActive;
};

struct FormListBox : public FormList {
  FormListBox(ListModel *model, const char *help, int x, int y, int w, int h);
  void clicked(int x, int y, bool pressed) override;
  void draw(int x, int y, int w, int h, int chw) override;
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) override;
  ~FormListBox() override = default;

private:
  String _help;
};

struct FormImage : public FormInput {
  FormImage(ImageDisplay *image, int x, int y);
  ~FormImage() override;
  void draw(int x, int y, int w, int h, int chw) override;

private:
  ImageDisplay *_image;
};

struct MenuButton : public FormButton {
  MenuButton(int index, int &selectedIndex,
             const char *caption, int x, int y, int w, int h);
  void clicked(int x, int y, bool pressed) override;
  void draw(int x, int y, int w, int h, int chw) override;

  int &_selectedIndex;
  int _index;
};

FormEditInput *get_focus_edit();

#endif
