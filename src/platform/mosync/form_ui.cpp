// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ma.h>

#include <MAUtil/String.h>
#include <MAUtil/Vector.h>

#include "config.h"
#include "common/sys.h"
#include "common/blib_ui.h"

#include "platform/mosync/controller.h"
#include "platform/mosync/utils.h"
#include "platform/mosync/form_ui.h"

extern Controller *controller;
Form *form;

//
// ListModel
//
ListModel::ListModel(const char *items, var_t *v) :
  focusIndex(-1) {
  create(items, v);
}

void ListModel::clear() {
  Vector_each(String*, it, list) {
    delete (*it);
  }
  list.clear();
  focusIndex = -1;
}

void ListModel::create(const char *items, var_t *v) {
  if (v && v->type == V_ARRAY) {
    fromArray(items, v);
  } else {
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
            strcasecmp((const char *)v->v.p.ptr, s->c_str()) == 0) {
          focusIndex = item_index;
        }
        item_index++;
      }
    }
  }
}

// construct from an array of values
void ListModel::fromArray(const char *caption, var_t *v) {
  for (int i = 0; i < v->v.a.size; i++) {
    var_t *el_p = (var_t *)(v->v.a.ptr + sizeof(var_t) * i);
    if (el_p->type == V_STR) {
      list.add(new String((const char *)el_p->v.p.ptr));
      if (caption && strcasecmp((const char *)el_p->v.p.ptr, caption) == 0) {
        focusIndex = i;
      }
    } else if (el_p->type == V_INT) {
      char buff[40];
      sprintf(buff, VAR_INT_FMT, el_p->v.i);
      list.add(new String(buff));
    } else if (el_p->type == V_ARRAY) {
      fromArray(caption, el_p);
    }
  }
}

// return the text at the given index
const char *ListModel::getTextAt(int index) {
  const char *s = 0;
  if (index > -1 && index < list.size()) {
    s = list[index]->c_str();
  }
  return s;
}

// returns the model index corresponding to the given string
int ListModel::getIndex(const char *t) {
  int size = list.size();
  for (int i = 0; i < size; i++) {
    if (!strcasecmp(list[i]->c_str(), t)) {
      return i;
    }
  }
  return -1;
}

//
// Form
//
Form::Form() :
  mode(m_init),
  var(0),
  cmd(0), 
  kbHandle(false),
  prevX(0),
  prevY(0) {
} 

Form::~Form() {
  logEntered();
  Vector_each(WidgetDataPtr, it, items) {
    delete (*it);
  }
}

// setup the widget
void Form::setupWidget(WidgetDataPtr widgetData) { 
  items.add(widgetData);

  IFormWidget *widget = widgetData->widget;
  const char *caption = widget->getText();

  int textW = 0;
  int textH = 0;
  if (caption) {
    MAExtent extent = maGetTextSize(caption);
    textW = EXTENT_X(extent);
    textH = EXTENT_Y(extent);
  }

  if (widget->getW() <= 0 && caption != NULL) {
    widget->setW(textW + BN_W);
  }

  if (widget->getH() <= 0 || widget->getH() < (textH + widgetData->padding())) {
    widget->setH(textH + widgetData->padding());
  }

  if (widget->getX() < 0) {
    widget->setX((prevX - widget->getX()) + 1);
  }

  if (widget->getY() < 0) {
    widget->setY((prevY - widget->getY()) + 1);
  }

  prevX = widget->getX() + widget->getW();
  prevY = widget->getY() + widget->getH();
}

