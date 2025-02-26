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

  char *_filename;
  unsigned char *_image;
  unsigned _bid;
  unsigned _width;
  unsigned _height;
};

struct ImageDisplay : public Shape {
  ImageDisplay();
  ImageDisplay(ImageDisplay &imageDisplay);
  ~ImageDisplay() override = default;

  void copyImage(ImageDisplay &imageDisplay);
  void draw(int x, int y, int bw, int bh, int cw) override;

  int _offsetLeft;
  int _offsetTop;
  int _zIndex;
  int _opacity;
  unsigned _id;
  unsigned _bid;
  ImageBuffer *_buffer;
};

ImageDisplay *create_display_image(var_p_t var, const char *name);
void reset_image_cache();
void screen_dump();

#endif
