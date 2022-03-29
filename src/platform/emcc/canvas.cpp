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
    return window.innerHeight;
  });

EM_JS(int, get_screen_width, (), {
    return window.innerWidth;
  });

EM_JS(int, get_text_size, (int id, const char *str, const char *face), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.font = UTF8ToString(face);

    var s = UTF8ToString(str);
    var metrics = ctx.measureText(s);
    var height = metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent + 2;
    var width = s == "W" ? metrics.width : (metrics.actualBoundingBoxRight + 2);
    return (Math.round(width) << 16) + height;
  });

EM_JS(void, draw_arc, (int id, int xc, int yc, double r, double start, double end, double aspect), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.beginPath();
    ctx.arc(100, 75, 50, 0, 2 * Math.PI);
    ctx.stroke();
  });

EM_JS(void, draw_ellipse, (int id, int xc, int yc, int rx, int ry, int fill), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");

  });

EM_JS(void, draw_line, (int id, int x1, int y1, int x2, int y2, const char *color), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.beginPath();
    ctx.moveTo(x1, y1 + 1);
    ctx.lineTo(x2, y2 + 1);
    ctx.lineWidth = 1;
    ctx.strokeStyle = UTF8ToString(color);
    ctx.stroke();
  });

EM_JS(void, draw_pixel, (int id, int x, int y, int r, int g, int b), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
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
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.fillStyle = UTF8ToString(color);
    ctx.fillRect(x, y, w, h);
  });

EM_JS(void, draw_region, (int id, int srcId, int x, int y, int w, int h, int dx, int dy), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var destination = canvas.getContext("2d");
    var source = document.getElementById(srcId == -1 ? "canvas" : "canvas_" + srcId);
    destination.drawImage(source, x, y, w, h, dx, dy, w, h);
  });

EM_JS(void, draw_text, (int id, int x, int y, const char *str, int len, const char *color, const char *face), {
    var canvas = document.getElementById(id == -1 ? "canvas" : "canvas_" + id);
    var ctx = canvas.getContext("2d");
    ctx.font = UTF8ToString(face);

    var s = UTF8ToString(str).substring(0, len);
    var metrics = ctx.measureText(s);
    ctx.fillStyle = UTF8ToString(color);
    ctx.fillText(s, x, y + metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent);
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
    sprintf(buf, "#%02x%02x%02x", sR, sG, sB);
    result.append(buf);
  }
  return result;
}

Font::Font(int size, bool bold, bool italic) :
  _size(size),
  _bold(bold),
  _italic(italic) {
  _face.empty();
  if (italic) {
    _face.append("italic ");
  }
  if (bold) {
    _face.append("bold ");
  }
  _face.append(size).append("px monospace");
}

Font::~Font() {
  if (this == font) {
    font = nullptr;
  }
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
        var element = document.getElementById($0 == -1 ? "canvas" : "canvas_" + $0);
        document.body.removeChild(element);
      }, _id);
  }
  if (this == screen) {
    screen = nullptr;
  } else if (this == drawTarget) {
    drawTarget = nullptr;
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
      canvas.style.display = "none";
      canvas.style.position = "absolute";
      canvas.className = "emscripten";
      document.body.appendChild(canvas);
    }, _id, _w, _h);
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
  if (str && str[0]) {
    result = (MAExtent)get_text_size(drawTarget ? drawTarget->_id : -1, str, font->_face.c_str());
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
  logEntered();
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
    draw_text(drawTarget->_id, left, top, str, length, get_color(), font->_face.c_str());
  }
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect, const MAPoint2d *dstPoint, int transformMode) {
  logEntered();
  Canvas *src = (Canvas *)maHandle;
  if (drawTarget && drawTarget != src) {
    draw_region(drawTarget->_id, src->_id, srcRect->left, srcRect->top, srcRect->width, srcRect->height, dstPoint->x, dstPoint->y);
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
  Font *newFont = new Font(size, bold, italic);
  return (MAHandle)newFont;
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  font = ((Font *)maHandle);
  return maHandle;
}

int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    Font *handleFont = (Font *)maHandle;
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
