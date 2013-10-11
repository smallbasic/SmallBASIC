// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "platform/common/ansiwidget.h"
#include "platform/common/utils.h"

/* class AnsiWidget

  Displays ANSI escape codes.

  Escape sequences start with the characters ESC (ASCII 27d / 1Bh / 033o )
  and [ (left bracket). This sequence is called CSI for
  "Control Sequence Introducer".

  For more information about ANSI code see:
  http://en.wikipedia.org/wiki/ANSI_escape_code
  http://www.uv.tietgen.dk/staff/mlha/PC/Soft/Prog/BAS/VB/Function.html
  http://bjh21.me.uk/all-escapes/all-escapes.txt

  Supported control codes:
  \t      tab (20 px)
  \a      beep
  \r      return
  \n      next line
  \xC     clear screen (new page)
  \e[K    clear to end of line
  \e[0m   reset all attributes to their defaults
  \e[1m   set bold on
  \e[4m   set underline on
  \e[7m   reverse video
  \e[21m  set bold off
  \e[24m  set underline off
  \e[27m  set reverse off
*/

#define BUTTON_PADDING 10
#define OVER_SCROLL 100
#define SWIPE_MAX_TIMER 6000
#define SWIPE_DELAY_STEP 250
#define SWIPE_SCROLL_FAST 30
#define SWIPE_SCROLL_SLOW 20
#define SWIPE_TRIGGER_FAST 55
#define SWIPE_TRIGGER_SLOW 80
#define FONT_FACTOR 30

char *options = NULL;
FormList *clickedList = NULL;

#if !defined(_FLTK) && !defined(_TIZEN)
void form_ui::optionsBox(StringList *items) {
  if (items->size()) {
    // calculate the size of the options buffer
    int optionsBytes = sizeof(int);
    List_each(String*, it, *items) {
      const char *str = (*it)->c_str();
      optionsBytes += (strlen(str) + 1) * sizeof(wchar);
    }

    // create the options buffer
    delete [] options;
    options = new char[optionsBytes];
    *(int *)options = items->size();
    wchar_t *dst = (wchar_t *)(options + sizeof(int));

    List_each(String*, it, *items) {
      const char *str = (*it)->c_str();
      int len = strlen(str);
      swprintf(dst, len + 1, L"%hs", str);
      dst[len] = 0;
      dst += (len + 1);
    }
    maOptionsBox(L"SmallBASIC", NULL, L"Close", (MAAddress)options, optionsBytes);
  }
}
#endif

Widget::Widget(int bg, int fg, int x, int y, int w, int h) :
  Shape(x, y, w, h),
  _pressed(false),
  _bg(bg),
  _fg(fg) {
}

