// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "common/sys.h"
#include "common/sbapp.h"
#include "common/keymap.h"
#include "ui/utils.h"
#include "ui/inputs.h"
#include "ui/image.h"
#include "ui/system.h"

extern System *g_system;

FormList *activeList = NULL;
FormInput *focusInput = NULL;
FormEditInput *focusEdit = NULL;

#define LINE_Y (_height - 2)
#define LINE_W (_width - 2)

FormEditInput *get_focus_edit() {
  return focusEdit;
}

bool form_ui::optionSelected(int index) {
  bool result = false;
  if (activeList != NULL) {
    activeList->optionSelected(index);
    activeList = NULL;
    result = false;
  }
  return result;
}

int get_color(var_p_t value, int def) {
  int result = def;
  if (value != NULL && value->type == V_INT) {
    result = value->v.i;
    if (result < 0) {
      result = -result;
    } else if (result < 16) {
      result = colors[result];
    }
  } else if (value != NULL && value->type == V_STR &&
             value->v.p.length) {
    const char *n = value->v.p.ptr;
    if (n[0] == '0' && n[1] == 'x' && n[2]) {
      result = strtol(n + 2, NULL, 16);
    } else if (n[0] == '#' && n[1]) {
      result = strtol(n + 1, NULL, 16);
    } else if (strcasecmp(n, "black") == 0) {
      result = DEFAULT_BACKGROUND;
    } else if (strcasecmp(n, "red") == 0) {
      result = 0x800000;
    } else if (strcasecmp(n, "green") == 0) {
      result = 0x008000;
    } else if (strcasecmp(n, "yellow") == 0) {
      result = 0x808000;
    } else if (strcasecmp(n, "blue") == 0) {
      result = 0x000080;
    } else if (strcasecmp(n, "magenta") == 0 ||
               strcasecmp(n, "fuchsia") == 0) {
      result = 0x800080;
    } else if (strcasecmp(n, "cyan") == 0 ||
               strcasecmp(n, "aqua") == 0) {
      result = 0x008080;
    } else if (strcasecmp(n, "white") == 0) {
      result = 0xFFFFFF;
    } else if (strcasecmp(n, "gray") == 0 ||
               strcasecmp(n, "grey") == 0) {
      result = 0x808080;
    }
  }
  return result;
}

int lerp(int c0, int c1, float weight) {
  int result;
  if (weight <= 0) {
    result = c0;
  } else if (weight >= 1) {
    result = c1;
  } else {
    uint8_t r = ((uint8_t)(c1 >> 24)) * weight + ((uint8_t)(c0 >> 24)) * (1 - weight);
    uint8_t g = ((uint8_t)(c1 >> 16)) * weight + ((uint8_t)(c0 >> 16)) * (1 - weight);
    uint8_t b = ((uint8_t)(c1 >>  8)) * weight + ((uint8_t)(c0 >>  8)) * (1 - weight);
    result = (r << 24) + (g << 16) + (b << 8);
  }
  return result;
}

//
// FormInput
//
FormInput::FormInput(int x, int y, int w, int h) :
  Shape(x, y, w, h),
  _pressed(false),
  _id(0),
  _exit(false),
  _visible(true),
  _noFocus(false),
  _resizable(false),
  _bg(DEFAULT_BACKGROUND),
  _fg(DEFAULT_FOREGROUND),
  _onclick(0) {
}

FormInput::~FormInput() {
  if (focusInput == this) {
    focusInput = NULL;
  }
}

void FormInput::construct(var_p_t form, var_p_t field, int id) {
  _exit = (map_get_bool(field, FORM_INPUT_IS_EXIT));
  if (!_noFocus) {
    _noFocus = (map_get_bool(field, FORM_INPUT_NO_FOCUS));
  }
  _id = id;

  var_p_t v_id = map_get(field, FORM_INPUT_ID);
  if (v_id == NULL) {
    map_add_var(field, FORM_INPUT_ID, _id);
  } else {
    v_setint(v_id, _id);
  }

  var_p_t v_onclick = map_get(field, FORM_INPUT_ONCLICK);
  if (v_onclick != NULL && v_onclick->type == V_PTR) {
    _onclick = v_onclick->v.ap.p;
  }

  const char *caption = getText();
  MAExtent extent = maGetTextSize(caption != NULL && caption[0] ? caption : "Z");
  int textW = EXTENT_X(extent);
  int textH = EXTENT_Y(extent);

  if (_width <= 0 && caption != NULL) {
    _width = textW + padding(false);
  }

  if (_height <= 0 || _height < (textH + padding(true))) {
    _height = textH + padding(true);
  }

  AnsiWidget *out = g_system->getOutput();
  int formX = out->getX();
  int formY = out->getY();

  if (_x < 0) {
    _x = (formX - _x) - 1;
  }

  if (_y < 0) {
    _y = (formY - _y) - 1;
  }

  formX = _x + _width;
  formY = _y;

  if (formX > out->getWidth()) {
    formX = 0;
  }

  out->setXY(formX, formY);
  updateUI(form, field);
}

