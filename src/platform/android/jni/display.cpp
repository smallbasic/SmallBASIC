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
#include "ui/utils.h"
#include "common/device.h"

#define FONT_FACE_REGULAR_0 "Inconsolata-Regular.ttf"
#define FONT_FACE_BOLD_0    "Inconsolata-Bold.ttf"
#define FONT_FACE_REGULAR_1 "Envy Code R.ttf"
#define FONT_FACE_BOLD_1    "Envy Code R Bold.ttf"

extern ui::Graphics *graphics;

//
// Canvas implementation
//
Canvas::Canvas() :
  _w(0),
  _h(0),
  _pixels(NULL),
  _clip(NULL) {
}

Canvas::~Canvas() {
  delete [] _pixels;
  delete _clip;
  _pixels = NULL;
  _clip = NULL;
}

bool Canvas::create(int w, int h) {
  logEntered();
  bool result;
  _w = w;
  _h = h;
  _pixels = new pixel_t[w * h];
  if (_pixels) {
    memset(_pixels, 0, w * h);
    result = true;
  } else {
    result = false;
  }
  return result;
}

void Canvas::drawRegion(Canvas *src, const MARect *srcRect, int destX, int destY) {
  int srcH = srcRect->height;
  if (srcRect->top + srcRect->height > src->_h) {
    srcH = src->_h - srcRect->top;
  }
  for (int y = 0; y < srcH && destY < _h; y++, destY++) {
    pixel_t *line = src->getLine(y + srcRect->top) + srcRect->left;
    pixel_t *dstLine = getLine(destY) + destX;
    memcpy(dstLine, line, srcRect->width * sizeof(pixel_t));
  }
}

void Canvas::fillRect(int left, int top, int width, int height, pixel_t drawColor) {
  int dtX = x();
  int dtY = y();
  uint8_t dR, dG, dB;

  GET_RGB(drawColor, dR, dG, dB);
  if (left == 0 && _w == width && top < _h && top > -1 &&
      dR == dG && dR == dB) {
    // contiguous block of uniform colour
    unsigned blockH = height;
    if (top + height > _h) {
      blockH = height - top;
    }
    memset(getLine(top), drawColor, 4 * width * blockH);
  } else {
    for (int y = 0; y < height; y++) {
      int posY = y + top;
      if (posY == _h) {
        break;
      } else if (posY >= dtY) {
        pixel_t *line = getLine(posY);
        for (int x = 0; x < width; x++) {
          int posX = x + left;
          if (posX == _w) {
            break;
          } else if (posX >= dtX) {
            line[posX] = drawColor;
          }
        }
      }
    }
  }
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
Graphics::Graphics(android_app *app) : ui::Graphics(),
  _fontBuffer(NULL),
  _fontBufferB(NULL),
  _app(app),
  _w(0),
  _h(0) {
}

Graphics::~Graphics() {
  logEntered();

  delete [] _fontBuffer;
  delete [] _fontBufferB;
  _fontBuffer = NULL;
  _fontBufferB = NULL;
}

bool Graphics::construct(int fontId) {
  logEntered();

  _w = ANativeWindow_getWidth(_app->window);
  _h = ANativeWindow_getHeight(_app->window);

  bool result = false;
  if (loadFonts(fontId)) {
    _screen = new Canvas();
    if (_screen && _screen->create(_w, _h)) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
      ANativeWindow_setBuffersGeometry(_app->window, 0, 0, WINDOW_FORMAT_RGBA_8888);
      result = true;
    } else {
      trace("Failed to create canvas");
    }
  } else {
    trace("Failed to load font resources");
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
      int width = MIN(_w, MIN(buffer.width, _screen->_w));
      int height = MIN(_h, MIN(buffer.height, _screen->_h));
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
  _screen->create(_w, _h);
  _drawTarget = NULL;
}

bool Graphics::loadFonts(int fontId) {
  const char *regularName;
  const char *boldName;
  switch (fontId) {
  case 1:
    regularName = FONT_FACE_REGULAR_1;
    boldName = FONT_FACE_BOLD_1;
    break;
  default:
    regularName = FONT_FACE_REGULAR_0;
    boldName = FONT_FACE_BOLD_0;
    break;
  }
  return (!FT_Init_FreeType(&_fontLibrary) &&
          loadFont(regularName, _fontFace, &_fontBuffer) &&
          loadFont(boldName, _fontFaceB, &_fontBufferB));
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