void Widget::drawButton(const char *caption, int dx, int dy) {
  int r = dx + width;
  int b = dy + height - 2;
  MAExtent textSize = maGetTextSize(caption);
  int textW = EXTENT_X(textSize);
  int textH = EXTENT_Y(textSize);
  int textX = dx + (width - textW - 1) / 2;
  int textY = dy + (height - textH - 1) / 2;

  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(dx, dy, r-dx, b-dy);

  if (_pressed) {
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

  maSetColor(_fg);
  maDrawText(textX, textY, caption);
}

void Widget::drawLink(const char *caption, int dx, int dy) {
  maSetColor(_fg);
  maDrawText(dx, dy, caption);
  maSetColor(_pressed ? _fg : _bg);
  maLine(dx + 2, dy + height + 1, dx + width, dy + height + 1);
}

bool Widget::overlaps(MAPoint2d pt, int offsX, int offsY) {
  return !(OUTSIDE_RECT(pt.x, pt.y, x + offsX, y + offsY, width, height));
}

// returns setBG when the program colours are default
int Widget::getBackground(int buttonColor) {
  int result = _bg;
  if (_fg == DEFAULT_FOREGROUND && _bg == DEFAULT_BACKGROUND) {
    result = buttonColor;
  }
  return result;
}

Button::Button(Screen *screen, const char* action, const char *label,
               int x, int y, int w, int h) :
  Widget(screen->_bg, screen->_fg, x, y, w, h),
  _action(action),
  _label(label) {
  screen->_shapes.add(this);
}

void Button::clicked(IButtonListener *listener, int x, int y) {
  listener->buttonClicked(_action.c_str());
}

FormWidget::FormWidget(Screen *screen, int x, int y, int w, int h) :
  Widget(screen->_bg, screen->_fg, x, y, w, h),
  _screen(screen),
  _listener(NULL) {
  getScreen()->add(this); 
}

FormWidget::~FormWidget() {
  getScreen()->remove(this);
}

void FormWidget::clicked(IButtonListener *, int x, int y) {
  if (this->_listener) {
    this->_listener->buttonClicked(NULL); 
  }
}

FormButton::FormButton(Screen *screen, const char *caption, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  _caption(caption) {
}

void FormButton::clicked(IButtonListener *listener, int x, int y) {
  if (this->_listener) {
    this->_listener->buttonClicked(_caption); 
  } else {
    listener->buttonClicked(_caption); 
  }
}

FormLabel::FormLabel(Screen *screen, const char *caption, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  _caption(caption) {
}

FormLink::FormLink(Screen *screen, const char *link, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  _link(link) {
}

FormLineInput::FormLineInput(Screen *screen, char *buffer, int maxSize, 
                             int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  _buffer(buffer),
  _maxSize(maxSize),
  _scroll(0) {
}

void FormLineInput::draw(int dx, int dy) {
  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(dx, dy, width, height);
  maSetColor(_fg);
  maDrawText(dx, dy, _buffer + _scroll);
}

bool FormLineInput::edit(int key) {
  bool changed = false;
  int len = strlen(_buffer);

  if (key >= MAK_SPACE && key <= 255) {
    // insert
    if (len < _maxSize - 1) {
      _buffer[len] = key;
      _buffer[++len] = '\0';
      MAExtent textSize = maGetTextSize(_buffer);
      int textWidth = EXTENT_X(textSize);
      if (textWidth > width) {
        if (textWidth > getScreen()->width) {
          _scroll++;
        } else {
          width += getScreen()->_charWidth;
        }
      }
      changed = true;
    }
  } else if (key == MAK_CLEAR) {
    // delete
    if (len > 0) {
      _buffer[len - 1] = '\0';
      if (_scroll) {
        _scroll--;
      }
      changed = true;
    }
  } else if (key) {
    maShowVirtualKeyboard();
  }

  return changed;
}

FormList::FormList(Screen *screen, IFormWidgetListModel *model,
                   int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  _model(model) {
  if (model->rows()) {
    model->selected(0);
  }
}

void FormList::clicked(IButtonListener *listener, int x, int y) {
  List<String *> options;
  for (int i = 0; i < _model->rows(); i++) {
    options.add(_model->getTextAt(i));
  }
  clickedList = this;
  form_ui::optionsBox(&options);
}

void FormList::draw(int x, int y) {
  if (_model) {
    drawButton(getText(), x, y);
  }
}

void FormList::optionSelected(int index) {
  if (index < _model->rows()) {
    _model->selected(index);
    FormWidget::clicked(NULL, -1, -1);
  }
}

AnsiWidget::AnsiWidget(IButtonListener *listener, int width, int height) :
  _back(NULL),
  _front(NULL),
  _focus(NULL),
  _width(width),
  _height(height),
  _xTouch(-1),
  _yTouch(-1),
  _xMove(-1),
  _yMove(-1),
  _moveTime(0),
  _moveDown(false),
  _swipeExit(false),
  _buttonListener(listener),
  _activeButton(NULL) {
  for (int i = 0; i < MAX_SCREENS; i++) {
    _screens[i] = NULL;
  }
  _fontSize = MIN(width, height) / FONT_FACTOR;
  trace("width: %d height: %d fontSize:%d", _width, height, _fontSize);
}

bool AnsiWidget::construct() {
  bool result = false;
  _back = new GraphicScreen(_width, _height, _fontSize);
  if (_back && _back->construct()) {
    _screens[0] = _front = _back;
    clearScreen();
    result = true;
  }
  return result;
}

// widget clean up
AnsiWidget::~AnsiWidget() {
  for (int i = 0; i < MAX_SCREENS; i++) {
    delete _screens[i];
  }
  delete [] options;
}

// create audible beep sound
void AnsiWidget::beep() const {
  // http://www.mosync.com/documentation/manualpages/using-audio-api
}

// creates a Button attached to the current back screen
IFormWidget *AnsiWidget::createButton(char *caption, int x, int y, int w, int h) {
  FormButton *button = new FormButton(_back, caption, x, y, w, h);
  return button;
}

// creates a Label attached to the current back screen
IFormWidget *AnsiWidget::createLabel(char *caption, int x, int y, int w, int h) {
  FormLabel *label = new FormLabel(_back, caption, x, y, w, h);
  return label;
}

// creates a LineInput attached to the current back screen
IFormWidget *AnsiWidget::createLineInput(char *buffer, int maxSize,
                                         int x, int y, int w, int h) {
  FormLineInput *lineInput = new FormLineInput(_back, buffer, maxSize, x, y, w, h);
  return lineInput;
}

// creates a form based hyperlink
IFormWidget *AnsiWidget::createLink(char *caption, int x, int y, int w, int h) {
  FormLink *result;
  if (w == 0 && h == 0) {
    result = (FormLink *)createLink(caption, caption, true, false);
  } else {
    result = new FormLink(_back, caption, x, y, w, h);
  }
  return result;
}

// creates a List attached to the current back screen
IFormWidget *AnsiWidget::createList(IFormWidgetListModel *model, 
                                    int x, int y, int w, int h) {
  FormList *list = new FormList(_back, model, x, y, w, h);
  return list;
}

// draws the given image onto the offscreen buffer
void AnsiWidget::drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h) {
  // TODO - draw image
  flush(false);
}

// draw a line onto the offscreen buffer
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  _back->drawLine(x1, y1, x2, y2);
  flush(false, false, MAX_PENDING_GRAPHICS);
}

// draw a rectangle onto the offscreen buffer
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  _back->drawRect(x1, y1, x2, y2);
  flush(false, false, MAX_PENDING_GRAPHICS);
}

// draw a filled rectangle onto the offscreen buffer
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  _back->drawRectFilled(x1, y1, x2, y2);
  flush(false, false, MAX_PENDING_GRAPHICS);
}

// perform editing when the formWidget belongs to the front screen
void AnsiWidget::edit(IFormWidget *formWidget, int c) {
  if (_front == ((FormWidget *)formWidget)->getScreen()) {
    if (formWidget->edit(c)) {
      redraw();
    }
  }
}

// display and pending images changed
void AnsiWidget::flush(bool force, bool vscroll, int maxPending) {
  bool update = false;
  if (_front != NULL) {
    if (force) {
      update = _front->_dirty;
    } else if (_front->_dirty) {
      update = (maGetMilliSecondCount() - _front->_dirty >= maxPending);
    }
    if (update) {
      _front->drawBase(vscroll);
    }
  }
}

// returns the color of the pixel at the given xy location
int AnsiWidget::getPixel(int x, int y) {
  return _back->getPixel(x, y);
}

// Returns the height in pixels using the current font setting
int AnsiWidget::textHeight(void) {
  return _back->_charHeight;
}

bool AnsiWidget::optionSelected(int index) {
  bool result = false;
  if (clickedList != NULL) {
    clickedList->optionSelected(index);
    clickedList = NULL;
    result = false;
  } 
  return result;
}

// prints the contents of the given string onto the backbuffer
void AnsiWidget::print(const char *str) {
  int len = strlen(str);
  if (len) {
    _back->drawInto();

    int lineHeight = textHeight();

    // copy the string to allow length manipulation
    char *buffer = new char[len + 1];
    strncpy(buffer, str, len);
    buffer[len] = 0;
    char *p = buffer;

    while (*p) {
      switch (*p) {
      case '\a':   // beep
        beep();
        break;
      case '\t':
        _back->calcTab();
        break;
      case '\003': // end of text
        flush(true);
        break;
      case '\xC':
        clearScreen();
        break;
      case '\033': // ESC ctrl chars
        handleEscape(p, lineHeight);
        break;
      case '\034':
        // file separator
        break;
      case '\n':   // new line
        _back->newLine(lineHeight);
        break;
      case '\r':   // return
        _back->_curX = INITXY;     // erasing the line will clear any previous text
        break;
      default:
        p += _back->print(p, lineHeight) - 1; // allow for p++
        break;
      };

      if (*p == '\0') {
        break;
      }
      p++;
    }

    // cleanup
    delete [] buffer;
    flush(false);
  }
}

// redraws and flushes the front screen
void AnsiWidget::redraw() {
  _front->drawInto();
  flush(true);
}

// reinit for new program run
void AnsiWidget::reset() {
  _back = _front = _screens[0];
  _back->reset(_fontSize);
  _back->clear();

  // reset non-system screens
  for (int i = 1; i < SYSTEM_SCREENS; i++) {
    delete _screens[i];
    _screens[i] = NULL;
  }
}

// update the widget to new dimensions
void AnsiWidget::resize(int newWidth, int newHeight) {
  int lineHeight = textHeight();
  for (int i = 0; i < MAX_SCREENS; i++) {
    Screen *screen = _screens[i];
    if (screen) {
      screen->resize(newWidth, newHeight, _width, _height, lineHeight);
      if (screen == _front || i < SYSTEM_SCREENS) {
        screen->drawBase(false);
      }
    }
  }
  _width = newWidth;
  _height = newHeight;
}

// sets the current drawing color
void AnsiWidget::setColor(long fg) {
  _back->setColor(fg);
}

// sets the text font size
void AnsiWidget::setFontSize(int fontSize) {
  this->_fontSize = fontSize;
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (_screens[i] != NULL) {
      _screens[i]->reset(fontSize);
    }
  }
  redraw();
}

// sets the pixel to the given color at the given xy location
void AnsiWidget::setPixel(int x, int y, int c) {
  _back->setPixel(x, y, c);
  flush(false, false, MAX_PENDING_GRAPHICS);
}

// sets the current text drawing color
void AnsiWidget::setTextColor(long fg, long bg) {
  _back->setTextColor(fg, bg);
}

// handler for pointer touch events
bool AnsiWidget::pointerTouchEvent(MAEvent &event) {
  bool result = false;
  // hit test buttons on the front screen
  if (setActiveButton(event, _front)) {
    _focus = _front;
  } else {
    // hit test buttons on remaining screens
    for (int i = 0; i < MAX_SCREENS; i++) {
      if (_screens[i] != NULL && _screens[i] != _front) {
        if (setActiveButton(event, _screens[i])) {
          _focus = _screens[i];
          break;
        }
      }
    }
  }
  // paint the pressed button
  if (_activeButton != NULL) {
    drawActiveButton();
  }
  // setup vars for page scrolling
  if (_front->overlaps(event.point.x, event.point.y)) {
    _xTouch = _xMove = event.point.x;
    _yTouch = _yMove = event.point.y;
    result = true;
  }
  return result;
}

// handler for pointer move events
bool AnsiWidget::pointerMoveEvent(MAEvent &event) {
  bool result = false;
  if (_activeButton != NULL) {
    bool pressed = _activeButton->overlaps(event.point, _focus->x,
                                          _focus->y - _focus->_scrollY);
    if (pressed != _activeButton->_pressed) {
      _activeButton->_pressed = pressed;
      drawActiveButton();
      result = true;
    }
  } else if (!_swipeExit) {
    // scroll up/down
    if (_front->overlaps(event.point.x, event.point.y)) {
      int vscroll = _front->_scrollY + (_yMove - event.point.y);
      int maxScroll = (_front->_curY - _front->height) + (2 * _fontSize);
      if (vscroll < 0) {
        vscroll = 0;
      }
      if (vscroll != _front->_scrollY && maxScroll > 0 && 
          vscroll < maxScroll + OVER_SCROLL) {
        _moveTime = maGetMilliSecondCount();
        _moveDown = (_front->_scrollY < vscroll);
        _front->drawInto();
        _front->_scrollY = vscroll;
        _xMove = event.point.x;
        _yMove = event.point.y;
        flush(true, true);
        result = true;
      }
    }
  }
  return result;
}

// handler for pointer release events
void AnsiWidget::pointerReleaseEvent(MAEvent &event) {
  if (_activeButton != NULL && _activeButton->_pressed) {
    _activeButton->_pressed = false;
    drawActiveButton();
    _activeButton->clicked(_buttonListener, event.point.x, event.point.y);
  } else if (_swipeExit) {
    _swipeExit = false;
  } else {
    int maxScroll = (_front->_curY - _front->height) + (2 * _fontSize);
    if (_yMove != -1 && maxScroll > 0) {
      _front->drawInto();
      bool swiped = (abs(_xTouch - _xMove) > (_width / 3) ||
                     abs(_yTouch - _yMove) > (_height / 3));
      int start = maGetMilliSecondCount();
      if (swiped && start - _moveTime < SWIPE_TRIGGER_SLOW) {
        doSwipe(start, maxScroll);
      } else if (_front->_scrollY > maxScroll) {
        _front->_scrollY = maxScroll;
      }
      // ensure the scrollbar is removed
      _front->_dirty = true;
      flush(true);
      _moveTime = 0;
    }
  }

  _xTouch = _xMove = -1;
  _yTouch = _yMove = -1;
  _activeButton = NULL;
  _focus = NULL;
}

// creates a status-bar label
void AnsiWidget::createLabel(char *&p) {
  List<String *> *items = getItems(p);
  const char *label = items->size() > 0 ? (*items)[0]->c_str() : "";
  _back->_label = label;
  delete items;
}

// creates a hyperlink, eg // ^[ hwww.foo.com|title;More text
Widget *AnsiWidget::createLink(char *&p, bool formLink, bool button) {
  List<String *> *items = getItems(p);
  const char *action = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *text = items->size() > 1 ? (*items)[1]->c_str() : action;
  Widget *result = createLink(action, text, formLink, button);
  delete items;
  return result;
}

Widget *AnsiWidget::createLink(const char *action, const char *text,
                               bool formLink, bool button) {
  MAExtent textSize = maGetTextSize(text);
  int w = EXTENT_X(textSize) + 2;
  int h = EXTENT_Y(textSize) + 2;
  int x = _back->_curX;
  int y = _back->_curY;

  if (button) {
    w += BUTTON_PADDING;
    h += BUTTON_PADDING;
    _back->_linePadding = BUTTON_PADDING;
  }
  if (_back->_curX + w >= _width) {
    w = _width - _back->_curX; // clipped
    _back->newLine(EXTENT_Y(textSize) + LINE_SPACING);
  } else {
    _back->_curX += w;
  }
  Widget *result;
  if (formLink) {
    result = new FormLink(_back, action, x, y, w, h);
  } else if (button) {
    result = new BlockButton(_back, action, text, x, y, w, h);
  } else {
    result = new TextButton(_back, action, text, x, y, w, h);
  }
  return result;
}

// create an options dialog
void AnsiWidget::createOptionsBox(char *&p) {
  List<String *> *items = getItems(p);
  form_ui::optionsBox(items);
  delete items;
}

// handles the characters following the \e[ sequence. Returns whether a further call
// is required to complete the process.
bool AnsiWidget::doEscape(char *&p, int textHeight) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue *10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'A':
      showAlert(p);
      break;
    case 'B':
      createLink(p, false, true);
      break;
    case 'C':
      // GSS Graphic Size Selection
      _back->_fontSize = escValue;
      _back->updateFont();
      break;
    case 'K':
      maShowVirtualKeyboard();
      break;
    case 'H':
      createLink(p, false, false);
      break;
    case 'L':
      createLabel(p);
      break;
    case 'O':
      createOptionsBox(p);
      break;
    case 'S':
      screenCommand(p);
      break;
    }
    if (p[1] == ';') {
      // advance to the separator
      p++;
    }
  } else if (_back->setGraphicsRendition(*p, escValue, textHeight)) {
    _back->updateFont();
  }

  bool result = false;
  if (*p == ';') {
    result = true;
    // advance to next rendition
    p++;  
  }
  return result;
}

