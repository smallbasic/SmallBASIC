//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "common/sys.h"
#include "common/var.h"
#include "common/kw.h"
#include "common/pproc.h"
#include "common/device.h"
#include "common/smbas.h"
#include "common/keymap.h"

#include <fltk3/Browser.h>
#include <fltk3/Button.h>
#include <fltk3/CheckButton.h>
#include <fltk3/Choice.h>
#include <fltk3/Group.h>
#include <fltk3/Input.h>
#include <fltk3/Item.h>
#include <fltk3/RadioButton.h>
#include <fltk3/Rectangle.h>
#include <fltk3/StringList.h>
#include <fltk3/draw.h>
#include <fltk3/events.h>
#include <fltk3/run.h>

#include "MainWindow.h"
#include "StringLib.h"

extern "C" {
#include "common/blib_ui.h"
} 

extern MainWindow *wnd;

struct Form : public Group {
  Form(int x1, int x2, int y1, int y2) : Group(x1, x2, y1, y2) {
    this->cmd = 0;
    this->var = 0;
    this->kb_handle = false;
  } 
  ~Form() {
  }
  void draw();                  // avoid drawing over the tab-bar
  var_t *var;                   // form variable contains the value of the event widget
  int cmd;                      // doform argument by value
  bool kb_handle;               // whether doform returns on a keyboard event
};

Form *form = 0;

// control types available using the BUTTON command
enum ControlType {
  ctrl_button,
  ctrl_radio,
  ctrl_check,
  ctrl_text,
  ctrl_label,
  ctrl_listbox,
  ctrl_dropdown
};

enum Mode { m_reset, m_init, m_active, m_selected } mode = m_reset;

// whether a widget event has fired
bool form_event() {
  return mode == m_selected;
}

int prev_x;
int prev_y;

// width and height fudge factors for when button w+h specified as -1
#define BN_W  16
#define BN_H   8
#define RAD_W 22
#define RAD_H  0

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
    case V_ARRAY:orig.ptr = var->v.a.ptr;
      break;
    case V_INT:orig.i = var->v.i;
      break;
    default:orig.i = 0;
    }
  }
};

// implements abstract StringList as a list of strings
struct DropListModel:StringList {
  strlib::List list;
  int focus_index;
  
  DropListModel(const char *items, var_t *v) : StringList() {
    focus_index = -1;
    
    if (v && v->type == V_ARRAY) {
      fromArray(items, v);
      return;
    }
    // construct from a string like "Easy|Medium|Hard" 
    int item_index = 0;
    int len = items ? strlen(items) : 0;
    for (int i = 0; i < len; i++) {
      const char *c = strchr(items + i, '|');
      int end_index = c ? c - items : len;
      if (end_index > 0) {
        String *s = new String(items + i, end_index - i);
        list.add(s);
        i = end_index;
        if (v != 0 && v->type == V_STR && v->v.p.ptr &&
            strcasecmp((const char *)v->v.p.ptr, s->toString()) == 0) {
          focus_index = item_index;
        }
        item_index++;
      }
    }
  }

  virtual ~DropListModel() {
    list.removeAll();
  }

  // construct from an array of values
  void fromArray(const char *caption, var_t *v) {
    for (int i = 0; i < v->v.a.size; i++) {
      var_t *el_p = (var_t *) (v->v.a.ptr + sizeof(var_t) * i);
      if (el_p->type == V_STR) {
        list.add((const char *)el_p->v.p.ptr);
        if (caption && strcasecmp((const char *)el_p->v.p.ptr, caption) == 0) {
          focus_index = i;
        }
      } else if (el_p->type == V_INT) {
        char buff[40];
        sprintf(buff, VAR_INT_FMT, el_p->v.i);
        list.add(buff);
      } else if (el_p->type == V_ARRAY) {
        fromArray(caption, el_p);
      }
    }
  }

  // return the number of elements
  int children(const Menu *) {
    return list.length();
  }

  // return the label at the given index
  const char *label(const Menu *, int index) {
    return getElementAt(index)->c_str();
  }

