// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "ui/graphics.h"
#include "ui/utils.h"
#include <math.h>

#include "common/smbas.h"
#include "common/device.h"

using namespace ui;

Graphics *graphics;

// fractional part of x
inline double fpart(double x) {
  double result;
  if (x < 0) {
    result = 1 - (x - floor(x));
  } else {
    result = x - floor(x);
  }
  return result;
}

inline double rfpart(double x) {
  return 1 - fpart(x);
}

#define _SWAP(a, b) \
  { __typeof__(a) tmp; tmp = a; a = b; b = tmp; }

Font::Font(FT_Face face, int size, bool italic) :
  _face(face) {
  FT_Set_Pixel_Sizes(face, 0, size);
  _spacing = 1 + (FT_MulFix(_face->height, _face->size->metrics.x_scale) / 64);
  _h = (FT_MulFix(_face->ascender, _face->size->metrics.x_scale) / 64) +
       (FT_MulFix(_face->descender, _face->size->metrics.x_scale) / 64);
  FT_Matrix  matrix;
  if (italic) {
    matrix.xx = 0x10000L;
    matrix.xy = 0.12 * 0x10000L;
    matrix.yx = 0;
    matrix.yy = 0x10000L;
  }

  for (int i = 0; i < MAX_GLYPHS; i++) {
    FT_UInt slot = FT_Get_Char_Index(face, i);
    FT_Error error = FT_Load_Glyph(face, slot, FT_LOAD_TARGET_LIGHT);
    if (error) {
      trace("Failed to load %d", i);
    }
    error = FT_Get_Glyph(face->glyph, &_glyph[i]._slot);
    if (error) {
      trace("Failed to get glyph %d", i);
    }
    if (italic) {
      FT_Glyph_Transform(_glyph[i]._slot, &matrix, 0 );
    }
    FT_Vector origin;
    origin.x = 0;
    origin.y = 0;
    error = FT_Glyph_To_Bitmap(&_glyph[i]._slot, FT_RENDER_MODE_LIGHT, &origin, 1);
    if (error) {
      trace("Failed to get bitmap %d", i);
    }
    _glyph[i]._w = (int)(face->glyph->metrics.horiAdvance / 64);
  }
}

Font::~Font() {
  for (int i = 0; i < MAX_GLYPHS; i++) {
    FT_Done_Glyph(_glyph[i]._slot);
  }
}

//
// Graphics implementation
//
Graphics::Graphics() :
  _screen(NULL),
  _drawTarget(NULL),
  _font(NULL) {
  graphics = this;
}

Graphics::~Graphics() {
  logEntered();

  FT_Done_FreeType(_fontLibrary);
  delete _screen;
  graphics = NULL;
  _screen = NULL;
}

Font *Graphics::createFont(int style, int size) {
  Font *result;
  bool italic = (style & FONT_STYLE_ITALIC);
  if (style & FONT_STYLE_BOLD) {
    result = new Font(_fontFaceB, size, italic);
  } else {
    result = new Font(_fontFace, size, italic);
  }
  return result;
}

void Graphics::deleteFont(Font *font) {
  if (font == _font) {
    _font = NULL;
  }
  delete font;
}

void Graphics::drawLine(int startX, int startY, int endX, int endY) {
  if (_drawTarget) {
    if (startY == endY) {
      // horizontal
      int x1 = startX;
      int x2 = endX;
      if (x1 > endX) {
        x1 = endX;
        x2 = startX;
      }
      if (x1 < 0) {
        x1 = 0;
      }
      if (x2 >= _drawTarget->_w) {
        x2 = _drawTarget->_w -1;
      }
      if (startY >= 0 && startY < _drawTarget->_h) {
        pixel_t *line = _drawTarget->getLine(startY);
        if (x1 < _drawTarget->x()) {
          x1 = _drawTarget->x();
        }
        if (x2 >= _drawTarget->w()) {
          x2 = _drawTarget->w() - 1;
        }
        for (int x = x1; x <= x2; x++) {
          line[x] = _drawColor;
        }
      }
    } else if (startX == endX) {
      // vertical
      int y1 = startY;
      int y2 = endY;
      if (y1 > y2) {
        y1 = endY;
        y2 = startY;
      }
      if (y1 < 0) {
        y1 = 0;
      }
      if (y2 >= _drawTarget->_h) {
        y2 = _drawTarget->_h - 1;
      }
      if (startX >= 0 && startX < _drawTarget->_w) {
        if (y1 < _drawTarget->y()) {
          y1 = _drawTarget->y();
        }
        if (y2 >= _drawTarget->h()) {
          y2 = _drawTarget->h() - 1;
        }
        for (int y = y1; y <= y2; y++) {
          pixel_t *line = _drawTarget->getLine(y);
          line[startX] = _drawColor;
        }
      }
    } else if (opt_antialias) {
      // gradient
      wuLine(startX, startY, endX, endY);
    } else {
      g_line(startX, startY, endX, endY, maPlot);
    }
  }
}

