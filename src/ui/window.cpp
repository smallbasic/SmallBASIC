// This file is part of SmallBASIC
//
// Window object
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2014 Chris Warren-Smith.

#include "common/sys.h"
#include "common/pproc.h"
#include "lib/maapi.h"
#include "ui/system.h"
#include "ui/textedit.h"

extern System *g_system;

#define WINDOW_SCREEN1  "graphicsScreen1"
#define WINDOW_SCREEN2  "graphicsScreen2"
#define WINDOW_SCREEN3  "textScreen"
#define WINDOW_ALERT    "alert"
#define WINDOW_ASK      "ask"
#define WINDOW_ASK_RTN  "answer"
#define WINDOW_MENU     "menu"
#define WINDOW_MESSAGE  "message"
#define WINDOW_VKEYPAD  "showKeypad"
#define WINDOW_INSET    "insetTextScreen"
#define WINDOW_SETFONT  "setFont"
#define WINDOW_SETSIZE  "setSize"
#define WINDOW_THEME    "theme"

// returns the next set of string variable arguments as a String list
StringList *get_items() {
  var_t arg;
  bool done = false;
  StringList *items = new StringList();
  do {
    switch (code_peek()) {
    case kwTYPE_LINE:
    case kwTYPE_EOC:
    case kwTYPE_LEVEL_END:
      done = true;
      break;
    case kwTYPE_SEP:
      code_skipnext();
      if (code_getnext() != ',') {
        err_missing_comma();
      }
      break;
    default:
      v_init(&arg);
      eval(&arg);
      if (arg.type == V_STR && !prog_error) {
        items->add(new String(arg.v.p.ptr));
      }
      v_free(&arg);
      break;
    }
  } while (!done && !prog_error);
  return items;
}

void cmd_window_select_screen1(var_s *self) {
  g_system->getOutput()->selectScreen(USER_SCREEN1);
}

void cmd_window_select_screen2(var_s *self) {
  g_system->getOutput()->selectScreen(USER_SCREEN2);
}

void cmd_window_select_screen3(var_s *self) {
  g_system->getOutput()->selectScreen(TEXT_SCREEN);
}

void cmd_window_show_keypad(var_s *self) {
  maShowVirtualKeyboard();
}

void cmd_window_inset(var_s *self) {
  var_int_t x, y, w, h;
  par_massget("IIII", &x, &y, &w, &h);
  if (!prog_error) {
    if (x < 0 || x > 100 ||
        y < 0 || y > 100 ||
        w < 1 || x + w > 100 ||
        h < 1 || y + h > 100) {
      err_out_of_range();
    } else {
      g_system->getOutput()->insetTextScreen(x, y, w, h);
    }
  }
}

void cmd_window_set_font(var_s *self) {
  var_num_t size;
  var_int_t bold, italic;
  char *unit = NULL;
  par_massget("FSII", &size, &unit, &bold, &italic);
  if (unit != NULL && strcmp(unit, "em") == 0) {
    size *= g_system->getOutput()->getFontSize();
  }
  g_system->getOutput()->setFont(size, bold, italic);
  pfree(unit);
}

void cmd_window_set_size(var_s *self) {
  var_int_t width, height;
  par_massget("II", &width, &height);
  g_system->setWindowSize(width, height);
}

void cmd_window_get_theme(var_s *self) {
  EditTheme theme;
  theme.setId(g_themeId);
  v_init(self);
  map_init(self);
  map_set_int(self, "background", -theme._background);
  map_set_int(self, "text1", -theme._color);
  map_set_int(self, "text2", -theme._syntax_text);
  map_set_int(self, "text3", -theme._syntax_comments);
  map_set_int(self, "text4", -theme._syntax_command);
  map_set_int(self, "text5", -theme._syntax_statement);
  map_set_int(self, "text6", -theme._syntax_digit);
}

void cmd_window_alert(var_s *self) {
  StringList *items = get_items();
  if (!prog_error && items->size() > 0) {
    const char *message = items->size() > 0 ? (*items)[0]->c_str() : "";
    const char *title   = items->size() > 1 ? (*items)[1]->c_str() : "";
    g_system->alert(title, message);
  }
  delete items;
}

void cmd_window_ask(var_s *self) {
  StringList *items = get_items();
  if (!prog_error && items->size() > 0) {
    const char *message = items->size() > 0 ? (*items)[0]->c_str() : "";
    const char *title   = items->size() > 1 ? (*items)[1]->c_str() : "";
    map_set_int(self, WINDOW_ASK_RTN, g_system->ask(title, message, false));
  }
  delete items;
}

void cmd_window_menu(var_s *self) {
  StringList *items = get_items();
  if (!prog_error && items->size() > 0) {
    g_system->optionsBox(items);
  }
  delete items;
}

void cmd_window_message(var_s *self) {
  var_t arg;
  v_init(&arg);
  eval(&arg);
  if (arg.type == V_STR && !prog_error) {
    g_system->getOutput()->setStatus(arg.v.p.ptr);
  }
  v_free(&arg);
}

extern "C" void v_create_window(var_p_t var) {
  map_init(var);
  v_create_func(var, WINDOW_SCREEN1, cmd_window_select_screen1);
  v_create_func(var, WINDOW_SCREEN2, cmd_window_select_screen2);
  v_create_func(var, WINDOW_SCREEN3, cmd_window_select_screen3);
  v_create_func(var, WINDOW_ALERT, cmd_window_alert);
  v_create_func(var, WINDOW_ASK, cmd_window_ask);
  v_create_func(var, WINDOW_MESSAGE, cmd_window_message);
  v_create_func(var, WINDOW_MENU, cmd_window_menu);
  v_create_func(var, WINDOW_VKEYPAD, cmd_window_show_keypad);
  v_create_func(var, WINDOW_INSET, cmd_window_inset);
  v_create_func(var, WINDOW_SETFONT, cmd_window_set_font);
  v_create_func(var, WINDOW_SETSIZE, cmd_window_set_size);
  v_create_func(var, WINDOW_THEME, cmd_window_get_theme);
}

extern "C" void dev_show_page() {
  byte code = code_peek();
  switch (code) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    break;
  default:
    g_system->getOutput()->setAutoflush(par_getint());
    break;
  }
  g_system->getOutput()->redraw();
}
