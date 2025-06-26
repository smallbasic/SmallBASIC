// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include "config.h"
#include <cstddef>
#include <cstdint>

class ImageCodec {
public:
  //
  // Decode IMG to raw RGBA image
  //
  ImageCodec();
  virtual ~ImageCodec();

  // Prevent copy and move
  ImageCodec(const ImageCodec &) = delete;
  ImageCodec& operator=(const ImageCodec &) = delete;

  //
  // decode the png image to a raw RGBA
  //
  bool decode(const uint8_t *data, size_t size);

  //
  // Encode the raw image RGBA data to png
  // The function allocates *data which must be freed.
  //
  bool encode(uint8_t **data, size_t *size) const;

  //
  // resize the image to the given dimensions
  //
  void resize(unsigned width, unsigned height);

  //
  // Returns the last error
  //
  static const char *getLastError(void);

  //
  // Returns the image width
  //
  int getWidth() const { return _width; }

  //
  // Returns the image height
  //
  int getHeight() const { return _height; }

protected:
  unsigned _width;
  unsigned _height;
  uint8_t *_pixels;
};

