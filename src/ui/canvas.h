// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef UI_CANVAS
#define UI_CANVAS

#if defined(_SDL)
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#define MAX_CANVAS_SIZE 20

struct Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void drawRegion(Canvas *src, const MARect *srcRect, int dstx, int dsty);
  void fillRect(int x, int y, int w, int h, pixel_t color);
  void setClip(int x, int y, int w, int h);
  void setSurface(SDL_Surface *surface, int w, int h);
  pixel_t *getLine(int y) { return _pixels + (y * _w); }
  int x() { return _clip ? _clip->x : 0; }
  int y() { return _clip ? _clip->y : 0; }
  int w() { return _clip ? _clip->w : _w; }
  int h() { return _clip ? _clip->h : _h; }

  int _w;
  int _h;
  pixel_t *_pixels;
  SDL_Surface *_surface{};
  SDL_Rect *_clip{};
  bool _ownerSurface{};
};

#else
#include <android/rect.h>
#define MAX_CANVAS_SIZE 20

struct Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void drawRegion(Canvas *src, const MARect *srcRect, int dstx, int dsty);
  void fillRect(int x, int y, int w, int h, pixel_t color);
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y) const { return _pixels + (y * _w); }
  int x() const { return _clip ? _clip->left : 0; }
  int y() const { return _clip ? _clip->top : 0; }
  int w() const { return _clip ? _clip->right : _w; }
  int h() const { return _clip ? _clip->bottom : _h; }

  int _w;
  int _h;
  pixel_t *_pixels;
  ARect *_clip;
};

#endif
#endif

