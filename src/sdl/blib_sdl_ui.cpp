// $Id$
//
// Copyright(C) 2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Implements GUI commands BUTTON, TEXT and DOFORM
// using guichan - see http://guichan.sourceforge.net/wiki/index.php

#include "sys.h"
#include "var.h"
#include "kw.h"
#include "pproc.h"
#include "device.h"
#include "smbas.h"
#include "osd.h"

#ifdef IMPL_UI

#include <map>
#include <vector>
#include <string>
#include <utility>
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "SDL.h"
#include "SDL_Image.h"

#include "fixedfont.xpm"
const char *font_chars =
  " abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789.,!?-+/():;%&`'*#=[]\"\'";

// SDL surface in dev_sdl.c
extern SDL_Surface *screen;
extern "C" void ui_reset();

enum { m_unset, m_init, m_modeless, m_modal, m_closed } mode = m_unset;
int modeless_x;
int modeless_y;
int modeless_w;
int modeless_h;
int cursor;

using namespace gcn;

// Guichan SDL components
gcn::SDLInput *input;          // Input driver
gcn::SDLGraphics *graphics;    // Graphics driver
gcn::Gui *gui;                 // A Gui object - binds it all together
gcn::ImageFont *font;          // A font

// width and height fudge factors for when button w+h specified as -1
#define BN_W  16
#define BN_H   2
#define RAD_W 22
#define RAD_H  0

enum ControlType {
  ctrl_button,
  ctrl_radio,
  ctrl_check,
  ctrl_text,
  ctrl_label,
  ctrl_listbox,
  ctrl_dropdown
};

struct WidgetInfo {
  ControlType type;
  var_t *var;
  bool is_group_radio;

  // startup value used to check if
  // exec has altered a bound variable
  union {
    long i;
    byte *ptr;
  } orig;

  void update_var_flag() {
    switch (var->type) {
    case V_STR:
      orig.ptr = var->v.p.ptr;
      break;
    case V_ARRAY:
      orig.ptr = var->v.a.ptr;
      break;
    case V_INT:
      orig.i = var->v.i;
      break;
    default:
      orig.i = 0;
    }
  }
};

// a ScrollArea with content cleanup
struct ScrollBox : gcn::ScrollArea {
  ScrollBox(gcn::Widget * w) : ScrollArea(w) {
    setBorderSize(1);
  } 
  ~ScrollBox() {
    delete getContent();
  }
};

// implements abstract gcn::ListModel as a list of strings
struct DropListModel : gcn::ListModel {
  std::vector < std::string > list;
  int focus_index;

  DropListModel(const char *items, var_t * v) {
    focus_index = -1;

    if (v && v->type == V_ARRAY) {
      fromArray(items, v);
      return;
    }
    // construct from a string like "Easy|Medium|Hard" int item_index = 0;
    int len = items ? strlen(items) : 0;
    for (int i = 0; i < len; i++) {
      char *c = strchr(items + i, '|');
      int end_index = c ? c - items : len;
      if (end_index > 0) {
        std::string s(items + i, end_index - i);
        list.push_back(s);
        i = end_index;
        if (v != 0 && v->type == V_STR && v->v.p.ptr &&
            strcasecmp((const char *)v->v.p.ptr, s.c_str()) == 0) {
          focus_index = item_index;
        }
        item_index++;
      }
    }
  }

  // construct from an array of values
  void fromArray(const char *caption, var_t * v) {
    for (int i = 0; i < v->v.a.size; i++) {
      var_t *el_p = (var_t *) (v->v.a.ptr + sizeof(var_t) * i);
      if (el_p->type == V_STR) {
        list.push_back((const char *)el_p->v.p.ptr);
        if (strcasecmp((const char *)el_p->v.p.ptr, caption) == 0) {
          focus_index = i;
        }
      }
      else if (el_p->type == V_INT) {
        char buff[40];
        sprintf(buff, "%d", el_p->v.i);
        list.push_back(buff);
      }
      else if (el_p->type == V_ARRAY) {
        fromArray(caption, el_p);
      }
    }
  }

  int getNumberOfElements() {
    return list.size();
  }

  std::string getElementAt(int i) {
    return list.at(i);
  }

  // returns the index corresponding to the given string
  int getPosition(const char *t) {
    int size = list.size();
    for (int i = 0; i < size; i++) {
      if (!strcasecmp(list.at(i).c_str(), t)) {
        return i;
      }
    }
    return -1;
  }

};