bool Form::execute() {
  switch (code_peek()) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    cmd = -1;
    var = 0;
    break;
  default:
    if (code_isvar()) {
      var = code_getvarptr();
      cmd = -1;
    } else {
      var_t variable;
      v_init(&variable);
      eval(&variable);
      cmd = v_getint(&variable);
      v_free(&variable);
      var = 0;

      // apply any configuration options
      switch (cmd) {
      case 0:
        // close the form
        return true;
      case 1:
        // turn on keyboard mode
        kbHandle = true;
        return false;
      default:
        break;
      }
    }
    break;
  };
  
  // apply any variable changes onto attached widgets
  if (controller->isRunning()) {
    Vector_each(WidgetDataPtr, it, items) {
      (*it)->updateGui();
    }
  }

  // pump system messages until there is a widget callback
  mode = m_active;

  // clear the keyboard when keyboard mode is active
  if (kbHandle) {
    dev_clrkb();
  }

  // process events
  while (controller->isRunning() && mode == m_active) {
    controller->processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_EXIT_ANY);
    if (kbHandle && keymap_kbhit()) {
      int key = keymap_kbpeek();
      if (key != SB_KEY_MK_PUSH && key != SB_KEY_MK_RELEASE) {
        // avoid exiting on "mouse" keyboard keys
        break;
      }
    }
  }

  // return whether to close the form
  return controller->isBreak();
}

void Form::invoke(WidgetDataPtr widgetData) {
  mode = m_selected;
  if (var) {
    // array type cannot be used in program select statement
    if (widgetData->var->type == V_ARRAY) {
      v_zerostr(var);
    } else {
      // set the form variable from the widget var
      v_set(var, widgetData->var);
    }
  }
}

//
// WidgetData
//
WidgetData::WidgetData(ControlType type, var_t *var) :
  widget(NULL),
  var(var),
  type(type) {
  orig.ptr = 0;
  orig.i = 0;
}

WidgetData::~WidgetData() {
  delete widget;
}

// convert a basic array into a String
void WidgetData::arrayToString(String &s, var_t *v) {
  for (int i = 0; i < v->v.a.size; i++) {
    var_t *el_p = (var_t *)(v->v.a.ptr + sizeof(var_t) * i);
    if (el_p->type == V_STR) {
      const char *p = (const char *)el_p->v.p.ptr;
      s.append(p, strlen(p));
      s.append("\n", 1);
    } else if (el_p->type == V_INT) {
      char buff[40];
      sprintf(buff, VAR_INT_FMT "\n", el_p->v.i);
      s.append(buff, strlen(buff));
    } else if (el_p->type == V_ARRAY) {
      arrayToString(s, el_p);
    }
  }
}

void WidgetData::setupWidget(IFormWidget *widget) {
  this->widget = widget;
  this->widget->setListener(this);

  if (form == NULL) {
    form = new Form();
  }

  form->setupWidget(this);
  
  // setup the originating variable
  updateVarFlag();
}

