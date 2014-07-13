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
