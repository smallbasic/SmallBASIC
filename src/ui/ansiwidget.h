// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#include <config.h>

#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/screen.h"
#include "ui/inputs.h"
#include "ui/image.h"

#define MAX_PENDING 250
#define MAX_PENDING_GRAPHICS 25

#define USER_SCREEN1   0
#define USER_SCREEN2   1
#define TEXT_SCREEN    2
#define SOURCE_SCREEN  3
#define CONSOLE_SCREEN 4
#define MENU_SCREEN    5
#define MAX_SCREENS    6

using namespace strlib;

struct AnsiWidget {
  explicit AnsiWidget(int width, int height);
  ~AnsiWidget();

  void addImage(ImageDisplay &image);
  void addInput(FormInput *input) { _back->_inputs.add(input); }
  void clearScreen() { _back->clear(); }
  bool construct();
  void draw();
  void drawOverlay(bool vscroll) { _back->drawOverlay(vscroll); }
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void flush(bool force, bool vscroll=false, int maxPending = MAX_PENDING);
  void flushNow() { if (_front) _front->drawBase(false); }
  int  getBackgroundColor() { return _back->_bg; }
  int  getCharHeight()  { return _back->_charHeight; }
  int  getCharWidth()  { return _back->_charWidth; }
  int  getColor() { return _back->_fg; }
  int  getFontSize() { return _fontSize; }
  FormInput *getNextField(FormInput *field) { return _back->getNextField(field); }
  int  getPixel(int x, int y) { return _back->getPixel(x, y); }
  int  getScreenWidth()  { return _back->_width; }
  void getScroll(int &x, int &y) { _back->getScroll(x, y); }
  int  getHeight() { return _height; }
  int  getWidth()  { return _width; }
  int  getX() { return _back->_curX; }
  int  getY() { return _back->_curY; }
  int  insetMenuScreen(int x, int y, int w, int h);
  int  insetTextScreen(int x, int y, int w, int h);
  bool pointerTouchEvent(MAEvent &event);
  bool pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);
  void print(const char *str);
  void redraw();
  void registerScreen(int screenId) { createScreen(screenId); }
  void removeImage(int imageId) { _back->removeImage(imageId); }
  void removeInput(FormInput *input) { _back->removeInput(input); }
  void removeInputs() { _back->removeInputs(); _activeButton = NULL; }
  void resetScroll() { _back->resetScroll(); }
  void reset();
  void resize(int width, int height);
  void scroll(bool up);
  int  selectBackScreen(int screenId);
  int  selectFrontScreen(int screenId);
  int  selectScreen(int screenId);
  void setColor(long color);
  void setDirty() { _back->setDirty(); }
  void setAutoflush(bool autoflush) { _autoflush = autoflush; }
  void setFontSize(int fontSize);
  void setPixel(int x, int y, int c);
  void setScroll(int x, int y) { _back->setScroll(x, y); }
  void setStatus(const char *label);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y);
  void setScrollSize(int scrollSize);
  int  textHeight(void) { return _back->_charHeight; }
  void updateInputs(var_p_t form) { _back->updateInputs(form); }

private:
  Screen *createScreen(int screenId);
  bool doEscape(const char *&p, int textHeight);
  void doSwipe(int start, bool moveDown, int distance, int maxScroll);
  void drawActiveButton();
  void handleEscape(const char *&p, int textHeight);
  bool setActiveButton(MAEvent &event, Screen *screen);

  Screen *_screens[MAX_SCREENS];
  Screen *_back;   // screen being painted/written
  Screen *_front;  // screen to display
  Screen *_focus;  // screen with the active button
  int _width;      // device screen width
  int _height;     // device screen height
  int _fontSize;   // font height based on screen size
  int _xTouch;     // touch x value
  int _yTouch;     // touch y value
  int _xMove;      // touch move x value
  int _yMove;      // touch move y value
  int _touchTime;  // last move time
  bool _swipeExit; // last touch-down was swipe exit
  bool _autoflush; // flush internally
  FormInput *_activeButton;
};

#endif // ANSIWIDGET_H
