// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk/ask.h>
#include <fltk/Cursor.h>
#include <fltk/Font.h>
#include <fltk/Image.h>
#include <fltk/Monitor.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Group.h>
#include <fltk/layout.h>
#include <fltk/run.h>

#include <time.h>
#include "platform/fltk/MosyncWidget.h"
#include "platform/fltk/utils.h"
#include "platform/common/ansiwidget.h"
#include "platform/common/form_ui.h"

using namespace fltk;

#define SIZE_LIMIT 4

MosyncWidget *widget;
Canvas *drawTarget;
bool mouseActive;
Color drawColor;
int drawColorRaw;

//
// Canvas
//
Canvas::Canvas(int size) :
  _img(NULL),
  _clip(NULL),
  _size(size),
  _style(0),
  _isScreen(false) {
}

Canvas::~Canvas() {
  if (_img) {
    _img->destroy();
  }
  delete _img;
  delete _clip;
}

void Canvas::beginDraw() {
  _img->make_current();
  setcolor(drawColor);
  if (_clip) {
    push_clip(*_clip);
  }
}

void Canvas::create(int w, int h) {
  _img = new Image(w, h);

  GSave gsave;
  beginDraw();
  setcolor(drawColor);
  fillrect(0, 0, _img->w(), _img->h());
  endDraw();
}

void Canvas::drawImageRegion(Canvas *dst, const MAPoint2d *dstPoint, const MARect *srcRect) {
  fltk::Rectangle from = fltk::Rectangle(srcRect->left, srcRect->top, srcRect->width, srcRect->height);
  fltk::Rectangle to = fltk::Rectangle(dstPoint->x, dstPoint->y, srcRect->width, srcRect->height);
  GSave gsave;
  dst->beginDraw();
  _img->draw(from, to);
  dst->endDraw();
}

void Canvas::drawLine(int startX, int startY, int endX, int endY) {
  if (_isScreen) {
    fltk::setcolor(drawColor);
    fltk::drawline(startX, startY, endX, endY);
  } else {
    GSave gsave;
    beginDraw();
    fltk::drawline(startX, startY, endX, endY);
    endDraw();
  }
}

void Canvas::drawPixel(int posX, int posY) {
  if (posX > -1 && posY > -1
      && posX < _img->buffer_width()
      && posY < _img->buffer_height() - 1) {
    int delta = _img->buffer_linedelta();
    U32 *row = (U32 *) (_img->buffer() + (posY * delta));
    row[posX] = drawColorRaw;
  }
#if !defined(_Win32)
  GSave gsave;
  beginDraw();
  drawpoint(posX, posY);
  endDraw();
#endif
}

void Canvas::drawRectFilled(int left, int top, int width, int height) {
  if (_isScreen) {
    fltk::setcolor(drawColor);
    fltk::fillrect(left, top, width, height);
  } else {
#if defined(_Win32)
    int w = _img->buffer_width();
    int h = _img->buffer_height();
    for (int y = 0; y < height; y++) {
      int yPos = y + top;
      if (yPos > -1 && yPos < h) {
        U32 *row = (U32 *)_img->linebuffer(yPos);
        for (int x = 0; x < width; x++) {
          int xPos = x + left;
          if (xPos > -1 && xPos < w) {
            row[xPos] = drawColorRaw;
          }
        }
      }
    }
#else
    GSave gsave;
    beginDraw();
    fltk::fillrect(left, top, width, height);
    endDraw();
#endif
  }
}

void Canvas::drawText(int left, int top, const char *str) {
  setFont();
  if (_isScreen) {
    fltk::setcolor(drawColor);
    fltk::drawtext(str, left, top + (int)getascent());
  } else {
    GSave gsave;
    beginDraw();
    fltk::drawtext(str, left, top + (int)getascent());
    endDraw();
  }
}

void Canvas::endDraw() {
  if (_clip) {
    pop_clip();
  }
}

int Canvas::getPixel(int x, int y) {
  int result = 0;
  if (x > -1 && x < _img->w() &&
      y > -1 && y < _img->h()) {
    int delta = _img->buffer_linedelta();
    U32 *row = (U32 *) (_img->buffer() + (y * delta));
    result = row[x];
  }
  return result;
}

void Canvas::resize(int w, int h) {
  if (_img) {
    Image *old = _img;
    GSave gsave;
    _img = new Image(w, h);
    _img->make_current();
    setcolor(DEFAULT_BACKGROUND);
    fillrect(0, 0, w, h);
    old->draw(fltk::Rectangle(old->w(), old->h()));
    old->destroy();
    delete old;
  }
}

