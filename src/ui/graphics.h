// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef UI_GRAPHICS
#define UI_GRAPHICS

#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/canvas.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define MAX_GLYPHS 256

using namespace strlib;

inline void RGB888_to_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (c & 0xff0000) >> 16;
  g = (c & 0xff00) >> 8;
  b = (c & 0xff);
}

inline pixel_t RGB888_to_RGBA8888(unsigned c) {
  uint8_t r = (c & 0xff0000) >> 16;
  uint8_t g = (c & 0xff00) >> 8;
  uint8_t b = (c & 0xff);
  return ((0xff000000) | (b << 16) | (g << 8) | (r));
}

inline void RGB888_BE_to_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  b = (c & 0xff0000) >> 16;
  g = (c & 0xff00) >> 8;
  r = (c & 0xff);
}

#if defined(PIXELFORMAT_RGBA8888)
  #define SET_RGB(r, g, b) ((0xff000000) | (r << 16) | (g << 8) | (b))
  #define GET_RGB RGB888_to_RGB
  #define GET_RGB2 RGB888_BE_to_RGB
  #define GET_FROM_RGB888 RGB888_to_RGBA8888
#else
  #define SET_RGB(r, g, b) ((r << 16) | (g << 8) | (b))
  #define GET_RGB  RGB888_to_RGB
  #define GET_RGB2 RGB888_to_RGB
  #define GET_FROM_RGB888(c) (c)
#endif

namespace ui {

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

struct Graphics {
  Graphics();
  virtual ~Graphics();

  Font *createFont(int style, int size);
  void deleteFont(Font *font);
  void drawImageRegion(Canvas *src, const MAPoint2d *dstPoint, const MARect *srcRect);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRectFilled(int left, int top, int width, int height);
  void drawRGB(const MAPoint2d *dstPoint, const void *src,
               const MARect *srcRect, int opacity, int bytesPerLine);
  void drawText(int left, int top, const char *str, int len);
  pixel_t getDrawColor() { return _drawColor; }
  Canvas *getDrawTarget() { return _drawTarget; }
  void getImageData(Canvas *canvas, uint8_t *image, 
                    const MARect *srcRect, int bytesPerLine);
  int  getPixel(Canvas *canvas, int x, int y);
  MAExtent getTextSize(const char *str, int len);
  int getHeight() { return _screen->_h; }
  int getWidth() { return _screen->_w; }
  void setClip(int x, int y, int w, int h);
  void setColor(pixel_t color) { _drawColor = color; }
  void setFont(Font *font) { _font = font; }
  MAHandle setDrawTarget(MAHandle maHandle);

protected:
  void drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y);
  void wuLine(int x0, int y0, int x1, int y1);
  void wuPlot(int x, int y, double c);

  FT_Library _fontLibrary;
  FT_Face _fontFace;
  FT_Face _fontFaceB;
  Canvas *_screen;
  Canvas *_drawTarget;
  Font *_font;
  pixel_t _drawColor;
};

}

#endif
