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
#include <android_native_app_glue.h>

#include "platform/common/maapi.h"
#include "platform/common/StringLib.h"

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace strlib;

struct Font {
};

struct Drawable {
  Drawable();
  virtual ~Drawable();

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

struct Window {
  Window(android_app *app);
  virtual ~Window();

  bool construct();
  Font *createFont(int style, int size);
  int getWidth();
  int getHeight();
  MAHandle setDrawTarget(MAHandle maHandle);
  void redraw();

private:
  bool loadFont();

  FT_Library _fontLibrary;
  FT_Face _fontFace;
  FT_Byte *_fontBuffer;
  Drawable *_screen;
  android_app *_app;
};

#endif
