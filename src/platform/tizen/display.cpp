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
// CanvasWidget
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

bool Drawable::create(int w, int h) {
  Rectangle rect = Rectangle(0, 0, w, h);
  bool result = false;
  _canvas = new Canvas();
  if (_canvas && E_SUCCESS == _canvas->Construct(rect)) {
    _canvas->FillRectangle(drawColor, rect);
    result = true;
  }
  return result;
}

void Drawable::drawImageRegion(Drawable *dst, const MAPoint2d *dstPoint, const MARect *srcRect) {
//  fltk::Rectangle from = fltk::Rectangle(srcRect->left, srcRect->top, srcRect->width, srcRect->height);
//  fltk::Rectangle to = fltk::Rectangle(dstPoint->x, dstPoint->y, srcRect->width, srcRect->height);
//  GSave gsave;
//  dst->beginDraw();
//  _canvas->draw(from, to);
//  dst->endDraw();
}

void Drawable::drawLine(int startX, int startY, int endX, int endY) {
//  if (_isScreen) {
//    fltk::setcolor(drawColor);
//    fltk::drawline(startX, startY, endX, endY);
//  } else {
//    GSave gsave;
//    beginDraw();
//    fltk::drawline(startX, startY, endX, endY);
//    endDraw();
//  }
}

void Drawable::drawPixel(int posX, int posY) {
//  if (posX > -1 && posY > -1
//      && posX < _canvas->buffer_width()
//      && posY < _canvas->buffer_height()) {
//    U32 *row = (U32 *)_canvas->linebuffer(posY);
//    row[posX] = drawColorRaw;
//  }
//#if !defined(_Win32)
//  GSave gsave;
//  beginDraw();
//  drawpoint(posX, posY);
//  endDraw();
//#endif
}

void Drawable::drawRectFilled(int left, int top, int width, int height) {
//  if (_isScreen) {
//    fltk::setcolor(drawColor);
//    fltk::fillrect(left, top, width, height);
//  } else {
//#if defined(_Win32)
//    int w = _canvas->buffer_width();
//    int h = _canvas->buffer_height();
//    for (int y = 0; y < height; y++) {
//      int yPos = y + top;
//      if (yPos > -1 && yPos < h) {
//        U32 *row = (U32 *)_canvas->linebuffer(yPos);
//        for (int x = 0; x < width; x++) {
//          int xPos = x + left;
//          if (xPos > -1 && xPos < w) {
//            row[xPos] = drawColorRaw;
//          }
//        }
//      }
//    }
//#else
//    GSave gsave;
//    beginDraw();
//    fltk::fillrect(left, top, width, height);
//    endDraw();
//#endif
//  }
}

void Drawable::drawText(int left, int top, const char *str) {
//  setFont();
//  if (_isScreen) {
//    fltk::setcolor(drawColor);
//    fltk::drawtext(str, left, top + (int)getascent());
//  } else {
//    GSave gsave;
//    beginDraw();
//    fltk::drawtext(str, left, top + (int)getascent());
//    endDraw();
//  }
}

void Drawable::endDraw() {
//  if (_clip) {
//    pop_clip();
//  }
}

int Drawable::getPixel(int x, int y) {
  int result = 0;
//  if (x > -1 && x < _canvas->w() &&
//      y > -1 && y < _canvas->h()) {
//    U32 *pixel = (U32 *)_canvas->linebuffer(y);
//    result = pixel[x];
//  }
  return result;
}

void Drawable::resize(int w, int h) {
//  if (_canvas) {
//    Image *old = _canvas;
//    GSave gsave;
//    _canvas = new Image(w, h);
//    _canvas->make_current();
//    setcolor(DEFAULT_BACKGROUND);
//    fillrect(0, 0, w, h);
//    old->draw(fltk::Rectangle(old->w(), old->h()));
//    old->destroy();
//    delete old;
//  }
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
    //    canvas->Copy(Point(rect.x, rect.y), *_canvas, rect);

//  if (_resized) {
//    // resize the backing screens
//    _screen->resize(w(), h());
//    _ansiWidget->resize(w(), h());
//    _resized = false;
//  }
//
//  if (_screen->_canvas) {
//    int xScroll, yScroll;
//    _ansiWidget->getScroll(xScroll, yScroll);
//    fltk::Rectangle from = fltk::Rectangle(xScroll, yScroll, w(), h());
//    fltk::Rectangle to = fltk::Rectangle(0, 0, w(), h());
//    drawTarget->_canvas->draw(from, to);
//    // draw the overlay onto the screen
//    bool isScreen = drawTarget->_isScreen;
//    drawTarget->_isScreen = true;
//    _ansiWidget->drawOverlay(mouseActive);
//    drawTarget->_isScreen = isScreen;
//  } else {
//    setcolor(drawColor);
//    fillrect(fltk::Rectangle(w(), h()));
//  }

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
  //widget->redraw();
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
  //  const fltk::Monitor &monitor = fltk::Monitor::all();
  //if (height > monitor.h() * SIZE_LIMIT) {
  //  result -= 1;
  //} else {
  //  CanvasWidget *canvas = (CanvasWidget *)maHandle;
  //  canvas->create(width, height);
  //}
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
  //CanvasWidget *holder = (CanvasWidget *)maHandle;
  //U32 *dest = (U32 *)dst;
  //int index = 0;
  //  for (int y = 0; y < srcRect->height && y + srcRect->top < holder->_canvas->buffer_height(); y++) {
  //  for (int x = 0; x < srcRect->width && x + srcRect->left < holder->_canvas->buffer_width(); x++) {
  //    U32 *pixel = (U32 *)holder->_canvas->linebuffer(y + srcRect->top);
  //    dest[index++] = pixel[x + srcRect->left];
  //  }
  //}
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
  //delete drawTarget->_clip;
  //drawTarget->_clip = NULL;
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

