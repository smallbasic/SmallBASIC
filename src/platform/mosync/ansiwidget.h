// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#include <maapi.h>
#include <MAUtil/String.h>
#include <MAUtil/Vector.h>

#define DEFAULT_COLOR 0xa1a1a1
#define LINE_SPACING 4
#define MAX_SCREENS  4

using namespace MAUtil;

struct IButtonListener {
  virtual ~IButtonListener() {}
  virtual void buttonClicked(const char *action) = 0;
};

struct IFormWidgetListModel {
  virtual ~IFormWidgetListModel() {}
  virtual const char *getTextAt(int index) = 0;
  virtual int getIndex(const char *label) = 0;
  virtual int rowCount() const = 0;
  virtual int selectedIndex() const = 0;
  virtual void setSelectedIndex(int index) = 0;
};

struct IFormWidget {
  virtual ~IFormWidget() {}
  virtual void edit(int key) = 0;
  virtual IFormWidgetListModel *getList() const = 0;
  virtual const char *getText() const = 0;
  virtual void setText(const char *text) = 0;
  virtual void setWidth(int w) = 0;
  virtual void setHeight(int h) = 0;
  virtual void setListener(IButtonListener *listener) = 0;
};

struct Widget;

struct Screen {
  Screen(int x, int y, int width, int height, int fontSize);
  virtual ~Screen();

  void add(Widget *button) { buttons.add(button); }
  void calcTab();
  bool construct();
  void clear();
  void draw(bool vscroll);
  void drawInto(bool background=false);
  void drawText(const char *text, int len, int x, int lineHeight);
  void newLine(int lineHeight);
  int  print(const char *p, int lineHeight);
  void remove(Widget *button);
  void reset(int fontSize = -1);
  void resize(int newWidth, int newHeight, int oldWidth, int oldHeight, int lineHeight);
  void setColor(long color);
  void setTextColor(long fg, long bg);
  bool setGraphicsRendition(char c, int escValue, int lineHeight);
  void updateFont();

  MAHandle image;
  MAHandle font;
  bool underline;
  bool invert;
  bool bold;
  bool italic;
  int bg;
  int fg;
  int x,y;
  int width;
  int height;
  int imageWidth;
  int imageHeight;
  int pageHeight;
  int scrollY;
  int curY;
  int curX;
  int curYSaved;
  int curXSaved;
  int tabSize;
  int fontSize; 
  int charWidth;
  int charHeight;
  int dirty;
  int linePadding;
  Vector <Widget *>buttons;
  String label;
};

// base implementation for all buttons
struct Widget {
  Widget(int bg, int fg, int x, int y, int w, int h);
  virtual ~Widget() {}

  virtual void draw() = 0;
  virtual void clicked(IButtonListener *listener) = 0;
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY);
  int getBackground(int buttonColor);

  bool pressed;
  int bg, fg;
  int x,y,w,h;
};

// base implementation for all internal buttons
struct Button : public Widget {
  Button(Screen *screen, const char *action, const char *label,
         int x, int y, int w, int h);
  virtual ~Button() {}

  void clicked(IButtonListener *listener);

  String action;
  String label;
};

// internal text button
struct TextButton : public Button {
  TextButton(Screen *screen, const char *action, const char *label,
             int x, int y, int w, int h);
  void draw();
};

// internal block button
struct BlockButton : public Button {
  BlockButton(Screen *screen, const char *action, const char *label,
              int x, int y, int w, int h);
  void draw();
};

// base implementation for all external buttons
struct FormWidget : public Widget, IFormWidget {
  FormWidget(Screen *screen, int x, int y, int w, int h);
  virtual ~FormWidget() {}

  void setListener(IButtonListener *listener) { this->listener = listener; }
  Screen *getScreen() { return screen; }
  void clicked(IButtonListener *listener);

  IFormWidgetListModel *getList() const { return NULL; }
  void setText(const char *text) {}
  void setWidth(int w) { this->w = w; }
  void setHeight(int h) { this->h = h; }

private:
  Screen *screen;
  IButtonListener *listener;
};

struct FormLabel : public FormWidget {
  FormLabel(Screen *screen, int x, int y, int w, int h);
  virtual ~FormLabel() {}
};

struct FormLineInput : public FormWidget {
  FormLineInput(Screen *screen, char *buffer, int maxSize, 
                int x, int y, int w, int h);
  virtual ~FormLineInput();

  void close();
  void draw();
  void edit(int key);
  const char *getText() const { return buffer; }

private:
  char *buffer;
  int maxSize;
  int scroll;
};

struct FormList : public FormWidget {
  FormList(Screen *screen, char *buffer, int maxSize, 
           int x, int y, int w, int h);
  virtual ~FormList() {}
};

struct AnsiWidget {
  explicit AnsiWidget(IButtonListener *listener, int width, int height);
  ~AnsiWidget();

  void beep() const;
  void clearScreen() { back->clear(); }
  bool construct();
  IFormWidget *createButton(char *caption, int x, int y, int w, int h);
  IFormWidget *createLabel(char *caption, int x, int y, int w, int h);
  IFormWidget *createLineInput(char *buffer, int maxSize, int x, int y, int w, int h);
  IFormWidget *createList(IFormWidgetListModel *model, int x, int y, int w, int h);
  void draw();
  void drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void edit(IFormWidget *formWidget, int c);
  void flush(bool force, bool vscroll=false);
  int getBackgroundColor() { return back->bg; }
  int getColor() { return back->fg; }
  int getPixel(int x, int y);
  int getHeight() { return height; }
  int getWidth()  { return width; }
  int getX() { return back->curX; }
  int getY() { return back->curY; }
  int textHeight(void);
  int textWidth(const char *s, int len=-1);
  void print(const char *str);
  void reset();
  void resize(int width, int height);
  void setColor(long color);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) { back->curX=x; back->curY=y; }
  void setScrollSize(int scrollSize);

  bool hasUI();
  void pointerTouchEvent(MAEvent &event);
  void pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);

private:
  void createLabel(char *&p);
  void createLink(char *&p, bool execLink, bool button);
  void createOptionsBox(char *&p);
  void deleteItems(Vector<String *> *items);
  bool doEscape(char *&p, int textHeight);
  void doSwipe(int start, int maxScroll);
  Vector<String *> *getItems(char *&p);
  void removeScreen(char *&p);
  void screenCommand(char *&p);
  Screen *selectScreen(char *&p);
  void showAlert(char *&p);
  void swapScreens();

  Screen *screens[MAX_SCREENS];
  Screen *back;   // screen being painted/written
  Screen *front;  // screen to display 
  Screen *pushed; // saved/previous screen
  int width;      // device screen width
  int height;     // device screen height
  int fontSize;   // font height based on screen size
  int touchX;     // active touch x value
  int touchY;     // active touch y value
  int moveTime;   // last move time
  bool moveDown;  // last move direction was down
  bool swipeExit; // last touch-down was swipe exit
  IButtonListener *buttonListener;
  Widget *activeButton;
};

#endif // ANSIWIDGET_H
