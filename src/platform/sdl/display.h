// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>

#include "platform/common/maapi.h"
#include "platform/common/StringLib.h"
#include "platform/common/graphics.h"

struct Canvas : public common::Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void freeClip();
  void setClip(int x, int y, int w, int h);
  pixel_t *getLine(int y);
  int x() { return _clip ? _clip->x : 0; }
  int y() { return _clip ? _clip->y : 0; }
  int w() { return _clip ? _clip->w : _w; }
  int h() { return _clip ? _clip->h : _h; }

  SDL_Surface *_canvas;
  SDL_Rect *_clip;
};

struct Graphics : common::Graphics {
  Graphics(SDL_Window *window);
  virtual ~Graphics();

  bool construct(const char *font, const char *boldFont);
  void redraw();
  void resize();

private:
  bool loadFonts(const char *font, const char *boldFont);
  bool loadFont(const char *filename, FT_Face &face);

  SDL_Window *_window;
};

#endif