  String *getElementAt(int index) {
    return (String *) list.get(index);
  }

  // returns the index corresponding to the given string
  int getPosition(const char *t) {
    int size = list.length();
    for (int i = 0; i < size; i++) {
      if (!strcasecmp(((String *) list.get(i))->c_str(), t)) {
        return i;
      }
    }
    return -1;
  }
};

void Form::draw() {
  int numchildren = children();
  Rectangle r(w(), h());
  push_clip(r);
  for (int n = 0; n < numchildren; n++) {
    Widget & w = *child(n);
    draw_child(w);
    draw_outside_label(w);
  }
  pop_clip();
}

// convert a basic array into a std::string
void array_to_string(String &s, var_t *v) {
  for (int i = 0; i < v->v.a.size; i++) {
    var_t *el_p = (var_t *) (v->v.a.ptr + sizeof(var_t) * i);
    if (el_p->type == V_STR) {
      s.append((const char *)el_p->v.p.ptr);
      s.append("\n");
    } else if (el_p->type == V_INT) {
      char buff[40];
      sprintf(buff, VAR_INT_FMT "\n", el_p->v.i);
      s.append(buff);
    } else if (el_p->type == V_ARRAY) {
      array_to_string(s, el_p);
    }
  }
}

// set basic string variable to widget state when the variable has changed
bool update_gui(Widget *w, WidgetInfo *inf) {
  Choice *dropdown;
  Browser *listbox;
  DropListModel *model;

  if (inf->var->type == V_INT && inf->var->v.i != inf->orig.i) {
    // update list control with new int variable
    switch (inf->type) {
    case ctrl_dropdown:
      ((Choice *) w)->value(inf->var->v.i);
      return true;

    case ctrl_listbox:
      ((Browser *) w)->select(inf->var->v.i);
      return true;

    default:
      return false;
    }
  }

  if (inf->var->type == V_ARRAY && inf->var->v.p.ptr != inf->orig.ptr) {
    // update list control with new array variable
    String s;

    switch (inf->type) {
    case ctrl_dropdown:
      delete((Choice *) w)->list();
      ((Choice *) w)->list(new DropListModel(0, inf->var));
      w->layout();
      return true;

    case ctrl_listbox:
      delete((Browser *) w)->list();
      ((Browser *) w)->list(new DropListModel(0, inf->var));
      ((Browser *) w)->xposition(0);
      ((Browser *) w)->yposition(0);
      ((Browser *) w)->select(0);
      w->layout();
      return true;

    case ctrl_label:
      array_to_string(s, inf->var);
      w->copy_label(s.c_str());
      break;

    case ctrl_text:
      array_to_string(s, inf->var);
      ((Input *) w)->text(s.c_str());
      break;

    default:
      return false;
    }
  }

  if (inf->var->type == V_STR && inf->orig.ptr != inf->var->v.p.ptr) {
    // update list control with new string variable
    switch (inf->type) {
    case ctrl_dropdown:
      dropdown = (Choice *) w;
      model = (DropListModel *) dropdown->list();
      if (strchr((const char *)inf->var->v.p.ptr, '|')) {
        // create a new list of items
        delete model;
        model = new DropListModel((const char *)inf->var->v.p.ptr, 0);
        dropdown->list(model);
      } else {
        // select one of the existing list items
        int selection = model->getPosition((const char *)inf->var->v.p.ptr);
        if (selection != -1) {
          dropdown->value(selection);
        }
      }
      break;

    case ctrl_listbox:
      listbox = (Browser *) w;
      model = (DropListModel *) listbox->list();
      if (strchr((const char *)inf->var->v.p.ptr, '|')) {
        // create a new list of items
        delete model;
        model = new DropListModel((const char *)inf->var->v.p.ptr, 0);
        listbox->list(model);
      } else {
        int selection = model->getPosition((const char *)inf->var->v.p.ptr);
        if (selection != -1) {
          listbox->value(selection);
        }
      }
      break;

    case ctrl_check:
    case ctrl_radio:
      ((CheckButton *) w)->value(!strcasecmp((const char *)inf->var->v.p.ptr, w->label()));
      break;

    case ctrl_label:
      w->copy_label((const char *)inf->var->v.p.ptr);
      break;

    case ctrl_text:
      ((Input *) w)->text((const char *)inf->var->v.p.ptr);
      break;

    case ctrl_button:
      w->copy_label((const char *)inf->var->v.p.ptr);
      break;

    default:
      break;
    }
    return true;
  }
  return false;
}

