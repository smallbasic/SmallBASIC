// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "common/sbapp.h"
#include "module.h"

// see: https://www.pjrc.com/teensy/td_libs_SSD1306.html
// https://core-electronics.com.au/piicodev-oled-display-module-128x64-ssd1306.html#guides

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool init_done = false;

static int cmd_init(int argc, slib_par_t *params, var_t *retval) {
  if (!init_done) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, true)) {
      dev_print("SSD1306 initialization failed");
      for (;;);
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.fillScreen(SSD1306_BLACK);
    display.setTextSize(1);

    os_graf_mx = SCREEN_WIDTH;
    os_graf_my = SCREEN_HEIGHT;
    setsysvar_int(SYSVAR_XMAX, os_graf_mx - 1);
    setsysvar_int(SYSVAR_YMAX, os_graf_my - 1);
    init_done = true;
  }
  return 1;
}

// void clearDisplay(void); // clear
static int cmd_cleardisplay(int argc, slib_par_t *params, var_t *retval) {
  display.clearDisplay();
  return 1;
}

// void dim(bool dim);
static int cmd_dim(int argc, slib_par_t *params, var_t *retval) {
  auto dim = get_param_int(argc, params, 0, 0);
  display.dim(dim);
  return 1;
}

// void display(void); // flush
static int cmd_display(int argc, slib_par_t *params, var_t *retval) {
  display.display();
  return 1;
}

// void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
static int cmd_drawchar(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x = get_param_int(argc, params, arg++, 0);
  auto y = get_param_int(argc, params, arg++, 0);
  auto chr = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, 0);
  auto bg = get_param_int(argc, params, arg++, 0);
  auto size = get_param_int(argc, params, arg++, 0);
  display.drawChar(x, y, chr, color, bg, size);
  return 1;
}

// void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
static int cmd_drawcircle(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto r = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.drawCircle(x0, y0, r, color);
  return 1;
}

// void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
static int cmd_drawline(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto x1 = get_param_int(argc, params, arg++, 0);
  auto y1 = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.drawLine(x0, y0, x1, y1, color);
  return 1;
}

// void drawPixel(int16_t x, int16_t y, uint16_t color); // setPixel
static int cmd_drawpixel(int argc, slib_par_t *params, var_t *retval) {
  auto x = get_param_int(argc, params, 0, 0);
  auto y = get_param_int(argc, params, 1, 0);
  auto color = get_param_int(argc, params, 2, SSD1306_WHITE);
  display.drawPixel(x, y, color);
  return 1;
}

// void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
static int cmd_drawrect(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x = get_param_int(argc, params, arg++, 0);
  auto y = get_param_int(argc, params, arg++, 0);
  auto w = get_param_int(argc, params, arg++, 0);
  auto h = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.drawRect(x, y, w, h, color);
  return 1;
}

// void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
static int cmd_drawroundrect(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto w = get_param_int(argc, params, arg++, 0);
  auto h = get_param_int(argc, params, arg++, 0);
  auto radius = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.drawRoundRect(x0, y0, w, h, radius, color);
  return 1;
}

// void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
static int cmd_drawtriangle(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto x1 = get_param_int(argc, params, arg++, 0);
  auto y1 = get_param_int(argc, params, arg++, 0);
  auto x2 = get_param_int(argc, params, arg++, 0);
  auto y2 = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.drawTriangle(x0, y0, x1, y1, x2, y2, color);
  return 1;
}

// void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
static int cmd_fillcircle(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto r = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.fillCircle(x0, y0, r, color);
  return 1;
}

// void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
static int cmd_fillrect(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x = get_param_int(argc, params, arg++, 0);
  auto y = get_param_int(argc, params, arg++, 0);
  auto w = get_param_int(argc, params, arg++, 0);
  auto h = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.fillRect(x, y, w, h, color);
  return 1;
}

