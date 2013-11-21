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

#define FONT_FACE_REGULAR "Envy Code R.ttf"
#define FONT_FACE_BOLD    "Envy Code R Bold.ttf"
#define FONT_FACE_ITALIC  "Envy Code R Italic.ttf"

Graphics *graphics;

pixel_t RGB888_to_RGB565(unsigned rgb) {
  return ((((rgb >> 19) & 0x1f) << 11) |
          (((rgb >> 10) & 0x3f) <<  5) |
          (((rgb >>  3) & 0x1f)));
}

void RGB565_to_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (c >> 11) & 0x1f;
  g = (c >> 5) & 0x3f;
  b = (c) & 0x1f;
}

Font::Font(int size, FT_Face face) :
  _chW(0),
  _chH(0),
  _face(face) {
  FT_Set_Pixel_Sizes(face, 0, size);
  for (int i = 0; i < MAX_GLYPHS; i++) {
    FT_UInt slot = FT_Get_Char_Index(face, i);
    FT_Load_Glyph(face, slot, FT_LOAD_TARGET_LIGHT);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LIGHT);
    FT_Get_Glyph(face->glyph, &_glyph[i]);
    int w = (int)(face->glyph->metrics.horiAdvance / 64);
    int h = face->glyph->bitmap.rows + (face->glyph->metrics.horiBearingX / 64);
    _chW = max(_chW, w);
    _chH = max(_chH, h);
  }
}

Font::~Font() {
  for (int i = 0; i < MAX_GLYPHS; i++) {
    FT_Done_Glyph(_glyph[i]);
  }
}

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
    _clip->right = w;
    _clip->bottom = h;
  } else {
    _clip = NULL;
  }
}

//
// Graphics implementation
//
Graphics::Graphics(android_app *app) :
  _fontBuffer(NULL),
  _fontBufferB(NULL),
  _fontBufferI(NULL),
  _screen(NULL),
  _drawTarget(NULL),
  _font(NULL),
  _app(app) {
  graphics = this;
}

Graphics::~Graphics() {
  delete _screen;

  if (_fontBuffer) {
    FT_Done_Face(_fontFace);
    FT_Done_FreeType(_fontLibrary);
    delete [] _fontBuffer;
  }
  _fontBuffer = NULL;
}

bool Graphics::construct() {
  logEntered();
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

Font *Graphics::createFont(int style, int size) {
  Font *result;
  if (style & FONT_STYLE_BOLD) {
    result = new Font(size, _fontFaceB);
  } else if (style & FONT_STYLE_ITALIC) {
    result = new Font(size, _fontFaceI);
  } else {
    result = new Font(size, _fontFace);
  }
  return result;
}

void Graphics::deleteFont(Font *font) {
  if (font == _font) {
    _font = NULL;
  }
  delete font;
}

void Graphics::drawImageRegion(Canvas *src, const MAPoint2d *dstPoint, const MARect *srcRect) {
  logEntered();
  if (_drawTarget && _drawTarget != src) {
    int destY = dstPoint->y;
    for (int y = srcRect->top; y < srcRect->height; y++, destY++) {
      pixel_t *line = src->getLine(y) + srcRect->left;
      pixel_t *dstLine = _drawTarget->getLine(destY) + dstPoint->x;
      memcpy(dstLine, line, srcRect->width * sizeof(pixel_t));
    }
  }
}

void Graphics::drawLine(int startX, int startY, int endX, int endY) {
  if (_drawTarget) {
  }
}

void Graphics::drawPixel(int posX, int posY) {
  if (_drawTarget
      && posX >= _drawTarget->x() 
      && posY >= _drawTarget->y()
      && posX < _drawTarget->w()
      && posY < _drawTarget->h()) {
    pixel_t *line = _drawTarget->getLine(posY);
    line[posX] = _drawColor;
  }
}

void Graphics::drawRectFilled(int left, int top, int width, int height) {
  logEntered();
  if (_drawTarget) {
    int w = _drawTarget->w();
    int h = _drawTarget->h();
    for (int y = _drawTarget->y(); y < height; y++) {
      int posY = y + top;
      if (posY >= _drawTarget->y() && posY < h) {
        pixel_t *line = _drawTarget->getLine(posY);
        for (int x = _drawTarget->x(); x < width; x++) {
          int posX = x + left;
          if (posX > _drawTarget->x() && posX < w) {
            line[posX] = _drawColor;
          }
        }
      }
    }
  }
}

void Graphics::drawChar(FT_Bitmap *bitmap, FT_Int x, FT_Int y) {
  FT_Int p, q;
  FT_Int xMax = x + bitmap->width;
  FT_Int yMax = y + bitmap->rows;

  uint8_t sR, sG, sB;
  RGB565_to_RGB(_drawColor, sR, sG, sB);

  for (FT_Int i = x, p = 0; i < xMax; i++, p++) {
    for (FT_Int j = y, q = 0; j < yMax; j++, q++) {
      if (i >= _drawTarget->x() &&
          i <  _drawTarget->w() &&
          j >= _drawTarget->y() &&
          j <  _drawTarget->h()) {
        pixel_t *line = _drawTarget->getLine(j);
        uint8_t a = bitmap->buffer[q * bitmap->width + p];
        if (a == 255) {
          line[i] = _drawColor;
        } else {
          // blend drawColor to the background
          uint8_t dR, dG, dB;
          RGB565_to_RGB(line[i], dR, dG, dB);
          dR = dR + ((sR - dR) * a / 255);
          dG = dG + ((sG - dG) * a / 255);
          dB = dB + ((sB - dB) * a / 255);
          line[i] = ((dR << 11) | (dG << 5) | dB);
        }
      }
    }
  }
}

void Graphics::drawText(int left, int top, const char *str, int len) {
  if (_drawTarget && _font) {
    FT_Vector pen;
    pen.x = left - _font->_face->glyph->bitmap_left;
    pen.y = top + _font->_face->glyph->bitmap_top;
    for (int i = 0; i < len; i++) {
      FT_BitmapGlyph glyph = (FT_BitmapGlyph)_font->_glyph[str[i]];
      drawChar(&glyph->bitmap, 
               pen.x + glyph->left, 
               pen.y - glyph->top);
      pen.x += _font->_chW;
    }
  }
}

int Graphics::getPixel(int posX, int posY) {
  int result = 0;
  if (_drawTarget
      && posX > -1 
      && posY > -1
      && posX < _drawTarget->_w
      && posY < _drawTarget->_h - 1) {
    pixel_t *line = _drawTarget->getLine(posY);
    result = line[posX];
  }
  return result;
}

MAExtent Graphics::getTextSize(const char *str, int len) {
  int width = 0;
  int height = 0;
  if (_font) {
    width = len * _font->_chW;
    height = _font->_chH;
  }
  return (MAExtent)((width << 16) + height);
}

int Graphics::getHeight() {
  return ANativeWindow_getHeight(_app->window);
}

int Graphics::getWidth() {
  return ANativeWindow_getWidth(_app->window);
}

void Graphics::redraw() {
  if (_app->window != NULL) {
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(_app->window, &buffer, NULL) < 0) {
      trace("Unable to lock window buffer");
    } else {
      void *pixels = buffer.bits;
      int width = min(buffer.width, _screen->_w);
      int height = min(buffer.height, _screen->_h);
      for (int y = 0; y < height; y++) {
        pixel_t *line = _screen->getLine(y);
        memcpy((pixel_t *)pixels, line, width * sizeof(pixel_t));
        pixels = (pixel_t*)pixels + buffer.stride;
      }
      ANativeWindow_unlockAndPost(_app->window);
    }
  }
}