// update the smallbasic variable
void WidgetData::updateVarFlag() {
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

// callback for the widget info called when the widget has been invoked
void WidgetData::buttonClicked(const char *action) {
  logEntered();
  if (controller->isRunning()) {
    if (!updateGui()) {
      transferData();
    }
    form->invoke(this);
  }
}

// set basic string variable to widget state when the variable has changed
bool WidgetData::updateGui() {
  ListModel *model;
  bool updated = false;

  if (var->type == V_INT && var->v.i != orig.i) {
    // update list control with new int variable
    if (type == ctrl_listbox) {
      model = (ListModel *)widget->getList();
      model->selected(var->v.i);
      updated = true;
    }
  } else if (var->type == V_ARRAY && var->v.p.ptr != orig.ptr) {
    // update list control with new array variable
    String s;

    switch (type) {
    case ctrl_listbox:
      model = (ListModel *)widget->getList();
      model->clear();
      model->create(0, var);
      updated = true;
      break;

    case ctrl_label:
    case ctrl_text:
      arrayToString(s, var);
      widget->setText(s.c_str());
      updated = true;
      break;

    default:
      break;
    }
  } else if (var->type == V_STR && orig.ptr != var->v.p.ptr) {
    // update list control with new string variable
    switch (type) {
    case ctrl_listbox:
      model = (ListModel *)widget->getList();
      if (strchr((const char *)var->v.p.ptr, '|')) {
        // create a new list of items
        model->clear();
        model->create((const char *)var->v.p.ptr, 0);
      } else {
        int selection = model->getIndex((const char *)var->v.p.ptr);
        if (selection != -1) {
          model->selected(selection);
        }
      }
      updated = true;
      break;

    case ctrl_label:
    case ctrl_text:
    case ctrl_button:
      widget->setText((const char *)var->v.p.ptr);
      updated = true;
      break;

    default:
      break;
    }
  }
  if (updated) {
    updateVarFlag();
  }
  return updated;
}

// transfer the widget state onto the assiciated variable
void WidgetData::transferData() {
  logEntered();
  const char *s;
  ListModel *model;

  switch (type) {
  case ctrl_button:
  case ctrl_text:
    s = widget->getText();
    if (s && s[0]) {
      v_setstr(var, s);
    } else {
      v_zerostr(var);
    }
    break;
    
  case ctrl_listbox:
    model = (ListModel *)widget->getList();
    const char *s = model->getTextAt(model->selected());
    if (s) {
      v_setstr(var, s);
    }
    break;

  case ctrl_exit_link:
  case ctrl_exit_button:
    controller->buttonClicked((const char *)var->v.p.ptr);
    brun_break();
    break;
    
  default:
    break;
  }
}

C_LINKAGE_BEGIN 

// destroy the form
void ui_reset() {
  if (form != NULL) {
    delete form;
    form = NULL;
  }
}

//
// BUTTON x, y, w, h, variable, caption [,type]
//
void cmd_button() {
  var_int_t x, y, w, h;
  var_t *var = 0;
  char *caption = 0;
  char *type = 0;

  if (par_massget("IIIIPSs", &x, &y, &w, &h, &var, &caption, &type) != -1) {
    IFormWidget *widget = NULL;
    WidgetData *wd = NULL;
    if (type) {
      if (strcasecmp("button", type) == 0) {
        wd = new WidgetData(ctrl_button, var);
        widget = controller->output->createButton(caption, x, y, w, h);
      } else if (strcasecmp("exit_button", type) == 0) {
        wd = new WidgetData(ctrl_exit_button, var);
        widget = controller->output->createButton(caption, x, y, w, h);
      } else if (strcasecmp("label", type) == 0) {
        wd = new WidgetData(ctrl_label, var);
        widget = controller->output->createLabel(caption, x, y, w, h);
      } else if (strcasecmp("link", type) == 0) {
        wd = new WidgetData(ctrl_link, var);
        widget = controller->output->createLink(caption, x, y, w, h);
      } else if (strcasecmp("exit_link", type) == 0) {
        wd = new WidgetData(ctrl_exit_link, var);
        widget = controller->output->createLink(caption, x, y, w, h);
      } else if (strcasecmp("listbox", type) == 0 || 
                 strcasecmp("list", type) == 0) {
        ListModel *model = new ListModel(caption, var);
        wd = new WidgetData(ctrl_listbox, var);
        widget = controller->output->createList(model, x, y, w, h);
      } else if (strcasecmp("choice", type) == 0 || 
                 strcasecmp("dropdown", type) == 0) {
        ListModel *model = new ListModel(caption, var);
        wd = new WidgetData(ctrl_listbox, var);
        widget = controller->output->createList(model, x, y, w, h);
      } else {
        ui_reset();
        rt_raise("UI: UNKNOWN BUTTON TYPE: %s", type);
      }
    } else {
      wd = new WidgetData(ctrl_button, var);
      widget = controller->output->createButton(caption, x, y, w, h);
    }
    if (widget) {
      wd->setupWidget(widget);
    }
  }

  pfree2(caption, type);
}

//
// TEXT x, y, w, h, variable
// When DOFORM returns the variable contains the user entered value
//
void cmd_text() {
  var_int_t x, y, w, h;
  var_t *var = 0;

  if (-1 != par_massget("IIIIP", &x, &y, &w, &h, &var)) {
    WidgetData *wd = new WidgetData(ctrl_text, var);
    IFormWidget *widget = controller->output->createLineInput(NULL, 0, x, y, w, h);
    wd->setupWidget(widget);
  }
}

//
// DOFORM [FLAG|VAR] - executes the form
//
void cmd_doform() {
  if (!form) {
    rt_raise("UI: FORM NOT READY");
  } else if (form->execute()) {
    ui_reset();
  }
}

C_LINKAGE_END
