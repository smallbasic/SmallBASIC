// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
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
#include "platform/common/graphics.h"

using namespace strlib;

struct Canvas : public common::Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void freeClip();
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y);
  int x() { return _clip ? _clip->left : 0; }
  int y() { return _clip ? _clip->top : 0; }
  int w() { return _clip ? _clip->right : _w; }
  int h() { return _clip ? _clip->bottom : _h; }
  int width()  { return _w; }
  int height() { return _h; }

  int _w;
  int _h;
  pixel_t *_canvas;
  ARect *_clip;
};

struct Graphics : common::Graphics {
  Graphics(android_app *app);
  virtual ~Graphics();

  bool construct();
  void redraw();
  void resize();

private:
  bool loadFonts();
  bool loadFont(const char *name, FT_Face &face, FT_Byte **buffer);

  FT_Byte *_fontBuffer;
  FT_Byte *_fontBufferB;
  android_app *_app;
};

#endif