// swipe handler for pointerReleaseEvent()
void AnsiWidget::doSwipe(int start, int maxScroll) {
  MAEvent event;
  int elapsed = 0;
  int vscroll = _front->_scrollY;
  int scrollSize = (start - _moveTime < SWIPE_TRIGGER_FAST) ? 
                   SWIPE_SCROLL_FAST : SWIPE_SCROLL_SLOW;
  int swipeStep = SWIPE_DELAY_STEP;
  while (elapsed < SWIPE_MAX_TIMER) {
    if (maGetEvent(&event) && event.type == EVENT_TYPE_POINTER_PRESSED) {
      // ignore the next move and release events
      _swipeExit = true;
      break;
    }
    elapsed += (maGetMilliSecondCount() - start);
    if (elapsed > swipeStep && scrollSize > 1) {
      // step down to a lesser scroll amount
      scrollSize -= 1;
      swipeStep += SWIPE_DELAY_STEP;
    }
    if (scrollSize == 1) {
      maWait(20);
    }
    vscroll += _moveDown ? scrollSize : -scrollSize;
    if (vscroll < 0) {
      vscroll = 0;
    } else if (vscroll > maxScroll) {
      vscroll = maxScroll;
    }
    if (vscroll != _front->_scrollY) {
      _front->_dirty = true; // forced
      _front->_scrollY = vscroll;
      flush(true, true);
    } else {
      break;
    }
  }
  
  // pause before removing the scrollbar
  maWait(500);
}