void FormInput::drawButton(const char *caption, int dx, int dy,
                           int w, int h, bool pressed) {
  int r = dx + w;
  int b = dy + h - 2;
  MAExtent textSize = maGetTextSize(caption);
  int textW = EXTENT_X(textSize);
  int textH = EXTENT_Y(textSize);
  int textX = dx + (w - textW - 1) / 2;
  int textY = dy + (h - textH - 1) / 2;

  maSetColor(_bg);
  maFillRect(dx, dy, r-dx, b-dy);

  if (pressed) {
    maSetColor(0x909090);
    maLine(dx, dy, r, dy); // top
    maLine(dx, dy, dx, b); // left
    maSetColor(0xd0d0d0);
    maLine(dx+1, b, r, b); // bottom
    maLine(r, dy+1, r, b); // right
    maSetColor(0x606060);
    maLine(dx+1, dy+1, r-1, dy+1); // top
    maLine(dx+1, dy+1, dx+1, b-1); // left
    textX += 2;
    textY += 2;
  } else {
    maSetColor(0xd0d0d0);
    maLine(dx, dy, r, dy); // top
    maLine(dx, dy, dx, b); // left
    maSetColor(0x606060);
    maLine(dx, b, r, b); // bottom
    maLine(r, dy, r, b); // right
    maSetColor(0x909090);
    maLine(dx+1, b-1, r-1, b-1); // bottom
    maLine(r-1, dy+1, r-1, b-1); // right
  }

  if (caption && caption[0]) {
    setTextColor();
    maDrawText(textX, textY, caption, strlen(caption));
  }
}

void FormInput::drawHover(int dx, int dy, bool selected) {
  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
  maSetColor(selected ? _fg : _bg);
  int y = _y + dy + LINE_Y;
  maLine(dx + _x + 2, y, dx + _x + LINE_W, y);
  maUpdateScreen();
  maSetDrawTarget(currentHandle);
}

void FormInput::drawLink(const char *caption, int dx, int dy, int sw, int chw) {
  maSetColor(_fg);
  drawText(caption, dx, dy, sw, chw);
}

void FormInput::drawText(const char *caption, int dx, int dy, int sw, int chw) {
  int strWidth = chw * (caption == NULL ? 0 : strlen(caption));
  int width = sw - dx;
  if (width > _width) {
    width = _width;
  }
  if (caption == NULL) {
    // nothing to draw
  } else if (strWidth > width) {
    int len = width / chw;
    if (len > 0) {
      char *buffer = new char[len + 1];
      strncpy(buffer, caption, len - 1);
      buffer[len - 1] = '~';
      buffer[len] = 0;
      maDrawText(dx, dy, buffer, len);
      delete [] buffer;
    }
  } else if (strWidth) {
    maDrawText(dx, dy, caption, strlen(caption));
  }
}

void FormInput::draw(int x, int y, int w, int h, int chw) {
  drawButton(getText(), x, y, _width, _height, _pressed);
}

bool FormInput::overlaps(MAPoint2d pt, int offsX, int offsY) {
  return !(OUTSIDE_RECT(pt.x, pt.y, _x + offsX, _y + offsY, _width, _height));
}

bool FormInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  return FormInput::overlaps(pt, scrollX, scrollY);
}

void FormInput::setTextColor() {
  maSetColor(hasFocus() || _noFocus ? _fg : lerp(_bg, _fg, 0.7));
}

void FormInput::setHelpTextColor() {
  maSetColor(lerp(_bg, _fg, 0.4));
}

