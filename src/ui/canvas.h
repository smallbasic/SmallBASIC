// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef COMMON_CANVAS
#define COMMON_CANVAS

#if defined(PIXELFORMAT_RGB565)
  typedef uint16_t pixel_t;
#else
  typedef uint32_t pixel_t;
#endif

#if defined(_SDL)
#include <SDL.h>
#define MAX_CANVAS_SIZE 20

struct Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  bool create(int w, int h, SDL_Window *window);
  void copy(Canvas *src, const MARect *srcRect, int dstx, int dsty);
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y);
  int x() { return _clip ? _clip->x : 0; }
  int y() { return _clip ? _clip->y : 0; }
  int w() { return _clip ? _clip->w : _w; }
  int h() { return _clip ? _clip->h : _h; }

  int _w;
  int _h;
  SDL_Surface *_canvas;
  SDL_Rect *_clip;
};

#else
#include <android/rect.h>
#define MAX_CANVAS_SIZE 20

struct Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void copy(Canvas *src, const MARect *srcRect, int dstx, int dsty);
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y) { return _canvas + (y * _w); }
  int x() { return _clip ? _clip->left : 0; }
  int y() { return _clip ? _clip->top : 0; }
  int w() { return _clip ? _clip->right : _w; }
  int h() { return _clip ? _clip->bottom : _h; }

  int _w;
  int _h;
  pixel_t *_canvas;
  ARect *_clip;
};

#endif
#endif

