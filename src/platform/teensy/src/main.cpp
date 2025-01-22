// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>

#include "config.h"
#include "common/sbapp.h"
#include "main_bas.h"

#define MAIN_BAS "__main_bas__"

char *dev_read(const char *fileName) {
  char *buffer;
  if (strcmp(fileName, MAIN_BAS) == 0) {
    buffer = (char *)malloc(main_bas_len + 1);
    memcpy(buffer, main_bas, main_bas_len);
    buffer[main_bas_len] = '\0';
  } else {
    buffer = NULL;
  }
  return buffer;
}

void blink(int pauseMillis) {
  digitalWriteFast(13, HIGH);
  delay(pauseMillis);
  digitalWriteFast(13, LOW);
  delay(pauseMillis);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // wait
  }

  opt_autolocal = 0;
  opt_command[0] = '\0';
  opt_modpath[0] = '\0';
  opt_file_permitted = 0;
  opt_ide = 0;
  opt_nosave = 1;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_graphics = 0;
}

extern "C" int main(void) {
  setup();
  if (!sbasic_main(MAIN_BAS)) {
    dev_print("run failed");
  } else {
    dev_print("main.bas ended");
  }

  pinMode(13, OUTPUT);
  while (1) {
    dev_print("blink?");
    blink(500);
    yield();
  }
}

