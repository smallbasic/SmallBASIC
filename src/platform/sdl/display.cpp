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
  _ownerSurface(false),
  _pixels(NULL),
  _surface(NULL),
  _clip(NULL) {
}

Canvas::~Canvas() {
  if (_surface != NULL && _ownerSurface) {
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
  _ownerSurface = true;
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

void Canvas::setSurface(SDL_Surface *surface, int w, int h) {
  _surface = surface;
  _pixels = (pixel_t *)_surface->pixels;
  _ownerSurface = false;
  _w = w;
  _h = h;
}

//
// Graphics implementation
//
Graphics::Graphics(SDL_Window *window) : ui::Graphics(),
  _window(window),
  _surface(NULL) {
}

Graphics::~Graphics() {
  logEntered();
}

bool Graphics::construct(const char *font, const char *boldFont) {
  logEntered();

  int w, h;
  bool result = false;
  SDL_GetWindowSize(_window, &w, &h);

  SDL_Surface *surface = SDL_GetWindowSurface(_window);
  if (surface->format->format != PIXELFORMAT) {
    deviceLog("Unexpected window surface format %d", surface->format->format);
    _surface = surface;
  }

  if (loadFonts(font, boldFont)) {
    _screen = new Canvas();
    if (_screen != NULL) {
      if (_surface == NULL) {
        _screen->setSurface(SDL_GetWindowSurface(_window), w, h);
        result = true;
      } else {
        result = _screen->create(w, h);
      }
    }
    if (result) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
    }
  }
  return result;
}

void Graphics::redraw() {
  if (_surface != NULL) {
    SDL_Surface *src = ((Canvas *)_screen)->_surface;
    SDL_BlitSurface(src, NULL, _surface, NULL);
  }
  SDL_UpdateWindowSurface(_window);
}

void Graphics::resize(int w, int h) {
  logEntered();
  SDL_Surface *surface = SDL_GetWindowSurface(_window);
  if (_surface == NULL) {
    _screen->setSurface(surface, w, h);
  } else {
    bool drawScreen = (_drawTarget == _screen);
    delete _screen;
    _screen = new ::Canvas();
    _screen->create(w, h);
    _drawTarget = drawScreen ? _screen : NULL;
    _surface = surface;
  }
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