// draws the focus screen
void AnsiWidget::drawActiveButton() {
#if defined(_FLTK)
  maUpdateScreen();
#else
  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
  int x = _focus->x + _activeButton->x;
  int y = _focus->y + _activeButton->y - _focus->_scrollY;
  maSetClipRect(x, y, _activeButton->width, _activeButton->height + 2);
  _activeButton->draw(x, y);
  maUpdateScreen();
  maResetBacklight();
  maSetDrawTarget(currentHandle);
#endif
}

// returns list of strings extracted from the vertical-bar separated input string
List<String *> *AnsiWidget::getItems(char *&p) {
  List<String *> *result = new List<String *>();
  char *next = p + 1;
  bool eot = false;

  while (*p && !eot) {
    if (p[1] < 32) {
      // end when control character encountered
      int len = (p - next) + 1;
      if (len) {
        result->add(new String((const char *)next, len));
      }
      eot = true;
    } else {
      p++;
      switch (*p) {
      case ';':
        eot = true;
        // fallthru
      case '|':
        result->add(new String((const char *)next, (p - next)));
        next = p + 1;
        break;
      default:
        break;
      }
    }
  }
  return result;
}

// print() helper
void AnsiWidget::handleEscape(char *&p, int lineHeight) {
  if (*(p + 1) == '[') {
    p += 2;
    while (doEscape(p, lineHeight)) {
      // continue
    }
  }
}

