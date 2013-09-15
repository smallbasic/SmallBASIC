// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef TIZEN_DISPLAY
#define TIZEN_DISPLAY

#include <FApp.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>

#include "platform/common/maapi.h"
#include "platform/tizen/runtime.h"

using namespace Tizen::Ui;
using namespace Tizen::Graphics;

struct CanvasWidget {
  CanvasWidget(int size);
  ~CanvasWidget();

  void beginDraw();
  void create(int w, int h);
  void drawImageRegion(CanvasWidget *dst, const MAPoint2d *dstPoint, const MARect *srcRect);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRectFilled(int left, int top, int width, int height);
  void drawText(int left, int top, const char *str);
  void endDraw();
  int  getPixel(int x, int y);
  void resize(int w, int h);
  void setClip(int x, int y, int w, int h);
  void setFont();

  //  fltk::Image *_img;
  //fltk::Rectangle *_clip;
  int _size;
  int _style;
  bool _isScreen;
};

struct DisplayWidget : public Control {
  DisplayWidget();
  virtual ~DisplayWidget();

  result Construct(void);
  CanvasWidget *getScreen() { return _screen; }
  int getDefaultSize() { return _defsize; }

private:
  result OnDraw();
  CanvasWidget *_screen;
  bool _resized;
  int _defsize;
};

#endif
