// This file is part of SmallBASIC
//
// Window object
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2014 Chris Warren-Smith.

#include "common/sys.h"
#include "common/var.h"
#include "common/var_map.h"
#include "common/pproc.h"
#include "common/device.h"
#include "lib/maapi.h"
#include "ui/system.h"

extern System *g_system;

#define WINDOW_SCREEN1  "graphicsScreen1"
#define WINDOW_SCREEN2  "graphicsScreen2"
#define WINDOW_SCREEN3  "textScreen"
#define WINDOW_ALERT    "alert"
#define WINDOW_MENU     "menu"
#define WINDOW_MESSAGE  "message"
#define WINDOW_VKEYPAD  "showKeypad"
#define WINDOW_INSET    "insetTextScreen"

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

void cmd_window_alert(var_s *self) {
  StringList *items = get_items();
  if (!prog_error && items->size() > 0) {
    const char *message = items->size() > 0 ? (*items)[0]->c_str() : "";
    const char *title   = items->size() > 1 ? (*items)[1]->c_str() : "";
    const char *button1 = items->size() > 2 ? (*items)[2]->c_str() : "";
    const char *button2 = items->size() > 3 ? (*items)[3]->c_str() : "";
    const char *button3 = items->size() > 4 ? (*items)[4]->c_str() : "";
    maAlert(title, message, button1, button2, button3);
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

  var_p_t v_select_screen1 = map_add_var(var, WINDOW_SCREEN1, 0);
  v_select_screen1->type = V_FUNC;
  v_select_screen1->v.fn.self = var;
  v_select_screen1->v.fn.cb = cmd_window_select_screen1;

  var_p_t v_select_screen2 = map_add_var(var, WINDOW_SCREEN2, 0);
  v_select_screen2->type = V_FUNC;
  v_select_screen2->v.fn.self = var;
  v_select_screen2->v.fn.cb = cmd_window_select_screen2;

  var_p_t v_select_screen3 = map_add_var(var, WINDOW_SCREEN3, 0);
  v_select_screen3->type = V_FUNC;
  v_select_screen3->v.fn.self = var;
  v_select_screen3->v.fn.cb = cmd_window_select_screen3;

  var_p_t v_alert = map_add_var(var, WINDOW_ALERT, 0);
  v_alert->type = V_FUNC;
  v_alert->v.fn.self = var;
  v_alert->v.fn.cb = cmd_window_alert;

  var_p_t v_message = map_add_var(var, WINDOW_MESSAGE, 0);
  v_message->type = V_FUNC;
  v_message->v.fn.self = var;
  v_message->v.fn.cb = cmd_window_message;

  var_p_t v_menu = map_add_var(var, WINDOW_MENU, 0);
  v_menu->type = V_FUNC;
  v_menu->v.fn.self = var;
  v_menu->v.fn.cb = cmd_window_menu;

  var_p_t v_vkeypad = map_add_var(var, WINDOW_VKEYPAD, 0);
  v_vkeypad->type = V_FUNC;
  v_vkeypad->v.fn.self = var;
  v_vkeypad->v.fn.cb = cmd_window_show_keypad;

  var_p_t v_inset = map_add_var(var, WINDOW_INSET, 0);
  v_inset->type = V_FUNC;
  v_inset->v.fn.self = var;
  v_inset->v.fn.cb = cmd_window_inset;
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
