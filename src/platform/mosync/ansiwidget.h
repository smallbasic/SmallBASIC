// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if defined(_FLTK)
  #include "platform/common/maapi.h"
#else
  #include <maapi.h>
#endif

#include "platform/common/StringLib.h"
#include "platform/mosync/screen.h"
#include "interface.h"

#define MAX_SCREENS 8
#define SYSTEM_SCREENS 6

using namespace strlib;

// base implementation for all buttons
struct Widget : public Shape {
  Widget(int bg, int fg, int x, int y, int w, int h);
  virtual ~Widget() {}

  virtual void clicked(IButtonListener *listener, int x, int y) = 0;

  void drawButton(const char *caption, int x, int y);
  void drawLink(const char *caption, int x, int y);
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY);
  int getBackground(int buttonColor);

  bool _pressed;
  int _bg, _fg;
};

// base implementation for all internal buttons
struct Button : public Widget {
  Button(Screen *screen, const char *action, const char *label,
         int x, int y, int w, int h);
  virtual ~Button() {}

  void clicked(IButtonListener *listener, int x, int y);

  String _action;
  String _label;
};

// internal text button
struct TextButton : public Button {
  TextButton(Screen *screen, const char *action, const char *label,
             int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {}
  void draw(int x, int y) { drawLink(_label.c_str(), x, y); }
};

// internal block button
struct BlockButton : public Button {
  BlockButton(Screen *screen, const char *action, const char *label,
              int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {}
  void draw(int x, int y) { drawButton(_label.c_str(), x, y); }
};

// base implementation for all external buttons
struct FormWidget : public Widget, IFormWidget {
  FormWidget(Screen *screen, int x, int y, int w, int h);
  virtual ~FormWidget();

  void setListener(IButtonListener *listener) { this->_listener = listener; }
  Screen *getScreen() { return _screen; }
  void clicked(IButtonListener *listener, int x, int y);

  IFormWidgetListModel *getList() const { return NULL; }
  void setText(const char *text) {}
  bool edit(int key) { return false; }

  int getX() { return this->x; }
  int getY() { return this->y; }
  int getW() { return this->width; }
  int getH() { return this->height; }
  void setX(int x) { this->x = x; }
  void setY(int y) { this->y = y; }
  void setW(int w) { this->width = w; }
  void setH(int h) { this->height = h; }

private:
  Screen *_screen;
  IButtonListener *_listener;
};

struct FormButton : public FormWidget {
  FormButton(Screen *screen, const char *caption, int x, int y, int w, int h);
  virtual ~FormButton() {}

  const char *getText() const { return _caption.c_str(); }
  void draw(int x, int y) { drawButton(_caption.c_str(), x, y); }

private:
  String _caption;
};

struct FormLabel : public FormWidget {
  FormLabel(Screen *screen, const char *caption, int x, int y, int w, int h);
  virtual ~FormLabel() {}

  const char *getText() const { return _caption.c_str(); }
  void draw(int x, int y) { drawButton(_caption.c_str(), x, y); }

private:
  String _caption;
};

struct FormLink : public FormWidget {
  FormLink(Screen *screen, const char *link, int x, int y, int w, int h);
  virtual ~FormLink() {}

  const char *getText() const { return _link.c_str(); }
  void draw(int x, int y) { drawLink(_link.c_str(), x, y); }

private:
  String _link;
};

struct FormLineInput : public FormWidget {
  FormLineInput(Screen *screen, char *buffer, int maxSize, 
                int x, int y, int w, int h);
  virtual ~FormLineInput() {}

  void close();
  void draw(int x, int y);
  bool edit(int key);
  const char *getText() const { return _buffer; }
  void setText(const char *text) {}

private:
  char *_buffer;
  int _maxSize;
  int _scroll;
};

struct FormList : public FormWidget {
  FormList(Screen *screen, IFormWidgetListModel *model, 
           int x, int y, int w, int h);
  virtual ~FormList() {}

  IFormWidgetListModel *getList() const { return _model; }
  const char *getText() const { return _model->getTextAt(_model->selected()); }
  void clicked(IButtonListener *listener, int x, int y);
  void draw(int dx, int dy);
  void optionSelected(int index);

private:
  IFormWidgetListModel *_model;
};

struct AnsiWidget {
  explicit AnsiWidget(IButtonListener *listener, int width, int height);
  ~AnsiWidget();

  void beep() const;
  void clearScreen() { _back->clear(); }
  bool construct();
  IFormWidget *createButton(char *caption, int x, int y, int w, int h);
  IFormWidget *createLabel(char *caption, int x, int y, int w, int h);
  IFormWidget *createLineInput(char *buffer, int maxSize, int x, int y, int w, int h);
  IFormWidget *createLink(char *caption, int x, int y, int w, int h);
  IFormWidget *createList(IFormWidgetListModel *model, int x, int y, int w, int h);
  void draw();
  void drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void edit(IFormWidget *formWidget, int c);
  void flush(bool force, bool vscroll=false);
  int getBackgroundColor() { return _back->_bg; }
  int getColor() { return _back->_fg; }
  int getFontSize() { return _fontSize; }
  int getPixel(int x, int y);
  int getHeight() { return _height; }
  int getWidth()  { return _width; }
  int getX() { return _back->_curX; }
  int getY() { return _back->_curY; }
  int textHeight(void);
  bool optionSelected(int index);
  void print(const char *str);
  void redraw();
  void reset();
  void resize(int width, int height);
  void setColor(long color);
  void setFontSize(int fontSize);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) { _back->_curX=x; _back->_curY=y; }
  void setScrollSize(int scrollSize);
  void pointerTouchEvent(MAEvent &event);
  void pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);

private:
  void createLabel(char *&p);
  Widget *createLink(char *&p, bool formLink, bool button);
  Widget *createLink(const char *action, const char *text,
                     bool formLink, bool button);
  void createOptionsBox(char *&p);
  void deleteItems(List<String *> *items);
  bool doEscape(char *&p, int textHeight);
  void doSwipe(int start, int maxScroll);
  void drawActiveButton();
  List<String *> *getItems(char *&p);
  void handleEscape(char *&p, int textHeight);
  void removeScreen(char *&p);
  void screenCommand(char *&p);
  bool setActiveButton(MAEvent &event, Screen *screen);
  Screen *selectScreen(char *&p);
  void showAlert(char *&p);
  void swapScreens();

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
  int _moveTime;   // last move time
  bool _moveDown;  // last move direction was down
  bool _swipeExit; // last touch-down was swipe exit
  IButtonListener *_buttonListener;
  Widget *_activeButton;
};

#endif // ANSIWIDGET_H