// returns the field var attached to the field
var_p_t FormInput::getField(var_p_t form) {
  var_p_t result = NULL;
  if (form->type == V_MAP) {
    var_p_t inputs = map_get(form, FORM_INPUTS);
    if (inputs != NULL && inputs->type == V_ARRAY) {
      for (unsigned i = 0; i < v_asize(inputs) && !result; i++) {
        var_p_t elem = v_elem(inputs, i);
        if (elem->type == V_MAP && (_id == map_get_int(elem, FORM_INPUT_ID, -1))) {
          result = elem;
        }
      }
    }
  }
  return result;
}

// copy basic variable to widget state when the variable has changed
bool FormInput::updateUI(var_p_t form, var_p_t field) {
  bool updated = false;

  var_p_t var = map_get(field, FORM_INPUT_LABEL);
  if (var == NULL) {
    var = map_get(form, FORM_INPUT_LABEL);
  }
  if (var != NULL) {
    setText(v_getstr(var));
    updated = true;
  }

  var_p_t v_bg = map_get(field, FORM_INPUT_BG);
  if (v_bg == NULL) {
    v_bg = map_get(form, FORM_INPUT_BG);
  }
  if (v_bg != NULL) {
    int bg = get_color(v_bg, _bg);
    if (bg != _bg) {
      updated = true;
      _bg = bg;
    }
  }

  var_p_t v_fg = map_get(field, FORM_INPUT_FG);
  if (v_fg == NULL) {
    v_fg = map_get(form, FORM_INPUT_FG);
  }
  if (v_fg != NULL) {
    int fg = get_color(v_fg, _fg);
    if (fg != _fg) {
      updated = true;
      _fg = fg;
    }
  }

  bool visible = map_get_int(field, FORM_INPUT_VISIBLE, 1) != 0;
  if (visible != _visible) {
    updated = true;
    _visible = visible;
  }

  bool resizable = map_get_bool(field, FORM_INPUT_RESIZABLE);
  if (resizable != _resizable) {
    updated = true;
    _resizable = resizable;
  }

  return updated;
}

bool FormInput::edit(int key, int screenWidth, int charWidth) {
  bool result = false;
  if (key == SB_KEY_ENTER) {
    clicked(-1, -1, false);
    result = true;
  }
  return result;
}

// set the widget value onto the form value
void FormInput::updateForm(var_p_t form) {
  var_p_t field = getField(form);
  if (field != NULL) {
    var_p_t inputValue = map_get(field, FORM_INPUT_VALUE);
    var_p_t value = map_get(form, FORM_VALUE);
    if (value != NULL && inputValue != NULL) {
      v_set(value, inputValue);
    }
  }
}

bool FormInput::hasFocus() const {
  return (focusInput == this);
}

void FormInput::setFocus(bool focus) {
  focusEdit = NULL;
  if (!_noFocus && focus == (focusInput != this)) {
    focusInput = focus ? this : NULL;
    g_system->getOutput()->setDirty();
  }
}

