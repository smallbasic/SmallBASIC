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

#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/graphics.h"

using namespace strlib;

struct Graphics : ui::Graphics {
  explicit Graphics(android_app *app);
  ~Graphics() override;

  bool construct(int fontId);
  void redraw();
  bool resize();
  void onPaused(bool paused) { _paused=paused; }
  void setSize(int w, int h) { _w = w; _h = h; }

private:
  bool loadFonts(int fontId);
  bool loadFont(const char *name, FT_Face &face, FT_Byte **buffer);

  FT_Byte *_fontBuffer;
  FT_Byte *_fontBufferB;
  android_app *_app;
  int _w, _h;
  bool _paused;
};

#endif