// load the font image from a static xpixmap structure
struct FontImageLoader:gcn::SDLImageLoader {
  Image *load(const std::string & filename, bool convertToDisplayFormat = true) {
    SDL_Surface *loadedSurface = NULL;
    if (filename == "fixed_font.xpm") {
      loadedSurface = IMG_ReadXPMFromArray((char **)fixedfont_xpm);
    } // else test and load other embedded fonts if (loadedSurface == NULL) {
      throw std::string("Unable to load font data");
    }

    SDL_Surface *surface = convertToStandardFormat(loadedSurface);
    SDL_FreeSurface(loadedSurface);

    if (surface == NULL) {
      throw std::string("Not enough memory to load font");
    }
    return new SDLImage(surface, true);
  }
};

FontImageLoader *imageLoader = 0;

// convert a basic array into a std::string
void array_to_string(std::string & s, var_t * v)
{
  for (int i = 0; i < v->v.a.size; i++) {
    var_t *el_p = (var_t *) (v->v.a.ptr + sizeof(var_t) * i);
    if (el_p->type == V_STR) {
      s.append((const char *)el_p->v.p.ptr);
      s.append("\n");
    }
    else if (el_p->type == V_INT) {
      char buff[40];
      sprintf(buff, "%d\n", el_p->v.i);
      s.append(buff);
    }
    else if (el_p->type == V_ARRAY) {
      array_to_string(s, el_p);
    }
  }
}

// map iterator
typedef std::map < gcn::Widget *, WidgetInfo * >::iterator WI;

struct Form:gcn::Container, gcn::ActionListener {
  void update();
  void action(const ActionEvent & actionEvent);
  void add_button(gcn::Widget * widget, WidgetInfo * inf,
                  const char *caption, Rectangle & rect, int def_w, int def_h);
  void add_widget(gcn::Widget * widget, WidgetInfo * inf, Rectangle & rect);
  bool update_gui(gcn::Widget * w, WidgetInfo * inf);
  void transfer_data(gcn::Widget * w, WidgetInfo * inf);
  bool set_radio_group(var_t * v, RadioButton * radio);
  std::map < gcn::Widget *, WidgetInfo * >widget_map;
  ~Form();
  Form() : prev_x(0), prev_y(0) {
  } 
  int prev_x, prev_y;
};

Form *form = 0;

Form::~Form()
{
  Container::clear();
  for (WI iter = widget_map.begin(); iter != widget_map.end(); iter++) {
    delete iter->first;         // cleanup widget
    delete iter->second;        // cleanup widgetinfo
  }
}

void Form::add_button(gcn::Widget * widget, WidgetInfo * inf,
                      const char *caption, Rectangle & rect, int def_w, int def_h)
{
  if (rect.width == -1 && caption != 0) {
    rect.width = font->getWidth(caption) + def_w;
  }

  if (rect.height == -1) {
    rect.height = font->getHeight() + def_h;
  }

  add_widget(widget, inf, rect);
}

void Form::add_widget(gcn::Widget * widget, WidgetInfo * inf, Rectangle & rect)
{
  if (rect.width != -1) {
    widget->setWidth(rect.width);
  }

  if (rect.height != -1) {
    widget->setHeight(rect.height);
  }

  if (rect.x < 0) {
    rect.x = prev_x - rect.x;
  }

  if (rect.y < 0) {
    rect.y = prev_y - rect.y;
  }

  prev_x = rect.x + rect.width;
  prev_y = rect.y + rect.height;

  widget_map.insert(std::make_pair(widget, inf));
  add(widget, rect.x, rect.y);
  inf->update_var_flag();

  widget->addActionListener(form);
  widget->setFocusable(true);
  widget->requestFocus();
}

void Form::action(const gcn::ActionEvent & actionEvent)
{
  // find the WidgetInfo for the event's widget
  WI iter = widget_map.find(actionEvent.getSource());
  if (iter != widget_map.end()) {
    transfer_data(iter->first, iter->second);
  }

  if (iter->second->type == ctrl_button || mode == m_modeless) {
    // any button type = end modal loop -or- exit modeless
    // loop in cmd_doform() and continue basic statements
    // cmd_doform() can later be re-entered to continue form
    mode = m_closed;
  }

  if (iter->second->type == ctrl_button) {
    // update the basic variable with the button pressed
    v_setstr(iter->second->var, ((Button *) iter->first)->getCaption().c_str());
  }
}

// copy all modeless widget fields into variables
void Form::update()
{
  for (WI iter = widget_map.begin(); iter != widget_map.end(); iter++) {
    transfer_data(iter->first, iter->second);
  }
}

