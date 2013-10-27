// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#if defined(_FLTK) || defined(_TIZEN)
  #include "platform/common/maapi.h"
#else
  #include <maapi.h>
#endif

#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"

#include "platform/common/utils.h"
#include "platform/common/form_ui.h"

Form *form;

//
// ListModel
//
ListModel::ListModel(const char *items, var_t *v) :
  _focusIndex(-1) {
  create(items, v);
}

void ListModel::clear() {
  _list.removeAll();
  _focusIndex = -1;
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
        strlib::String *s = new strlib::String(items + i, end_index - i);
        _list.add(s);
        i = end_index;
        if (v != 0 && v->type == V_STR && v->v.p.ptr &&
            strcasecmp((const char *)v->v.p.ptr, s->c_str()) == 0) {
          _focusIndex = item_index;
        }
        item_index++;
      }
    }
  }
  if (_focusIndex == -1) {
    _focusIndex = 0;
  }
}

// construct from an array of values
void ListModel::fromArray(const char *caption, var_t *v) {
  for (int i = 0; i < v->v.a.size; i++) {
    var_t *el_p = (var_t *)(v->v.a.ptr + sizeof(var_t) * i);
    if (el_p->type == V_STR) {
      _list.add(new strlib::String((const char *)el_p->v.p.ptr));
      if (caption && strcasecmp((const char *)el_p->v.p.ptr, caption) == 0) {
        _focusIndex = i;
      }
    } else if (el_p->type == V_INT) {
      char buff[40];
      sprintf(buff, VAR_INT_FMT, el_p->v.i);
      _list.add(new strlib::String(buff));
    } else if (el_p->type == V_ARRAY) {
      fromArray(caption, el_p);
    }
  }
}

// return the text at the given index
const char *ListModel::getTextAt(int index) {
  const char *s = 0;
  if (index > -1 && index < _list.size()) {
    s = _list[index]->c_str();
  }
  return s;
}

// returns the model index corresponding to the given string
int ListModel::getIndex(const char *t) {
  int size = _list.size();
  for (int i = 0; i < size; i++) {
    if (!strcasecmp(_list[i]->c_str(), t)) {
      return i;
    }
  }
  return -1;
}

//
// Form
//
Form::Form() :
  _mode(m_init),
  _var(0),
  _cmd(0), 
  _kbHandle(false),
  _prevX(0),
  _prevY(0) {
} 

Form::~Form() {
  logEntered();
}

// setup the widget
void Form::setupWidget(WidgetDataPtr widgetData) { 
  _items.add(widgetData);

  IFormWidget *widget = widgetData->_widget;
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
    widget->setX((_prevX - widget->getX()) + 1);
  }

  if (widget->getY() < 0) {
    widget->setY((_prevY - widget->getY()) + 1);
  }

  _prevX = widget->getX() + widget->getW();
  _prevY = widget->getY() + widget->getH();
}

bool Form::execute() {
  switch (code_peek()) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    _cmd = -1;
    _var = 0;
    break;
  default:
    if (code_isvar()) {
      _var = code_getvarptr();
      _cmd = -1;
    } else {
      var_t variable;
      v_init(&variable);
      eval(&variable);
      _cmd = v_getint(&variable);
      v_free(&variable);
      _var = 0;

      // apply any configuration options
      switch (_cmd) {
      case 0:
        // close the form
        return true;
      case 1:
        // turn on keyboard mode
        _kbHandle = true;
        return false;
      default:
        break;
      }
    }
    break;
  };
  
  // apply any variable changes onto attached widgets
  if (form_ui::isRunning()) {
    List_each(WidgetDataPtr, it, _items) {
      (*it)->updateGui();
    }
  }

  // pump system messages until there is a widget callback
  _mode = m_active;

  // clear the keyboard when keyboard mode is active
  if (_kbHandle) {
    dev_clrkb();
  }

  // process events
  while (form_ui::isRunning() && _mode == m_active) {
    form_ui::processEvents();
    if (_kbHandle && keymap_kbhit()) {
      int key = keymap_kbpeek();
      if (key != SB_KEY_MK_PUSH && key != SB_KEY_MK_RELEASE) {
        // avoid exiting on "mouse" keyboard keys
        break;
      }
    }
  }

  // return whether to close the form
  return form_ui::isBreak();
}

void Form::invoke(WidgetDataPtr widgetData) {
  _mode = m_selected;
  if (_var) {
    // array type cannot be used in program select statement
    if (widgetData->_var->type == V_ARRAY) {
      v_zerostr(_var);
    } else {
      // set the form variable from the widget var
      v_set(_var, widgetData->_var);
    }
  }
}

//
// WidgetData
//
WidgetData::WidgetData(ControlType type, var_t *var) :
  _widget(NULL),
  _var(var),
  _type(type) {
  orig.ptr = 0;
  orig.i = 0;
}

WidgetData::~WidgetData() {
  delete _widget;
}