// remove the specified screen
void AnsiWidget::removeScreen(char *&p) {
  logEntered();
  List<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  if (n < 1 || n >= MAX_SCREENS) {
    print("ERR invalid screen number");
  } else if (_screens[n] != NULL) {
    if (_back == _screens[n]) {
      _back = _screens[0];
    }
    if (_front == _screens[n]) {
      _front = _screens[0];
    }
    delete _screens[n];
    _screens[n] = NULL;
  }
  delete items;
}

// screen escape commands
void AnsiWidget::screenCommand(char *&p) {
  Screen *selected;
  p++;

  switch (*p) {
  case 'X': // double buffering - transpose write and display screens
    swapScreens();
    break;
  case 'E': // erase a screen
    removeScreen(p);
    break;
  case 'R': // redraw all user screens
    for (int i = 0; i < MAX_SCREENS; i++) {
      if (_screens[i] != NULL && i < SYSTEM_SCREENS) {
        _screens[i]->drawBase(false);
        _front = _back = _screens[i];
      }
    }
    break;
  case 'W': // select write/back screen
    selected = selectScreen(p);
    if (selected) {
      _back = selected;
    }
    break;
  case 'w': // revert write/back to front screen
    _back = _front;
    break;
  case 'D': // select display/front screen
    selected = selectScreen(p);
    if (selected) {
      _front = selected;
      _front->_dirty = true;
      flush(true);
    }
    break;
  case '#': // open screen # for write/display
    selected = selectScreen(p);
    if (selected) {
      _front = _back = selected;
      _front->_dirty = true;
      flush(true);
    }
    break;
  default:
    print("ERR unknown screen command");
    break;
  }    
}