// void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
static int cmd_fillroundrect(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto w = get_param_int(argc, params, arg++, 0);
  auto h = get_param_int(argc, params, arg++, 0);
  auto radius = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.fillRoundRect(x0, y0, w, h, radius, color);
  return 1;
}

// void fillScreen(uint16_t color);
static int cmd_fillscreen(int argc, slib_par_t *params, var_t *retval) {
  auto color = get_param_int(argc, params, 0, SSD1306_BLACK);
  display.fillScreen(color);
  return 1;
}

// void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
static int cmd_filltriangle(int argc, slib_par_t *params, var_t *retval) {
  auto arg = 0;
  auto x0 = get_param_int(argc, params, arg++, 0);
  auto y0 = get_param_int(argc, params, arg++, 0);
  auto x1 = get_param_int(argc, params, arg++, 0);
  auto y1 = get_param_int(argc, params, arg++, 0);
  auto x2 = get_param_int(argc, params, arg++, 0);
  auto y2 = get_param_int(argc, params, arg++, 0);
  auto color = get_param_int(argc, params, arg++, SSD1306_WHITE);
  display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
  return 1;
}

// void invertDisplay(bool i); // invert
static int cmd_invertdisplay(int argc, slib_par_t *params, var_t *retval) {
  auto i = get_param_int(argc, params, 0, 0);
  display.invertDisplay(i);
  return 1;
}

// void print(uint8_t);
static int cmd_print(int argc, slib_par_t *params, var_t *retval) {
  auto str = get_param_str(argc, params, 0, 0);
  display.print(str);
  return 1;
}

// void setRotation(uint8_t r);
static int cmd_setrotation(int argc, slib_par_t *params, var_t *retval) {
  auto r = get_param_int(argc, params, 0, 0);
  display.setRotation(r);
  return 1;
}

// void setCursor(int16_t x, int16_t y) {
static int cmd_setcursor(int argc, slib_par_t *params, var_t *retval) {
  auto x = get_param_int(argc, params, 0, 0);
  auto y = get_param_int(argc, params, 1, 0);
  display.setCursor(x, y);
  return 1;
}

// void setTextSize(uint8_t s);
static int cmd_settextsize(int argc, slib_par_t *params, var_t *retval) {
   auto s = get_param_int(argc, params, 0, 0);
   display.setTextSize(s);
  return 1;
}

// void setTextWrap(bool w) { wrap = w; }
static int cmd_settextwrap(int argc, slib_par_t *params, var_t *retval) {
  auto w = get_param_int(argc, params, 0, 0);
  display.setTextWrap(w);
  return 1;
}

static int cmd_scrollleft(int argc, slib_par_t *params, var_t *retval) {
  auto start = get_param_int(argc, params, 0, 0);
  auto stop = get_param_int(argc, params, 1, 0);
  display.startscrollleft(start, stop);
  return 1;
}

static int cmd_scrollright(int argc, slib_par_t *params, var_t *retval) {
  auto start = get_param_int(argc, params, 0, 0);
  auto stop = get_param_int(argc, params, 1, 0);
  display.startscrollright(start, stop);
  return 1;
}

static int cmd_stopscroll(int argc, slib_par_t *params, var_t *retval) {
  display.stopscroll();
  return 1;
}

static int cmd_gettextsize(int argc, slib_par_t *params, var_t *retval) {
  auto str = get_param_str(argc, params, 0, 0);
  uint16_t w, h;
  if (str && str[0] != '\0') {
    int16_t x1, y1;
    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  } else {
    w = 0;
    h = 0;
  }
  map_init(retval);
  v_setint(map_add_var(retval, "width", 0), w);
  v_setint(map_add_var(retval, "height", 0), h);
  return 1;
}

