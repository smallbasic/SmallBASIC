// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include <emscripten.h>
#include <emscripten/val.h>
#include "ui/utils.h"
#include "ui/rgb.h"
#include "lib/maapi.h"

struct Canvas;
struct Font;
static int nextId = 1000;
pixel_t drawColor;
Canvas *screen;
Canvas *drawTarget;
Font *font;

EM_JS(int, get_canvas_width, (), {
    return document.getElementById("canvas").width;
  });

EM_JS(int, get_canvas_height, (), {
    return document.getElementById("canvas").height;
  });

struct Font {
  Font(int size, bool bold, bool italic) :
    _size(size),
    _bold(bold),
    _italic(italic) {
  }

  int _size;
  bool _bold;
  bool _italic;
};

struct Canvas {
  Canvas() : _clip(nullptr), _id(-1), _w(0), _h(0) {}
  ~Canvas();

  bool create(int width, int height);
  void drawArc(int xc, int yc, double r, double start, double end, double aspect);
  void drawEllipse(int xc, int yc, int rx, int ry, bool fill);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRGB(const MAPoint2d *dstPoint, const void *src, const MARect *srcRect, int opacity, int bytesPerLine);
  void drawRegion(Canvas *src, const MARect *srcRect, int destX, int destY);
  void drawText(int left, int top, const char *str, int len);
  void fillRect(int x, int y, int w, int h);
  MAExtent getTextSize(const char *str, int len);
  int  getPixel(Canvas *canvas, int x, int y);
  void getImageData(Canvas *canvas, uint8_t *image, const MARect *srcRect, int bytesPerLine);
  void setClip(int x, int y, int w, int h);

  MARect *_clip;
  int _id;
  int _w;
  int _h;
};

Canvas::~Canvas() {
  if (_id != -1) {
    EM_ASM_({
        var element = document.getElementById($0);
        document.body.removeChild(element);
      }, _id);
  }
}

bool Canvas::create(int width, int height) {
  _id = ++nextId;
  _w = width;
  _h = height;
  EM_ASM_({
      var canvas = document.createElement("canvas");
      canvas.id = "_canvas" + $0;
      canvas.width = $1;
      canvas.height = $2;
      canvas.style.zIndex = 1;
      canvas.style.position = "absolute";
      canvas.style.border = "1px solid";
      document.body.appendChild(canvas);
    }, _id, width, height);
  return true;
};

void Canvas::drawArc(int xc, int yc, double r, double start, double end, double aspect) {
}

void Canvas::drawEllipse(int xc, int yc, int rx, int ry, bool fill) {
}

void Canvas::drawLine(int startX, int startY, int endX, int endY) {
}

void Canvas::drawPixel(int posX, int posY) {
}

void Canvas::drawRGB(const MAPoint2d *dstPoint, const void *src, const MARect *srcRect, int opacity, int bytesPerLine) {
}

void Canvas::drawRegion(Canvas *src, const MARect *srcRect, int destX, int destY) {
}

void Canvas::drawText(int left, int top, const char *str, int len) {
}

void Canvas::fillRect(int x, int y, int w, int h) {
}

MAExtent Canvas::getTextSize(const char *str, int len) {
  int width = 0;
  int height = 0;

  return (MAExtent)((width << 16) + height);
}

int  Canvas::getPixel(Canvas *canvas, int x, int y) {
  return 0;
}

void Canvas::getImageData(Canvas *canvas, uint8_t *image, const MARect *srcRect, int bytesPerLine) {
}

void Canvas::setClip(int x, int y, int w, int h) {
  delete _clip;
  if (x != 0 || y != 0 || _w != w || _h != h) {
    _clip = new MARect();
    _clip->left = x;
    _clip->top = y;
    _clip->width = x + w;
    _clip->height = y + h;
  } else {
    _clip = nullptr;
  }
}

//
// maapi implementation
//

MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Canvas();
  return maHandle;
}

int maSetColor(int c) {
  drawColor = GET_FROM_RGB888(c);
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  if (drawTarget) {
    drawTarget->setClip(left, top, width, height);
  }
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (str && str[0] && drawTarget) {
    result = drawTarget->getTextSize(str, strlen(str));
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = get_canvas_width();
  short height = get_canvas_height();
  return (MAExtent)((width << 16) + height);
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Canvas *holder = (Canvas *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int stride) {
  Canvas *holder = (Canvas *)maHandle;
  if (srcRect->width == 1 && srcRect->height == 1) {
    *((int *)dst) = drawTarget->getPixel(holder, srcRect->left, srcRect->top);
  } else {
    drawTarget->getImageData(holder, (uint8_t *)dst, srcRect, stride);
  }
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  MAHandle result = (MAHandle) drawTarget;
  if (maHandle == (MAHandle) HANDLE_SCREEN ||
      maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    drawTarget = screen;
  } else {
    drawTarget = (Canvas *)maHandle;
  }
  delete drawTarget->_clip;
  drawTarget->_clip = nullptr;
  return result;
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  Canvas *drawable = (Canvas *)maHandle;
  return drawable->create(width, height) ? RES_OK : -1;
}

//
// drawing
//

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect, const MAPoint2d *dstPoint, int transformMode) {
  Canvas *src = (Canvas *)maHandle;
  if (drawTarget && drawTarget != src) {
    drawTarget->drawRegion(src, srcRect, dstPoint->x, dstPoint->y);
  }
}

void maPlot(int posX, int posY) {
  if (drawTarget) {
    drawTarget->drawPixel(posX, posY);
  }
}

void maLine(int startX, int startY, int endX, int endY) {
  if (drawTarget) {
    drawTarget->drawLine(startX, startY, endX, endY);
  }
}

void maFillRect(int left, int top, int width, int height) {
  if (drawTarget) {
    drawTarget->fillRect(left, top, width, height);
  }
}

void maArc(int xc, int yc, double r, double start, double end, double aspect) {
  if (drawTarget) {
    drawTarget->drawArc(xc, yc, r, start, end, aspect);
  }
}

void maEllipse(int xc, int yc, int rx, int ry, int fill) {
  if (drawTarget) {
    drawTarget->drawEllipse(xc, yc, rx, ry, fill);
  }
}

void maDrawText(int left, int top, const char *str, int length) {
  if (str && str[0] && drawTarget) {
    drawTarget->drawText(left, top, str, length);
  }
}

void maDrawRGB(const MAPoint2d *dstPoint, const void *src,  const MARect *srcRect, int opacity, int stride) {
  drawTarget->drawRGB(dstPoint, src, srcRect, opacity, stride);
}

//
// font
//
MAHandle maFontLoadDefault(int type, int style, int size) {
  bool italic = (style & FONT_STYLE_ITALIC);
  bool bold = (style & FONT_STYLE_BOLD);
  if (font) {
    delete font;
  }
  font = new Font(size, bold, italic);
  return (MAHandle)font;
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  font = ((Font *)maHandle);
  return maHandle;
}

int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    Font *handleFont = (Font *)maHandle;
    if (font == handleFont) {
      font = nullptr;
    }
    delete handleFont;
  }
  return RES_FONT_OK;
}

//
// not implemented
//
void maShowVirtualKeyboard(void) {
}

void maHideVirtualKeyboard(void) {
}

void maUpdateScreen(void) {
}