// set basic string variable to widget state
bool Form::update_gui(gcn::Widget * w, WidgetInfo * inf)
{
  DropDown *dropdown;
  ListBox *listbox;
  DropListModel *model;

  if (inf->var->type == V_INT && inf->var->v.i != inf->orig.i) {
    // update list control with new int variable
    switch (inf->type) {
    case ctrl_dropdown:
      ((DropDown *) w)->setSelected(inf->var->v.i);
      return true;

    case ctrl_listbox:
      ((ListBox *) w)->setSelected(inf->var->v.i);
      return true;

    default:
      return false;
    }
  }

  if (inf->var->type == V_ARRAY && inf->var->v.p.ptr != inf->orig.ptr) {
    // update list control with new array variable
    std::string s;

    switch (inf->type) {
    case ctrl_dropdown:
      delete((DropDown *) w)->getListModel();
      ((DropDown *) w)->setListModel(new DropListModel(0, inf->var));
      return true;

    case ctrl_listbox:
      delete((ListBox *) w)->getListModel();
      ((ListBox *) w)->setListModel(new DropListModel(0, inf->var));
      return true;

    case ctrl_label:
      array_to_string(s, inf->var);
      ((Label *) w)->setCaption(s.c_str());
      break;

    case ctrl_text:
      array_to_string(s, inf->var);
      ((TextBox *) ((ScrollBox *) w)->getContent())->setText(s.c_str());
      break;

    default:
      return false;
    }
  }

  if (inf->var->type == V_STR && inf->orig.ptr != inf->var->v.p.ptr) {
    // update list control with new string variable
    switch (inf->type) {
    case ctrl_dropdown:
      dropdown = (DropDown *) w;
      model = (DropListModel *) dropdown->getListModel();
      if (strchr((const char *)inf->var->v.p.ptr, '|')) {
        // create a new list of items
        delete model;
        model = new DropListModel((const char *)inf->var->v.p.ptr, 0);
        dropdown->setListModel(model);
      }
      else {
        // select one of the existing list items
        int selection = model->getPosition((const char *)inf->var->v.p.ptr);
        if (selection != -1) {
          dropdown->setSelected(selection);
        }
      }
      break;

    case ctrl_listbox:
      listbox = (ListBox *) w;
      model = (DropListModel *) listbox->getListModel();
      if (strchr((const char *)inf->var->v.p.ptr, '|')) {
        // create a new list of items
        delete model;
        model = new DropListModel((const char *)inf->var->v.p.ptr, 0);
        listbox->setListModel(model);
      }
      else {
        int selection = model->getPosition((const char *)inf->var->v.p.ptr);
        if (selection != -1) {
          listbox->setSelected(selection);
        }
      }
      break;

    case ctrl_check:
      ((CheckBox *) w)->setMarked(!strcasecmp((const char *)inf->var->v.p.ptr,
                                              ((CheckBox *) w)->getCaption().
                                              c_str()));
      break;

    case ctrl_label:
      ((Label *) w)->setCaption((const char *)inf->var->v.p.ptr);
      break;

    case ctrl_text:
      ((TextBox *) ((ScrollBox *) w)->getContent())->setText((const char *)inf->var->
                                                             v.p.ptr);
      break;
    }
    return true;
  }
  return false;
}

// synchronise basic variable and widget state
void Form::transfer_data(gcn::Widget * w, WidgetInfo * inf)
{
  DropDown *dropdown;
  ListBox *listbox;
  DropListModel *model;

  if (update_gui(w, inf)) {
    inf->update_var_flag();
    return;
  }

  // set widget state to basic variable
  switch (inf->type) {
  case ctrl_check:
    if (((CheckBox *) w)->isMarked()) {
      v_setstr(inf->var, ((CheckBox *) w)->getCaption().c_str());
    }
    else {
      v_zerostr(inf->var);
    }
    break;

  case ctrl_radio:
    if (((RadioButton *) w)->isMarked()) {
      v_setstr(inf->var, ((RadioButton *) w)->getCaption().c_str());
    }
    else if (!inf->is_group_radio) {
      // reset radio variable for radio that is not part of a group
      v_zerostr(inf->var);
    }
    break;

  case ctrl_text:
    v_setstr(inf->var,
             ((TextBox *) ((ScrollBox *) w)->getContent())->getText().c_str());
    break;

  case ctrl_dropdown:
    dropdown = (DropDown *) w;
    model = (DropListModel *) dropdown->getListModel();
    if (dropdown->getSelected() != -1) {
      std::string s = model->getElementAt(dropdown->getSelected());
      v_setstr(inf->var, s.c_str());
    }
    break;

  case ctrl_listbox:
    listbox = (ListBox *) w;
    model = (DropListModel *) listbox->getListModel();
    if (listbox->getSelected() != -1) {
      std::string s = model->getElementAt(listbox->getSelected());
      v_setstr(inf->var, s.c_str());
    }
    break;

  default:
    break;
  }

  // only update the gui when the variable is changed in basic code
  inf->update_var_flag();
}

