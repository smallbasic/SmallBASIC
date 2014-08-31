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
#include <FUiContainer.h>

#include "ui/maapi.h"
#include "ui/StringLib.h"

#define MSG_ID_REDRAW 5001
#define MSG_ID_SHOW_KEYPAD 5002
#define MSG_ID_SHOW_MENU 5003
#define MSG_ID_SHOW_ALERT 5004

using namespace Tizen::Ui;
using namespace Tizen::Graphics;
using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace strlib;

struct Drawable {
  Drawable();
  ~Drawable();

  void beginDraw();
  bool create(int w, int h);
  void drawImageRegion(Drawable *dst, const MAPoint2d *dstPoint, const MARect *srcRect);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRectFilled(int left, int top, int width, int height);
  void drawText(int left, int top, const char *str);
  void endDraw();
  int  getPixel(int x, int y);
  void resize(int w, int h);
  void setClip(int x, int y, int w, int h);

  Canvas *_canvas;
  Rectangle *_clip;
};

struct FormViewable : public Control {
  FormViewable();
  virtual ~FormViewable();

  result Construct(String &resourcePath, int w, int h);
  Font *createFont(int style, int size);
  int getWidth() { return _w; }
  int getHeight() { return _h; }
  MAHandle setDrawTarget(MAHandle maHandle);
  result OnDraw();
  void redraw();
  void resize(int w, int h) { _w = w; _h = h; }

private:
  Tizen::Base::String _fontPath;
  Mutex *_canvasLock;
  Drawable *_screen;
  int _w, _h;
};

#endif
