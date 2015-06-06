// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
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
#include <math.h>

#include "ui/ansiwidget.h"
#include "ui/inputs.h"
#include "ui/utils.h"

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
#define H_SCROLL_SIZE 10
#define SWIPE_MAX_TIMER 3000
#define SWIPE_DELAY_STEP 200
#define SWIPE_MAX_DURATION 300
#define SWIPE_MIN_DISTANCE 60
#define FONT_FACTOR 30

extern "C" void dev_beep(void);

AnsiWidget::AnsiWidget(int width, int height) :
  _back(NULL),
  _front(NULL),
  _focus(NULL),
  _width(width),
  _height(height),
  _xTouch(-1),
  _yTouch(-1),
  _xMove(-1),
  _yMove(-1),
  _touchTime(0),
  _swipeExit(false),
  _autoflush(true),
  _activeButton(NULL),
  _hoverInput(NULL) {
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
  logEntered();
  for (int i = 0; i < MAX_SCREENS; i++) {
    delete _screens[i];
  }
}

Screen *AnsiWidget::createScreen(int screenId) {
  Screen *result = _screens[screenId];
  if (result == NULL) {
    if (screenId == TEXT_SCREEN || screenId == MENU_SCREEN) {
      result = new TextScreen(_width, _height, _fontSize);
    } else {
      result = new GraphicScreen(_width, _height, _fontSize);
    }
    if (result && result->construct()) {
      _screens[screenId] = result;
      result->drawInto();
      result->clear();
    } else {
      trace("Failed to create screen %d", screenId);
    }
  }
  return result;
}

