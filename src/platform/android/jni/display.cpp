// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "platform/android/jni/display.h"
#include "platform/common/form_ui.h"
#include "platform/common/utils.h"
#include "common/device.h"

#define FONT_FACE_REGULAR "Envy Code R.ttf"
#define FONT_FACE_BOLD    "Envy Code R Bold.ttf"
#define SIZE_LIMIT 10

extern common::Graphics *graphics;

//
// Canvas implementation
//
Canvas::Canvas() :
  _canvas(NULL),
  _clip(NULL) {
}

Canvas::~Canvas() {
  delete _canvas;
  delete _clip;
  _canvas = NULL;
  _clip = NULL;
}

bool Canvas::create(int w, int h) {
  logEntered();
  bool result;
  _w = w;
  _h = h;
  _canvas = new pixel_t[w * h];
  if (_canvas) {
    memset(_canvas, 0, w * h);
    result = true;
  } else {
    result = false;
  }
  return result;
}

void Canvas::setClip(int x, int y, int w, int h) {
  delete _clip;
  if (x != 0 || y != 0 || _w != w || _h != h) {
    _clip = new ARect();
    _clip->left = x;
    _clip->top = y;
    _clip->right = x + w;
    _clip->bottom = y + h;
  } else {
    _clip = NULL;
  }
}

//
// Graphics implementation
//
Graphics::Graphics(android_app *app) : common::Graphics(),
  _fontBuffer(NULL),
  _fontBufferB(NULL),
  _app(app) {
}

Graphics::~Graphics() {
  logEntered();

  delete [] _fontBuffer;
  delete [] _fontBufferB;
  _fontBuffer = NULL;
  _fontBufferB = NULL;
}

bool Graphics::construct() {
  logEntered();

  _w = ANativeWindow_getWidth(_app->window);
  _h = ANativeWindow_getHeight(_app->window);

  bool result = false;
  if (loadFonts()) {
    _screen = new Canvas();
    if (_screen && _screen->create(getWidth(), getHeight())) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
      ANativeWindow_setBuffersGeometry(_app->window, 0, 0, WINDOW_FORMAT_RGB_565);
      result = true;
    }
  }
  return result;
}

void Graphics::redraw() {
  if (_app->window != NULL) {
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(_app->window, &buffer, NULL) < 0) {
      trace("Unable to lock window buffer");
    } else {
      void *pixels = buffer.bits;
      int width = min(_w, min(buffer.width, _screen->_w));
      int height = min(_h, min(buffer.height, _screen->_h));
      for (int y = 0; y < height; y++) {
        pixel_t *line = _screen->getLine(y);
        memcpy((pixel_t *)pixels, line, width * sizeof(pixel_t));
        pixels = (pixel_t*)pixels + buffer.stride;
      }
      ANativeWindow_unlockAndPost(_app->window);
    }
  }
}

void Graphics::resize() {
  delete _screen;
  _screen = new Canvas();
  _screen->create(getWidth(), getHeight());
  _drawTarget = NULL;
}

bool Graphics::loadFonts() {
  return (!FT_Init_FreeType(&_fontLibrary) &&
          loadFont(FONT_FACE_REGULAR, _fontFace, &_fontBuffer) &&
          loadFont(FONT_FACE_BOLD, _fontFaceB, &_fontBufferB));
}

bool Graphics::loadFont(const char *name, FT_Face &face, FT_Byte **buffer) {
  bool result = false;
  AAssetManager *assetManager = _app->activity->assetManager;
  AAsset *fontFile = AAssetManager_open(assetManager, name, AASSET_MODE_BUFFER);
  if (fontFile) {
    off_t len = AAsset_getLength(fontFile);
    *buffer = new FT_Byte[len + 1];
    if (AAsset_read(fontFile, *buffer, len) >= 0) {
      if (!FT_New_Memory_Face(_fontLibrary, *buffer, len, 0, &face)) {
        trace("loaded freetype face %s", name);
        result = true;
      }
    }
    AAsset_close(fontFile);
  }
  return result;
}

//
// maapi implementation
//
void maUpdateScreen(void) {
  ((Graphics *)graphics)->redraw();
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  if (height > graphics->getHeight() * SIZE_LIMIT) {
    result -= 1;
  } else {
    Canvas *drawable = (Canvas *)maHandle;
    result = drawable->create(width, height) ? RES_OK : -1;
  }
  return result;
}


