// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL_video.h>

#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/graphics.h"

struct Graphics : ui::Graphics {
  Graphics(SDL_Window *window);
  virtual ~Graphics();

  bool construct(const char *font, const char *boldFont);
  void redraw();
  void resize(int w, int h);

private:
  bool loadFonts(const char *font, const char *boldFont);
  bool loadFont(const char *filename, FT_Face &face);

  SDL_Window *_window;
  SDL_Surface *_surface;
};

#endif