// synchronise basic variable and widget state
void transfer_data(Widget *w, WidgetInfo *inf) {
  Choice *dropdown;
  Browser *listbox;
  DropListModel *model;

  if (update_gui(w, inf)) {
    inf->update_var_flag();
    return;
  }
  // set widget state to basic variable
  switch (inf->type) {
  case ctrl_check:
    if (((CheckButton *) w)->value()) {
      v_setstr(inf->var, w->label());
    } else {
      v_zerostr(inf->var);
    }
    break;

  case ctrl_radio:
    if (((RadioButton *) w)->value() && w->label() != 0) {
      v_setstr(inf->var, w->label());
    } else if (!inf->is_group_radio) {
      // reset radio variable for radio that is not part of a group
      v_zerostr(inf->var);
    }
    break;

  case ctrl_text:
    if (((Input *) w)->text()) {
      v_setstr(inf->var, ((Input *) w)->text());
    } else {
      v_zerostr(inf->var);
    }
    break;

  case ctrl_dropdown:
    dropdown = (Choice *) w;
    model = (DropListModel *) dropdown->list();
    if (dropdown->value() != -1) {
      String *s = model->getElementAt(dropdown->value());
      if (s) {
        v_setstr(inf->var, s->c_str());
      }
    }
    break;

  case ctrl_listbox:
    listbox = (Browser *) w;
    model = (DropListModel *) listbox->list();
    if (listbox->value() != -1) {
      String *s = model->getElementAt(listbox->value());
      if (s) {
        v_setstr(inf->var, s->c_str());
      }
    }
    break;

  case ctrl_button:
    // update the basic variable with the button label
    v_setstr(inf->var, w->label());
    break;

  default:
    break;
  }

  // only update the gui when the variable is changed in basic code
  inf->update_var_flag();
}

// find the radio group of the given variable from within the parent
Group *findRadioGroup(Group *parent, var_t *v) {
  Group *radioGroup = 0;
  int n = parent->children();
  for (int i = 0; i < n && !radioGroup; i++) {
    Widget *w = parent->child(i);
    if (!w->user_data()) {
      radioGroup = findRadioGroup((Group *) w, v);
    } else {
      WidgetInfo *inf = (WidgetInfo *) w->user_data();
      if (inf->type == ctrl_radio &&
          inf->var->type == V_STR && (inf->var == v || inf->var->v.p.ptr == v->v.p.ptr)) {
        // another ctrl_radio is linked to the same variable
        inf->is_group_radio = true;
        radioGroup = parent;
      }
    }
  }
  return radioGroup;
}

// radio control's belong to the same group when they share
// a common basic variable
void update_radio_group(WidgetInfo *radioInf, RadioButton *radio) {
  var_t *v = radioInf->var;

  if (v == 0 || v->type != V_STR) {
    return;
  }

  Group *radioGroup = findRadioGroup(form, v);

  if (!radioGroup) {
    radioGroup = new Group(0, 0, form->w(), form->h());
    form->add(radioGroup);
  } else {
    radioInf->is_group_radio = true;
  }

  radioGroup->add(radio);
}

void widget_cb(Widget *w, void *v) {
  if (wnd->isRunning()) {
    WidgetInfo *inf = (WidgetInfo *) v;
    transfer_data(w, (WidgetInfo *) v);

    mode = m_selected;

    if (form->var) {
      // array type cannot be used in program select statement
      if (inf->var->type == V_ARRAY) {
        v_zerostr(form->var);
      } else {
        // set the form variable from the widget var
        v_set(form->var, inf->var);
      }
    }
  }
}

