#include <Arduino.h>

#include "config.h"
#include "common/sbapp.h"

extern "C" void setup() {
	pinMode(13, OUTPUT);

  opt_autolocal = 0;
  opt_command[0] = '\0';
  opt_modpath[0] = '\0';
  opt_file_permitted = 1;
  opt_ide = 0;
  opt_nosave = 1;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_graphics = 0;
  sbasic_main("foo");
  
}

extern "C" void loop() {
  digitalWriteFast(13, HIGH);
  delay(500);
  digitalWriteFast(13, LOW);
  delay(500);
}

