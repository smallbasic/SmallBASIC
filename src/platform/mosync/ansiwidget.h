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

using namespace MAUtil;

struct Hyperlink {
  Hyperlink(const char *url, const char *label, int x, int y, int w, int h);
  virtual ~Hyperlink() {};
  void draw();
  bool overlaps(MAPoint2d pt);
  String url;
  String label;
  bool pressed;
  int x,y,w,h;
};

struct HyperlinkListener {
  virtual void linkClicked(const char *url) = 0;
};

#define DEFAULT_COLOR 0xffba00

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
  void flush(bool force);
  int getBackgroundColor() { return bg; }
  int getColor() { return fg; }
  int getPixel(int x, int y);
  int getHeight() { return height; }
  int getWidth()  { return width; }
  int getX() { return curX; }
  int getY() { return curY; }
  int textHeight(void);
  int textWidth(const char *s, int len=-1);
  void print(const char *str);
  void resize(int width, int height);
  void setColor(long color);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) { curX=x; curY=y; }
  void setScrollSize(int scrollSize);

  // mouse support
  int getMouseX(bool down) { return down ? markX : pointX; }
  int getMouseY(bool down) { return down ? markY : pointY; }
  bool getMouseMode() { return mouseMode; }
  bool hasLinks() { return hyperlinks.size() > 0; }
  void resetMouse();
  void setMouseMode(bool mode);
  void setHyperlinkListener(HyperlinkListener *hll) { hyperlinkListener = hll; }
  void pointerTouchEvent(MAEvent &event);
  void pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);

private:
  int ansiToMosync(long color);
  int calcTab(int x) const;
  int charWidth(char c);
  void createLink(char *&p, bool execLink);
  void deleteItems(Vector<String *> *items);
  bool doEscape(char *&p);
  Vector<String *> *getItems(char *&p);
  void newLine();
  void reset(bool init);
  bool setGraphicsRendition(char c, int escValue);
  void setPopupMode(int mode);
  void showAlert(char *&p);
  void updateFont();

  MAHandle image;
  MAHandle font;

  int bg;
  int fg;
  bool underline;
  bool invert;
  bool bold;
  bool italic;
  int curY;
  int curX;
  int curYSaved;
  int curXSaved;
  int tabSize;
  int textSize;
  int scrollSize;
  int width;
  int height;
  int dirty;

  // clipboard handling
  int markX, markY, pointX, pointY;
  bool copyMode;

  // mouse handling
  bool mouseMode;               // PEN ON/OFF
  HyperlinkListener *hyperlinkListener;
  Hyperlink *activeLink;
  Vector <Hyperlink *>hyperlinks;
};

#endif // ANSIWIDGET_H
