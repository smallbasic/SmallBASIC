// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef IMAGE_H
#define IMAGE_H

#include "common/var.h"
#include "ui/shape.h"

struct ImageBuffer {
  ImageBuffer();
  ImageBuffer(ImageBuffer &imageBuffer);
  virtual ~ImageBuffer();

  unsigned _bid;
  char *_filename;
  unsigned char *_image;
  int _width;
  int _height;
};

struct ImageDisplay : public Shape {
  ImageDisplay();
  ImageDisplay(ImageDisplay &imageDisplay);
  virtual ~ImageDisplay() {}

  void copyImage(ImageDisplay &imageDisplay);
  void draw(int x, int y, int bw, int bh, int cw);

  int _offsetLeft;
  int _offsetTop;
  int _zIndex;
  int _opacity;
  unsigned _id;
  unsigned _bid;
  ImageBuffer *_buffer;
};

ImageDisplay *create_display_image(var_p_t var, const char *name);
void screen_dump();
extern "C" int xpm_decode32(uint8_t **image, unsigned *width, unsigned *height, 
                            const char *const *xpm);

#endif
