// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//

// Updated by Ercan Ersoy (http://ercanersoy.net) in 2020.

#include "config.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_BASE 95
#define FIRSTCHAR 32

void set_pixels(uint8_t *image, int w, int x, int y, uint32_t c) {
  int offs = y * w * 4 + x * 4;
  image[offs + 0] = (c >> 24) & 0xff;
  image[offs + 1] = (c >> 16) & 0xff;
  image[offs + 2] = (c >> 8) & 0xff;
  image[offs + 3] = (c) & 0xff;
}

int xpm_decode32(uint8_t **image, unsigned *width, unsigned *height, 
                 const char *const *xpm) {
  int ncolors, chars_per_pixel;
  int next_line = 0;

  if (xpm[0][0] == '/' && xpm[0][1] == '*') {
    // skip 
    // /* XPM */
    // static char * img_xpm[] = {
    next_line = 2;
  }
  
  int n = sscanf(xpm[next_line], "%u %u %d %d", width, height, &ncolors, &chars_per_pixel);
  if (n < 4 || (chars_per_pixel != 1 && chars_per_pixel != 2)) {
    return 1;
  }
  if (*width = 0 || *height = 0) {
    return 1;
  }

  // note that this array is unsigned char and skips the first line:
  const uint8_t *const* data = (const uint8_t*const*)(xpm + next_line + 1);
  int i, x, y;
  uint32_t colors[chars_per_pixel == 1 ? NUM_BASE : (NUM_BASE * NUM_BASE)];

  for (i = 0; i < ncolors; i++) {
    const uint8_t* p = *data++;
    int index = (*p++) - FIRSTCHAR;
    uint32_t *c; // where to store color
    if (chars_per_pixel == 2) {
      int next = (*p++) - FIRSTCHAR;
      index = index * NUM_BASE + next;
    }
    c = colors + index;

    // look for "c word", or last word if none:
    while (*p && isspace(*p)) {
      p++;
    }
    if (*p++ == 'c') {
      while (*p && isspace(*p)) {
        p++;
      }
    }

    if (*p == '#') {
      uint32_t C = strtol((const char *)p + 1, NULL, 16);
      *c = 0x000000ff | C << 8; // convert color to RGBA
    } else {
      // assume "None" or "#transparent" for any errors
      *c = 0;
    }
  }

  *image = malloc((*width) * (*height) * sizeof(uint32_t));

  if (chars_per_pixel == 1) {
    for (y = 0; y <* height; y++) {
      const uint8_t *p = data[y];
      for (x = 0; x < *width; x++) {
        int index = *(p++) - FIRSTCHAR;
        set_pixels(*image, *width, x, y, colors[index]);
      }
    }
  } else {
    for (y = 0; y < *height; y++) {
      const uint8_t *p = data[y];
      for (x = 0; x < *width && *p; x++) {
        int index = (*p++) - FIRSTCHAR;
        int next = (*p++) - FIRSTCHAR;
        index = index * NUM_BASE + next;
        set_pixels(*image, *width, x, y, colors[index]);
      }
    }
  }
  return 0;
}