FuncSpec lib_proc[] = {
  {0, 0, "INIT", cmd_init},
  {0, 0, "CLEAR", cmd_cleardisplay},
  {1, 1, "DIM", cmd_dim},
  {0, 0, "FLUSH", cmd_display},
  {6, 6, "DRAWCHAR", cmd_drawchar},
  {3, 4, "DRAWCIRCLE", cmd_drawcircle},
  {4, 5, "DRAWLINE", cmd_drawline},
  {2, 3, "DRAWPIXEL", cmd_drawpixel},
  {4, 5, "DRAWRECT", cmd_drawrect},
  {5, 6, "DRAWROUNDRECT", cmd_drawroundrect},
  {6, 7, "DRAWTRIANGLE", cmd_drawtriangle},
  {3, 4, "FILLCIRCLE", cmd_fillcircle},
  {5, 5, "FILLRECT", cmd_fillrect},
  {5, 6, "FILLROUNDRECT", cmd_fillroundrect},
  {0, 1, "FILLSCREEN", cmd_fillscreen},
  {6, 7, "FILLTRIANGLE", cmd_filltriangle},
  {1, 1, "INVERTDISPLAY", cmd_invertdisplay},
  {1, 1, "PRINT", cmd_print},
  {1, 1, "SETROTATION", cmd_setrotation},
  {2, 2, "SETCURSOR", cmd_setcursor},
  {1, 1, "SETTEXTSIZE", cmd_settextsize},
  {1, 1, "SETTEXTWRAP", cmd_settextwrap},
  {2, 2, "SCROLLRIGHT", cmd_scrollright},
  {2, 2, "SCROLLLEFT", cmd_scrollleft},
  {0, 0, "STOPSCROLL", cmd_stopscroll},
};

static int ssd1306_proc_count() {
  return (sizeof(lib_proc) / sizeof(lib_proc[0]));
}

static int ssd1306_proc_getname(int index, char *proc_name) {
  int result;
  if (index < ssd1306_proc_count()) {
    strcpy(proc_name, lib_proc[index]._name);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int ssd1306_proc_exec(int index, int argc, slib_par_t *params, var_t *retval) {
  int result;
  if (index >= 0 && index < ssd1306_proc_count()) {
    if (argc < lib_proc[index]._min || argc > lib_proc[index]._max) {
      if (lib_proc[index]._min == lib_proc[index]._max) {
        error(retval, lib_proc[index]._name, lib_proc[index]._min);
      } else {
        error(retval, lib_proc[index]._name, lib_proc[index]._min, lib_proc[index]._max);
      }
      result = 0;
    } else {
      result = lib_proc[index]._command(argc, params, retval);
    }
  } else {
    error(retval, "FUNC index error");
    result = 0;
  }
  return result;
}

static FuncSpec lib_func[] = {
  {1, 1, "GETTEXTSIZE", cmd_gettextsize}
};

static int ssd1306_func_count(void) {
  return (sizeof(lib_func) / sizeof(lib_func[0]));
}

static int ssd1306_func_getname(int index, char *func_name) {
  int result;
  if (index < ssd1306_func_count()) {
    strcpy(func_name, lib_func[index]._name);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int ssd1306_func_exec(int index, int argc, slib_par_t *params, var_t *retval) {
  int result;
  if (index >= 0 && index < ssd1306_func_count()) {
    if (argc < lib_func[index]._min || argc > lib_func[index]._max) {
      if (lib_func[index]._min == lib_func[index]._max) {
        error(retval, lib_func[index]._name, lib_func[index]._min);
      } else {
        error(retval, lib_func[index]._name, lib_func[index]._min, lib_func[index]._max);
      }
      result = 0;
    } else {
      result = lib_func[index]._command(argc, params, retval);
    }
  } else {
    error(retval, "FUNC index error");
    result = 0;
  }
  return result;
}

static ModuleConfig ssd1306Module = {
  ._func_exec = ssd1306_func_exec,
  ._func_count = ssd1306_func_count,
  ._func_getname = ssd1306_func_getname,
  ._proc_exec = ssd1306_proc_exec,
  ._proc_count = ssd1306_proc_count,
  ._proc_getname = ssd1306_proc_getname,
  ._free = nullptr
};

ModuleConfig *get_ssd1306_module() {
  return &ssd1306Module;
}
