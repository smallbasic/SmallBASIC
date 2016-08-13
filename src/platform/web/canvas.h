// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef CANVAS_H
#define CANVAS_H

#include <config.h>
#include <microhttpd.h>
#include "ui/strlib.h"

using namespace strlib;

struct Canvas {
  Canvas();
  virtual ~Canvas() {}
  void clearScreen();
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  String getPage() { return _html; }
  void print(const char *str);
  void reset();
  void setTextColor(long fg, long bg);
  void setColor(long fg);
  void setPixel(int x, int y, int c);
  void setXY(int x, int y);
 
private:    
  void newLine();
  String getColor(long c);
  bool setGraphicsRendition(char c, int escValue);
  bool doEscape(unsigned char* &p);

  String _html;
  String _bg;
  String _fg;
  bool _invert;
  bool _underline;
  bool _bold;
  bool _italic;
  int _curx;
  int _cury;
};

#endif