//
// FormButton
//
FormButton::FormButton(const char *caption, int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _label(caption) {
}

//
// FormLabel
//
FormLabel::FormLabel(const char *caption, int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _label(caption) {
}

bool FormLabel::updateUI(var_p_t form, var_p_t field) {
  bool updated = FormInput::updateUI(form, field);
  var_p_t var = map_get(field, FORM_INPUT_LABEL);
  if (var != NULL && var->type == V_STR && !_label.equals(var->v.p.ptr)) {
    _label = var->v.p.ptr;
    updated = true;
  }
  return updated;
}

void FormLabel::draw(int x, int y, int w, int h, int chw) {
  int len = _label.length();
  if (len > 0) {
    if (len * chw >= _width) {
      len = _width / chw;
    }
    maSetColor(_fg);
    maDrawText(x, y, _label.c_str(), len);
  }
}

//
// FormLink
//
FormLink::FormLink(const char *link, bool external, int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _link(link),
  _external(external) {
}

void FormLink::draw(int x, int y, int w, int h, int chw) {
  drawLink(_link.c_str(), x, y, w, chw);
  if (_pressed) {
    maSetColor(_pressed ? _fg : _bg);
    maLine(x, y + LINE_Y, x + LINE_W, y + LINE_Y);
  }
}

//
// FormEditInput
//
FormEditInput::FormEditInput(int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _controlMode(false) {
}

FormEditInput::~FormEditInput() {
  if (focusEdit == this) {
    focusEdit = NULL;
  }
}

int FormEditInput::getControlKey(int key) {
  int result = key;
  if (_controlMode) {
    switch (key) {
    case 'x':
      g_system->setClipboardText(copy(true));
      result = -1;
      break;
    case 'c':
      g_system->setClipboardText(copy(false));
      result = -1;
      break;
    case 'v':
      paste(g_system->getClipboardText());
      result = -1;
      break;
    case 'h':
      result = SB_KEY_LEFT;
      break;
    case 'l':
      result = SB_KEY_RIGHT;
      break;
    case 'j':
      result = SB_KEY_HOME;
      break;
    case 'k':
      result = SB_KEY_END;
      break;
    case 'a':
      selectAll();
      break;
    }
  }
  return result;
}

void FormEditInput::setFocus(bool focus) {
  if (!_noFocus && focus == (focusInput != this)) {
    focusInput = focus ? this : NULL;
    focusEdit = focus ? this : NULL;
    g_system->getOutput()->setDirty();
  }
}

//
// FormLineInput
//
FormLineInput::FormLineInput(const char *value, const char *help, int size, bool grow,
                             int x, int y, int w, int h) :
  FormEditInput(x, y, w, h),
  _help(help),
  _buffer(NULL),
  _size(size),
  _scroll(0),
  _mark(-1),
  _point(0),
  _grow(grow) {
  _buffer = new char[_size + 1];
  _buffer[0] = '\0';
  if (value != NULL && value[0]) {
    int len = MIN(strlen(value), (unsigned)_size);
    memcpy(_buffer, value, len);
    _buffer[len] = '\0';
    _mark = _point = len;
  }
}

FormLineInput::~FormLineInput() {
  delete [] _buffer;
  _buffer = NULL;
}

void FormLineInput::clicked(int x, int y, bool pressed) {
  if (pressed && g_system->isRunning()) {
    AnsiWidget *out = g_system->getOutput();
    dev_clrkb();
    setFocus(true);
    focusEdit = this;
    int charWidth = out->getCharWidth();
    int selected = (x - _x) / charWidth;
    int len = strlen(_buffer);
    if (selected > len) {
      selected = len;
      _mark = selected;
    }
    if (selected > -1) {
      _point = selected;
      out->setDirty();
    }
  }
}

void FormLineInput::draw(int x, int y, int w, int h, int chw) {
  maSetColor(_bg);
  maFillRect(x, y, _width, _height);
  setTextColor();

  int len = strlen(_buffer + _scroll);
  if (len * chw >= _width) {
    len = _width / chw;
  }

  if (!len && !_help.empty()) {
    if (hasFocus()) {
      setTextColor();
      maFillRect(x, y, chw, _height);
    }
    setHelpTextColor();
    drawText(_help.c_str(), x + (chw * 0), y, w, chw);
  } else {
    maDrawText(x, y, _buffer + _scroll, len);
    if (_mark != _point && _mark != -1) {
      int chars = abs(_mark - _point);
      int start = MIN(_mark, _point);
      int width = chars * chw;
      int px = x + (start * chw);
      setTextColor();
      maFillRect(px, y, width, _height);
      maSetColor(_bg);
      maDrawText(px, y, _buffer + _scroll + start, chars);
    } else if (hasFocus()) {
      int px = x + (_point * chw);
      maFillRect(px, y, chw, _height);
      if (_point < len) {
        maSetColor(_bg);
        maDrawText(px, y, _buffer + _scroll + _point, 1);
      }
    }
  }
}

bool FormLineInput::edit(int key, int screenWidth, int charWidth) {
  int wChars;
  int len = _buffer == NULL ? 0 : strlen(_buffer);

  key = getControlKey(key);
  switch (key) {
  case SB_KEY_BACKSPACE:
    // backspace
    if (len > 0) {
      if (_mark != _point) {
        cut();
      } else {
        if (_scroll) {
          _scroll--;
        } else if (_point > 0) {
          _point--;
        }
        int j = _scroll + _point;
        while (j < len - 1) {
          _buffer[j] = _buffer[j + 1];
          j++;
        }
        _buffer[j] = '\0';
      }
    }
    break;
  case SB_KEY_DELETE:
    if (_mark != _point) {
      cut();
    } else {
      int j = _point + _scroll;
      while (j < len - 1) {
        _buffer[j] = _buffer[j + 1];
        j++;
      }
      _buffer[j] = '\0';
    }
    break;
  case SB_KEY_LEFT:
    if (_point > 0) {
      _point--;
    } else if (_scroll > 0) {
      _scroll--;
    }
    break;
  case SB_KEY_RIGHT:
    if ((_scroll + _point) < len) {
      int maxPoint = (_width / charWidth) - 1;
      if (_point < maxPoint) {
        _point++;
      } else {
        _scroll++;
      }
    }
    break;
  case SB_KEY_HOME:
    _scroll = 0;
    _mark = _point = 0;
    break;
  case SB_KEY_END:
    wChars = (_width / charWidth) - 1;
    if (len > wChars) {
      _mark = _point = wChars;
      _scroll = len - wChars;
    } else {
      _mark = _point = len;
      _scroll = 0;
    }
    break;
  default:
    if (key >= SB_KEY_SPACE && !_controlMode) {
      // insert
      if (_mark != _point) {
        cut();
      }
      if (len < _size - 1) {
        int j = len;
        int point = _scroll + _point;
        if (point < len) {
          // insert
          while (j >= point) {
            _buffer[j + 1] = _buffer[j];
            j--;
          }
        }

        _buffer[point] = key;
        _buffer[++len] = '\0';

        if (_grow && (_x + _width + charWidth < screenWidth)
            && (len * charWidth) >= _width) {
          _width += charWidth;
        }
        int maxPoint = (MIN(_width, screenWidth) / charWidth) - 1;
        if (_point < maxPoint) {
          _point++;
        } else {
          _scroll++;
        }
      }
    } else if (key > 0) {
      maShowVirtualKeyboard();
    }
    break;
  }
  if (key != -1) {
    _mark = _point;
  }
  return true;
}

void FormLineInput::selectAll() {
  _point = 0;
  _mark = _buffer == NULL ? -0 : strlen(_buffer);
}

void FormLineInput::updateField(var_p_t form) {
  var_p_t field = getField(form);
  if (field != NULL) {
    var_p_t value = map_get(field, FORM_INPUT_VALUE);
    if (value != NULL) {
      v_setstr(value, _buffer);
    }
  }
}

bool FormLineInput::updateUI(var_p_t form, var_p_t field) {
  bool updated = FormInput::updateUI(form, field);
  var_p_t var = map_get(field, FORM_INPUT_VALUE);
  if (var != NULL) {
    const char *value = v_getstr(var);
    if (value && strcmp(value, _buffer) != 0) {
      int len = MIN(strlen(value), (unsigned)_size);
      memcpy(_buffer, value, len);
      _buffer[len] = '\0';
      _mark = _point = len;
      updated = true;
    }
  }
  return updated;
}

char *FormLineInput::copy(bool cut) {
  char *result = NULL;
  int len = strlen(_buffer);
  int start = MIN(_mark, _point);
  if (_mark != -1 && start < len) {
    int chars = _mark == _point ? 1 : abs(_mark - _point);
    char *offset = _buffer + _scroll + start;
    result = (char *)malloc(chars + 1);
    memcpy(result, offset, chars);
    result[chars] = '\0';
    if (cut) {
      _mark = _point = start;
      memcpy(offset, offset + chars, chars);
      len -= chars;
      _buffer[len] = '\0';
    }
  }
  return result;
}

void FormLineInput::cut() {
  int len = strlen(_buffer);
  int start = MIN(_mark, _point);
  if (_mark != -1 && start < len) {
    int chars = _mark == _point ? 1 : abs(_mark - _point);
    char *offset = _buffer + _scroll + start;
    _mark = _point = start;
    memcpy(offset, offset + chars, chars);
    len -= chars;
    _buffer[len] = '\0';
  }
}

void FormLineInput::paste(const char *text) {
  int len = strlen(_buffer);
  int avail = _size - (len + 1);
  if (text != NULL && avail > 0) {
    int index = _scroll + _point;
    int count = strlen(text);
    if (count > avail) {
      count = avail;
    }

    // move existing text to the right
    int size = len - index;
    int end = len + count - 1;
    for (int i = 0; i < size; i++) {
      _buffer[end - i] = _buffer[len - i - 1];
    }

    // terminate buffer
    _buffer[len + count] = '\0';

    // insert new text
    for (int i = 0; i < count; i++) {
      _buffer[index + i] = text[i];
    }

    // remove selection
    _mark = _point;

    if (_grow) {
      // enlarge the edit box for the number of inserted characters
      int charWidth = g_system->getOutput()->getCharWidth();
      int maxSize = g_system->getOutput()->getScreenWidth() - _x;
      int textSize = (len + count + 1) * charWidth;
      if (textSize > maxSize) {
        textSize = maxSize;
      }
      if (textSize > _width) {
        _width = textSize;
      }
    }
  }
}

//
// ListModel
//
ListModel::ListModel(int index, var_t *v) :
  _selectedIndex(index) {
  create(v);
}

void ListModel::clear() {
  _list.removeAll();
}

void ListModel::create(var_t *v) {
  if (v->type == V_ARRAY) {
    fromArray(v);
  } else if (v->type == V_STR) {
    // construct from a string like "Easy|Medium|Hard"
    const char *items = v->v.p.ptr;
    int len = items ? strlen(items) : 0;
    for (int i = 0; i < len; i++) {
      const char *c = strchr(items + i, '|');
      int end_index = c ? c - items : len;
      if (end_index > 0) {
        strlib::String *s = new strlib::String(items + i, end_index - i);
        _list.add(s);
        i = end_index;
      }
    }
  }
  if (_selectedIndex == -1) {
    _selectedIndex = 0;
  }
}

// construct from an array of values
void ListModel::fromArray(var_t *v) {
  for (unsigned i = 0; i < v_asize(v); i++) {
    var_t *el_p = v_elem(v, i);
    if (el_p->type == V_STR) {
      _list.add(new strlib::String((const char *)el_p->v.p.ptr));
    } else if (el_p->type == V_INT) {
      char buff[40];
      sprintf(buff, VAR_INT_FMT, el_p->v.i);
      _list.add(new strlib::String(buff));
    } else if (el_p->type == V_ARRAY) {
      fromArray(el_p);
    }
  }
}

// return the text at the given index
const char *ListModel::getTextAt(int index) {
  const char *s = 0;
  if (index > -1 && index < _list.size()) {
    s = _list[index]->c_str();
  }
  return s;
}

// returns the model index corresponding to the given string
int ListModel::getIndex(const char *t) {
  int size = _list.size();
  for (int i = 0; i < size; i++) {
    if (!strcasecmp(_list[i]->c_str(), t)) {
      return i;
    }
  }
  return -1;
}

//
// FormList
//
FormList::FormList(ListModel *model, int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _model(model),
  _topIndex(0),
  _activeIndex(-1) {
}

FormList::~FormList() {
  if (activeList == this) {
    activeList = NULL;
  }
  delete _model;
  _model = NULL;
}

void FormList::optionSelected(int index) {
  if (index > -1 && index < _model->rows()) {
    _model->selected(index);
  }
}

void FormList::selectIndex(int index) {
  if (index > -1 && index < _model->rows()) {
    MAExtent textSize = maGetTextSize(_model->getTextAt(0));
    int rowHeight = EXTENT_Y(textSize) + 1;
    int visibleRows = getListHeight() / rowHeight;
    if (index < visibleRows) {
      _activeIndex = index;
      _topIndex = 0;
    } else {
      _topIndex = index - visibleRows + 1;
      _activeIndex = index - _topIndex;
    }
    _model->selected(index);
  }
}

bool FormList::updateUI(var_p_t form, var_p_t field) {
  bool updated = FormInput::updateUI(form, field);
  var_p_t var = map_get(field, FORM_INPUT_VALUE);
  if (var != NULL && (var->type == V_ARRAY || var->type == V_STR)) {
    // update list control with new array or string variable
    _model->clear();
    _model->create(var);
    updated = true;
  }

  // set the selectedIndex
  var = map_get(field, FORM_INPUT_INDEX);
  if (var != NULL) {
    selectIndex(v_getint(var));
    updated = true;
  }
  return updated;
}

// transfer the widget state onto the associated variable
void FormList::updateForm(var_p_t form) {
  const char *selected = getText();

  // set the form value
  var_p_t value = map_get(form, FORM_VALUE);
  if (value == NULL) {
    value = map_add_var(form, FORM_VALUE, 0);
  }
  if (selected) {
    v_setstr(value, selected);
  } else {
    v_zerostr(value);
  }

  // set the selectedIndex
  var_p_t field = getField(form);
  if (field != NULL) {
    value = map_get(field, FORM_INPUT_INDEX);
    if (value != NULL) {
      v_setint(value, _model->selected());
    }
  }
}

bool FormList::edit(int key, int screenWidth, int charWidth) {
  if (key == SB_KEY_UP || key == SB_KEY_DOWN) {
    MAExtent textSize = maGetTextSize(_model->getTextAt(0));
    int rowHeight = EXTENT_Y(textSize) + 1;
    int visibleRows = getListHeight() / rowHeight;
    int activeIndex = _activeIndex;
    int topIndex = _topIndex;

    if (key == SB_KEY_UP) {
      if (_activeIndex > 0) {
        _activeIndex--;
      } else if (_topIndex > 0) {
        _topIndex--;
      }
    } else if (key == SB_KEY_DOWN &&
               _activeIndex + _topIndex < _model->rows() - 1) {
      if (_activeIndex == visibleRows - 1) {
        _topIndex++;
      } else {
        _activeIndex++;
      }
    }
    if (activeIndex != _activeIndex || topIndex != _topIndex) {
      _model->selected(_topIndex + _activeIndex);
      selected();
    }
  } else if (key == SB_KEY_ENTER) {
    clicked(-1, -1, false);
  } else {
    // match by keystroke
    int firstIndex = -1;
    int lastIndex = -1;
    for (int index = 0; index < _model->rows(); index++) {
      const char *text = _model->getTextAt(index);
      if (text && tolower(text[0]) == tolower(key)) {
        if (firstIndex == -1) {
          firstIndex = index;
        }
        if (index > _activeIndex + _topIndex) {
          lastIndex = index;
          break;
        }
      }
    }
    int index = lastIndex != -1 ? lastIndex : firstIndex;
    if (index != -1) {
      selectIndex(index);
      selected();
    }
  }
  return true;
}

//
// FormListBox
//
FormListBox::FormListBox(ListModel *model, const char *help, int x, int y, int w, int h) :
  FormList(model, x, y, w, h), _help(help) {
}

void FormListBox::draw(int x, int y, int w, int h, int chw) {
  maSetColor(_bg);
  maFillRect(x, y, _width, _height);
  MAExtent textSize = maGetTextSize(_model->getTextAt(0));
  int rowHeight = EXTENT_Y(textSize) + 1;
  int textY = y;
  if (!_model->rows() && !_help.empty()) {
    setHelpTextColor();
    drawText(_help.c_str(), x, textY, w, chw);
  } else {
    for (int i = 0; i < _model->rows(); i++) {
      const char *str = _model->getTextAt(i + _topIndex);
      if (textY + rowHeight >= y + _height) {
        break;
      }
      if (i == _activeIndex) {
        setTextColor();
        maFillRect(x, textY, _width, rowHeight);
        maSetColor(_bg);
        drawText(str, x, textY, w, chw);
      } else {
        setTextColor();
        drawText(str, x, textY, w, chw);
      }
      textY += rowHeight;
    }
  }
}

bool FormListBox::selected(MAPoint2d pt, int offsX, int offsY, bool &redraw) {
  bool result = FormInput::overlaps(pt, offsX, offsY);
  MAExtent textSize = maGetTextSize(_model->getTextAt(0));
  int rowHeight = EXTENT_Y(textSize) + 1;
  int visibleRows = _height / rowHeight;
  if (result) {
    int y = pt.y - (_y + offsY);
    _activeIndex = y / rowHeight;
    activeList = this;
    redraw = true;
  } else if (activeList == this &&
             pt.y < _y + offsY + _height &&  _topIndex > 0) {
    _topIndex--;
    redraw = true;
  } else if (activeList == this &&
             pt.y > _y + offsY + _height &&
             _topIndex + visibleRows < _model->rows()) {
    _topIndex++;
    redraw = true;
  } else {
    activeList = NULL;
  }
  return result;
}

//
// FormDropList
//
FormDropList::FormDropList(ListModel *model, int x, int y, int w, int h) :
  FormList(model, x, y, w, h),
  _listWidth(w),
  _listHeight(h),
  _visibleRows(0),
  _listActive(false) {
}

void FormDropList::draw(int x, int y, int w, int h, int chw) {
  if (!hasFocus()) {
    _listActive = false;
  }
  bool pressed = _listActive ? false : _pressed;
  drawButton(getText(), x, y, _width - CHOICE_BN_W, _height, pressed);
  drawButton("", x + _width - CHOICE_BN_W, y, CHOICE_BN_W, _height, false);
  if (_listActive) {
    drawList(x, y, h);
  }
}

void FormDropList::drawList(int dx, int dy, int sh) {
  int availHeight = sh - (dy + _y + _height + _height);
  int textHeight = 0;
  int textY = dy + _height;

  // determine the available boundary
  _listHeight = 0;
  _visibleRows = 0;
  for (int i = _topIndex; i < _model->rows(); i++) {
    MAExtent textSize = maGetTextSize(_model->getTextAt(i));
    int textWidth = EXTENT_X(textSize);
    textHeight = EXTENT_Y(textSize) + 1;
    if (textWidth > _listWidth) {
      _listWidth = textWidth;
    }
    if (_listHeight + textHeight >= availHeight) {
      break;
    }
    _listHeight += textHeight;
    _visibleRows++;
  }
  maSetColor(_bg);
  maFillRect(dx, dy + _height, _listWidth, _listHeight);
  for (int i = 0; i < _visibleRows; i++) {
    const char *str = _model->getTextAt(i + _topIndex);
    if (i == _activeIndex) {
      setTextColor();
      maFillRect(dx, textY, _listWidth, textHeight);
      maSetColor(_bg);
      maDrawText(dx, textY, str, strlen(str));
    } else {
      setTextColor();
      maDrawText(dx, textY, str, strlen(str));
    }
    textY += textHeight;
  }
}

bool FormDropList::selected(MAPoint2d pt, int offsX, int offsY, bool &redraw) {
  bool result;
  if (_listActive) {
    result = true;
    _activeIndex = -1;
    if (!(OUTSIDE_RECT(pt.x, pt.y, _x + offsX, _y + offsY + _height,
                       _listWidth, _listHeight))) {
      int y = pt.y - (_y + offsY + _height);
      int rowHeight = _listHeight / _visibleRows;
      _activeIndex = y / rowHeight;
      activeList = this;
      redraw = true;
    } else if (activeList == this &&
               pt.y < _y + offsY + _height && _topIndex > 0) {
      _topIndex--;
      redraw = true;
    } else if (activeList == this &&
               pt.y > _y + offsY + _height + _listHeight &&
               _topIndex + _visibleRows < _model->rows()) {
      _topIndex++;
      redraw = true;
    }
  } else {
    result = FormInput::overlaps(pt, offsX, offsY);
  }
  return result;
}

void FormDropList::updateForm(var_p_t form) {
  if (!_listActive) {
    _activeIndex = -1;
  } else if (_activeIndex != -1) {
    optionSelected(_activeIndex + _topIndex);
  }
  FormList::updateForm(form);
}

//
// FormImage
//
FormImage::FormImage(ImageDisplay *image, int x, int y) :
  FormInput(x, y, image->_width + 2, image->_height + 2),
  _image(image) {
  _image->_x = x;
  _image->_y = y;
}

FormImage::~FormImage() {
  delete _image;
}

void FormImage::draw(int x, int y, int w, int h, int chw) {
  int dx = _pressed ? x + 1 : x;
  int dy = _pressed ? y + 1 : y;
  maSetColor(_bg);
  maFillRect(x, y, _width, _height);
  _image->draw(dx + 1, dy + 1, w, h, chw);
}

//
// MenuButton
//
MenuButton::MenuButton(int index, int &selectedIndex,
                           const char *caption, int x, int y, int w, int h) :
  FormButton(caption, x, y, w, h),
  _index(index),
  _selectedIndex(selectedIndex) {
}

void MenuButton::clicked(int x, int y, bool pressed) {
  if (!pressed) {
    _selectedIndex = _index;
  }
}

void MenuButton::draw(int x, int y, int w, int h, int chw) {
  maSetColor(_pressed ? _bg : _fg);
  maFillRect(x, y, _width, _height);
  int len = _label.length();
  if (len > 0) {
    if (len * chw >= _width) {
      len = _width / chw;
    }
    int charHeight = g_system->getOutput()->getCharHeight();
    int textY = y + (_height - charHeight) / 2;

    maSetColor(_pressed ? _fg : _bg);
    maDrawText(x + 4, textY, _label.c_str(), len);
    if (!_pressed && _index > 0 && _index % 2 == 0) {
      maSetColor(0x3b3a36);
      maLine(x + 2, y, x + LINE_W, y);
      maSetColor(0x46453f);
      maLine(x + 2, y - 1, x + LINE_W, y - 1);
    }
  }
}