// radio control's belong to the same group when they share
// a common basic variable
bool Form::set_radio_group(var_t * v, RadioButton * radio)
{
  if (v == 0 || v->type != V_STR) {
    return false;
  }

  for (WI iter = widget_map.begin(); iter != widget_map.end(); iter++) {
    gcn::Widget * widget = iter->first;
    WidgetInfo *inf = iter->second;
    if (inf->type == ctrl_radio &&
        inf->var->type == V_STR &&
        (inf->var == v || inf->var->v.p.ptr == v->v.p.ptr)) {
      // another ctrl_radio is linked to the same variable
      radio->setGroup(((RadioButton *) widget)->getGroup());
      inf->is_group_radio = true;
      return true;
    }
  }
  return false;
}

// initialize and create the Guichan gui
void form_begin()
{
  if (form == 0) {
    form = new Form();
    graphics = new gcn::SDLGraphics();

    // Set the target for the graphics object to be the screen.
    // In other words, we will draw to the screen.
    // Note, any surface will do, it doesn't have to be the screen.
    graphics->setTarget(screen);

    // Set the dimension of the top container to match the screen.
    gui = new gcn::Gui();

    // Set gui to use the SDLGraphics object.
    gui->setGraphics(graphics);

    // Set gui to use the SDLInput object
    input = new gcn::SDLInput();
    gui->setInput(input);

    // Set the top container
    gui->setTop(form);

    imageLoader = new FontImageLoader();

    // The ImageLoader in use is static and must be set to be
    // able to load images
    gcn::Image::setImageLoader(imageLoader);

    try {
      font = new gcn::ImageFont("fixed_font.xpm", font_chars);
      // The global font is static and must be set.
      gcn::Widget::setGlobalFont(font);
    }
    catch(gcn::Exception e) {
      rt_raise("UI: Failed to load font file: %s", e.getMessage().c_str());
    }
    catch(std::string s) {
      rt_raise("UI: Failed to load font file: %s", s.c_str());
    }
    cursor = SDL_ShowCursor(SDL_ENABLE);
  }
}

// poll SDL events
void form_iteration()
{
  try {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_c) {
          if (event.key.keysym.mod & KMOD_CTRL) {
            mode = m_closed;
            brun_break();
            return;
          }
        }
        break;
      case SDL_QUIT:
        mode = m_closed;
        brun_break();
        return;
      }
      input->pushInput(event);  // handle the event in guichan
    }

    gui->logic();               // Let the gui perform it's logic (like handle
                                // input)
    gui->draw();                // Draw the gui
    SDL_Flip(graphics->getTarget());  // Update the screen
    if (mode != m_closed) {
      SDL_WaitEvent(NULL);
    }
  }
  catch(gcn::Exception e) {
    rt_raise("UI: Event failed: %s", e.getMessage().c_str());
    mode = m_closed;
  }
}

extern "C" void ui_reset()
{
  if (form) {
    delete form;
    delete font;
    delete gui;
    delete input;
    delete graphics;
    delete imageLoader;
    form = 0;
    osd_cls();
    SDL_ShowCursor(cursor);
    SDL_Flip(screen);
  }
  mode = m_unset;
}

