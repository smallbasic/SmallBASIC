// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <math.h>
#include "ui/utils.h"
#include "platform/fltk/display.h"

GraphicsWidget *graphics;

#define MAX_CANVAS_SIZE 20

//
// Canvas implementation
//
Canvas::Canvas() :
  _w(0),
  _h(0),
  _scale(0),
  _offscreen(0),
  _clip(NULL),
  _drawColor(fl_rgb_color(31, 28, 31)) {
}

Canvas::~Canvas() {
  if (_offscreen) {
    fl_delete_offscreen(_offscreen);
    _offscreen = 0;
  }
  delete _clip;
  _clip = NULL;
}

bool Canvas::create(int w, int h) {
  logEntered();
  _w = w;
  _h = h;
  _offscreen = fl_create_offscreen(_w, _h);
  _scale = Fl_Graphics_Driver::default_driver().scale();
  return _offscreen != 0;
}

void Canvas::drawArc(int xc, int yc, double r, double start, double end, double aspect) {
  fl_begin_offscreen(_offscreen);
  fl_push_clip(x(), y(), w(), h());
  fl_color(_drawColor);
  if (r < 1) {
    r = 1;
  }
  while (end < start) {
    end += M_PI * 2.0;
  }
  double th = (end - start) / r;
  double xs = xc + r * cos(start);
  double ys = yc + r * aspect * sin(start);
  double xe = xc + r * cos(end);
  double ye = yc + r * aspect * sin(end);
  int x = xs;
  int y = ys;
  for (int i = 1; i < r; i++) {
    double ph = start + i * th;
    xs = xc + r * cos(ph);
    ys = yc + r * aspect * sin(ph);
    fl_line(x, y, xs, ys);
    x = xs;
    y = ys;
  }
  fl_line(x, y, xe, ye);
  fl_pop_clip();
  fl_end_offscreen();
}

void Canvas::drawEllipse(int xc, int yc, int rx, int ry, bool fill) {
  fl_begin_offscreen(_offscreen);
  fl_push_clip(x(), y(), w(), h());
  fl_color(_drawColor);
  int x = xc - rx;
  int y = yc - ry;
  int w = rx * 2;
  int h = ry * 2;
  if (fill) {
    fl_begin_polygon();
    fl_pie(x, y, w, h, 0.0, 360.0);
    fl_end_polygon();
  } else {
    fl_begin_line();
    fl_arc(x, y, w, h, 0.0, 360.0);
    fl_end_line();
  }
  fl_pop_clip();
  fl_end_offscreen();
}

void Canvas::drawLine(int startX, int startY, int endX, int endY) {
  fl_begin_offscreen(_offscreen);
  fl_push_clip(x(), y(), w(), h());
  fl_color(_drawColor);
  fl_line(startX, startY, endX, endY);
  fl_pop_clip();
  fl_end_offscreen();
}

void Canvas::drawPixel(int posX, int posY) {
  fl_begin_offscreen(_offscreen);
  fl_color(_drawColor);
  fl_line(posX, posY, posX + 1, posY + 1);
  fl_end_offscreen();
}

// x, y, w are position and width of scan line in image. copy w
// pixels from scanline y, starting at pixel x to this buffer.
void drawImage(void *data, int x, int y, int w, uchar *out) {
  uint8_t *image = (uint8_t *)data;
  int scanLine = w * 3;
  int offs = y * w * 4;

  for (int sx = 0; sx < scanLine; sx += 3, offs += 4) {
    out[sx + 0] = image[offs + 0];
    out[sx + 1] = image[offs + 1];
    out[sx + 2] = image[offs + 2];
  }
}

void Canvas::drawRGB(const MAPoint2d *dstPoint, const void *src, const MARect *srcRect, int opacity, int bytesPerLine) {
  int x = dstPoint->x;
  int y = dstPoint->y;
  int w = srcRect->width;
  int h = srcRect->height;
  fl_begin_offscreen(_offscreen);
  fl_draw_image(drawImage, (void *)src, x, y, w, h);
  fl_end_offscreen();
}

void Canvas::drawRegion(Canvas *src, const MARect *srcRect, int destX, int destY) {
  int width = MIN(_w, srcRect->width);
  int height = MIN(_h, srcRect->height);
  fl_begin_offscreen(_offscreen);
  fl_copy_offscreen(destX, destY, width, height, src->_offscreen, srcRect->left, srcRect->top);
  fl_end_offscreen();
}

void Canvas::drawText(Font *font, int left, int top, const char *str, int len) {
  fl_begin_offscreen(_offscreen);
  if (font) {
    font->setCurrent();
  }
  fl_push_clip(x(), y(), w(), h());
  fl_color(_drawColor);
  fl_draw(str, len, x() + left, y() + top);
  fl_pop_clip();
  fl_end_offscreen();
}

void Canvas::fillRect(int left, int top, int width, int height, Fl_Color color) {
  fl_begin_offscreen(_offscreen);
  fl_color(color);
  fl_push_clip(x(), y(), w(), h());
  fl_rectf(left, top, width, height);
  fl_pop_clip();
  fl_end_offscreen();
}

