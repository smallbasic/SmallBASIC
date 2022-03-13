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
#include "ui/strlib.h"
#include "lib/maapi.h"
#include "platform/emcc/canvas.h"

static int nextId = 1000;
pixel_t drawColor;
Canvas *screen;
Canvas *drawTarget;
Font *font;

const char *colors[] = {
  "#000",    // 0 black
  "#000080", // 1 blue
  "#008000", // 2 green
  "#008080", // 3 cyan
  "#800000", // 4 red
  "#800080", // 5 magenta
  "#800800", // 6 yellow
  "#c0c0c0", // 7 white
  "#808080", // 8 gray
  "#0000ff", // 9 light blue
  "#00ff00", // 10 light green
  "#00ffff", // 11 light cyan
  "#ff0000", // 12 light red
  "#ff00ff", // 13 light magenta
  "#ffff00", // 14 light yellow
  "#fff"     // 15 bright white
};

const uint32_t colors_i[] = {
  0x000000, // 0 black
  0x000080, // 1 blue
  0x008000, // 2 green
  0x008080, // 3 cyan
  0x800000, // 4 red
  0x800080, // 5 magenta
  0x808000, // 6 yellow
  0xC0C0C0, // 7 white
  0x808080, // 8 gray
  0x0000FF, // 9 light blue
  0x00FF00, // 10 light green
  0x00FFFF, // 11 light cyan
  0xFF0000, // 12 light red
  0xFF00FF, // 13 light magenta
  0xFFFF00, // 14 light yellow
  0xFFFFFF  // 15 bright white
};

EM_JS(int, draw_pixel, (int id, int x, int y, int r, int g, int b), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    var pxId = ctx.createImageData(1, 1);
    var pxData = pxId.data;
    pxData[0] = r;
    pxData[1] = g;
    pxData[2] = b;
    pxData[3] = 255;
    ctx.putImageData(pxId, x, y);
  });

EM_JS(int, draw_text, (int id, int x, int y, const char *str, int len, const char *fg), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    var s = new String(str).substring(0, len);
    var face = (i ? "italic" : "") + " " + (b ? "bold" : "");
    var width = ctx.measureText(s).width;
    var y1 = y * fontHeight;
    ctx.font=face + " " + fontSize + "pt monospace";
    ctx.fillStyle = fg;
    ctx.fillText(s, x, y1);
  });

EM_JS(int, get_screen_width, (), {
    return document.getElementById("canvas").width;
  });

EM_JS(int, get_screen_height, (), {
    return document.getElementById("canvas").height;
  });

EM_JS(int, get_text_size, (int id, const char *str, int len), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    var s = new String(str).substring(0, len);
    var metrics = ctx.measureText(s);
    var result = (metrics.width << 16) + (metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent);
    return result;
  });

strlib::String get_color() {
  strlib::String result;
  long c = drawColor;
  if (c < 0) {
    c = -c;
    int r = (c>>16) & 0xFF;
    int g = (c>>8) & 0xFF;
    int b = (c) & 0xFF;
    char buf[8];
    sprintf(buf, "#%x%x%x", b, g, r);
    result.append(buf);
  } else {
    result.append((colors[c > 15 ? 15 : c]));
  }
  return result;
}

Canvas::Canvas() :
  _clip(nullptr),
  _id(-1),
  _w(0),
  _h(0) {
}

Canvas::Canvas(int width, int height) :
  _clip(nullptr),
  _id(-1),
  _w(width),
  _h(height) {
}

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
      canvas.id = "canvas_" + $0;
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
  EM_ASM_({
      var c = document.getElementById($0 == -1 ? "canvas" : "canvas_" + $0);
      var ctx = c.getContext("2d");
      ctx.beginPath();
      ctx.arc(100, 75, 50, 0, 2 * Math.PI);
      ctx.stroke();
    }, _id, xc, yc, r, start, end, aspect);
}

void Canvas::drawEllipse(int xc, int yc, int rx, int ry, bool fill) {
  logEntered();
}

void Canvas::drawLine(int startX, int startY, int endX, int endY) {
  logEntered();
}

void Canvas::drawRGB(const MAPoint2d *dstPoint, const void *src, const MARect *srcRect, int opacity, int bytesPerLine) {
  logEntered();
}

void Canvas::drawRegion(Canvas *src, const MARect *srcRect, int destX, int destY) {
  logEntered();
}

void Canvas::fillRect(int x, int y, int w, int h) {
  logEntered();
}

MAExtent Canvas::getTextSize(const char *str, int len) {
  return (MAExtent)get_text_size(_id, str, len);
}

void Canvas::getImageData(Canvas *canvas, uint8_t *image, const MARect *srcRect, int bytesPerLine) {
  logEntered();
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
  if (screen == nullptr) {
    screen = new Canvas(get_screen_width(), get_screen_height());
  }
  return (MAExtent)((screen->_w << 16) + screen->_h);
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Canvas *holder = (Canvas *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int stride) {
  Canvas *holder = (Canvas *)maHandle;
  drawTarget->getImageData(holder, (uint8_t *)dst, srcRect, stride);
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  logEntered();
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

void maUpdateScreen(void) {
  logEntered();
  trace("%d \n", screen->_id);
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
    int c = drawColor;
    if (c < 0) {
      c = -c;
    } else {
      c = colors_i[c > 15 ? 15 : c];
    }
    int r = (c & 0xff0000) >> 16;
    int g = (c & 0xff00) >> 8;
    int b = (c & 0xff);
    draw_pixel(drawTarget->_id, posX, posY, r, g, b);
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
    draw_text(drawTarget->_id, left, top, str, length, get_color());
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
