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

#include "platform/common/maapi.h"

using namespace Tizen::Ui;
using namespace Tizen::Graphics;
using namespace Tizen::Base::Collection;

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

struct FontHolder {
  FontHolder(int style, int size);
  ~FontHolder();
  bool create();

  Font *_font;
  int _style;
  int _size;
};

struct FormViewable : public Control {
  FormViewable();
  virtual ~FormViewable();

  result Construct(int w, int h);
  Drawable *getScreen() { return _screen; }
  int getDefaultSize() { return _defsize; }
  result OnDraw();

private:
  void OnUserEventReceivedN(RequestId requestId, IList* pArgs);
  Drawable *_screen;
  bool _resized;
  int _defsize;
};

#endif
