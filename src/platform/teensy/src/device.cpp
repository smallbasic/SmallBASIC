// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>

#include <stdarg.h>
#include "config.h"
#include "common/device.h"
#include "common/smbas.h"
#include "common/keymap.h"
#include "lib/maapi.h"
#include "serial.h"

//
// setup the Serial device
// Teensy 4.x uses USB serial. Setup is done at startup. Communication
// is always at full USB speed. Either 12 or 480 Mbit/s.
//
void serial_init() {
  if (CrashReport) {
    while(!Serial) {
      delay(10);
    }
    Serial.print(CrashReport);
  }
}

//
// delay for the specified number of milliseconds
//
void dev_delay(uint32_t timeout) {
  delay(timeout);
}

//
// returns the millisecond count
//
uint64_t dev_get_millisecond_count() {
  return millis();
}

//
// returns the millisecond count (UI abstraction layer)
//
int maGetMilliSecondCount() {
  return millis();
}

//
// dispays the currently executing line number
//
void dev_trace_line(int lineNo) {
  dev_printf("Line: %d\r\n", lineNo);
}

//
// process events
//
int dev_events(int wait_flag) {
  int result;
  if (wait_flag) {
    delay(10);
  }
#if INTERACTIVE
  // break when new code available
  result = Serial.available() > 0 ? -2 : 0;
#else
  result = 0;
#endif
  yield();
  return result;
}

//
// returns hardcoded folder name '/tmp'
//
const char *dev_getcwd() {
  return "/tmp";
}

//
// logical dedvice initialisation
//
int dev_init(int mode, int flags) {
  keymap_init();
  return 1;
}

//
// print assumming the desktop terminal can display ansi escapes
//
void dev_print(const char *str) {
  // only initialise Serial when PRINT statement is used
  serial_init();
  if (str != nullptr && str[0] != '\0') {
    Serial.printf("%s", str);
    int len = strlen(str);
    if (str[len - 1] == '\n') {
      Serial.print('\r');
    }
  }
}

//
// var-args version of dev_print
//
void dev_printf(const char *format, ...) {
  static char _buffer[1024];
  va_list args;
  va_start(args, format);
  unsigned size = vsnprintf(nullptr, 0, format, args);
  va_end(args);

  if (size < sizeof(_buffer)) {
    _buffer[0] = '\0';
    va_start(args, format);
    vsnprintf((char *)_buffer, size + 1, format, args);
    va_end(args);
    _buffer[size] = '\0';
    dev_print(_buffer);
  }
}

//
// LOGPRINT statement implementation
//
void log_printf(const char *format, ...) {
  if (opt_verbose) {
    va_list args;
    va_start(args, format);
    unsigned size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    if (size) {
      char *buf = (char *)malloc(size + 3);
      buf[0] = '\0';
      va_start(args, format);
      vsnprintf(buf, size + 1, format, args);
      va_end(args);

      buf[size] = '\0';
      int i = size - 1;
      while (i >= 0 && isspace(buf[i])) {
        buf[i--] = '\0';
      }
      strcat(buf, "\r\n");
      dev_print(buf);
      free(buf);
    }
  }
}

//
// print when opt_verbose enabled
//
void lwrite(const char *buf) {
  if (opt_verbose) {
    dev_print(buf);
  }
}

//
// unrecoverable error
//
void panic(const char *fmt, ...) {
  dev_print("Fatal error");
  for (;;);
}

