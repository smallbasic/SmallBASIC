// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "platform/android/jni/display.h"
#include "platform/common/form_ui.h"
#include "platform/common/utils.h"

#define SIZE_LIMIT 4
#define FONT_FACE_NAME "Envy Code R.ttf"

Window *widget;

uint16_t make565(int red, int green, int blue) {
  return (uint16_t)(((red   << 8) & 0xf800) |
                    ((green << 2) & 0x03e0) |
                    ((blue  >> 3) & 0x001f));
}

//
// Drawable implementation
//
Drawable::Drawable() :
  _canvas(NULL),
  _clip(NULL) {
}

Drawable::~Drawable() {
  delete _canvas;
  delete _clip;
}

bool Drawable::create(int w, int h) {
  logEntered();
  bool result;
  _w = w;
  _h = h;
  _canvas = new uint16_t[w * h];
  if (_canvas) {
    memset(_canvas, 0, w * h);
    result = true;
  } else {
    result = false;
  }
  return result;
}

void Drawable::setClip(int x, int y, int w, int h) {
  delete _clip;
  if (x != 0 || y != 0 || _w != w || _h != h) {
    _clip = new ARect();
    _clip->left = x;
    _clip->top = y;
    _clip->right = w;
    _clip->bottom = h;
  } else {
    _clip = NULL;
  }
}

//
// Window implementation
//
Window::Window(android_app *app) :
  _fontBuffer(NULL),
  _screen(NULL),
  _drawTarget(NULL),
  _font(NULL),
  _app(app) {
  widget = this;
}

Window::~Window() {
  delete _screen;

  if (_fontBuffer) {
    FT_Done_Face(_fontFace);
    FT_Done_FreeType(_fontLibrary);
    delete [] _fontBuffer;
  }
  _fontBuffer = NULL;
}

void Window::beginDraw() {
}

bool Window::construct() {
  logEntered();
  bool result = false;
  if (loadFont()) {
    _screen = new Drawable();
    if (_screen && _screen->create(getWidth(), getHeight())) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
      result = true;
    }
  }
  return result;
}

Font *Window::createFont(int style, int size) {
  return new Font(style, size);
}

void Window::deleteFont(Font *font) {
  if (font == _font) {
    _font = NULL;
  }
  delete font;
}

void Window::drawImageRegion(Drawable *src, const MAPoint2d *dstPoint, const MARect *srcRect) {
  if (_drawTarget && _drawTarget != src) {
  }
}

void Window::drawLine(int startX, int startY, int endX, int endY) {
  if (_drawTarget) {
  }
}

void Window::drawPixel(int posX, int posY) {
  if (_drawTarget) {
  }
}

void Window::drawRectFilled(int left, int top, int width, int height) {
  if (_drawTarget) {
  }
}

void Window::drawText(int left, int top, const char *str, int len) {
  if (_drawTarget) {
    if (_font) {
      FT_Set_Pixel_Sizes(_fontFace, 0, _font->_size);
    }
    FT_Vector pen;
    FT_GlyphSlot slot = _fontFace->glyph;
    pen.x = left * 64;
    pen.y = (_drawTarget->_h - top) * 64;
    for (int i = 0; i < len; i++) {
      FT_Load_Char(_fontFace, str[i], FT_LOAD_RENDER);
      FT_Int p, q;
      FT_Int x = slot->bitmap_left;
      FT_Int y = _drawTarget->_h - slot->bitmap_top;
      FT_Int xMax = x + slot->bitmap.width;
      FT_Int yMax = y + slot->bitmap.rows;
      for (FT_Int i = x, p = 0; i < xMax; i++, p++) {
        for (FT_Int j = y, q = 0; j < yMax; j++, q++) {
          if (i >= 0 && j >= 0 && 
              i < _drawTarget->_w &&
              j < _drawTarget->_h) {
            uint16_t *line = _drawTarget->_canvas + j;
            line[i] |= slot->bitmap.buffer[q * slot->bitmap.width + p];
          }
        }
      }
      pen.x += slot->advance.x;
      pen.y += slot->advance.y;
    }
  }
}

void Window::endDraw()  {
}

int Window::getPixel(int x, int y) {
  if (_drawTarget) {
  }
}

