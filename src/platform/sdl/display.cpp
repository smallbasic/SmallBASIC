// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "ui/utils.h"
#include "platform/sdl/display.h"

extern common::Graphics *graphics;

#if defined(PIXELFORMAT_RGB565)
#define PIXEL_FORMAT SDL_PIXELFORMAT_RGB565
#else
#define PIXEL_FORMAT SDL_PIXELFORMAT_RGB888
#endif

//
// Canvas implementation
//
Canvas::Canvas() :
  _surface(NULL),
  _clip(NULL) {
}

Canvas::~Canvas() {
  if (_surface != NULL) {
    SDL_FreeSurface(_surface);
  }
  delete _clip;
  _surface = NULL;
  _clip = NULL;
}

bool Canvas::create(int w, int h) {
  logEntered();
  _w = w;
  _h = h;
  int bpp;
  Uint32 rmask, gmask, bmask, amask;
  SDL_PixelFormatEnumToMasks(PIXEL_FORMAT, &bpp, &rmask, &gmask, &bmask, &amask);
  _surface = SDL_CreateRGBSurface(0, w, h, bpp, rmask, gmask, bmask, amask);
  return _surface != NULL;
}

void Canvas::copy(Canvas *src, const MARect *srcRect, int destX, int destY) {
  SDL_Rect srcrect;
  srcrect.x = srcRect->left;
  srcrect.y = srcRect->top;
  srcrect.w = srcRect->width;
  srcrect.h = srcRect->height;

  SDL_Rect dstrect;
  dstrect.x = destX;
  dstrect.y = destY;
  dstrect.w = _w;
  dstrect.h = _h;

  SDL_BlitSurface(src->_surface, &srcrect, _surface, &dstrect);
}

pixel_t *Canvas::getLine(int y) {
  pixel_t *pixels = (pixel_t *)_surface->pixels;
  return pixels + (y * _w);
}

void Canvas::setClip(int x, int y, int w, int h) {
  delete _clip;
  if (x != 0 || y != 0 || _w != w || _h != h) {
    _clip = new SDL_Rect();
    _clip->x = x;
    _clip->y = y;
    _clip->w = x + w;
    _clip->h = y + h;
  } else {
    _clip = NULL;
  }
}

//
// Graphics implementation
//
Graphics::Graphics(SDL_Window *window) : common::Graphics(),
  _renderer(NULL),
  _texture(NULL),
  _window(window) {
}

Graphics::~Graphics() {
  logEntered();
  SDL_DestroyTexture(_texture);
  SDL_DestroyRenderer(_renderer);
}

bool Graphics::construct(const char *font, const char *boldFont) {
  logEntered();

  bool result = false;
  SDL_GetWindowSize(_window, &_w, &_h);
  _renderer = SDL_CreateRenderer(_window, -1, 0);
  if (_renderer != NULL) {
    _texture = SDL_CreateTexture(_renderer, PIXEL_FORMAT,
                                 SDL_TEXTUREACCESS_STREAMING, _w, _h);
  }
  if (_texture != NULL && loadFonts(font, boldFont)) {
    _screen = new Canvas();
    if (_screen && _screen->create(getWidth(), getHeight())) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
      result = true;
    }
  }
  return result;
}

void Graphics::redraw() {
  SDL_UpdateTexture(_texture, NULL, _screen->_surface->pixels, _w * sizeof (pixel_t));
  SDL_RenderClear(_renderer);
  SDL_RenderCopy(_renderer, _texture, NULL, NULL);
  SDL_RenderPresent(_renderer);
}

void Graphics::resize() {
  logEntered();
  bool drawScreen = (_drawTarget == _screen);
  delete _screen;
  _screen = new ::Canvas();
  _screen->create(getWidth(), getHeight());
  _drawTarget = drawScreen ? _screen : NULL;

  SDL_DestroyTexture(_texture);
  _texture = SDL_CreateTexture(_renderer, PIXEL_FORMAT,
                               SDL_TEXTUREACCESS_STREAMING, _w, _h);
}

bool Graphics::loadFonts(const char *font, const char *boldFont) {
  return (!FT_Init_FreeType(&_fontLibrary) &&
          loadFont(font, _fontFace) &&
          loadFont(boldFont, _fontFaceB));
}

bool Graphics::loadFont(const char *filename, FT_Face &face) {
  bool result = !FT_New_Face(_fontLibrary, filename, 0, &face);
  trace("load font %s = %d", filename, result);
  return result;
}

//
// maapi implementation
//
void maUpdateScreen(void) {
  ((::Graphics *)graphics)->redraw();
}

int maShowVirtualKeyboard(void) {
  return 0;
}
