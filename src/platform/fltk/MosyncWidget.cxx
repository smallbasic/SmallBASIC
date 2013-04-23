// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/layout.h>
#include <fltk/group.h>
#include <fltk/Image.h>
#include <fltk/Monitor.h>

#include <time.h>
#include "MosyncWidget.h"
#include "platform/mosync/ansiwidget.h"

using namespace fltk;

MosyncWidget *widget;
MosyncObject *drawTarget;

MosyncObject::MosyncObject() : 
  _img(NULL), 
  _gsave(NULL) {
}

MosyncObject::~MosyncObject() {
  if (_img) {
    _img->destroy();
  }
  delete _gsave;
}

void MosyncObject::createImage(int w, int h) {
  _img = new Image(w, h);
}

void MosyncObject::draw(int w, int h) {
  push_clip(Rectangle(w, h));
  _img->draw(Rectangle(_img->w(), _img->h()));
  pop_clip();
}

void MosyncObject::resize(int w, int h) {
  int W = _img->w();
  int H = _img->h();
  if (w > W) {
    W = w;
  }
  if (h > H) {
    H = h;
  }
  Image *old = _img;
  _img = new Image(W, H);

  GSave gsave;
  _img->make_current();
  setcolor(widget->getBackgroundColor());
  fillrect(0, 0, W, H);
  old->draw(Rectangle(old->w(), old->h()));
  old->destroy();
  delete old;
}

MosyncWidget::MosyncWidget(int x, int y, int w, int h, int defsize) :
  Widget(x, y, w, h, 0), _resized(false) {
  _ansiWidget = new AnsiWidget(this, w, h);
  drawTarget = _screen = new MosyncObject();
  widget = this;
  logEntered();
}

MosyncWidget::~MosyncWidget() {
  delete _ansiWidget;
}

bool MosyncWidget::construct() {
  return _ansiWidget->construct();
}

void MosyncWidget::layout() {
  if (drawTarget->_img && (layout_damage() & LAYOUT_WH)) {
    // can't use GSave here in X
    _resized = true;
  }
  Widget::layout();
}

void MosyncWidget::draw() {
  // ensure this widget has lowest z-order
  int siblings = parent()->children();
  for (int n = 0; n < siblings; n++) {
    Widget *w = parent()->child(n);
    if (w != this) {
      w->redraw();
    }
  }
  if (_screen->_img) {
    if (_resized) {
      _screen->resize(w(), h());
      _resized = false;
    }
    _screen->draw(w(), h());
  } else {
    setcolor(getBackgroundColor());
    fillrect(Rectangle(w(), h()));
  }
}

int MosyncWidget::handle(int e) {
  MAEvent event;

  switch (e) {
  case PUSH:
    fltk::get_mouse(event.point.x, event.point.y);
    _ansiWidget->pointerTouchEvent(event);
    break;

  case DRAG:
  case MOVE:
    fltk::get_mouse(event.point.x, event.point.y);
    _ansiWidget->pointerMoveEvent(event);
    break;

  case RELEASE:
    fltk::get_mouse(event.point.x, event.point.y);
    _ansiWidget->pointerReleaseEvent(event);
    break;
  }

  return Widget::handle(e);
}

void MosyncWidget::buttonClicked(const char *action) {
  
}

void MosyncWidget::clearScreen() {
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

void MosyncWidget::setFontSize(float i) {
  _ansiWidget->setFontSize(i);
}

int MosyncWidget::getFontSize() {
  return _ansiWidget->getFontSize();
}

void MosyncWidget::resize(int x, int y, int w, int h) {
  _ansiWidget->resize(w, h);
  Widget::resize(x, y, w, h);
}

int MosyncWidget::getBackgroundColor() {
  return _ansiWidget->getBackgroundColor();
}

int get_text_width(char *s) {
  return fltk::getwidth(s);
}

int maFontDelete(MAHandle font) {
  return RES_FONT_OK;
}

int maSetColor(int rgb) {
  fltk::setcolor(rgb);
  return rgb;
}

void maSetClipRect(int left, int top, int width, int height) {
}

void maPlot(int posX, int posY) {
  fltk::drawpoint(posX, posY);
}

void maLine(int startX, int startY, int endX, int endY) {
  fltk::drawline(startY, startY, endX, endY);
}

void maFillRect(int left, int top, int width, int height) {
  fltk::fillrect(left, top, width, height);
}

void maDrawText(int left, int top, const char* str) {
  fltk::drawtext(str, left, top);
}

void maUpdateScreen(void) {
  fltk::flush();
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char* str) {
  short width = (int)getwidth(str);
  short height = (int)(getascent() + getdescent());
  return (MAExtent)((width << 16) && height);
}

MAExtent maGetScrSize(void) {
  short width = widget->w();
  short height = widget->h();
  return (MAExtent)((width << 16) && height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return 0;
}

MAHandle maFontSetCurrent(MAHandle font) {
  // affects maGetTextSize() result
  
  return 0;
}

void maDrawImageRegion(MAHandle image, const MARect* srcRect, const MAPoint2d* dstPoint, int transformMode) {
}

int maCreateDrawableImage(MAHandle placeholder, int width, int height) {
  int result = RES_OK;
  const fltk::Monitor &monitor = fltk::Monitor::all();

  if (height > monitor.h()) {
    result -= 1;
  } else {
    MosyncObject *holder = (MosyncObject *)placeholder;
    holder->_img = new Image(width, height);
    
    GSave gsave;
    holder->_img->make_current();
    setcolor(widget->getBackgroundColor());
    fillrect(0, 0, width, height);
  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  return (MAHandle) new MosyncObject();
}

void maDestroyPlaceholder(MAHandle handle) {
  MosyncObject *holder = (MosyncObject *)handle;
  delete holder;
}

void maGetImageData(MAHandle image, void* dst, const MARect* srcRect, int scanlength) {
}

MAHandle maSetDrawTarget(MAHandle image) {
  if (image == (MAHandle) HANDLE_SCREEN) {
    drawTarget = widget->getScreen();
  } else {
    drawTarget = (MosyncObject *)handle;
  }

  //GSave gsave;
  //img->make_current();

  return (MAHandle) drawTarget;
}

int maGetMilliSecondCount(void) {
  return clock();
}

int maShowVirtualKeyboard(void) {
  return 0;
}

void maOptionsBox(const wchar* title, const wchar* destructiveButtonTitle,
                  const wchar* cancelButtonTitle, const void* otherButtonTitles,
                  int otherButtonTitlesSize) {
}

int maGetEvent(MAEvent* event) {
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

void maAlert(const char* title, const char* message, const char* button1,
             const char* button2, const char* button3) {
}
