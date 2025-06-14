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

static char g_last_error[256] = {0};

static float lerp(float a, float b, float t) {
  return a + t * (b - a);
}

static void to_argb(unsigned char *image, unsigned w, unsigned h) {
#if defined(_SDL)
  // convert from LCT_RGBA to ARGB
  for (unsigned y = 0; y < h; y++) {
    unsigned yoffs = (y * w * 4);
    for (unsigned x = 0; x < w; x++) {
      unsigned offs = yoffs + (x * 4);
      uint8_t r = image[offs + 2];
      uint8_t b = image[offs + 0];
      image[offs + 2] = b;
      image[offs + 0] = r;
    }
  }
#endif
}

ImageCodec::ImageCodec() :
  _width(0),
  _height(0),
  _pixels(nullptr) {
}

bool ImageCodec::decode(const uint8_t *data, size_t size) {
  auto error = lodepng_decode32(&_pixels, &_width, &_height, data, size);
  if (error) {
    snprintf(g_last_error, sizeof(g_last_error), "%s", lodepng_error_text(error));
  } else {
    to_argb(_pixels, _width, _height);
  }

  return !error;
}

ImageCodec::~ImageCodec() {
  free(_pixels);
  _pixels = nullptr;
}

const char *ImageCodec::getLastError(void) {
  return g_last_error;
}

bool ImageCodec::encode(uint8_t **data, size_t *size) const {
  bool result;
  if (!data || !size) {
    sprintf(g_last_error, "Invalid args");
    result = false;
  } else {
    uint8_t *img = nullptr;
    size_t img_size = 0;
    auto error = lodepng_encode32(&img, &img_size, _pixels, _width, _height);
    if (error) {
      snprintf(g_last_error, sizeof(g_last_error), "%s", lodepng_error_text(error));
    } else {
      *data = img;
      *size = img_size;
    }
    result = !error;
  }
  return result;
}

void ImageCodec::resize(unsigned width, unsigned height) {
  uint8_t *pixels = new uint8_t[width * height * 4];

  for (int y = 0; y < height; y++) {
    float gy = ((float)y + 0.5f) * _height / height - 0.5f;
    int y0 = (int)gy, y1 = y0 + 1;
    float fy = gy - y0;
    y0 = y0 < 0 ? 0 : (y0 >= _height ? _height - 1 : y0);
    y1 = y1 < 0 ? 0 : (y1 >= _height ? _height - 1 : y1);

    for (int x = 0; x < width; x++) {
      float gx = ((float)x + 0.5f) * _width / width - 0.5f;
      int x0 = (int)gx, x1 = x0 + 1;
      float fx = gx - x0;
      x0 = x0 < 0 ? 0 : (x0 >= _width ? _width - 1 : x0);
      x1 = x1 < 0 ? 0 : (x1 >= _width ? _width - 1 : x1);

      int offset = 4 * (y * width + x);
      for (int c = 0; c < 4; c++) {
        // A, R, G, B
        float tl = _pixels[4 * (y0 * _width + x0) + c];
        float tr = _pixels[4 * (y0 * _width + x1) + c];
        float bl = _pixels[4 * (y1 * _width + x0) + c];
        float br = _pixels[4 * (y1 * _width + x1) + c];

        float top = lerp(tl, tr, fx);
        float bottom = lerp(bl, br, fx);
        float value = lerp(top, bottom, fy);

        pixels[offset + c] = (uint8_t)(value + 0.5f);
      }
    }
  }

  free(_pixels);
  _pixels = pixels;
  _width = width;
  _height = height;
}

