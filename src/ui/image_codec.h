// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef IMG_CODEC_H
#define IMG_CODEC_H

#include "config.h"
#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

struct ImageData {
  ImageData();
  ~ImageData();

  unsigned _width;
  unsigned _height;
  uint8_t *_pixels;
};

//
// Decode IMG to raw RGBA image
// Caller allocates and passes in imgData and its size.
// The function allocates ImageData.pixels (must be freed with img_free).
//
int img_decode(const uint8_t *imgData, size_t imgSize, ImageData *outImage);

//
// Encode raw RGBA image to IMG
// Caller allocates and passes in image.
// The function allocates *outImgData which must be freed.
//
int img_encode(const ImageData *image, uint8_t **outImgData, size_t *outImgSize);

//
// Get last error string
//
const char *img_get_last_error(void);

#ifdef __cplusplus
}
#endif

#endif // IMG_CODEC_H
