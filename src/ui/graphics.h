// This file is part of SmallBASIC
//
// Copyright(C) 2001-2018 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef UI_GRAPHICS
#define UI_GRAPHICS

#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/rgb.h"
#include "ui/canvas.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define MAX_GLYPHS 256

using namespace strlib;

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
  Glyph _glyph[MAX_GLYPHS]{};
};

struct Graphics {
  Graphics();
  virtual ~Graphics();

  Font *createFont(int style, int size) const;
  void deleteFont(const Font *font);
  void drawArc(int xc, int yc, double r, double start, double end, double aspect);
  void drawEllipse(int xc, int yc, int rx, int ry, bool fill);
  void drawAaEllipse(int xc, int yc, int rx, int ry, bool fill);
  void drawImageRegion(Canvas *src, const MAPoint2d *dstPoint, const MARect *srcRect);
  void drawLine(int startX, int startY, int endX, int endY) const;
  void drawPixel(int posX, int posY) const;
  void drawRectFilled(int left, int top, int width, int height);
  void drawRGB(const MAPoint2d *dstPoint, const void *src,
               const MARect *srcRect, int opacity, int bytesPerLine) const;
  void drawText(int left, int top, const char *str, int len) const;
  pixel_t getDrawColor() const { return _drawColor; }
  Canvas *getDrawTarget() { return _drawTarget; }
  void getImageData(Canvas *canvas, uint8_t *image, 
                    const MARect *srcRect, int bytesPerLine) const;
  int  getPixel(Canvas *canvas, int x, int y) const;
  MAExtent getTextSize(const char *str, int len) const;
  int getHeight() { return _screen->_h; }
  int getWidth() { return _screen->_w; }
  void setClip(int x, int y, int w, int h) const;
  void setColor(pixel_t color) { _drawColor = color; }
  void setFont(Font *font) { _font = font; }
  MAHandle setDrawTarget(MAHandle maHandle);

protected:
  void drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y) const;
  void aaLine(int x0, int y0, int x1, int y1) const;
  void aaPlot(int x, int y, double c) const;
  void aaPlotX8(int xc, int yc, int x, int y, double c, bool fill) const;
  void aaPlotY8(int xc, int yc, int x, int y, double c, bool fill) const;
  void plot4(int xc, int yc, int x, int y);
  void line2(int xc, int yc, int x, int y) const;

  FT_Library _fontLibrary{};
  FT_Face _fontFace{};
  FT_Face _fontFaceB{};
  Canvas *_screen;
  Canvas *_drawTarget;
  Font *_font;
  pixel_t _drawColor{};
};

}

#endif
