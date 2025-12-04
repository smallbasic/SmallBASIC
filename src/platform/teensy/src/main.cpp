// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "common/device.h"
#include "config.h"
#include "common/sbapp.h"
#include "main_bas.h"
#include "module.h"
#include "ui/strlib.h"

#define MAIN_BAS "__main_bas__"
#define SERIAL_SD_BAS "__serial_sd_bas__"

strlib::String buffer;

void *plugin_lib_open(const char *name) {
  void *result = nullptr;
  if (strcmp(name, "/libteensy") == 0) {
    result = get_teensy_module();
  } else if (strcmp(name, "/libssd1306") == 0) {
    result = get_ssd1306_module();
  }

  return result;
}

void *plugin_lib_address(void *handle, const char *name) {
  auto *pModule = (ModuleConfig *) handle;
  void *result = nullptr;
  if (strcmp(name, "sblib_func_exec") == 0) {
    result = (void *)pModule->_func_exec;
  } else if (strcmp(name, "sblib_proc_exec") == 0) {
    result = (void *)pModule->_proc_exec;
  } else if (strcmp(name, "sblib_free") == 0) {
    result = (void *)pModule->_free;
  } else if (strcmp(name, "sblib_proc_count") == 0) {
    result = (void *)pModule->_proc_count;
  } else if (strcmp(name, "sblib_proc_getname") == 0) {
    result = (void *)pModule->_proc_getname;
  } else if (strcmp(name, "sblib_func_count") == 0) {
    result = (void *)pModule->_func_count;
  } else if (strcmp(name, "sblib_func_getname") == 0) {
    result = (void *)pModule->_func_getname;
  }
  return result;
}

char *dev_read(const char *fileName) {
  char *data;
  if (strcmp(fileName, MAIN_BAS) == 0) {
    data = (char *)malloc(main_bas_len + 1);
    memcpy(data, main_bas, main_bas_len);
    data[main_bas_len] = '\0';
  } else if (strcmp(fileName, SERIAL_SD_BAS) == 0) {
    int len = buffer.length();
    data = (char *)malloc(len + 1);
    memcpy(data, buffer.c_str(), len);
    data[len] = '\0';
  } else {
    data = nullptr;
  }
  return data;
}

int sys_search_path(const char *path, const char *file, char *retbuf) {
  int result;
  if (strcmp(file, "libteensy") == 0 || strcmp(file, "libssd1306") == 0) {
    strcpy(retbuf, "/");
    result = 1;
  }
  return result;
}

void setup() {
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
  dev_init(0, 0);
}

void serial_read() {
  bool eof = false;
  int lastRead = -1;
  buffer.clear();

  while (!eof) {
    if (Serial.available()) {
      char ch = (char)Serial.read();
      if (ch != '\r' && ch != '\0') {
        buffer.append(ch);
      }
      lastRead = millis();
    } else if (lastRead != -1 && buffer.length() > 1 && millis() - lastRead > 1000) {
      eof = true;
    } else {
      delay(250);
    }
  }

  dev_printf("Received %d bytes.\r\n", buffer.length());
}

void print_source_line(const char *text, int line, bool last) {
  char lineMargin[32];
  sprintf(lineMargin, "\033[7m%03d\033[0m ", line);
  dev_print(lineMargin);
  if (line == gsb_last_line && gsb_last_error) {
    dev_print("\033[7m");
    dev_print(text);
    if (last) {
      dev_print("\n");
    }
    dev_print("\033[27;31m  --^\n");
    dev_print(gsb_last_errmsg);
    dev_print("\033[0m\n");
  } else {
    dev_print(text);
  }
}

void print_error(char *source) {
  char *ch = source;
  char *nextLine = source;
  int errorLine = gsb_last_error ? gsb_last_line : -1;
  int line = 1;
  int pageLines = 25;

  dev_print("\007");
  while (*ch) {
    while (*ch && *ch != '\n') {
      ch++;
    }
    if (*ch == '\n') {
      ch++;
      char end = *ch;
      *ch = '\0';
      print_source_line(nextLine, line, false);
      *ch = end;
      nextLine = ch;
    } else {
      print_source_line(nextLine, line, true);
    }
    line++;

    if (errorLine != -1 && line == errorLine + pageLines) {
      // avoid scrolling past the error line
      if (*ch) {
        dev_print("... \n");
      }
      break;
    }
  }
}

void interactive_main() {
  while (true) {
    dev_print("\r\n\033[30;47mInteractive mode - waiting for data...\033[0m\r\n");
    serial_read();
    if (!sbasic_main(SERIAL_SD_BAS)) {
      print_error((char *)buffer.c_str());
    }
  }
}

extern "C" int main(void) {
  setup();

  if (SD.begin(BUILTIN_SDCARD)) {
    File sdFile = SD.open("/MAIN.BAS", FILE_READ);
    if (sdFile) {
      uint32_t fileSize = sdFile.available();
      char *bufferSD = new char[fileSize + 1];

      sdFile.read(bufferSD, fileSize);
      bufferSD[fileSize] = '\0';
      buffer.clear();
      buffer.append(bufferSD);

      delete[]bufferSD;
      sdFile.close();

      if (!sbasic_main(SERIAL_SD_BAS)) {
        while (!Serial) {
          delay(250);
        }
        dev_print("Error executing main.bas from SD card:\n");
        print_error((char *)buffer.c_str());
      } else {
        dev_print("main.bas from SD card ended\n");
      }
    }
  } else if (main_bas_len > 0) {
    if (!sbasic_main(MAIN_BAS)) {
      while (!Serial) {
        delay(250);
      }
      dev_print("Error executing main.bas from memory:\n");
      print_error((char *)main_bas);
    } else {
      dev_print("main.bas ended");
    }
  } else {
    interactive_main();
  }
}
