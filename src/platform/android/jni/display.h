// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANDROID_DISPLAY
#define ANDROID_DISPLAY

#include <android/rect.h>
#include <android/native_window.h>

#include "platform/common/maapi.h"
#include "platform/common/StringLib.h"

#define MSG_ID_REDRAW 5001
#define MSG_ID_SHOW_KEYPAD 5002
#define MSG_ID_SHOW_MENU 5003
#define MSG_ID_SHOW_ALERT 5004

using namespace strlib;

struct Font {
};

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

  uint16_t *_canvas;
  ARect *_clip;
};

struct Viewable {
  Viewable(ANativeWindow *window);
  virtual ~Viewable();

  bool construct(String &resourcePath);
  Font *createFont(int style, int size);
  int getWidth();
  int getHeight();
  MAHandle setDrawTarget(MAHandle maHandle);
  void redraw();

private:
  String _fontPath;
  Drawable *_screen;
  ANativeWindow *_window;
};

#endif
