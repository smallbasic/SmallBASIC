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

#define DEFAULT_COLOR 0xffba00
#define MAX_SCREENS   4

using namespace MAUtil;

struct Hyperlink {
  Hyperlink(const char *url, const char *label, int x, int y, int w, int h);
  virtual ~Hyperlink() {};
  void draw();
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY);
  String url;
  String label;
  bool pressed;
  int x,y,w,h;
};

struct HyperlinkListener {
  virtual void linkClicked(const char *url) = 0;
};

struct Screen {
  Screen(int width, int height);
  virtual ~Screen();

  int ansiToMosync(long color);
  void calcTab();
  bool construct();
  void draw(int w, int h, bool vscroll);
  void drawInto(bool background=false);
  void drawText(const char *text, int len, int x, int lineHeight);
  void setColor(long color) { fg = ansiToMosync(color); }
  void setTextColor(long fg, long bg);
  void reset(bool init);
  void resize(int width, int height, int lineHeight);
  void newLine(int displayHeight, int lineHeight);
  bool setGraphicsRendition(char c, int escValue, int lineHeight);
  void updateFont();

  MAHandle image;
  MAHandle font;
  bool underline;
  bool invert;
  bool bold;
  bool italic;
  int width;
  int height;
  int scrollX;
  int scrollY;
  int bg;
  int fg;
  int curY;
  int curX;
  int curYSaved;
  int curXSaved;
  int tabSize;
  int fontSize; 
};

class AnsiWidget {
public:
  explicit AnsiWidget(int width, int height);
  ~AnsiWidget();

  void beep() const;
  void clearScreen();
  bool construct();
  void draw();
  void drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
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
  void resize(int width, int height);
  void setColor(long color);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) { back->curX=x; back->curY=y; }
  void setScrollSize(int scrollSize);

  // mouse support
  int getTouchX() { return touchX; }
  int getTouchY() { return touchY; }
  bool getTouchMode() { return touchMode; }
  bool hasLinks() { return hyperlinks.size() > 0; }
  void resetMouse();
  void setMouseMode(bool mode);
  void setHyperlinkListener(HyperlinkListener *hll) { hyperlinkListener = hll; }
  void pointerTouchEvent(MAEvent &event);
  void pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);

private:
  int charWidth(char c);
  void createLink(char *&p, bool execLink);
  void deleteItems(Vector<String *> *items);
  bool doEscape(char *&p, int textHeight);
  Vector<String *> *getItems(char *&p);
  void reset(bool init);
  void showAlert(char *&p);

  Screen *screens[MAX_SCREENS];
  Screen *back;   // screen being painted
  Screen *front;  // screen to display 
  int dirty;      // whether refresh is required
  int width;      // device screen width
  int height;     // device screen height
  int touchX;     // active touch x value
  int touchY;     // active touch y value
  bool touchMode; // PEN ON/OFF
  HyperlinkListener *hyperlinkListener;
  Hyperlink *activeLink;
  Vector <Hyperlink *>hyperlinks;
};

#endif // ANSIWIDGET_H