// convert a basic array into a String
void WidgetData::arrayToString(strlib::String &s, var_t *v) {
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
  this->_widget = widget;
  this->_widget->setListener(this);

  if (form == NULL) {
    form = new Form();
  }

  updateGui();
  updateVarFlag();

  form->setupWidget(this);
}

// update the smallbasic variable
void WidgetData::updateVarFlag() {
  switch (_var->type) {
  case V_STR:
    orig.ptr = _var->v.p.ptr;
    break;
  case V_ARRAY:
    orig.ptr = _var->v.a.ptr;
    break;
  case V_INT:
    orig.i = _var->v.i;
    break;
  default:
    orig.i = 0;
    break;
  }
}

// callback for the widget info called when the widget has been invoked
void WidgetData::buttonClicked(const char *action) {
  logEntered();
  if (form_ui::isRunning()) {
    if (!updateGui()) {
      transferData();
    }
    form->invoke(this);
  }
}

// set basic string variable to widget state when the variable has changed
bool WidgetData::updateGui() {
  logEntered();
  ListModel *model;
  bool updated = false;

  if (_var->type == V_INT && _var->v.i != orig.i) {
    // update list control with new int variable
    if (_type == ctrl_listbox) {
      model = (ListModel *)_widget->getList();
      model->selected(_var->v.i);
      updated = true;
    }
  } else if (_var->type == V_ARRAY && _var->v.p.ptr != orig.ptr) {
    // update list control with new array variable
    strlib::String s;

    switch (_type) {
    case ctrl_listbox:
      model = (ListModel *)_widget->getList();
      model->clear();
      model->create(0, _var);
      updated = true;
      break;

    case ctrl_label:
    case ctrl_text:
      arrayToString(s, _var);
      _widget->setText(s.c_str());
      updated = true;
      break;

    default:
      break;
    }
  } else if (_var->type == V_STR && orig.ptr != _var->v.p.ptr) {
    // update list control with new string variable
    switch (_type) {
    case ctrl_listbox:
      model = (ListModel *)_widget->getList();
      if (strchr((const char *)_var->v.p.ptr, '|')) {
        // create a new list of items
        model->clear();
        model->create((const char *)_var->v.p.ptr, 0);
      } else {
        int selection = model->getIndex((const char *)_var->v.p.ptr);
        if (selection != -1) {
          model->selected(selection);
        }
      }
      updated = true;
      break;

    case ctrl_label:
    case ctrl_text:
    case ctrl_button:
      _widget->setText((const char *)_var->v.p.ptr);
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

// transfer the widget state onto the associated variable
void WidgetData::transferData() {
  logEntered();
  const char *s;
  ListModel *model;

  switch (_type) {
  case ctrl_button:
  case ctrl_text:
    s = _widget->getText();
    if (s && s[0]) {
      v_setstr(_var, s);
    } else {
      v_zerostr(_var);
    }
    break;
    
  case ctrl_listbox:
    model = (ListModel *)_widget->getList();
    s = model->getTextAt(model->selected());
    if (s && s[0]) {
      v_setstr(_var, s);
    }
    break;

  case ctrl_exit_link:
  case ctrl_exit_button:
    form_ui::buttonClicked((const char *)_var->v.p.ptr);
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
        widget = form_ui::getOutput()->createButton(caption, x, y, w, h);
      } else if (strcasecmp("exit_button", type) == 0) {
        wd = new WidgetData(ctrl_exit_button, var);
        widget = form_ui::getOutput()->createButton(caption, x, y, w, h);
      } else if (strcasecmp("label", type) == 0) {
        wd = new WidgetData(ctrl_label, var);
        widget = form_ui::getOutput()->createLabel(caption, x, y, w, h);
      } else if (strcasecmp("link", type) == 0) {
        wd = new WidgetData(ctrl_link, var);
        widget = form_ui::getOutput()->createLink(caption, x, y, w, h);
      } else if (strcasecmp("exit_link", type) == 0) {
        wd = new WidgetData(ctrl_exit_link, var);
        widget = form_ui::getOutput()->createLink(caption, x, y, w, h);
      } else if (strcasecmp("listbox", type) == 0 || 
                 strcasecmp("list", type) == 0) {
        ListModel *model = new ListModel(caption, var);
        wd = new WidgetData(ctrl_listbox, var);
        widget = form_ui::getOutput()->createListBox(model, x, y, w, h);
      } else if (strcasecmp("choice", type) == 0 || 
                 strcasecmp("dropdown", type) == 0) {
        ListModel *model = new ListModel(caption, var);
        wd = new WidgetData(ctrl_listbox, var);
        widget = form_ui::getOutput()->createDropList(model, x, y, w, h);
      } else {
        ui_reset();
        rt_raise("UI: UNKNOWN BUTTON TYPE: %s", type);
      }
    } else {
      wd = new WidgetData(ctrl_button, var);
      widget = form_ui::getOutput()->createButton(caption, x, y, w, h);
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
    IFormWidget *widget = form_ui::getOutput()->createLineInput(NULL, 0, x, y, w, h);
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
