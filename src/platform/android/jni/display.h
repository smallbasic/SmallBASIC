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

#define MAX_GLYPHS 256

using namespace strlib;

typedef uint16_t pixel_t;

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

  bool create(int w, int h);
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y) { return _canvas + (y * _w); }
  int x() { return _clip ? _clip->left : 0; }
  int y() { return _clip ? _clip->top : 0; }
  int w() { return _clip ? _clip->right : _w; }
  int h() { return _clip ? _clip->bottom : _h; }

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
  int getHeight() { return _h; }
  int getWidth() { return _w; }
  void redraw();
  void resize();
  void setSize(int w, int h) { _w = w; _h = h; }
  void setClip(int x, int y, int w, int h);
  void setColor(pixel_t color) {  _drawColor = color; }
  void setFont(Font *font) { _font = font; }
  MAHandle setDrawTarget(MAHandle maHandle);

private:
  bool loadFonts();
  bool loadFont(const char *name, FT_Face &face, FT_Byte **buffer);
  void drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y);

  FT_Library _fontLibrary;
  FT_Face _fontFace;
  FT_Face _fontFaceB;
  FT_Byte *_fontBuffer;
  FT_Byte *_fontBufferB;
  Canvas *_screen;
  Canvas *_drawTarget;
  Font *_font;
  android_app *_app;
  pixel_t _drawColor;
  int _w, _h;
};

#endif