void update_widget(Widget *widget, WidgetInfo *inf, Rectangle &rect) {
  if (rect.w() != -1) {
    widget->w(rect.w());
  }

  if (rect.h() != -1) {
    widget->h(rect.h());
  }

  if (rect.x() < 0) {
    rect.x(prev_x - rect.x());
  }

  if (rect.y() < 0) {
    rect.y(prev_y - rect.y());
  }

  prev_x = rect.x() + rect.w();
  prev_y = rect.y() + rect.h();

  widget->x(rect.x());
  widget->y(rect.y());
  widget->callback(widget_cb);
  widget->user_data(inf);

  // allow form init to update widget from basic variable
  inf->orig.ptr = 0;
  inf->orig.i = 0;

  // copy output widget colors
  widget->color(wnd->out->color());
  widget->labelcolor(wnd->out->labelcolor());
  widget->textcolor(wnd->out->labelcolor());
}

void update_button(Widget *widget, WidgetInfo *inf,
                   const char *caption, Rectangle &rect, int def_w, int def_h) {
  if (rect.w() < 0 && caption != 0) {
    rect.w((int)getwidth(caption) + def_w + (-rect.w() - 1));
  }

  if (rect.h() < 0) {
    rect.h((int)(getascent() + getdescent() + def_h + (-rect.h() - 1)));
  }

  update_widget(widget, inf, rect);
  widget->copy_label(caption);
}

// create a new form
void form_create() {
  if (form == 0) {
    wnd->outputGroup->begin();
    form = new Form(wnd->out->x() + 2, wnd->out->y() + 2, wnd->out->w() - 2, wnd->out->h() - 2);
    form->resizable(0);
    wnd->outputGroup->end();
  }
  form->begin();
  mode = m_init;
}

// prepare the form for display
void form_init() {
  if (form) {
    form->resize(2, 2, form->w() + 4, form->h() + 4);
    form->take_focus();
    form->show();
  }
}

// copy all widget fields into variables
void form_update(Group *group) {
  if (group && wnd->isRunning()) {
    int n = group->children();
    for (int i = 0; i < n; i++) {
      Widget *w = group->child(i);
      if (!w->user_data()) {
        form_update((Group *) w);
      } else {
        transfer_data(w, (WidgetInfo *) w->user_data());
      }
    }
  }
}

// close the form
void form_end() {
  if (form != 0) {
    form->end();
  }
}

// destroy the form
C_LINKAGE_BEGIN void ui_reset() {
  if (form != 0) {
    form->hide();
    int n = form->children();
    for (int i = 0; i < n; i++) {
      WidgetInfo *inf = (WidgetInfo *) form->child(i)->user_data();
      delete inf;
    }
    form->clear();
    wnd->outputGroup->remove(form);
    form->parent(0);
    delete form;
    form = 0;

    wnd->out->show();
    wnd->out->redraw();
    wnd->take_focus();
  }
  mode = m_reset;
}