// BUTTON x, y, w, h, variable, caption [,type]
extern "C" void cmd_button()
{
  int x, y, w, h;
  var_t *v = 0;
  char *caption = 0;
  char *type = 0;

  if (-1 != par_massget("IIIIPSs", &x, &y, &w, &h, &v, &caption, &type)) {
    WidgetInfo *inf = new WidgetInfo();
    inf->var = v;
    Rectangle rect(x, y, w, h);

    form_begin();
    if (prog_error) {
      return;
    }
    if (type) {
      if (strcasecmp("radio", type) == 0) {
        inf->type = ctrl_radio;
        inf->is_group_radio = false;
        RadioButton *widget = new RadioButton();
        widget->setCaption(caption);
        widget->setGroup(caption);
        inf->is_group_radio = form->set_radio_group(v, widget);
        form->add_button(widget, inf, caption, rect, RAD_W, RAD_H);
        form->update_gui(widget, inf);
      }
      else if (strcasecmp("checkbox", type) == 0) {
        inf->type = ctrl_check;
        CheckBox *widget = new CheckBox();
        widget->setCaption(caption);
        form->add_button(widget, inf, caption, rect, RAD_W, RAD_H);
        form->update_gui(widget, inf);
      }
      else if (strcasecmp("button", type) == 0) {
        inf->type = ctrl_button;
        Button *widget = new Button();
        widget->setCaption(caption);
        form->add_button(widget, inf, caption, rect, BN_W, BN_H);
      }
      else if (strcasecmp("label", type) == 0) {
        inf->type = ctrl_label;
        Label *widget = new Label();
        widget->setCaption(caption);
        form->add_button(widget, inf, caption, rect, BN_W, BN_H);
      }
      else if (strcasecmp("listbox", type) == 0) {
        inf->type = ctrl_listbox;
        ListBox *widget = new ListBox();
        DropListModel *model = new DropListModel(caption, v);
        widget->setListModel(model);
        widget->setBorderSize(1);
        if (model->focus_index != -1) {
          widget->setSelected(model->focus_index);
        }
        form->add_widget(widget, inf, rect);
      }
      else if (strcasecmp("dropdown", type) == 0 || strcasecmp("choice", type) == 0) {
        inf->type = ctrl_dropdown;
        DropDown *widget = new DropDown();
        DropListModel *model = new DropListModel(caption, v);
        widget->setListModel(model);
        if (model->focus_index != -1) {
          widget->setSelected(model->focus_index);
        }
        form->add_widget(widget, inf, rect);
      }
      else {
        ui_reset();
        rt_raise("UI: UNKNOWN TYPE: %s", type);
      }
    }
    else {
      inf->type = ctrl_button;
      Button *widget = new Button();
      widget->setCaption(caption);
      form->add_button(widget, inf, caption, rect, BN_W, BN_H);
    }
  }
  pfree2(caption, type);
}

// TEXT x, y, w, h, variable
// When DOFORM returns the variable contains the user entered value
extern "C" void cmd_text()
{
  int x, y, w, h;
  var_t *v = 0;

  if (-1 != par_massget("IIIIP", &x, &y, &w, &h, &v)) {
    form_begin();
    TextBox *widget = new TextBox();
    ScrollBox *scrollBox = new ScrollBox(widget);
    Rectangle rect(x, y, w, h);
    WidgetInfo *inf = new WidgetInfo();
    inf->var = v;
    inf->type = ctrl_text;
    form->add_widget(scrollBox, inf, rect);
    form->update_gui(widget, inf);
  }
}

// DOFORM [x,y,w,h]
//
// Modal syntax:
//   BUTTON ...
//   DOFORM ...
//
// Modeless syntax:
//   DOFORM 'begin modeless form
//   BUTTON ....
//   DOFORM 'continue modeless form
//
extern "C" void cmd_doform()
{
  int x, y, w, h;
  int num_args;

  x = y = w = h = 0;
  num_args = par_massget("iiii", &x, &y, &w, &h);

  if (form == 0) {
    // begin modeless state - m_unset, m_init, m_modeless
    mode = m_init;
    modeless_x = x;
    modeless_y = y;
    modeless_w = w;
    modeless_h = h;
    return;
  }

  if (mode != m_unset) {
    // continue modeless state
    if (form == 0) {
      rt_raise("UI: FORM HAS CLOSED");
      return;
    }

    // set form position in initial iteration
    if (mode == m_init) {
      mode = m_modeless;
      if (num_args == 0) {
        // apply coordinates from inital doform call
        x = modeless_x;
        y = modeless_y;
        w = modeless_w;
        h = modeless_h;
      }
    }
    else {
      // pump system messages until button is clicked
      form->update();
      while (mode == m_modeless) {
        form_iteration();
      }
      mode = m_modeless;
      form->update();
      return;
    }
  }

  switch (num_args) {
  case 0:
  case 2:
  case 4:
    break;
  default:
    ui_reset();
    rt_raise("UI: INVALID FORM ARGUMENTS: %d", num_args);
    return;
  }

  if (w < 1 || x + w > screen->w) {
    w = screen->w - x;
  }
  if (h < 1 || y + h > screen->h) {
    h = screen->h - y;
  }

  form->setDimension(gcn::Rectangle(x, y, w, h));

  if (mode == m_unset) {
    mode = m_modal;
    while (mode == m_modal) {
      form_iteration();
    }
    form->update();
    ui_reset();
  }
}

#endif

// End of "$Id$".
