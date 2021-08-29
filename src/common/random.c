// This file is part of SmallBASIC
//
// random number generator, see:
// https://en.wikipedia.org/wiki/Permuted_congruential_generator
// https://www.pcg-random.org/download.html
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2021 Chris Warren-Smith

#include "config.h"

#include "common/sys.h"
#include <stdint.h>
#include <limits.h>

static uint64_t state      = 0x4d595df4d0f33173;
static uint64_t multiplier = 6364136223846793005u;
static uint64_t increment  = 1442695040888963407u;

static uint32_t rotr32(uint32_t x, unsigned r) {
  return x >> r | x << (-r & 31);
}

var_num_t pcg32_rand() {
  uint64_t x = state;
  unsigned count = (unsigned)(x >> 59);
  state = x * multiplier + increment;
  x ^= x >> 18;
  int32_t r = rotr32((uint32_t)(x >> 27), count);
  return ((var_num_t)abs(r)) / (INT_MAX + 1.0);
}

void pcg32_srand(var_int_t seed) {
  state = seed + increment;
  pcg32_rand();
}
