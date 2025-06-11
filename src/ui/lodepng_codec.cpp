// This file is part of SmallBASIC
//
// Image handling
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2002-2025 Chris Warren-Smith.

#include "image_codec.h"
#include "lib/lodepng/lodepng.h"
#include <cstdlib>
#include <cstring>

static char g_last_error[256] = {0};

ImageData::ImageData() :
  _width(0),
  _height(0),
  _pixels(nullptr) {
}

ImageData::~ImageData() {
  free(_pixels);
  _pixels = nullptr;
}

int img_decode(const uint8_t *imgData, size_t imgSize, ImageData *outImage) {
  if (!imgData || !outImage) {
    sprintf(g_last_error, "Invalid args");
    return -1;
  }

  unsigned w, h;
  uint8_t* pixels = nullptr;
  unsigned err = lodepng_decode32(&pixels, &w, &h, imgData, imgSize);

  if (err) {
    snprintf(g_last_error, sizeof(g_last_error), "%s", lodepng_error_text(err));
    return -2;
  }

  outImage->_width = w;
  outImage->_height = h;
  outImage->_pixels = pixels;
  return 0;
}

int img_encode(const ImageData *image, uint8_t **outImgData, size_t *outImgSize) {
  if (!image || !outImgData || !outImgSize) {
    sprintf(g_last_error, "Invalid args");
    return -1;
  }

  unsigned char *img = nullptr;
  size_t img_size = 0;

  unsigned err = lodepng_encode32(&img, &img_size,
                                  image->_pixels,
                                  image->_width,
                                  image->_height);
  if (err) {
    snprintf(g_last_error, sizeof(g_last_error), "%s", lodepng_error_text(err));
    return -2;
  }

  *outImgData = img;
  *outImgSize = img_size;
  return 0;
}

const char *img_get_last_error(void) {
  return g_last_error;
}
