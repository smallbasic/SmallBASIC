// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "platform/tizen/display.h"
#include "platform/common/utils.h"

#define SIZE_LIMIT 4
#define TIZEN_FONT_STYLE_BOLD 0x0002
#define TIZEN_FONT_STYLE_ITALIC 0x0004

FormViewable *widget;
Drawable *drawTarget;
bool mouseActive;
Color drawColor;
int drawColorRaw;

//
// Drawable
//
Drawable::Drawable(int size) :
  _canvas(NULL),
  _clip(NULL),
  _font(NULL),
  _size(size),
  _style(0),
  _isScreen(false) {
}

Drawable::~Drawable() {
  delete _canvas;
  delete _clip;
  delete _font;
}

void Drawable::beginDraw() {
  _canvas->SetForegroundColor(drawColor);
  if (_clip) {
    _canvas->SetClipBounds(*_clip);
  }
}

bool Drawable::create(int w, int h) {
  Rectangle rect = Rectangle(0, 0, w, h);
  _canvas = new Canvas();
  return (_canvas && E_SUCCESS == _canvas->Construct(rect)
          && (E_SUCCESS == _canvas->FillRectangle(drawColor, rect)));
}

void Drawable::drawImageRegion(Drawable *dst, const MAPoint2d *dstPoint, const MARect *srcRect) {
  const Rectangle from = Rectangle(srcRect->left, srcRect->top, srcRect->width, srcRect->height);
  const Rectangle to = Rectangle(dstPoint->x, dstPoint->y, srcRect->width, srcRect->height);
  dst->beginDraw();
  if (E_SUCCESS != dst->_canvas->Copy(to, *_canvas, from)) {
    AppLog("Canvas copy error");
  }
  dst->endDraw();
}

void Drawable::drawLine(int startX, int startY, int endX, int endY) {
  if (_isScreen) {
    // TODO
  } else {
    beginDraw();
    if (E_SUCCESS != _canvas->DrawLine(Point(startX, startY), Point(endX, endY))) {
      AppLog("drawLine error");
    }
    endDraw();
  }
}

void Drawable::drawPixel(int posX, int posY) {
  beginDraw();
  _canvas->SetPixel(Point(posX, posY));
  endDraw();
}

void Drawable::drawRectFilled(int left, int top, int width, int height) {
  if (_isScreen) {
    // TODO
  } else {
    beginDraw();
    _canvas->FillRectangle(drawColor, Rectangle(left, top, width, height));
    endDraw();
  }
}

void Drawable::drawText(int left, int top, const char *str) {
  setFont();
  if (_isScreen) {
    // TODO
  } else {
    beginDraw();
    if (E_SUCCESS != _canvas->DrawText(Point(left, top), Tizen::Base::String(str))) {
      AppLog("drawText error");
    }
    endDraw();
  }
}

void Drawable::endDraw() {
  if (_clip) {
    _canvas->SetClipBounds(widget->GetBounds());
  }
}

int Drawable::getPixel(int x, int y) {
  int result = 0;
  Color color;
  if (E_SUCCESS == _canvas->GetPixel(Point(x, y), color)) {
    result = color.GetRGB32();
  } else {
    AppLog("getPixel error");
  }
  return result;
}

void Drawable::resize(int w, int h) {
  if (_canvas) {
    Canvas *old = _canvas;
    create(w, h);
    _canvas->Copy(_canvas->GetBounds(), *old, old->GetBounds());
    delete old;
  }
}

void Drawable::setClip(int x, int y, int w, int h) {
  delete _clip;
  _clip = new Rectangle(x, y, w, h);
}

Font *Drawable::setFont() {
  delete _font;
  _font = new Font();
  _font->Construct(_style, _size);
  _canvas->SetFont(*_font);
  return _font;
}

//
// DisplayWidget
//
FormViewable::FormViewable() :
  Control(),
  _screen(NULL),
  _resized(false),
  _defsize(10) {
}

result FormViewable::Construct(void) {
  result r = E_FAILURE;
  _screen = new Drawable(_defsize);
  if (_screen && _screen->create(GetWidth(), GetHeight())) {
    drawTarget = _screen;
    drawColorRaw = DEFAULT_BACKGROUND;
    drawColor = Color(maSetColor(drawColorRaw));
    widget = this;
    r = Construct();
  }
  return r;
}

result FormViewable::OnDraw() {
  Canvas *canvas = GetCanvasN();
  if (canvas) {
    Rectangle rect = GetBounds();
    if (_screen->_canvas) {
      canvas->Copy(Point(rect.x, rect.y), *_screen->_canvas, rect);
    } else {
      canvas->FillRectangle(drawColor, rect);
    }
    delete canvas;
  }
  return E_SUCCESS;
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
  return RES_FONT_OK;
}

int maSetColor(int c) {
  int r = (c >> 16) & 0xFF;
  int g = (c >> 8) & 0xFF;
  int b = (c) & 0xFF;
  drawColor = Color(r, g, b);
  drawColorRaw = c;
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  if (drawTarget) {
    drawTarget->setClip(left, top, width, height);
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
    drawTarget->drawRectFilled(left, top, width, height);
  }
}

void maDrawText(int left, int top, const char *str) {
  if (drawTarget && str && str[0]) {
    drawTarget->drawText(left, top, str);
  }
}

void maUpdateScreen(void) {
  widget->RequestRedraw(true);
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (drawTarget && str && str[0]) {
    Dimension dim;
    drawTarget->setFont()->GetTextExtent(str, strlen(str), dim);
    result = (MAExtent)((dim.width << 16) + dim.height);
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = widget->GetWidth();
  short height = widget->GetHeight();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  MAHandle result;
  if (drawTarget) {
    switch (style) {
    case FONT_STYLE_NORMAL:
      drawTarget->_style = Tizen::Graphics::FONT_STYLE_PLAIN;
      break;
    case FONT_STYLE_BOLD:
      drawTarget->_style = TIZEN_FONT_STYLE_BOLD;
      break;
    case FONT_STYLE_ITALIC:
      drawTarget->_style = TIZEN_FONT_STYLE_ITALIC;
      break;
    }
    result = (MAHandle)drawTarget;
  } else {
    result = (MAHandle)NULL;
  }
  return result;
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  Drawable *canvas = (Drawable *)maHandle;
  if (!drawTarget->_isScreen && drawTarget != canvas) {
    canvas->drawImageRegion(drawTarget, dstPoint, srcRect);
  }
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  if (height > widget->GetHeight() * SIZE_LIMIT) {
    result -= 1;
  } else {
    Drawable *canvas = (Drawable *)maHandle;
    canvas->create(width, height);
  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Drawable(widget->getDefaultSize());
  return maHandle;
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Drawable *holder = (Drawable *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int scanlength) {
  Drawable *holder = (Drawable *)maHandle;
  // maGetImageData is only used for getPixel()
  *((int *)dst) = holder->getPixel(srcRect->left, srcRect->top);
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  if (maHandle == (MAHandle) HANDLE_SCREEN) {
    drawTarget->_isScreen = true;
  } else if (maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    drawTarget = widget->getScreen();
    drawTarget->_isScreen = false;
  } else {
    drawTarget = (Drawable *)maHandle;
    drawTarget->_isScreen = false;
  }
  delete drawTarget->_clip;
  drawTarget->_clip = NULL;
  return (MAHandle) drawTarget;
}

int maGetMilliSecondCount(void) {
  return clock();
}

int maShowVirtualKeyboard(void) {
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
  //fltk::alert("%s\n\n%s", title, message);
}

