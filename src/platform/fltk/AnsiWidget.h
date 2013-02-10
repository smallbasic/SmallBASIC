//
// Copyright 1998-2006 by Bill Spitzak and others.
// Original code Copyright Chris Warren-Smith.  Permission to distribute under
// the LGPL for the FLTK library granted by Chris Warren-Smith.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".

#ifndef FL_ANSI_WIDGET
#define FL_ANSI_WIDGET

#include <fltk3/Widget.h>
#include <fltk3/draw.h>
#include <fltk3/Image.h>

#include "fltk2.h"

using namespace fltk3;

class AnsiWidget : public Widget {
public:
  AnsiWidget(int x, int y, int w, int h, int defsize);
  virtual ~AnsiWidget();

  // inherited methods
  void draw();
  void layout();
  int handle(int e);

  // public api
  void clearScreen();
  void print(const char *str);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawImage(Image *img, int x, int y, int sx, int sy, int w, int h);
  void saveImage(const char *fn, int x, int y, int w, int h);
  void setTextColor(long fg, long bg);
  void setColor(long color);
  int getX() { return curX; }
  int getY() { return curY; }
  void setPixel(int x, int y, int c);
  int getPixel(int x, int y);
  void setXY(int x, int y) { curX = x; curY = y; }
  int textWidth(const char *s);
  int textHeight(void);
  int getWidth() { return w(); }
  int getHeight() { return h(); }
  void setFontSize(float i) { labelsize(i); }
  int getFontSize() { return (int)labelsize(); }
  void beep() const;
  static Color ansiToFltk(long color);

private:
  void init();
  void destroyImage();
  void initImage();
  bool setGraphicsRendition(char c, int escValue);
  bool doEscape(unsigned char *&p);
  int calcTab(int x) const;
  void newLine();
  void reset();
  void setFont();

  Image *img;
  bool underline;
  bool invert;
  bool bold;
  bool italic;
  bool resized;
  int curY;
  int curX;
  int curYSaved;
  int curXSaved;
  int tabSize;
};

#endif