void Canvas::getImageData(uint8_t *image, const MARect *srcRect, int bytesPerLine) {
  fl_begin_offscreen(_offscreen);
  int x = srcRect->left;
  int y = srcRect->top;
  int w = srcRect->width;
  int h = srcRect->height;
  fl_read_image(image, x, y, w, h, 1);
  fl_end_offscreen();

  if (srcRect->width == 1 && srcRect->height == 1) {
    // compatibility with PSET/POINT
    uchar r = image[0];
    uchar g = image[1];
    uchar b = image[2];
    uchar a = image[3];
    image[0] = b;
    image[1] = g;
    image[2] = r;
    image[3] = a;
  }
}

void Canvas::setClip(int x, int y, int w, int h) {
  delete _clip;
  if (x != 0 || y != 0 || _w != w || _h != h) {
    _clip = new Fl_Rect(x, y, x + w, y + h);
  } else {
    _clip = NULL;
  }
}

//
// Graphics implementation
//
GraphicsWidget::GraphicsWidget(int xx, int yy, int ww, int hh) :
  Fl_Widget(xx, yy, ww, hh),
  _screen(NULL),
  _drawTarget(NULL),
  _font(NULL),
  _textOffset(0) {
  logEntered();
  _screen = new Canvas();
  _screen->create(ww, hh);
  graphics = this;
}

GraphicsWidget::~GraphicsWidget() {
  delete _screen;
}

void GraphicsWidget::deleteFont(Font *font) {
  if (font == _font) {
    _font = NULL;
  }
  delete font;
}

void GraphicsWidget::draw() {
  if (_screen && _screen->_offscreen) {
    if (_screen->_scale != Fl_Graphics_Driver::default_driver().scale()) {
      fl_rescale_offscreen(_screen->_offscreen);
      _screen->_scale = Fl_Graphics_Driver::default_driver().scale();
    }
    fl_copy_offscreen(x(), y(), w(), h(), _screen->_offscreen, 0, 0);
  }
}

void GraphicsWidget::drawText(int left, int top, const char *str, int length) {
  if (_drawTarget) {
    _drawTarget->drawText(_font, left, top + _textOffset, str, length);
  }
}

MAExtent GraphicsWidget::getTextSize(const char *str) {
  if (_font) {
    _font->setCurrent();
  }
  int height = fl_height();
  int width = fl_width(str);
  _textOffset = height - fl_descent();
  return (MAExtent)((width << 16) + height);
}

void GraphicsWidget::resize(int x, int y, int w, int h) {
  Fl_Widget::resize(x, y, w, h);
  layout();
}

void GraphicsWidget::layout() {
  if (_screen->_w != w() || _screen->_h != h()) {
    logEntered();
    bool drawScreen = (_drawTarget == _screen);
    delete _screen;
    _screen = new Canvas();
    _screen->create(w(), h());
    _drawTarget = drawScreen ? _screen : NULL;
  }
}

MAHandle GraphicsWidget::setDrawTarget(MAHandle maHandle) {
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

//
// maapi implementation
//
MAHandle maCreatePlaceholder(void) {
  return (MAHandle) new Canvas();
}

int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    graphics->deleteFont((Font *)maHandle);
  }
  return RES_FONT_OK;
}

int maSetColor(int c) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    // Fl_Color => 0xrrggbbii
    canvas->setColor((c << 8) & 0xffffff00);
  }
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->setClip(left, top, width, height);
  }
}

void maPlot(int posX, int posY) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->drawPixel(posX, posY);
  }
}

void maLine(int startX, int startY, int endX, int endY) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->drawLine(startX, startY, endX, endY);
  }
}

void maFillRect(int left, int top, int width, int height) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->fillRect(left, top, width, height, canvas->_drawColor);
  }
}

void maArc(int xc, int yc, double r, double start, double end, double aspect) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->drawArc(xc, yc, r, start, end, aspect);
  }
}

void maEllipse(int xc, int yc, int rx, int ry, int fill) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->drawEllipse(xc, yc, rx, ry, fill);
  }
}

void maDrawText(int left, int top, const char *str, int length) {
  if (str && str[0]) {
    graphics->drawText(left, top, str, length);
  }
}

void maDrawRGB(const MAPoint2d *dstPoint, const void *src, const MARect *srcRect, int opacity, int bytesPerLine) {
  Canvas *canvas = graphics->getDrawTarget();
  if (canvas) {
    canvas->drawRGB(dstPoint, src, srcRect, opacity, bytesPerLine);
  }
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (str && str[0]) {
    result = graphics->getTextSize(str);
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
  Fl_Font font = FL_COURIER;
  if (style & FONT_STYLE_BOLD) {
    font += FL_BOLD;
  }
  if (style & FONT_STYLE_ITALIC) {
    font += FL_ITALIC;
  }
  return (MAHandle)new Font(font, size);
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  graphics->setFont((Font *)maHandle);
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect, const MAPoint2d *dstPoint, int transformMode) {
  Canvas *canvas = graphics->getDrawTarget();
  Canvas *src = (Canvas *)maHandle;
  if (canvas && canvas != src) {
    canvas->drawRegion(src, srcRect, dstPoint->x, dstPoint->y);
  }
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Canvas *canvas = (Canvas *)maHandle;
  delete canvas;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int bytesPerLine) {
  Canvas *canvas = (Canvas *)maHandle;
  if (canvas == HANDLE_SCREEN) {
    canvas = graphics->getScreen();
  }
  canvas->getImageData((uint8_t *)dst, srcRect, bytesPerLine);
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

void maUpdateScreen(void) {
  ((::GraphicsWidget *)graphics)->redraw();
}

int maShowVirtualKeyboard(void) {
  return 0;
}
