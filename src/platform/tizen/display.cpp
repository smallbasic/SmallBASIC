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
#define MSG_ID_REDRAW 5000
#define MSG_ID_SHOW_KEYPAD 5001

FormViewable *widget;
Drawable *drawTarget;
FontHolder *activeFont;
bool mouseActive;
Color drawColor;
int drawColorRaw;

//
// Drawable
//
Drawable::Drawable() :
  _canvas(NULL),
  _clip(NULL) {
}

Drawable::~Drawable() {
  delete _canvas;
  delete _clip;
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
  return (_canvas && _canvas->Construct(rect) == E_SUCCESS &&
          _canvas->FillRectangle(drawColor, rect) == E_SUCCESS);
}

void Drawable::drawImageRegion(Drawable *dst, const MAPoint2d *dstPoint, const MARect *srcRect) {
  const Rectangle from = Rectangle(srcRect->left, srcRect->top, srcRect->width, srcRect->height);
  const Rectangle to = Rectangle(dstPoint->x, dstPoint->y, srcRect->width, srcRect->height);
  dst->beginDraw();
  if (dst->_canvas->Copy(to, *_canvas, from) != E_SUCCESS) {
    AppLog("Canvas copy error");
  }
  dst->endDraw();
}

void Drawable::drawLine(int startX, int startY, int endX, int endY) {
  beginDraw();
  if (_canvas->DrawLine(Point(startX, startY), Point(endX, endY)) != E_SUCCESS) {
    AppLog("drawLine error");
  }
  endDraw();
}

void Drawable::drawPixel(int posX, int posY) {
  beginDraw();
  _canvas->SetPixel(Point(posX, posY));
  endDraw();
}

void Drawable::drawRectFilled(int left, int top, int width, int height) {
  beginDraw();
  _canvas->FillRectangle(drawColor, Rectangle(left, top, width, height));
  endDraw();
}

void Drawable::drawText(int left, int top, const char *str) {
  AppLog("draw text %d %d %s", left, top, str);
  if (activeFont) {
    if (_canvas->SetFont(*activeFont->_font) != E_SUCCESS) {
      AppLog("Failed to set active font onto canvas");
    }
  }
  beginDraw();
  if (_canvas->DrawText(Point(left, top), Tizen::Base::String(str)) != E_SUCCESS) {
    AppLog("drawText error");
  }
  endDraw();
}

void Drawable::endDraw() {
  if (_clip) {
    _canvas->SetClipBounds(widget->GetBounds());
  }
}

int Drawable::getPixel(int x, int y) {
  int result = 0;
  Color color;
  if (_canvas->GetPixel(Point(x, y), color) == E_SUCCESS) {
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

//
// FontHolder
//
FontHolder::FontHolder(int style, int size) :
  _font(NULL),
  _style(style),
  _size(size) {
}

FontHolder::~FontHolder() {
  delete _font;
}

bool FontHolder::create() {
  _font = new Font();
  return (_font && _font->Construct(_style, _size) == E_SUCCESS);
}

//
// FormViewable
//
FormViewable::FormViewable() :
  Control(),
  _screen(NULL),
  _resized(false) {
}

FormViewable::~FormViewable() {
  logEntered();
  delete _screen;
}

result FormViewable::Construct(int w, int h) {
  logEntered();
  result r = Control::Construct();
  if (!IsFailed(r)) {
    SetBounds(0, 0, w, h);
    _screen = new Drawable();
    if (_screen && _screen->create(w, h)) {
      drawTarget = _screen;
      drawColorRaw = DEFAULT_BACKGROUND;
      drawColor = Color(maSetColor(drawColorRaw));
      widget = this;
    } else {
      r = E_OUT_OF_MEMORY;
    }
  }

  if (IsFailed(r)) {
    AppLog("FormViewable::Construct failed");
  }
  return r;
}

result FormViewable::OnDraw() {
  logEntered();
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

void FormViewable::OnUserEventReceivedN(RequestId requestId, IList* args) {
  switch (requestId) {
  case MSG_ID_SHOW_KEYPAD:
    break;
  case MSG_ID_REDRAW:
    GetParent()->RequestRedraw(true);
    break;
  }
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
  FontHolder *fontHolder = (FontHolder *) maHandle;
  if (fontHolder == activeFont) {
    activeFont = NULL;
  }
  delete fontHolder;
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
  widget->SendUserEvent(MSG_ID_REDRAW, NULL);
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (activeFont && activeFont->_font && str && str[0]) {
    Dimension dim;
    activeFont->_font->GetTextExtent(str, strlen(str), dim);
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
  FontHolder *fontHolder = new FontHolder(style, size);
  if (!fontHolder->create()) {
    delete fontHolder;
    fontHolder = (FontHolder *)-1;
  }
  return (MAHandle) fontHolder;
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  activeFont = (FontHolder *) maHandle;
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  Drawable *drawable = (Drawable *)maHandle;
  if (drawTarget != drawable) {
    drawable->drawImageRegion(drawTarget, dstPoint, srcRect);
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
  MAHandle maHandle = (MAHandle) new Drawable();
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
  if (maHandle == (MAHandle) HANDLE_SCREEN ||
      maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    drawTarget = widget->getScreen();
  } else {
    drawTarget = (Drawable *)maHandle;
  }
  delete drawTarget->_clip;
  drawTarget->_clip = NULL;
  return (MAHandle) drawTarget;
}

int maGetMilliSecondCount(void) {
  return clock();
}

int maShowVirtualKeyboard(void) {
  widget->SendUserEvent(MSG_ID_SHOW_KEYPAD, NULL);
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
  //fltk::alert("%s\n\n%s", title, message);
}

