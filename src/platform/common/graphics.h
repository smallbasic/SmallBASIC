// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef COMMON_GRAPHICS
#define COMMON_GRAPHICS

#include "platform/common/maapi.h"
#include "platform/common/StringLib.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define MAX_GLYPHS 256

using namespace strlib;

typedef uint16_t pixel_t;

namespace common {

struct Glyph {
  FT_Glyph _slot;
  int _w;
};

struct Font {
  Font(FT_Face face, int size, bool italic);
  virtual ~Font();
  int _h;
  int _spacing;
  FT_Face _face;
  Glyph _glyph[MAX_GLYPHS];
};

struct Canvas {
  Canvas();
  virtual ~Canvas();

  virtual bool create(int w, int h) = 0;
  virtual pixel_t *getLine(int y) = 0;
  virtual void setClip(int x, int y, int w, int h) = 0;
  virtual void freeClip() = 0;
  virtual int x() = 0;
  virtual int y() = 0;
  virtual int w() = 0;
  virtual int h() = 0;
  virtual int width() = 0;
  virtual int height() = 0;
};

struct Graphics {
  Graphics();
  virtual ~Graphics();

  Font *createFont(int style, int size);
  void deleteFont(Font *font);
  void drawImageRegion(Canvas *src, const MAPoint2d *dstPoint, const MARect *srcRect);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRectFilled(int left, int top, int width, int height);
  void drawText(int left, int top, const char *str, int len);
  int  getPixel(int x, int y);
  MAExtent getTextSize(const char *str, int len);
  int getHeight() { return _h; }
  int getWidth() { return _w; }
  void setSize(int w, int h) { _w = w; _h = h; }
  void setClip(int x, int y, int w, int h);
  void setColor(pixel_t color) {  _drawColor = color; }
  void setFont(Font *font) { _font = font; }
  MAHandle setDrawTarget(MAHandle maHandle);

protected:
  void drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y);

  FT_Library _fontLibrary;
  FT_Face _fontFace;
  FT_Face _fontFaceB;
  Canvas *_screen;
  Canvas *_drawTarget;
  Font *_font;
  pixel_t _drawColor;
  int _w, _h;
};

}

#endif