void Canvas::setClip(int x, int y, int w, int h) {
  delete _clip;
  _clip = new fltk::Rectangle(x, y, w, h);
}

void Canvas::setFont() {
  fltk::Font *font = fltk::COURIER;
  if (_size && _style) {
    if (_style & FONT_STYLE_BOLD) {
      font = font->bold();
    }
    if (_style & FONT_STYLE_ITALIC) {
      font = font->italic();
    }
  }
  setfont(font, _size);
}

//
// MosyncWidget
//
MosyncWidget::MosyncWidget(int x, int y, int w, int h, int defsize) :
  Widget(x, y, w, h, 0),
  _ansiWidget(NULL),
  _screen(NULL),
  _resized(false),
  _defsize(defsize) {
  drawColorRaw = DEFAULT_BACKGROUND;
  drawColor = maSetColor(drawColorRaw);
  widget = this;
}

MosyncWidget::~MosyncWidget() {
  delete _ansiWidget;
  delete _screen;
}

void MosyncWidget::createScreen() {
  if (!_screen) {
    _screen = new Canvas(_defsize);
    _screen->create(w(), h());
    drawTarget = _screen;
  }
  if (!_ansiWidget) {
    _ansiWidget = new AnsiWidget(this, w(), h());
    _ansiWidget->construct();
    _ansiWidget->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
    _ansiWidget->setFontSize(_defsize);
  }
}

void MosyncWidget::layout() {
  if (_screen != NULL && _ansiWidget != NULL &&
      (layout_damage() & LAYOUT_WH)) {
    // can't use GSave here in X
    _resized = true;
  }
  Widget::layout();
}

void MosyncWidget::draw() {
  if (_resized) {
    // resize the backing screens
    _screen->resize(w(), h());
    _ansiWidget->resize(w(), h());
    _resized = false;
  }

  if (_screen->_img) {
    int xScroll, yScroll;
    _ansiWidget->getScroll(xScroll, yScroll);
    fltk::Rectangle from = fltk::Rectangle(xScroll, yScroll, w(), h());
    fltk::Rectangle to = fltk::Rectangle(0, 0, w(), h());
    drawTarget->_img->draw(from, to);
    // draw the overlay onto the screen
    bool isScreen = drawTarget->_isScreen;
    drawTarget->_isScreen = true;
    _ansiWidget->drawOverlay(mouseActive);
    drawTarget->_isScreen = isScreen;
  } else {
    setcolor(drawColor);
    fillrect(fltk::Rectangle(w(), h()));
  }
}

void MosyncWidget::flush(bool force) {
  _ansiWidget->flush(force);
}

void MosyncWidget::reset() {
  createScreen();
  _ansiWidget->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _ansiWidget->setFontSize(_defsize);
  _ansiWidget->reset();
}

int MosyncWidget::handle(int e) {
  MAEvent event;

  switch (e) {
  case SHOW:
    createScreen();
    break;

  case FOCUS:
    return 1;

  case PUSH:
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    mouseActive = _ansiWidget->pointerTouchEvent(event);
    return mouseActive;

  case DRAG:
  case MOVE:
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    if (mouseActive && _ansiWidget->pointerMoveEvent(event)) {
      Widget::cursor(fltk::CURSOR_HAND);
      return 1;
    }
    break;

  case RELEASE:
    if (mouseActive) {
      mouseActive = false;
      Widget::cursor(fltk::CURSOR_DEFAULT);
      event.point.x = fltk::event_x();
      event.point.y = fltk::event_y();
      _ansiWidget->pointerReleaseEvent(event);
    }
    break;
  }

  return Widget::handle(e);
}

void MosyncWidget::buttonClicked(const char *action) {
}

void MosyncWidget::clearScreen() {
  createScreen();
  _ansiWidget->clearScreen();
}

void MosyncWidget::print(const char *str) {
  _ansiWidget->print(str);
}

void MosyncWidget::drawLine(int x1, int y1, int x2, int y2) {
  _ansiWidget->drawLine(x1, y1, x2, y2);
}

void MosyncWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  _ansiWidget->drawRectFilled(x1, y1, x2, y2);
}

void MosyncWidget::drawRect(int x1, int y1, int x2, int y2) {
  _ansiWidget->drawRect(x1, y1, x2, y2);
}