MAExtent Window::getTextSize(const char *str, int len) {
  int width = 0;
  int height = 0;
  if (_font) {
    FT_Set_Pixel_Sizes(_fontFace, 0, _font->_size);
  }
  for (int i = 0; i < len; i++) {
    FT_Load_Char(_fontFace, str[i], FT_LOAD_RENDER);
    width += _fontFace->glyph->metrics.vertAdvance;
    if (_fontFace->glyph->metrics.height > height) {
      height = _fontFace->glyph->metrics.height;
    }
  }
  return (MAExtent)((width << 16) + height);
}

int Window::getHeight() {
  return ANativeWindow_getHeight(_app->window);
}

int Window::getWidth() {
  return ANativeWindow_getWidth(_app->window);
}

void Window::redraw() {
  if (_app->window != NULL) {
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(_app->window, &buffer, NULL) < 0) {
      trace("Unable to lock window buffer");
    } else {
      //memcpy(surface_buffer.bits, _buffer,  _bufferSize);
      // _screen;
      ANativeWindow_unlockAndPost(_app->window);
    }
  }
}

void Window::setClip(int x, int y, int w, int h) {
  if (_drawTarget) {
    _drawTarget->setClip(x, y, w, h);
  }
}

MAHandle Window::setDrawTarget(MAHandle maHandle) {
  if (maHandle == (MAHandle) HANDLE_SCREEN ||
      maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    _drawTarget = _screen;
  } else {
    _drawTarget = (Drawable *)maHandle;
  }
  delete _drawTarget->_clip;
  _drawTarget->_clip = NULL;
  return (MAHandle) _drawTarget;
}

bool Window::loadFont() {
  bool result = false;
  AAssetManager *assetManager = _app->activity->assetManager;
  AAsset *fontFile = AAssetManager_open(assetManager, FONT_FACE_NAME, AASSET_MODE_BUFFER);
  if (fontFile) {
    off_t len = AAsset_getLength(fontFile);
    _fontBuffer = new FT_Byte[len + 1];
    if (AAsset_read(fontFile, _fontBuffer, len) >= 0) {
      trace("loaded %s", FONT_FACE_NAME);
      if (!FT_Init_FreeType(&_fontLibrary) &&
          !FT_New_Memory_Face(_fontLibrary, _fontBuffer, len, 0, &_fontFace)) {
        trace("loaded freetype face");
        result = true;
      }
    }
    AAsset_close(fontFile);
  }
  return result;
}

//
// form_ui implementation
//
void form_ui::optionsBox(StringList *items) {
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    widget->deleteFont((Font *)maHandle);
  }
  return RES_FONT_OK;
}

int maSetColor(int c) {
  int r = (c >> 16) & 0xFF;
  int g = (c >> 8) & 0xFF;
  int b = (c) & 0xFF;
  widget->setColor(make565(r, g, b));
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  widget->setClip(left, top, width, height);
}

void maPlot(int posX, int posY) {
  widget->drawPixel(posX, posY);
}

void maLine(int startX, int startY, int endX, int endY) {
  widget->drawLine(startX, startY, endX, endY);
}

void maFillRect(int left, int top, int width, int height) {
  widget->drawRectFilled(left, top, width, height);
}

void maDrawText(int left, int top, const char *str) {
  if (str && str[0]) {
    widget->drawText(left, top, str, strlen(str));
  }
}

void maUpdateScreen(void) {
  widget->redraw();
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (str && str[0] && widget) {
    result = widget->getTextSize(str, strlen(str));
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = widget->getWidth();
  short height = widget->getHeight();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return (MAHandle) widget->createFont(style, size);
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  if (widget) {
    widget->setFont((Font *) maHandle);
  }
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  widget->drawImageRegion((Drawable *)maHandle, dstPoint, srcRect);
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  if (height > widget->getHeight() * SIZE_LIMIT) {
    result -= 1;
  } else {
    Drawable *drawable = (Drawable *)maHandle;
    drawable->create(width, height);
  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Drawable();
  return maHandle;
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Drawable *holder = (Drawable *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int scanlength) {
  Drawable *holder = (Drawable *)maHandle;
  // maGetImageData is only used for getPixel()
  *((int *)dst) = widget->getPixel(srcRect->left, srcRect->top);
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  return widget->setDrawTarget(maHandle);
}

int maGetMilliSecondCount(void) {
  struct timespec t;
  t.tv_sec = t.tv_nsec = 0;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (int)(((int64_t)t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000;
}

int maShowVirtualKeyboard(void) {
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
}

