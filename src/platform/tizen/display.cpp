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
#include "ui/form_ui.h"
#include "ui/utils.h"

#define SIZE_LIMIT 4
#define FONT_FACE_NAME "Envy Code R.ttf"

FormViewable *widget;
Drawable *drawTarget;
Font *activeFont;
bool mouseActive;
Color drawColor;
int drawColorRaw;
long long epoch;

using namespace Tizen::App;
using namespace Tizen::Ui::Controls;

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
  if (activeFont) {
    if (_canvas->SetFont(*activeFont) != E_SUCCESS) {
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
  Rectangle rect = _canvas->GetBounds();
  if (rect.x != x || rect.y != y || rect.width != w || rect.height != h) {
    _clip = new Rectangle(x, y, w, h);
  } else {
    _clip = NULL;
  }
}

//
// FormViewable
//
FormViewable::FormViewable() :
  Control(),
  _canvasLock(NULL),
  _screen(NULL),
  _w(0),
  _h(0) {
  Tizen::System::SystemTime::GetTicks(epoch);
}

FormViewable::~FormViewable() {
  logEntered();
  delete _screen;
  delete _canvasLock;
  _screen = NULL;
  _canvasLock = NULL;
}

result FormViewable::Construct(String &appRootPath, int w, int h) {
  logEntered();
  result r = Control::Construct();
  if (!IsFailed(r)) {
    _canvasLock = new Mutex();
    r = _canvasLock != NULL ? _canvasLock->Create() : E_OUT_OF_MEMORY;
  }
  if (!IsFailed(r)) {
    SetBounds(0, 0, w, h);
    _w = w; _h = h;
    _screen = new Drawable();
    if (_screen && _screen->create(w, h)) {
      drawTarget = _screen;
      drawColorRaw = DEFAULT_BACKGROUND;
      drawColor = Color(maSetColor(drawColorRaw));
      widget = this;
      _fontPath = appRootPath.c_str();
      _fontPath += "res/";
      _fontPath += FONT_FACE_NAME;
    } else {
      r = E_OUT_OF_MEMORY;
    }
  }
  if (IsFailed(r)) {
    AppLog("FormViewable::Construct failed");
  }
  return r;
}

Font *FormViewable::createFont(int style, int size) {
  logEntered();
  Font *font = new Font();
  if (!font || font->Construct(_fontPath, style, size) != E_SUCCESS) {
    delete font;
    font = (Font *)-1;
  }
  return font;
}

result FormViewable::OnDraw() {
  Canvas *canvas = GetCanvasN();
  if (canvas) {
    canvas->Copy(Point(0, 0), *_screen->_canvas, GetBounds());
  }
  delete canvas;
  return E_SUCCESS;
}

void FormViewable::redraw() {
  _canvasLock->Acquire();
  OnDraw();
  _canvasLock->Release();
}

MAHandle FormViewable::setDrawTarget(MAHandle maHandle) {
  if (maHandle == (MAHandle) HANDLE_SCREEN ||
      maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    _canvasLock->Acquire();
    drawTarget = _screen;
  } else {
    drawTarget = (Drawable *)maHandle;
    if (drawTarget == _screen) {
      // returning the screen
      _canvasLock->Release();
    }
  }
  delete drawTarget->_clip;
  drawTarget->_clip = NULL;
  return (MAHandle) drawTarget;
}

//
// form_ui implementation
//
void form_ui::optionsBox(StringList *items) {
  ArrayList *args = new ArrayList();
  args->Construct();
  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    args->Add(new Tizen::Base::String(str));
  }
  Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_MENU, args);
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    Font *font = (Font *) maHandle;
    if (font == activeFont) {
      activeFont = NULL;
    }
    AppLog("Delete font %x", font);
    delete font;
  }
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
  Application::GetInstance()->SendUserEvent(MSG_ID_REDRAW, NULL);
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (activeFont && str && str[0]) {
    Dimension dim;
    activeFont->GetTextExtent(str, strlen(str), dim);
    result = (MAExtent)((dim.width << 16) + dim.height);
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = widget->getWidth();
  short height = widget->getHeight();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return (MAHandle) widget->createFont(style, size);
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  activeFont = (Font *) maHandle;
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
  return widget->setDrawTarget(maHandle);
}

int maGetMilliSecondCount(void) {
  long long result, ticks = 0;
  Tizen::System::SystemTime::GetTicks(ticks);
  result = ticks - epoch;
  return result;
}

int maShowVirtualKeyboard(void) {
  Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_KEYPAD, NULL);
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
  ArrayList *args = new ArrayList();
  args->Construct();
  args->Add(new Tizen::Base::String(title));
  args->Add(new Tizen::Base::String(message));
  Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_ALERT, args);
}

