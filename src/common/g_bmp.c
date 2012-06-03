// This file is part of SmallBASIC
//
// Bitmap manipulation routines
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2002 Nicholas Christopoulos

#include "common/g_bmp.h"

// combine dest with source using AND and store result to dest
void bmp_combine_AND(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ &= *p++;
    bpl--;
  }
}

// combine dest with source using OR and store result to dest
void bmp_combine_OR(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ |= *p++;
    bpl--;
  }
}

// combine dest with source using XOR and store result to dest
void bmp_combine_XOR(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ ^= *p++;
    bpl--;
  }
}

// combine dest with source using NOT and store result to dest
void bmp_combine_NOT(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ = ~(*p);
    p++;
    bpl--;
  }
}

// combine dest with source using NAND and store result to dest
void bmp_combine_NAND(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ = ~(*d & *p);
    p++;
    bpl--;
  }
}

// combine dest with source using NOR and store result to dest
void bmp_combine_NOR(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ = ~(*d | *p);
    p++;
    bpl--;
  }
}

// combine dest with source using XNOR and store result to dest
void bmp_combine_XNOR(unsigned char *dest, unsigned char *source, int bpl) {
  unsigned char *p = source;
  unsigned char *d = dest;

  while (bpl) {
    *d++ = ~(*d ^ *p);
    p++;
    bpl--;
  }
}

/*
 *	returns the required size for the bitmap
 */
long bmp_size(int x1, int y1, int x2, int y2, int bpp) {
  long isize;

  isize = ((y2 - y1) + 1) * ((x2 - x1) + 1);

  if (bpp >= 8)
    return isize * (bpp / 8) + sizeof(bmp_header_t);

  return isize * (bpp / 8 + 1) + sizeof(bmp_header_t);
}

/*
 *	combine two bitmaps
 */
void bmp_combine(unsigned char *dest, unsigned char *source, int bpl, int lines, int mode) {
  int ofs, y;

  for (y = 0; y < lines; y++) {
    ofs = y + bpl;

    switch (mode) {
    case BMP_AND:
      bmp_combine_AND(dest + ofs, source + ofs, bpl);
      break;
    case BMP_OR:
      bmp_combine_OR(dest + ofs, source + ofs, bpl);
      break;
    case BMP_XOR:
      bmp_combine_XOR(dest + ofs, source + ofs, bpl);
      break;
    case BMP_NOT:
      bmp_combine_NOT(dest + ofs, source + ofs, bpl);
      break;
    case BMP_NAND:
      bmp_combine_NAND(dest + ofs, source + ofs, bpl);
      break;
    case BMP_NOR:
      bmp_combine_NOR(dest + ofs, source + ofs, bpl);
      break;
    case BMP_XNOR:
      bmp_combine_XNOR(dest + ofs, source + ofs, bpl);
      break;
    }
  }
}

/*
 *	generic bmp copy (using getpixel style routine)
 */
void bmp_get(bmp_t * bmp, int x1, int y1, int x2, int y2, int bpp, long(*pget)(int x, int y)) {
  int x, y, dx, dx24;
  long c;
  char *dest;
  bmp_header_t *hdr;

  hdr = (bmp_header_t *) bmp;
  dest = (char *) (bmp + sizeof(bmp_header_t));

  strcpy(hdr->sign, "$img1");
  hdr->dx = dx = (x2 - x1) + 1;
  hdr->dy = (y2 - y1) + 1;
  hdr->bpp = bpp;
  if (bpp > 8)
    hdr->bytespp = bpp / 8;
  else
    hdr->bytespp = 1;

  dx24 = dx * 3;

  for (y = y1; y <= y2; y++) {
    for (x = x1; x <= x2; x++) {
      switch (bpp) {
      case 8:
        ((unsigned char *) dest)[y * dx + x] = pget(x, y);
        break;
      case 16:
        ((unsigned short int *) dest)[y * dx + x] = pget(x, y);
        break;
      case 24:
        c = pget(x, y);
#if defined(CPU_BIGENDIAN)
        ((unsigned char *) dest)[y * dx + x] = (c & 0xFF0000) >> 16;
        ((unsigned char *) dest)[y * dx + x + 1] = (c & 0xFF00) >> 8;
        ((unsigned char *) dest)[y * dx + x + 2] = (c & 0xFF);
#else
        ((unsigned char *)dest)[y * dx + x] = (c & 0xFF);
        ((unsigned char *)dest)[y * dx + x + 1] = (c & 0xFF00) >> 8;
        ((unsigned char *)dest)[y * dx + x + 2] = (c & 0xFF0000) >> 16;
#endif
        break;
      case 32:
        ((unsigned int *) dest)[y * dx + x] = pget(x, y);
        break;
      }
    }
  }
}

/*
 *	generic bmp copy (using setpixel style routine)
 */
void bmp_put(bmp_t * bmp, int x1, int y1, void(*pset)(int x, int y, long c)) {
  int x, y, dx, dx24;
  int x2, y2, bpp;
  dword c;
  char *src;

  dx = bmp->dx;
  x2 = x1 + bmp->dx;
  y2 = y1 + bmp->dy;
  bpp = bmp->bpp;
  dx24 = dx * 3;
  src = (char *) (bmp + sizeof(bmp_header_t));
  for (y = y1; y <= y2; y++) {
    for (x = x1; x <= x2; x++) {
      switch (bpp) {
      case 8:
        pset(x, y, ((unsigned char *) src)[y * dx + x]);
        break;
      case 16:
        pset(x, y, ((unsigned short int *) src)[y * dx + x]);
        break;
      case 24:
#if defined(CPU_BIGENDIAN)
        c = ((unsigned char *) src)[y * dx + x] << 16;
        c |= ((unsigned char *) src)[y * dx + x + 1] << 8;
        c |= ((unsigned char *) src)[y * dx + x + 2];
#else
        c = ((unsigned char *)src)[y * dx + x];
        c |= ((unsigned char *)src)[y * dx + x + 1] << 8;
        c |= ((unsigned char *)src)[y * dx + x + 2] << 16;
#endif
        pset(x, y, c);
        break;
      case 32:
        pset(x, y, ((unsigned int *) src)[y * dx + x]);
        break;
      }
    }
  }
}

/*
 */
int bmp_isvalid(bmp_t * bmp) {
  return (memcmp(bmp, "$img1", 5) == 0);
}