void Graphics::drawPixel(int posX, int posY) {
  if (_drawTarget
      && posX >= _drawTarget->x()
      && posY >= _drawTarget->y()
      && posX < _drawTarget->w()
      && posY < _drawTarget->h()) {
    pixel_t *line = _drawTarget->getLine(posY);
    line[posX] = _drawColor;
  }
}

void Graphics::drawRGB(const MAPoint2d *dstPoint, const void *src,
                       const MARect *srcRect, int opacity, int bytesPerLine) {
  uint8_t *image = (uint8_t *)src;
  size_t scale = 1;
  int w = bytesPerLine;

  if (opacity > 0 && opacity < 100) {
    // higher opacity values should make the image less transparent
    opacity = 100 - opacity;
  }
  for (int y = srcRect->top; y < srcRect->height; y += scale) {
    int dY = dstPoint->y + y;
    if (dY >= _drawTarget->y() &&
        dY <  _drawTarget->h()) {
      pixel_t *line = _drawTarget->getLine(dY);
      for (int x = srcRect->left; x < srcRect->width; x += scale) {
        int dX = dstPoint->x + x;
        if (dX >= _drawTarget->x() &&
            dX <  _drawTarget->w()) {
          // get RGBA components
          uint8_t r,g,b,a;
          r = image[4 * y * w + 4 * x + 0]; // red
          g = image[4 * y * w + 4 * x + 1]; // green
          b = image[4 * y * w + 4 * x + 2]; // blue
          a = image[4 * y * w + 4 * x + 3]; // alpha

          uint8_t dR, dG, dB;
          GET_RGB(line[dX], dR, dG, dB);
          if (opacity > 0 && opacity < 100 && a > 64) {
            float op = opacity / 100.0f;
            dR = ((1-op) * r) + (op * dR);
            dG = ((1-op) * g) + (op * dG);
            dB = ((1-op) * b) + (op * dB);
          } else {
            dR = dR + ((r - dR) * a / 255);
            dG = dG + ((g - dG) * a / 255);
            dB = dB + ((b - dB) * a / 255);
          }
          line[dX] = SET_RGB(dR, dG, dB);
        }
      }
    }
  }
}

void Graphics::drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y) {
  FT_Int xMax = x + bitmap->width;
  FT_Int yMax = y + bitmap->rows;

  uint8_t sR, sG, sB;
  GET_RGB(_drawColor, sR, sG, sB);

  int dtX = _drawTarget->x();
  int dtY = _drawTarget->y();
  int dtW = _drawTarget->w();
  int dtH = _drawTarget->h();

  for (FT_Int j = y, q = 0; j < yMax; j++, q++) {
    if (j >= dtY && j < dtH) {
      pixel_t *line = _drawTarget->getLine(j);
      for (FT_Int i = x, p = 0; i < xMax; i++, p++) {
        if (i >= dtX && i < dtW) {
          uint8_t a = bitmap->buffer[q * bitmap->width + p];
          if (a == 255) {
            line[i] = _drawColor;
          } else {
            // blend drawColor to the background
            uint8_t dR, dG, dB;
            GET_RGB(line[i], dR, dG, dB);
            dR = dR + ((sR - dR) * a / 255);
            dG = dG + ((sG - dG) * a / 255);
            dB = dB + ((sB - dB) * a / 255);
            line[i] = SET_RGB(dR, dG, dB);
          }
        }
      }
    }
  }
}

