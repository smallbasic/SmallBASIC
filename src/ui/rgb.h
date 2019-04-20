// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef UI_RGB
#define UI_RGB

#if defined(PIXELFORMAT_RGB565)
  typedef uint16_t pixel_t;
#else
  typedef uint32_t pixel_t;
#endif

inline void RGB888_to_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (c & 0xff0000) >> 16;
  g = (c & 0xff00) >> 8;
  b = (c & 0xff);
}

inline pixel_t RGB888_to_RGBA8888(unsigned c) {
  uint8_t r = (c & 0xff0000) >> 16;
  uint8_t g = (c & 0xff00) >> 8;
  uint8_t b = (c & 0xff);
  return ((0xff000000) | (b << 16) | (g << 8) | (r));
}

inline void RGB888_BE_to_RGB(pixel_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  b = (c & 0xff0000) >> 16;
  g = (c & 0xff00) >> 8;
  r = (c & 0xff);
}

inline void GET_ARGB(pixel_t c, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b) {
  a = (c & 0xff000000) >> 24;
  r = (c & 0x00ff0000) >> 16;
  g = (c & 0x0000ff00) >> 8;
  b = (c & 0x000000ff);
}

#if defined(PIXELFORMAT_RGBA8888)
  #define SET_RGB(r, g, b) ((0xff000000) | (r << 16) | (g << 8) | (b))
  #define SET_ARGB(a, r, g, b) (a << 24 | (r << 16) | (g << 8) | (b))
  #define GET_RGB RGB888_to_RGB
  #define GET_RGB2 RGB888_BE_to_RGB
  #define GET_FROM_RGB888 RGB888_to_RGBA8888
#else
  #define SET_RGB(r, g, b) ((r << 16) | (g << 8) | (b))
  #define GET_RGB  RGB888_to_RGB
  #define GET_RGB2 RGB888_to_RGB
#define GET_FROM_RGB888(c) (c)
#endif

#endif
