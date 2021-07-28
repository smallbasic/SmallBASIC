// This file is part of SmallBASIC
//
// Copyright(C) 2001-2021 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef UI_RGB
#define UI_RGB

typedef uint32_t pixel_t;

#if defined(_SDL)
#define PIXELFORMAT SDL_PIXELFORMAT_RGB888
#elif defined(_ANDROID)
#define PIXELFORMAT AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM
#endif

inline void v_get_argb(int64_t c, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (c == 0) {
    a = 0;
    r = 0;
    g = 0;
    b = 0;
  } if (c < 0) {
    // from RGB
    a = 255;
    r = (-c & 0xff0000) >> 16;
    g = (-c & 0xff00) >> 8;
    b = (-c & 0xff);
  } else {
    a = (c & 0xff000000) >> 24;
    r = (c & 0xff0000) >> 16;
    g = (c & 0xff00) >> 8;
    b = (c & 0xff);
  }
}

#define v_get_argb_px(a, r, g, b) (a << 24 | (r << 16) | (g << 8) | (b))

#if defined(_SDL)
// SDL_PACKEDORDER_XRGB
// A = byte 3
// R = byte 2
// G = byte 1
// B = byte 0

#define GET_RGB_PX(r, g, b) ((0xff000000) | (r << 16) | (g << 8) | (b))

// same as internal format
#define GET_FROM_RGB888(c) (c)

inline void GET_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (c & 0xff0000) >> 16;
  g = (c & 0xff00) >> 8;
  b = (c & 0xff);
}

inline void GET_IMAGE_ARGB(const uint8_t *image, unsigned offs, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b) {
  a = image[offs + 3];
  r = image[offs + 2];
  g = image[offs + 1];
  b = image[offs + 0];
}

inline void SET_IMAGE_ARGB(uint8_t *image, unsigned offs, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  image[offs + 3] = a;
  image[offs + 2] = r;
  image[offs + 1] = g;
  image[offs + 0] = b;
}

#else

#define GET_RGB_PX(r, g, b) ((0xff000000) | (b << 16) | (g << 8) | (r))

inline pixel_t GET_FROM_RGB888(unsigned c) {
  uint8_t r = (c & 0xff0000) >> 16;
  uint8_t g = (c & 0xff00) >> 8;
  uint8_t b = (c & 0xff);
  return ((0xff000000) | (b << 16) | (g << 8) | (r));
}

inline void GET_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  b = (c & 0xff0000) >> 16;
  g = (c & 0xff00) >> 8;
  r = (c & 0xff);
}

inline void GET_IMAGE_ARGB(const uint8_t *image, unsigned offs, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b) {
  a = image[offs + 3];
  b = image[offs + 2];
  g = image[offs + 1];
  r = image[offs + 0];
}

inline void SET_IMAGE_ARGB(uint8_t *image, unsigned offs, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  image[offs + 3] = a;
  image[offs + 2] = b;
  image[offs + 1] = g;
  image[offs + 0] = r;
}

#endif

#endif