void Graphics::drawText(int left, int top, const char *str, int len) {
  if (_drawTarget && _font) {
    FT_Vector pen;
    pen.x = left;
    pen.y = top + _font->_h + ((_font->_spacing - _font->_h) / 2);
    for (int i = 0; i < len; i++) {
      uint8_t ch = str[i];
      FT_BitmapGlyph glyph = (FT_BitmapGlyph)_font->_glyph[ch]._slot;
      drawChar(&glyph->bitmap,
               pen.x + glyph->left,
               pen.y - glyph->top);
      pen.x += _font->_glyph[ch]._w;
    }
  }
}

void Graphics::getImageData(Canvas *canvas, uint8_t *image,
                            const MARect *srcRect, int bytesPerLine) {
  size_t scale = 1;
  int x_end = srcRect->left + srcRect->width;
  int y_end = srcRect->top + srcRect->height;
  if (canvas == HANDLE_SCREEN) {
    canvas = _screen;
  }
  for (int dy = 0, y = srcRect->top; y < y_end; y += scale, dy++) {
    if (y >= canvas->y() && y < canvas->h()) {
      pixel_t *line = canvas->getLine(y);
      int yoffs = (dy * bytesPerLine * 4);
      for (int dx = 0, x = srcRect->left; x < x_end; x += scale, dx++) {
        if (x >= canvas->x() && x < canvas->w()) {
          uint8_t r, g, b;
          GET_RGB2(line[x], r, g, b);
          int offs = yoffs + (dx * 4);
          image[offs + 0] = r;
          image[offs + 1] = g;
          image[offs + 2] = b;
          image[offs + 3] = 255;
        }
      }
    }
  }
}

int Graphics::getPixel(Canvas *canvas, int posX, int posY) {
  int result = 0;
  if (canvas == HANDLE_SCREEN) {
    canvas = _screen;
  }
  if (canvas
      && posX > -1
      && posY > -1
      && posX < canvas->_w
      && posY < canvas->_h - 1) {
    pixel_t *line = canvas->getLine(posY);
    result = line[posX];
#if defined(PIXELFORMAT_RGBA8888)
    uint8_t r,g,b;
    GET_RGB2(result, r, g, b);
    result = SET_RGB(r, g, b);
#endif
  }
  return result;
}

MAExtent Graphics::getTextSize(const char *str, int len) {
  int width = 0;
  int height = 0;
  if (_font) {
    for (int i = 0; i < len; i++) {
      uint8_t ch = str[i];
      width += _font->_glyph[ch]._w;
    }
    height = _font->_spacing;
  }
  return (MAExtent)((width << 16) + height);
}

void Graphics::setClip(int x, int y, int w, int h) {
  if (_drawTarget) {
    _drawTarget->setClip(x, y, w, h);
  }
}

MAHandle Graphics::setDrawTarget(MAHandle maHandle) {
  MAHandle result = (MAHandle) _drawTarget;
  if (maHandle == (MAHandle) HANDLE_SCREEN ||
      maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    _drawTarget = _screen;
  } else {
    _drawTarget = (Canvas *)maHandle;
  }
  delete _drawTarget->_clip;
  _drawTarget->_clip = NULL;
  return result;
}

// see: http://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
void Graphics::wuLine(int x0, int y0, int x1, int y1) {
  int steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep) {
    _SWAP(x0, y0);
    _SWAP(x1, y1);
  }
  if (x0 > x1) {
    _SWAP(x0, x1);
    _SWAP(y0, y1);
  }

  double dx = x1 - x0;
  double dy = y1 - y0;
  double gradient = dy / dx;

  // handle first endpoint
  double xend = round(x0);
  double yend = y0 + gradient * (xend - x0);
  double xgap = rfpart(x0 + 0.5);

  int xpxl1 = (int)xend; // this will be used in the main loop
  int ypxl1 = (int)yend;

  if (steep) {
    wuPlot(ypxl1,   xpxl1, rfpart(yend) * xgap);
    wuPlot(ypxl1+1, xpxl1,  fpart(yend) * xgap);
  } else {
    wuPlot(xpxl1, ypxl1  , rfpart(yend) * xgap);
    wuPlot(xpxl1, ypxl1+1,  fpart(yend) * xgap);
  }

  double intery = yend + gradient; // first y-intersection for the main loop

  // handle second endpoint
  xend = round(x1);
  yend = y1 + gradient * (xend - x1);
  xgap = fpart(x1 + 0.5);

  int xpxl2 = (int)xend;
  int ypxl2 = (int)yend;
  if (steep) {
    wuPlot(ypxl2  , xpxl2, rfpart(yend) * xgap);
    wuPlot(ypxl2+1, xpxl2,  fpart(yend) * xgap);
    for (int x = xpxl1 + 1; x < xpxl2; x++) {
      wuPlot((int)intery,   x, rfpart(intery));
      wuPlot((int)intery+1, x, fpart(intery));
      intery += gradient;
    }
  } else {
    wuPlot(xpxl2, ypxl2,  rfpart(yend) * xgap);
    wuPlot(xpxl2, ypxl2+1, fpart(yend) * xgap);
    for (int x = xpxl1 + 1; x < xpxl2; x++) {
      wuPlot(x, (int)intery,   rfpart(intery));
      wuPlot(x, (int)intery+1, fpart(intery));
      intery += gradient;
    }
  }
}

