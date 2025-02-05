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

#define SERIAL_BAUD_RATE 1000000

const int keymap[][2] = {
  {0x32, SB_KEY_INSERT},
  {0x33, SB_KEY_DELETE},
  {0x35, SB_KEY_PGUP},
  {0x36, SB_KEY_PGDN},
  {0x41, SB_KEY_UP},
  {0x42, SB_KEY_DOWN},
  {0x43, SB_KEY_RIGHT},
  {0x44, SB_KEY_LEFT},
  {0x46, SB_KEY_END},
  {0x48, SB_KEY_HOME}
};

const int keymapLen = sizeof(keymap) / sizeof(keymap[0]);

//
// read the next set of characters following escape
//
int get_escape() {
  int result = -1;
  if (Serial.available()) {
    char byte = Serial.read();
    if (byte == '[') {
      if (Serial.available()) {
        int key = Serial.read();
        for (int i = 0; i < keymapLen; i++) {
          if (key == keymap[i][0]) {
            result = keymap[i][1];
            break;
          }
        }
        if (result == -1) {
          dev_printf("Unknown Esc[ key [%x]\n", key);
        }
      }
    } else {
      result = SB_KEY_ESCAPE;
    }
  }
  return result;
}

//
// read the next key from the serial device
//
int get_key(void) {
  int result = -1;
  if (Serial.available()) {
    result = Serial.read();
    switch (result) {
    case 0x09:
      result = SB_KEY_TAB;
      break;
    case 0x0d:
      result = SB_KEY_ENTER;
      break;
    case 0x1b:
      result = get_escape();
      break;
    case 0x7e:
      result = SB_KEY_DELETE;
      break;
    case 0x7f:
      result = SB_KEY_BACKSPACE;
      break;
    }
    //dev_printf("got key %x\n", result);
  }
  return result;
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
  dev_printf("<%d>", lineNo);
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
  if (Serial) {
    int key = get_key();
    if (key != -1) {
      dev_pushkey(key);
    }
  }
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
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    // wait
  }
  if (CrashReport) {
    Serial.print(CrashReport);
  }
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
    Serial.println(buf);
  }
}

//
// unrecoverable error
//
void panic(const char *fmt, ...) {
  Serial.println("Fatal error");
  for (;;);
}

