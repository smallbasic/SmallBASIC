// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
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
  void clearScreen();
  bool construct();
  void draw();
  void drawArc(int xc, int yc, double r, double start, double end, double aspect);
  void drawEllipse(int xc, int yc, int rx, int ry, int fill);
  void drawImage(ImageDisplay &image);
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
  int  getScreenId(bool back);
  int  getScreenWidth()  { return _back->_width; }
  void getScroll(int &x, int &y) { _back->getScroll(x, y); }
  int  getHeight() { return _height; }
  int  getWidth()  { return _width; }
  int  getX() { return _back->_curX; }
  int  getY() { return _back->_curY; }
  int  getMenuIndex() const { return _back->getIndex(_activeButton); }
  bool hasActiveButton() const { return _activeButton != NULL; }
  bool hasHover() const { return _hoverInput != NULL; }
  bool hasMenu() const { return _back == _screens[MENU_SCREEN]; }
  void handleMenu(bool up);
  void insetMenuScreen(int x, int y, int w, int h);
  void insetTextScreen(int x, int y, int w, int h);
  bool overLabel(int x, int y) { return _back->overLabel(x, y); };
  bool overMenu(int x, int y) { return _back->overMenu(x, y); };
  bool pointerTouchEvent(MAEvent &event);
  bool pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);
  void print(const char *str);
  void redraw();
  void removeHover();
  void removeImage(int imageId) { _back->removeImage(imageId); }
  bool removeInput(FormInput *input) { return _back->removeInput(input); }
  void removeInputs();
  void resetScroll() { _back->resetScroll(); }
  void reset();
  void resetFont() { _back->reset(_fontSize); _back->updateFont(); }
  void resize(int width, int height);
  bool scroll(bool up, bool page);
  void selectBackScreen(int screenId);
  void selectFrontScreen(int screenId);
  int  selectScreen(int screenId, bool forceFlush=true);
  void setColor(long color);
  void setDirty() { _back->setDirty(); }
  void setAutoflush(bool autoflush) { _autoflush = autoflush; }
  void setFont(int size, bool bold, bool italic);
  void setFontSize(int fontSize);
  void setPixel(int x, int y, int c);
  void setScroll(int x, int y) { _back->setScroll(x, y); }
  void setStatus(const char *label);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y);
  void setScrollSize(int scrollSize);
  int  textHeight(void) { return _back->_charHeight; }
  void updateInputs(var_p_t form, bool setv) { _back->updateInputs(form, setv); }

private:
  Screen *createScreen(int screenId);
  bool doEscape(const char *&p, int textHeight);
  void doSwipe(int start, bool moveDown, int distance, int maxScroll);
  void drawActiveButton();
  bool drawHoverLink(MAEvent &event);
  void handleEscape(const char *&p, int textHeight);
  bool setActiveButton(MAEvent &event, Screen *screen);

  Screen *_screens[MAX_SCREENS]{};
  Screen *_back;   // screen being painted/written
  Screen *_front;  // screen to display
  Screen *_focus;  // screen with the active button
  FormInput *_activeButton;
  FormInput *_hoverInput;
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
};

#endif // ANSIWIDGET_H