// returns whether the event is over the given screen
bool AnsiWidget::setActiveButton(MAEvent &event, Screen *screen) {
  bool result = false;
  if (screen->overlaps(event.point.x, event.point.y)) {
    List_each(Shape*, it, screen->_shapes) {
      Widget *widget = (Widget *)(*it);
      if (widget->overlaps(event.point, screen->x,
                           screen->y - screen->_scrollY)) {
        _activeButton = widget;
        _activeButton->_pressed = true;
        break;
      }
    }
    // screen overlaps event - avoid search in other screens
    result = true;
  }
  return result;
}

// select the specified screen - returns whether the screen was changed
Screen *AnsiWidget::selectScreen(char *&p) {
  List<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  int x = items->size() > 1 ? atoi((*items)[1]->c_str()) : 0;
  int y = items->size() > 2 ? atoi((*items)[2]->c_str()) : 0;
  int w = items->size() > 3 ? atoi((*items)[3]->c_str()) : 100;
  int h = items->size() > 4 ? atoi((*items)[4]->c_str()) : 100;

  Screen *result = NULL;
  flush(true);

  if (n < 0 || n >= MAX_SCREENS) {
    n = 0;
  }

  x = MIN(MAX(x, 0), 100);
  y = MIN(MAX(y, 0), 100);
  w = MIN(MAX(w, 0), 100);
  h = MIN(MAX(h, 0), 100);

  result = _screens[n];

  if (result != NULL) {
    // specified screen already exists
    if (result->x != x ||
        result->y != y ||
        result->width != _width ||
        result->height != _height) {
      delete result;
      result = NULL;
    }
  }
  
  if (result == NULL) {
    if (n > 1) {
      result = new TextScreen(_width, _height, _fontSize, x, y, w, h);
    } else {
      result = new GraphicScreen(_width, _height, _fontSize);
    }
    if (result && result->construct()) {
      _screens[n] = result;
      result->_fg = _front->_fg;
      result->_bg = _front->_bg;
      result->drawInto();
      result->clear();
    } else {
      trace("Failed to create screen %d", n);
    }
  }
  
  delete items;
  return result;
}

// display an alert box - eg // ^[ aAlert!!;
void AnsiWidget::showAlert(char *&p) {
  List<String *> *items = getItems(p);

  const char *title = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *message = items->size() > 1 ? (*items)[1]->c_str() : "";
  const char *button1 = items->size() > 2 ? (*items)[2]->c_str() : "";
  const char *button2 = items->size() > 3 ? (*items)[3]->c_str() : "";
  const char *button3 = items->size() > 4 ? (*items)[4]->c_str() : "";

  maAlert(title, message, button1, button2, button3);
  delete items;
}

// transpose the front and back screens
void AnsiWidget::swapScreens() {
  if (_front == _back) {
    if (_screens[1] != NULL) {
      _front = _screens[1];
    } else {
      _front = new GraphicScreen(_width, _height, _fontSize);
      if (_front && _front->construct()) {
        _screens[1] = _front;
      } else {
        trace("Failed to create screen");
      }
    }
  } else {
    Screen *tmp = _front;
    _front = _back;
    _back = tmp;
    if (_front->_dirty) {
      _front->drawBase(false);
    }
  }
}
