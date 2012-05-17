// This file is part of SmallBASIC
//
// User Interface Lib
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/var.h"
#include "common/kw.h"
#include "common/pproc.h"
#include "common/blib_ui.h"
#include "common/device.h"
#include "common/smbas.h"

// check if driver implements the UI api
#ifndef IMPL_UI

#define UI_TITLE    1
#define UI_FRAME    2
#define UI_CLOSEBUTTON  4

// type of the node
typedef enum {
  ui_button, ui_text
} ui_type_t;

// button/prompt data
typedef struct {
  char *text;
  int flags;
  int x, y, w, h;
} ui_button_t;

// simple text
typedef struct {
  char *text;
  int flags;
  int x, y, w, h;
} ui_text_t;

// ui node
typedef struct {
  ui_type_t type;
  int state;
  union {
    ui_button_t button;
    ui_text_t text;
  } args;
} ui_node_t;

#if defined(OS_LIMITED)
#define UI_MAX_ELEMENTS   8
#else
#define UI_MAX_ELEMENTS   256
#endif

static ui_node_t ui_list[UI_MAX_ELEMENTS];
static int ui_count;
static int ui_nx, ui_ny;
static int ui_curel;

/**
 * close/reset ui data
 */
void ui_reset() {
  int i;

  for (i = 0; i < ui_count; i++) {
    switch (ui_list[i].type) {
    case ui_button:
      tmp_free(ui_list[i].args.button.text);
      break;
    case ui_text:
      tmp_free(ui_list[i].args.text.text);
      break;
    }
  }

  ui_curel = 0;
  ui_count = 0;
  ui_nx = ui_ny = 0;
}

/**
 *
 */
void ui_fix_cs2(int *x, int *y) {
  if (opt_uipos) {              // in chars
    if (os_graphics) {
      *x *= dev_textwidth("0");
      *y *= dev_textheight("0");
    }
  }
}

/**
 * add a button
 */
void ui_add_button(const char *text, int x, int y) {
  ui_node_t *node;

  ui_fix_cs2(&x, &y);

  if (ui_count == UI_MAX_ELEMENTS
    )
    rt_raise("UI: TOO MANY ELEMENTS");
  node = &ui_list[ui_count];
  node->type = ui_button;

  //
  if (x == -1)
    node->args.button.x = ui_nx;
  else
    node->args.button.x = x;
  if (y == -1)
    node->args.button.y = ui_ny;
  else
    node->args.button.y = y;

  node->args.button.text = tmp_alloc(strlen(text) + 1);
  strcpy(node->args.button.text, text);
  node->args.button.flags = UI_FRAME;

  node->args.button.w = dev_textwidth(text);
  node->args.button.h = dev_textheight(text);
  //
  ui_ny = y + node->args.button.h;
  ui_count++;
}

/**
 * add a prompt
 */
void ui_add_prompt(const char *text, int x, int y) {
  ui_node_t *node;

  ui_fix_cs2(&x, &y);

  if (ui_count == UI_MAX_ELEMENTS
    )
    rt_raise("UI: TOO MANY ELEMENTS");
  node = &ui_list[ui_count];
  node->type = ui_button;

  //
  if (x == -1)
    node->args.button.x = ui_nx;
  else
    node->args.button.x = x;
  if (y == -1)
    node->args.button.y = ui_ny;
  else
    node->args.button.y = y;

  node->args.button.text = tmp_alloc(strlen(text) + 1);
  strcpy(node->args.button.text, text);
  node->args.button.flags = 0;

  node->args.button.w = dev_textwidth(text);
  node->args.button.h = dev_textheight(text);

  //
  ui_ny = y + node->args.button.h;
  ui_count++;
}

/**
 * add a text
 */
void ui_add_text(const char *text, int x, int y) {
  ui_node_t *node;

  ui_fix_cs2(&x, &y);

  if (ui_count == UI_MAX_ELEMENTS
    )
    rt_raise("UI: TOO MANY ELEMENTS");
  node = &ui_list[ui_count];
  node->type = ui_text;

  //
  if (x == -1)
    node->args.text.x = ui_nx;
  else
    node->args.text.x = x;
  if (y == -1)
    node->args.text.y = ui_ny;
  else
    node->args.text.y = y;

  node->args.text.text = tmp_alloc(strlen(text) + 1);
  strcpy(node->args.text.text, text);
  node->args.text.flags = 0;

  node->args.text.w = dev_textwidth(text);
  node->args.text.h = dev_textheight(text);

  //
  ui_ny = y + node->args.text.h;
  ui_count++;
}

/* -------------------------------------------------------------------------- */

/**
 * draw button
 */
void ui_draw_button(ui_button_t * args) {
  int x1, y1, x2, y2;

  if (!os_graphics) {
    x1 = args->x - 1;
    y1 = args->y - 1;
    x2 = args->x + args->w;
    y2 = args->y + args->h;

    dev_rect(x1, y1, x2, y2, 0);
    dev_setxy(args->x, args->y);
    dev_print(args->text);
    return;
  }

  x1 = args->x - 2;
  y1 = args->y - 2;
  x2 = args->x + args->w;
  y2 = args->y + args->h;
  x1 -= 2;
  x2 += 2;

  dev_setcolor(15);
  dev_rect(x1, y1, x2, y2, 1);
  dev_setcolor(0);
  dev_rect(x1, y1, x2, y2, 0);
  dev_line(x1 + 1, y2 + 1, x2 + 1, y2 + 1);
  dev_line(x2 + 1, y1 + 1, x2 + 1, y2 + 1);
  dev_settextcolor(0, 15);
  dev_setxy(args->x, args->y);
  dev_print(args->text);
}

/**
 * draw text
 */
void ui_draw_text(ui_text_t * args) {
  dev_setxy(args->x, args->y);
  dev_print(args->text);
}

/**
 * draw form
 */
void ui_draw_all() {
  int i;

  for (i = 0; i < ui_count; i++) {
    switch (ui_list[i].type) {
    case ui_button:
      ui_draw_button(&ui_list[i].args.button);
      break;
    case ui_text:
      ui_draw_text(&ui_list[i].args.text);
      break;
    }
  }
}

/**
 * execute it
 */
void ui_exec() {
  long int key;

  ui_draw_all();
  do {
    key = dev_getch();
    switch (key) {
    case -2:
    case -1:
    case SB_KEY_BREAK:         // for FRANKLIN_EBM
      return;                   // BREAK
    case '\t':                 // next element
      ui_curel++;
      if (ui_curel >= ui_count)
        ui_curel = 0;
      break;
    };
  } while (key != 27 && key != SB_KEY_ALT('x'));
}

//
//      BUTTON x, y, text
//
void cmd_button() {
  int32 x, y;
  char *s = NULL;

  par_massget("IIS", &x, &y, &s);
  if (!prog_error)
    ui_add_button(s, x, y);

  // cleanup
  if (s)
    pfree(s);
}

//
//      TEXT x, y, var$
//
void cmd_text() {
  int32 x, y;
  char *s = NULL;

  par_massget("IIS", &x, &y, &s);
  if (!prog_error)
    ui_add_text(s, x, y);

  // cleanup
  if (s)
    pfree(s);
}

//
//      DOFORM
//
void cmd_doform() {
  ui_exec();
  ui_reset();
}

#endif