void Graphics::wuPlot(int posX, int posY, double c) {
  if (_drawTarget
      && posX >= _drawTarget->x()
      && posY >= _drawTarget->y()
      && posX < _drawTarget->w()
      && posY < _drawTarget->h()) {
    pixel_t *line = _drawTarget->getLine(posY);
    uint8_t sR, sG, sB;
    uint8_t dR, dG, dB;

    GET_RGB(_drawColor, sR, sG, sB);
    GET_RGB(line[posX], dR, dG, dB);

    dR = (uint8_t)(sR * c + dR * (1 - c));
    dG = (uint8_t)(sG * c + dG * (1 - c));
    dB = (uint8_t)(sB * c + dB * (1 - c));
    line[posX] = SET_RGB(dR, dG, dB);
  }
}

//
// maapi implementation
//
MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Canvas();
  return maHandle;
}

int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    graphics->deleteFont((Font *)maHandle);
  }
  return RES_FONT_OK;
}

int maSetColor(int c) {
  graphics->setColor(GET_FROM_RGB888(c));
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  graphics->setClip(left, top, width, height);
}

void maPlot(int posX, int posY) {
  graphics->drawPixel(posX, posY);
}

void maLine(int startX, int startY, int endX, int endY) {
  graphics->drawLine(startX, startY, endX, endY);
}

void maFillRect(int left, int top, int width, int height) {
  Canvas *drawTarget = graphics->getDrawTarget();
  if (drawTarget) {
    drawTarget->fillRect(left, top, width, height, graphics->getDrawColor());
  }
}

void maDrawText(int left, int top, const char *str, int length) {
  if (str && str[0]) {
    graphics->drawText(left, top, str, length);
  }
}

void maDrawRGB(const MAPoint2d *dstPoint, const void *src,
               const MARect *srcRect, int opacity, int bytesPerLine) {
  graphics->drawRGB(dstPoint, src, srcRect, opacity, bytesPerLine);
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (str && str[0] && graphics) {
    result = graphics->getTextSize(str, strlen(str));
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = graphics->getWidth();
  short height = graphics->getHeight();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return (MAHandle) graphics->createFont(style, size);
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  if (graphics) {
    graphics->setFont((Font *) maHandle);
  }
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  Canvas *drawTarget = graphics->getDrawTarget();
  Canvas *src = (Canvas *)maHandle;
  if (drawTarget && drawTarget != src) {
    drawTarget->drawRegion(src, srcRect, dstPoint->x, dstPoint->y);
  }
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Canvas *holder = (Canvas *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst,
                    const MARect *srcRect, int bytesPerLine) {
  Canvas *holder = (Canvas *)maHandle;
  if (srcRect->width == 1 && srcRect->height == 1) {
    *((int *)dst) = graphics->getPixel(holder, srcRect->left, srcRect->top);
  } else {
    graphics->getImageData(holder, (uint8_t *)dst, srcRect, bytesPerLine);
  }
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  return graphics->setDrawTarget(maHandle);
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  int maxSize = MAX(graphics->getWidth(), graphics->getHeight());
  if (height > maxSize * MAX_CANVAS_SIZE) {
    result -= 1;
  } else {
    Canvas *drawable = (Canvas *)maHandle;
    result = drawable->create(width, height) ? RES_OK : -1;
  }
  return result;
}
