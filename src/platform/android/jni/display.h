// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANDROID_DISPLAY
#define ANDROID_DISPLAY

#include <android/rect.h>
#include <android_native_app_glue.h>

#include "platform/common/maapi.h"
#include "platform/common/StringLib.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

using namespace strlib;

typedef uint16_t pixel_t;

struct Font {
  Font(int style, int size) :
    _style(style),
    _size(size) {}
  int _style;
  int _size;
};

struct Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y) { return _canvas + (y * _w); }
  int x() { return _clip ? _clip->left : 0; }
  int y() { return _clip ? _clip->top : 0; }
  int w() { return _clip ? _clip->right : _w; }
  int h() { return _clip ? _clip->bottom : _w; }

  int _w;
  int _h;
  pixel_t *_canvas;
  ARect *_clip;
};

struct Graphics {
  Graphics(android_app *app);
  virtual ~Graphics();

  bool construct();
  Font *createFont(int style, int size);
  void deleteFont(Font *font);
  void drawImageRegion(Canvas *src, const MAPoint2d *dstPoint, const MARect *srcRect);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRectFilled(int left, int top, int width, int height);
  void drawText(int left, int top, const char *str, int len);
  int  getPixel(int x, int y);
  MAExtent getTextSize(const char *str, int len);
  int getHeight();
  int getWidth();
  void redraw();
  void setClip(int x, int y, int w, int h);
  void setColor(pixel_t color) {  _drawColor = color; }
  void setFont(Font *font) { _font = font; }
  MAHandle setDrawTarget(MAHandle maHandle);

private:
  bool loadFont();
  void drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y);

  FT_Library _fontLibrary;
  FT_Face _fontFace;
  FT_Byte *_fontBuffer;
  Canvas *_screen;
  Canvas *_drawTarget;
  Font *_font;
  android_app *_app;
  pixel_t _drawColor;
};

#endif