void Graphics::setClip(int x, int y, int w, int h) {
  if (_drawTarget) {
    _drawTarget->setClip(x, y, w, h);
  }
}

MAHandle Graphics::setDrawTarget(MAHandle maHandle) {
  if (maHandle == (MAHandle) HANDLE_SCREEN ||
      maHandle == (MAHandle) HANDLE_SCREEN_BUFFER) {
    _drawTarget = _screen;
  } else {
    _drawTarget = (Canvas *)maHandle;
  }
  delete _drawTarget->_clip;
  _drawTarget->_clip = NULL;
  return (MAHandle) _drawTarget;
}

bool Graphics::loadFonts() {
  return (!FT_Init_FreeType(&_fontLibrary) &&
          loadFont(FONT_FACE_REGULAR, _fontFace, &_fontBuffer) &&
          loadFont(FONT_FACE_BOLD, _fontFaceB, &_fontBufferB) &&
          loadFont(FONT_FACE_ITALIC, _fontFaceI, &_fontBufferI));
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
// form_ui implementation
//
void form_ui::optionsBox(StringList *items) {
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    graphics->deleteFont((Font *)maHandle);
  }
  return RES_FONT_OK;
}

int maSetColor(int c) {
  graphics->setColor(RGB888_to_RGB565(c));
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  graphics->setClip(left, top, width, height);
}

void maPlot(int posX, int posY) {
  graphics->drawPixel(posX, posY);
}

void maLine(int startX, int startY, int endX, int endY) {
  graphics->drawLine(startX, startY, endX, endY);
}

void maFillRect(int left, int top, int width, int height) {
  graphics->drawRectFilled(left, top, width, height);
}

void maDrawText(int left, int top, const char *str) {
  if (str && str[0]) {
    graphics->drawText(left, top, str, strlen(str));
  }
}

void maUpdateScreen(void) {
  graphics->redraw();
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (str && str[0] && graphics) {
    result = graphics->getTextSize(str, strlen(str));
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = graphics->getWidth();
  short height = graphics->getHeight();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return (MAHandle) graphics->createFont(style, size);
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  if (graphics) {
    graphics->setFont((Font *) maHandle);
  }
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  graphics->drawImageRegion((Canvas *)maHandle, dstPoint, srcRect);
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  if (height > graphics->getHeight() * SIZE_LIMIT) {
    result -= 1;
  } else {
    Canvas *drawable = (Canvas *)maHandle;
    drawable->create(width, height);
  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Canvas();
  return maHandle;
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Canvas *holder = (Canvas *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int scanlength) {
  Canvas *holder = (Canvas *)maHandle;
  // maGetImageData is only used for getPixel()
  *((int *)dst) = graphics->getPixel(srcRect->left, srcRect->top);
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  return graphics->setDrawTarget(maHandle);
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

