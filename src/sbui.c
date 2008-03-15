/*
*	User Interface Lib
*
*	Nicholas Christopoulos
*/

#define	UI_TITLE		1
#define	UI_FRAME		2
#define	UI_CLOSEBUTTON	4

// type of the node
typed enum { ui_window, ui_button, ui_text, ui_ibox } ui_type_t;

// window data
typedef struct {
  char *title;
  int tw, th;
  int flags;
  int x, y, w, h;
} ui_window_t;

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

// input box
typedef struct {
  char *text;
  int flags;
  int x, y, w, h;
} ui_ibox_t;

// ui node
typedef struct {
  ui_type_t type;
  int state;
  union {
    ui_window_t window;
    ui_button_t button;
    ui_text_t text;
    ui_ibox_t ibox;
  } args;
} ui_node_t;

#define	UI_MAX_ELEMENTS		128

static ui_node_t ui_list[UI_MAX_ELEMENTS];
static int ui_count;
static int ui_nx, ui_ny;
static int ui_curel;

/* -------------------------------------------------------------------------- */

/*
*	close/reset ui data
*/
void ui_reset()
{
  int i;

  for (i = 0; i < ui_count; i++) {
    switch (ui_list[i].type) {
    case ui_button:
      tmp_free(ui_list[i].args.button.text);
      break;
    case ui_window:
      tmp_free(ui_list[i].args.window.prompt);
      break;
    case ui_text:
      tmp_free(ui_list[i].args.text.text);
      break;
    case ui_ibox:
      tmp_free(ui_list[i].args.ibox.text);
      // copy text to var
      break;
    }
  }

  ui_curel = 0;
  ui_count = 0;
  ui_nx = ui_ny = 0;
}

/*
*	setup the window
*/
void ui_add_window(const char *title, int x, int y, int w, int h, int frame)
{
  ui_node_t *node;

  if (ui_count == UI_MAX_ELEMENTS)
    rt_raise("UI: TOO MANY ELEMENTS");
  node = &ui_list[ui_count];
  node->type = ui_window;

  // 
  node->args.window.x = x;
  node->args.window.y = y;
  node->args.window.w = w;
  node->args.window.h = h;

  node->args.window.title = tmp_alloc(strlen(title) + 1);
  strcpy(node->args.window.title, title);
  node->args.window.flags = (frame) ? UI_FRAME : 0;
  node->args.window.flags |= UI_CLOSEBUTTON;

  node->args.window.tw = dev_textwidth(title);
  node->args.window.th = dev_textheight(title);

  // 
  ui_ny = y + node->args.window.th + ((frame) ? 1 : 0);
  ui_count++;
}

/*
*	add a button
*/
void ui_add_button(const char *text, int x, int y)
{
  ui_node_t *node;

  if (ui_count == UI_MAX_ELEMENTS)
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

  node->args.button.w = dev_textwidth(text) + dev_textwidth("X");
  node->args.button.h = dev_textheight(text) + dev_textheight("X") / 2;
  // 
  ui_ny = y + node->args.button.h;
  ui_count++;
}

/*
*	add a prompt
*/
void ui_add_prompt(const char *text, int x, int y)
{
  ui_node_t *node;

  if (ui_count == UI_MAX_ELEMENTS)
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

  node->args.button.w = dev_textwidth(text) + dev_textwidth("X");
  node->args.button.h = dev_textheight(text) + dev_textheight("X") / 2;

  // 
  ui_ny = y + node->args.button.h;
  ui_count++;
}

/*
*	add a text
*/
void ui_add_text(const char *text, int x, int y)
{
  ui_node_t *node;

  if (ui_count == UI_MAX_ELEMENTS)
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

  node->args.text.w = dev_textwidth(text) + dev_textwidth("X");
  node->args.text.h = dev_textheight(text) + dev_textheight("X") / 2;

  // 
  ui_ny = y + node->args.text.h;
  ui_count++;
}

/*
*	add an input box
*/
void ui_add_ibox(const char *text, int x, int y)
{
  ui_node_t *node;

  if (ui_count == UI_MAX_ELEMENTS)
    rt_raise("UI: TOO MANY ELEMENTS");
  node = &ui_list[ui_count];
  node->type = ui_ibox;

  // 
  if (x == -1)
    node->args.ibox.x = ui_nx;
  else
    node->args.ibox.x = x;
  if (y == -1)
    node->args.ibox.y = ui_ny;
  else
    node->args.ibox.y = y;

  node->args.ibox.text = tmp_alloc(strlen(text) + 1);
  strcpy(node->args.ibox.text, text);
  node->args.ibox.flags = 0;

  node->args.ibox.w = dev_textwidth(text) + dev_textwidth("X");
  node->args.ibox.h = dev_textheight(text) + dev_textheight("X") / 2;

  // 
  ui_ny = y + node->args.ibox.h;
  ui_count++;
}

/* -------------------------------------------------------------------------- */

/*
*	draw window
*/
void ui_draw_window(ui_window_t * args)
{
}

/*
*	draw button
*/
void ui_draw_button(ui_button_t * args)
{
}

/*
*	draw text
*/
void ui_draw_text(ui_text_t * args)
{
}

/*
*	draw input box
*/
void ui_draw_ibox(ui_ibox_t * args)
{
}

/*
*	draw form
*/
void ui_draw_all()
{
  for (i = 0; i < ui_count; i++) {
    switch (ui_list[i].type) {
    case ui_button:
      ui_draw_button(&ui_list[i].args.button);
      break;
    case ui_window:
      ui_draw_window(&ui_list[i].args.window);
      break;
    case ui_text:
      ui_draw_text(&ui_list[i].args.text);
      break;
    case ui_ibox:
      ui_draw_ibox(&ui_list[i].args.ibox);
      // copy text to var
      break;
    }
  }
}

/* -------------------------------------------------------------------------- */

/*
*	execute it
*/
void ui_exec()
{
  int key;

  ui_draw_all();
  do {
    key = dev_getch();
    switch (key) {
    case -2:
    case -1:
      return;                   // BREAK
    case '\t':                 // next element
      ui_curel++;
      if (ui_curel >= ui_count)
        ui_curel = 0;
      break;
    };
  } while (key != 27);
}