// BUTTON x, y, w, h, variable, caption [,type]
//
void cmd_button() {
  var_int_t x, y, w, h;
  var_t *v = 0;
  char *caption = 0;
  char *type = 0;

  form_create();
  if (-1 != par_massget("IIIIPSs", &x, &y, &w, &h, &v, &caption, &type)) {
    WidgetInfo *inf = new WidgetInfo();
    inf->var = v;
    Rectangle rect(x, y, w, h);

    if (prog_error) {
      return;
    }
    if (type) {
      if (strcasecmp("radio", type) == 0) {
        inf->type = ctrl_radio;
        inf->is_group_radio = false;
        form_end();             // add widget to RadioGroup
        RadioButton *widget = new RadioButton(x, y, w, h);
        update_radio_group(inf, widget);
        update_button(widget, inf, caption, rect, RAD_W, RAD_H);
      } else if (strcasecmp("checkbox", type) == 0 || strcasecmp("check", type) == 0) {
        inf->type = ctrl_check;
        CheckButton *widget = new CheckButton(x, y, w, h);
        update_button(widget, inf, caption, rect, RAD_W, RAD_H);
      } else if (strcasecmp("button", type) == 0) {
        inf->type = ctrl_button;
        Button *widget = new Button(x, y, w, h);
        update_button(widget, inf, caption, rect, BN_W, BN_H);
      } else if (strcasecmp("label", type) == 0) {
        inf->type = ctrl_label;
        Widget *widget = new Widget(x, y, w, h);
        widget->box(FLAT_BOX);
        widget->align(ALIGN_LEFT | ALIGN_INSIDE | ALIGN_CLIP);
        update_button(widget, inf, caption, rect, BN_W, BN_H);
      } else if (strcasecmp("listbox", type) == 0 || strcasecmp("list", type) == 0) {
        inf->type = ctrl_listbox;
        Browser *widget = new Browser(x, y, w, h);
        DropListModel *model = new DropListModel(caption, v);
        widget->list(model);
        widget->box(BORDER_BOX);
        if (model->focus_index != -1) {
          widget->value(model->focus_index);
        }
        update_widget(widget, inf, rect);
        widget->when(WHEN_RELEASE_ALWAYS);
      } else if (strcasecmp("dropdown", type) == 0 || strcasecmp("choice", type) == 0) {
        inf->type = ctrl_dropdown;
        Choice *widget = new Choice(x, y, w, h);
        DropListModel *model = new DropListModel(caption, v);
        widget->list(model);
        widget->box(BORDER_BOX);
        if (model->focus_index != -1) {
          widget->value(model->focus_index);
        }
        update_widget(widget, inf, rect);
      } else {
        ui_reset();
        rt_raise("UI: UNKNOWN BUTTON TYPE: %s", type);
      }
    } else {
      inf->type = ctrl_button;
      Button *widget = new Button(x, y, w, h);
      update_button(widget, inf, caption, rect, BN_W, BN_H);
    }
  }

  form_end();
  pfree2(caption, type);
}

// TEXT x, y, w, h, variable
// When DOFORM returns the variable contains the user entered value
void cmd_text() {
  var_int_t x, y, w, h;
  var_t *v = 0;

  if (-1 != par_massget("IIIIP", &x, &y, &w, &h, &v)) {
    form_create();
    Input *widget = new Input(x, y, w, h);
    widget->box(BORDER_BOX);
    Rectangle rect(x, y, w, h);
    WidgetInfo *inf = new WidgetInfo();
    inf->var = v;
    inf->type = ctrl_text;
    update_widget(widget, inf, rect);
    if (rect.h() > (getascent() + getdescent() + BN_H)) {
      widget->type(Input::MULTILINE | Input::WORDWRAP);
    }
    form->end();
  }
}

// DOFORM [FLAG|VAR]
// Executes the form
void cmd_doform() {
  if (form == 0) {
    rt_raise("UI: FORM NOT READY");
    return;
  } else if (mode == m_init) {
    form_init();
  }

  switch (code_peek()) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    form->cmd = -1;
    form->var = 0;
    break;
  default:
    if (code_isvar()) {
      form->var = code_getvarptr();
      form->cmd = -1;
    } else {
      var_t var;
      v_init(&var);
      eval(&var);
      form->cmd = v_getint(&var);
      form->var = 0;
      v_free(&var);

      // apply any configuration options
      switch (form->cmd) {
      case 1:
        form->kb_handle = true;
        return;
      default:
        break;
      }
    }
    break;
  };

  form_update(form);

  if (!form->cmd) {
    ui_reset();
  } else if (wnd->penMode) {
    mode = m_active;
    fltk::wait();
  } else {
    // pump system messages until there is a widget callback
    mode = m_active;

    if (form->kb_handle) {
      dev_clrkb();
    }
    while (wnd->isRunning() && mode == m_active) {
      fltk::wait();

      if (form->kb_handle && keymap_kbhit()) {
        break;
      }
    }
    form_update(form);
  }
}

C_LINKAGE_END