void MosyncWidget::drawImage(fltk::Image *img, int x, int y, int sx, int sy, int w, int h) {
  _ansiWidget->drawImage((MAHandle) img, x, y, sx, sy, w, h);
}

void MosyncWidget::saveImage(const char *fn, int x, int y, int w, int h) {
  // TODO
}

void MosyncWidget::setTextColor(long fg, long bg) {
  _ansiWidget->setTextColor(fg, bg);
}

void MosyncWidget::setColor(long color) {
  _ansiWidget->setColor(color);
}

int MosyncWidget::getX() {
  return _ansiWidget->getX();
}

int MosyncWidget::getY() {
  return _ansiWidget->getY();
}

void MosyncWidget::setPixel(int x, int y, int c) {
  _ansiWidget->setPixel(x, y, c);
}

int MosyncWidget::getPixel(int x, int y) {
  return _ansiWidget->getPixel(x, y);
}

void MosyncWidget::setXY(int x, int y) {
  _ansiWidget->setXY(x, y);
}

int MosyncWidget::textHeight(void) {
  return _ansiWidget->textHeight();
}

int MosyncWidget::textWidth(const char *str) {
  int result;
  if (drawTarget && str && str[0]) {
    drawTarget->setFont();
    result = (int)getwidth(str);
  } else {
    result = 0;
  }
  return result;
}

void MosyncWidget::setFontSize(float i) {
  _ansiWidget->setFontSize(i);
}

int MosyncWidget::getFontSize() {
  return _ansiWidget->getFontSize();
}

int MosyncWidget::getBackgroundColor() {
  return _ansiWidget->getBackgroundColor();
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
  drawColor = color(r,g,b);
  drawColorRaw = c;
  return drawColor;
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
  widget->redraw();
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (drawTarget && str && str[0]) {
    drawTarget->setFont();
    short width = (int)getwidth(str);
    short height = (int)(getascent() + getdescent());
    result = (MAExtent)((width << 16) + height);
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = widget->w();
  short height = widget->h();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  MAHandle result;
  if (drawTarget) {
    drawTarget->_style = style;
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
  Canvas *canvas = (Canvas *)maHandle;
  if (!drawTarget->_isScreen && drawTarget != canvas) {
    canvas->drawImageRegion(drawTarget, dstPoint, srcRect);
  }
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  const fltk::Monitor &monitor = fltk::Monitor::all();
  if (height > monitor.h() * SIZE_LIMIT) {
    result -= 1;
  } else {
    Canvas *canvas = (Canvas *)maHandle;
    canvas->create(width, height);
  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Canvas(widget->getDefaultSize());
  return maHandle;
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Canvas *holder = (Canvas *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int scanlength) {
  Canvas *holder = (Canvas *)maHandle;
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
    drawTarget = (Canvas *)maHandle;
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

int maGetEvent(MAEvent *event) {
  int result = 0;
  if (check()) {
    switch (fltk::event()) {
    case PUSH:
      event->type = EVENT_TYPE_POINTER_PRESSED;
      result = 1;
      break;
    case DRAG:
      event->type = EVENT_TYPE_POINTER_DRAGGED;
      result = 1;
      break;
    case RELEASE:
      event->type = EVENT_TYPE_POINTER_RELEASED;
      result = 1;
      break;
    }
  }
  return result;
}

void maWait(int timeout) {
  fltk::wait(timeout);
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
  fltk::alert("%s\n\n%s", title, message);
}

//
// Form UI
//
AnsiWidget *form_ui::getOutput() { 
  return widget->_ansiWidget;
}

struct Listener : IButtonListener {
  void buttonClicked(const char *action) {
    _action = action;
  }
  String _action;
};

void form_ui::optionsBox(StringList *items) {
  widget->_ansiWidget->print("\033[ S#6");
  int y = 0;
  Listener listener;
  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    int w = fltk::getwidth(str) + 20;
    IFormWidget *item = widget->_ansiWidget->createButton(str, 2, y, w, 22);
    item->setListener(&listener);
    y += 24;
  }
  while (form_ui::isRunning() && !listener._action.length()) {
    form_ui::processEvents();
  }
  if (!form_ui::isBreak()) {
    int index = 0;
    List_each(String *, it, *items) {
      char *str = (char *)(* it)->c_str();
      if (strcmp(str, listener._action.c_str()) == 0) {
        break;
      } else {
        index++;
      }
    }
    widget->_ansiWidget->print("\033[ SE6");
    widget->_ansiWidget->optionSelected(index);
    widget->redraw();
  }
}

