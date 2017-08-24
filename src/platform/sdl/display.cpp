// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "ui/utils.h"
#include "platform/sdl/display.h"

extern ui::Graphics *graphics;

#define PIXELFORMAT SDL_PIXELFORMAT_RGB888

//
// Canvas implementation
//
Canvas::Canvas() :
  _w(0),
  _h(0),
  _pixels(NULL),
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
  SDL_PixelFormatEnumToMasks(PIXELFORMAT, &bpp, &rmask, &gmask, &bmask, &amask);
  _surface = SDL_CreateRGBSurface(0, w, h, bpp, rmask, gmask, bmask, amask);
  _pixels = (pixel_t *)_surface->pixels;
  return _surface != NULL;
}

void Canvas::drawRegion(Canvas *src, const MARect *srcRect, int destX, int destY) {
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

void Canvas::fillRect(int x, int y, int w, int h, pixel_t color) {
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_FillRect(_surface, &rect, color);
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
Graphics::Graphics(SDL_Window *window) :
  ui::Graphics(),
  _window(window),
  _renderer(NULL),
  _texture(NULL) {
}

Graphics::~Graphics() {
  logEntered();
  SDL_DestroyTexture(_texture);
  SDL_DestroyRenderer(_renderer);
  _renderer = NULL;
  _texture = NULL;
}

bool Graphics::construct(const char *font, const char *boldFont) {
  logEntered();

  int w, h;
  bool result = true;
  SDL_GetWindowSize(_window, &w, &h);
  SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "1", SDL_HINT_OVERRIDE);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED |
                                 SDL_RENDERER_PRESENTVSYNC);

  if (_renderer == NULL) {
    deviceLog("SDL renderer is null\n");
    result = false;
  } else {
    _texture = SDL_CreateTexture(_renderer, PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (_texture == NULL) {
      deviceLog("SDL texture is null\n");
      result = false;
    }
  }
  if (result && loadFonts(font, boldFont)) {
    _screen = new Canvas();
    if (_screen != NULL) {
      result = _screen->create(w, h);
    }
    if (result) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
    }
  } else {
    result = false;
  }
  return result;
}

void Graphics::redraw() {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = _screen->_w;
  rect.h = _screen->_h;

  void *pixels;
  int pitch;
  if (SDL_LockTexture(_texture, &rect, &pixels, &pitch) == -1) {
    deviceLog("Unable to lock window buffer");
  } else {
    memcpy(pixels, _screen->_surface->pixels, _screen->_w * _screen->_h * 4);
    SDL_UnlockTexture(_texture);
    SDL_RenderCopy(_renderer, _texture, NULL, &rect);
    SDL_RenderPresent(_renderer);
  }
}

void Graphics::resize(int w, int h) {
  logEntered();
  bool drawScreen = (_drawTarget == _screen);
  delete _screen;
  _screen = new ::Canvas();
  _screen->create(w, h);

  SDL_DestroyTexture(_texture);
  _texture = SDL_CreateTexture(_renderer, PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, w, h);
  _drawTarget = drawScreen ? _screen : NULL;
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