void AnsiWidget::addImage(ImageDisplay &image) {
  _back->addImage(image);
  flush(false, false, MAX_PENDING_GRAPHICS);
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

// display and pending images changed
void AnsiWidget::flush(bool force, bool vscroll, int maxPending) {
  if (_front != NULL && _autoflush) {
    bool update = false;
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

// prints the contents of the given string onto the backbuffer
void AnsiWidget::print(const char *str) {
  int len = strlen(str);
  if (len) {
    _back->drawInto();

    int lineHeight = textHeight();
    const char *p = (char *)str;

    while (*p) {
      switch (*p) {
      case '\a':   // beep
        dev_beep();
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
    flush(false);
  }
}

// redraws and flushes the front screen
void AnsiWidget::redraw() {
  _front->drawInto();
  flushNow();
}

// reinit for new program run
void AnsiWidget::reset() {
  _back = _front = _screens[USER_SCREEN1];
  _back->reset(_fontSize);
  _back->clear();

  // reset user screens
  delete _screens[USER_SCREEN2];
  delete _screens[TEXT_SCREEN];
  _screens[USER_SCREEN2] = NULL;
  _screens[TEXT_SCREEN] = NULL;

  maFontSetCurrent(_back->_font);
  redraw();
}

// update the widget to new dimensions
void AnsiWidget::resize(int newWidth, int newHeight) {
  int lineHeight = textHeight();
  for (int i = 0; i < MAX_SCREENS; i++) {
    Screen *screen = _screens[i];
    if (screen) {
      screen->resize(newWidth, newHeight, _width, _height, lineHeight);
      if (screen == _front) {
        screen->drawBase(false);
      }
    }
  }
  _width = newWidth;
  _height = newHeight;
}

void AnsiWidget::removeHover() {
  if ( _hoverInput) {
    int dx = _front->_x;
    int dy = _front->_y - _front->_scrollY;
    _hoverInput->drawHover(dx, dy, false);
    _hoverInput = NULL;
  }
}

void AnsiWidget::scroll(bool up, bool page) {
  int h = page ? _front->_height - _front->_charHeight : _front->_charHeight;
  int vscroll = _front->_scrollY + (up ? - h : h);
  int maxVScroll = (_front->_curY - _front->_height) + (2 * _fontSize);

  if (page) {
    if (vscroll < 0 && _front->_scrollY > 0) {
      vscroll = 0;
    } else if (vscroll >= maxVScroll) {
      vscroll = maxVScroll - _front->_charHeight;
    }
  }

  if (vscroll >= 0 && vscroll < maxVScroll) {
    _front->drawInto();
    _front->_scrollY = vscroll;
    flush(true, true);
  }
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

void AnsiWidget::setStatus(const char *label) {
  _back->_label = label;
  _back->setDirty();
}

// sets the current text drawing color
void AnsiWidget::setTextColor(long fg, long bg) {
  _back->setTextColor(fg, bg);
}

void AnsiWidget::setXY(int x, int y) {
  if (x != _back->_curX || y != _back->_curY) {
    int lineHeight = textHeight();
    int newLines = (y - _back->_curY) / lineHeight;
    while (newLines-- > 0) {
      _back->newLine(lineHeight);
    }
    _back->_curX = (x == 0) ? INITXY : x;
    _back->_curY = y;
    flush(false, false, MAX_PENDING_GRAPHICS);
  }
}

int AnsiWidget::insetMenuScreen(int x, int y, int w, int h) {
  int result = 0;
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (_front == _screens[i]) {
      result = i;
      break;
    }
  }
  if (_back == _screens[MENU_SCREEN]) {
    _back = _screens[USER_SCREEN1];
  }
  TextScreen *menuScreen = (TextScreen *)createScreen(MENU_SCREEN);
  menuScreen->_x = x;
  menuScreen->_y = y;
  menuScreen->_width = w;
  menuScreen->_height = h;
  menuScreen->setOver(_front);
  _front = _back = menuScreen;
  _front->_dirty = true;
  flush(true);
  return result;
}

int AnsiWidget::insetTextScreen(int x, int y, int w, int h) {
  int result = 0;
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (_front == _screens[i]) {
      result = i;
      break;
    }
  }
  if (_back == _screens[TEXT_SCREEN]) {
    _back = _screens[USER_SCREEN1];
  }
  TextScreen *textScreen = (TextScreen *)createScreen(TEXT_SCREEN);
  textScreen->inset(x, y, w, h, _front);
  _front = _back = textScreen;
  _front->_dirty = true;
  flush(true);
  return result;
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
    _activeButton->clicked(event.point.x, event.point.y, true);
    drawActiveButton();
  }
  // setup vars for page scrolling
  if (_front->overlaps(event.point.x, event.point.y)) {
    _touchTime = maGetMilliSecondCount();
    _xTouch = _xMove = event.point.x;
    _yTouch = _yMove = event.point.y;
    result = true;
  }
  return result;
}

// handler for pointer move events
bool AnsiWidget::pointerMoveEvent(MAEvent &event) {
  bool result = false;
  if (_front == _screens[MENU_SCREEN]) {
    _activeButton = _front->getMenu(_activeButton, event.point.x, event.point.y);
  } else if (_activeButton != NULL) {
    bool redraw = false;
    bool pressed = _activeButton->selected(event.point, _focus->_x,
                                           _focus->_y - _focus->_scrollY, redraw);
    if (redraw || (pressed != _activeButton->_pressed)) {
      _activeButton->_pressed = pressed;
      drawActiveButton();
      result = true;
    }
  } else if (!_swipeExit && _xMove != -1 && _yMove != -1 &&
             _front->overlaps(event.point.x, event.point.y)) {
    int hscroll = _front->_scrollX + (_xMove - event.point.x);
    int vscroll = _front->_scrollY + (_yMove - event.point.y);
    int maxHScroll = MAX(0, _front->getMaxHScroll());
    int maxVScroll = (_front->_curY - _front->_height) + (2 * _fontSize);
    if (hscroll < 0) {
      hscroll = 0;
    } else if (hscroll > maxHScroll) {
      hscroll = maxHScroll;
    }
    if (vscroll < 0) {
      vscroll = 0;
    }
    if ((abs(hscroll - _front->_scrollX) > H_SCROLL_SIZE
         && maxHScroll > 0
         && hscroll <= maxHScroll) ||
        (vscroll != _front->_scrollY && maxVScroll > 0 &&
         vscroll < maxVScroll + OVER_SCROLL)) {
      _front->drawInto();
      _front->_scrollX = hscroll;
      _front->_scrollY = vscroll;
      _xMove = event.point.x;
      _yMove = event.point.y;
      flush(true, true);
      result = true;
    }
  } else {
    result = drawHoverLink(event);
  }
  return result;
}

// handler for pointer release events
void AnsiWidget::pointerReleaseEvent(MAEvent &event) {
  if (_activeButton != NULL && _front == _screens[MENU_SCREEN]) {
    _activeButton->clicked(event.point.x, event.point.y, false);
  } else if (_activeButton != NULL && _activeButton->_pressed) {
    _activeButton->_pressed = false;
    drawActiveButton();
    _activeButton->clicked(event.point.x, event.point.y, false);
  } else if (_swipeExit) {
    _swipeExit = false;
  } else {
    int maxScroll = (_front->_curY - _front->_height) + (2 * _fontSize);
    if (_yMove != -1 && maxScroll > 0) {
      _front->drawInto();

      // swipe test - min distance and not max duration
      int deltaX = _xTouch - event.point.x;
      int deltaY = _yTouch - event.point.y;
      int distance = (int) fabs(sqrt(deltaX * deltaX + deltaY * deltaY));
      int now = maGetMilliSecondCount();
      if (distance >= SWIPE_MIN_DISTANCE && (now - _touchTime) < SWIPE_MAX_DURATION) {
        bool moveDown = (deltaY >= SWIPE_MIN_DISTANCE);
        doSwipe(now, moveDown, distance, maxScroll);
      } else if (_front->_scrollY > maxScroll) {
        _front->_scrollY = maxScroll;
      }
      // ensure the scrollbar is removed
      _front->_dirty = true;
      flush(true);
      _touchTime = 0;
    }
  }

  if (_hoverInput) {
    int dx = _front->_x;
    int dy = _front->_y - _front->_scrollY;
    _hoverInput->drawHover(dx, dy, false);
    _hoverInput = NULL;
  }
  
  _xTouch = _xMove = -1;
  _yTouch = _yMove = -1;
  _activeButton = NULL;
  _focus = NULL;
}

// handles the characters following the \e[ sequence. Returns whether a further call
// is required to complete the process.
bool AnsiWidget::doEscape(const char *&p, int textHeight) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue * 10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'C':
      // GSS Graphic Size Selection
      _back->_fontSize = escValue;
      _back->updateFont();
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
void AnsiWidget::doSwipe(int start, bool moveDown, int distance, int maxScroll) {
  MAEvent event;
  int elapsed = 0;
  int vscroll = _front->_scrollY;
  int scrollSize = distance / 3;
  int swipeStep = SWIPE_DELAY_STEP;
  while (elapsed < SWIPE_MAX_TIMER) {
    if (maGetEvent(&event) && event.type == EVENT_TYPE_POINTER_RELEASED) {
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
    vscroll += moveDown ? scrollSize : -scrollSize;
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
  if (_focus != NULL && !_activeButton->hasHover()) {
#if defined(_FLTK)
    maUpdateScreen();
#else
    MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
    _focus->drawShape(_activeButton);
    maUpdateScreen();
    maSetDrawTarget(currentHandle);
#endif
  }
}

bool AnsiWidget::drawHoverLink(MAEvent &event) {
#if defined(_SDL)
  if (_front != _screens[MENU_SCREEN]) {
    int dx = _front->_x;
    int dy = _front->_y - _front->_scrollY;
    FormInput *active = NULL;
    if ( _front->overlaps(event.point.x, event.point.y)) {
      List_each(FormInput*, it, _front->_inputs) {
        FormInput *widget = (FormInput *)(*it);
        if (widget->hasHover() &&
            widget->overlaps(event.point, dx, dy)) {
        active = widget;
        break;
        }
      }
    }
    if (active && active != _hoverInput) {
      if (_hoverInput) {
        // remove old hover
        _hoverInput->drawHover(dx, dy, false);
      }
      // display new hover
      _hoverInput = active;
      _hoverInput->drawHover(dx, dy, true);
    } else if (!active && _hoverInput) {
      // no new hover, erase old hover
      _hoverInput->drawHover(dx, dy, false);
      _hoverInput = NULL;
    }
  }
#endif
  return _hoverInput != NULL;
}

// print() helper
void AnsiWidget::handleEscape(const char *&p, int lineHeight) {
  if (*(p + 1) == '[') {
    p += 2;
    while (doEscape(p, lineHeight)) {
      // continue
    }
  }
}

// returns whether the event is over the given screen
bool AnsiWidget::setActiveButton(MAEvent &event, Screen *screen) {
  bool result = false;
  if (_front != _screens[MENU_SCREEN] &&
      screen->overlaps(event.point.x, event.point.y)) {
    List_each(FormInput*, it, screen->_inputs) {
      FormInput *widget = (FormInput *)(*it);
      bool redraw = false;
      if (widget->selected(event.point, screen->_x,
                           screen->_y - screen->_scrollY, redraw)) {
        _activeButton = widget;
        _activeButton->_pressed = true;
        break;
      }
      if (redraw) {
        _front->_dirty = true;
        flush(true);
      }
    }
    // screen overlaps event - avoid search in other screens
    result = true;
  }
  return result;
}

int AnsiWidget::selectBackScreen(int screenId) {
  int result = 0;
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (_back == _screens[i]) {
      result = i;
      break;
    }
  }
  _back = createScreen(screenId);
  return result;
}

int AnsiWidget::selectFrontScreen(int screenId) {
  int result = 0;
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (_front == _screens[i]) {
      result = i;
      break;
    }
  }
  _front = createScreen(screenId);
  _front->_dirty = true;
  flush(true);
  return result;
}

int AnsiWidget::selectScreen(int screenId) {
  int result = selectBackScreen(screenId);
  _front = _back;
  _front->_dirty = true;
  flush(true);
  return result;
}
