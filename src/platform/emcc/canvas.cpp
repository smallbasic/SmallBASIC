// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <emscripten.h>
#include <stdint.h>
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

EM_JS(int, get_screen_height, (), {
    return document.getElementById("canvas").height;
  });

EM_JS(int, get_screen_width, (), {
    return document.getElementById("canvas").width;
  });

EM_JS(int, get_text_size, (int id, const char *str, int len), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    var s = new String(str).substring(0, len);
    var metrics = ctx.measureText(s);
    var result = (metrics.width << 16) + (metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent);
    return result;
  });

EM_JS(void, draw_arc, (int id, int xc, int yc, double r, double start, double end, double aspect), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.beginPath();
    ctx.arc(100, 75, 50, 0, 2 * Math.PI);
    ctx.stroke();
  });

EM_JS(void, draw_ellipse, (int id, int xc, int yc, int rx, int ry, int fill), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");

  });

EM_JS(void, draw_line, (int id, int x1, int y1, int x2, int y2, const char *color), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.beginPath();
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.lineWidth = 1;
    ctx.strokeStyle = UTF8ToString(color);
    ctx.stroke();
  });

EM_JS(void, draw_pixel, (int id, int x, int y, int r, int g, int b), {
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

EM_JS(void, draw_rect_filled, (int id, int x, int y, int w, int h, const char *color), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.fillStyle = UTF8ToString(color);
    ctx.fillRect(x, y, w, h);
  });

EM_JS(void, draw_region, (int id, int x, int y, int w, int h), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");

  });

EM_JS(void, draw_text, (int id, int x, int y, const char *str, int len, const char *color, const char *face), {
    var c = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    var s = UTF8ToString(str).substring(0, len);
    var width = ctx.measureText(s).width;
    var fontHeight = 15;
    var y1 = y * fontHeight;
    ctx.font = UTF8ToString(face);
    ctx.fillStyle = UTF8ToString(color);
    ctx.fillText(s, x, y1);
  });

strlib::String get_color() {
  strlib::String result;
  long c = drawColor;
  if (c < 0) {
    c = -c;
  }
  if (c >= 0 && c <= 15) {
    result.append(colors[c]);
  } else {
    uint8_t sR, sG, sB;
    GET_RGB(c, sR, sG, sB);
    char buf[8];
    sprintf(buf, "#%x%x%x", sR, sG, sB);
    result.append(buf);
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
      canvas.style.zIndex = 1;
      canvas.style.display = "none";
      canvas.style.position = "absolute";
      canvas.className = "emscripten";
      document.body.appendChild(canvas);
    }, _id);
  return true;
};

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
    result = (MAExtent)get_text_size(drawTarget->_id, str, strlen(str));
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
void maArc(int xc, int yc, double r, double start, double end, double aspect) {
  if (drawTarget) {
    draw_arc(drawTarget->_id, xc, yc, r, start, end, aspect);
  }
}

void maEllipse(int xc, int yc, int rx, int ry, int fill) {
  if (drawTarget) {
    draw_ellipse(drawTarget->_id, xc, yc, rx, ry, fill);
  }
}

void maLine(int startX, int startY, int endX, int endY) {
  if (drawTarget) {
    draw_line(drawTarget->_id, startX, startY, endX, endY, get_color());
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

void maFillRect(int left, int top, int width, int height) {
  logEntered();
  if (drawTarget) {
    draw_rect_filled(drawTarget->_id, left, top, width, height, get_color());
  }
}

void maDrawText(int left, int top, const char *str, int length) {
  if (str && str[0] && drawTarget) {
    strlib::String face;
    if (font->_italic) {
      face.append("italic ");
    }
    if (font->_bold) {
      face.append("bold ");
    }
    face.append(font->_size).append("pt monospace");
    draw_text(drawTarget->_id, left, top, str, length, get_color(), face);
  }
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect, const MAPoint2d *dstPoint, int transformMode) {
  logEntered();
  Canvas *src = (Canvas *)maHandle;
  if (drawTarget && drawTarget != src) {
    //draw_region(drawTarget->_id, src, srcRect, dstPoint->x, dstPoint->y);
  }
}

void maDrawRGB(const MAPoint2d *dstPoint, const void *src,  const MARect *srcRect, int opacity, int stride) {
  logEntered();
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int stride) {
  logEntered();
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
