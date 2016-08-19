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
  String getPage();
  void print(const char *str);
  void reset();
  void setTextColor(long fg, long bg);
  void setColor(long fg);
  void setPixel(int x, int y, int c);
  void setXY(int x, int y);
  void setGraphicText(bool graphicText) { _graphicText = graphicText; }
 
private:    
  bool doEscape(unsigned char* &p);
  void drawText(const char *str, int len);
  String getColor(long c);
  void newLine();
  void printColorSpan(String &bg, String &fg);
  void printEndSpan();
  void printSpan(const char *clazz);
  void resetStyle();
  void setGraphicsRendition(char c, int escValue);

  String _html;
  String _script;
  String _bg;
  String _fg;
  String _bgBody;
  String _fgBody;
  bool _invert;
  bool _underline;
  bool _bold;
  bool _italic;
  bool _graphicText;
  int _spanLevel;
  int _curx;
  int _cury;
};

#endif
