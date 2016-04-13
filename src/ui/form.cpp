// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "common/sys.h"
#include "common/sbapp.h"
#include "common/keymap.h"
#include "ui/system.h"
#include "ui/textedit.h"

extern System *g_system;
extern FormInput *focusInput;
var_p_t form = NULL;

enum Mode {
  m_init,
  m_active,
  m_selected
} mode = m_init;

void FormInput::clicked(int x, int y, bool pressed) {
  setFocus(true);
  if (!pressed && g_system->isRunning()) {
    if (_onclick) {
      bcip_t ip = prog_ip;
      prog_ip = _onclick;
      cmd_push_args(kwPROC, prog_ip, INVALID_ADDR);
      bc_loop(2);
      prog_ip = ip;
    } else if (form != NULL) {
      if (_exit) {
        const char *text = NULL;
        var_p_t field = getField(form);
        if (field != NULL) {
          var_p_t value = map_get(field, FORM_INPUT_VALUE);
          if (value->type == V_STR) {
            text = value->v.p.ptr;
          }
        }
        g_system->setLoadBreak(text != NULL ? text : getText());
      }
      else {
        selected();
      }
    }
  }
}

void FormInput::selected() {
  if (form != NULL && g_system->isRunning()) {
    setFocus(true);
    updateForm(form);
    if (focusInput != NULL) {
      focusInput->updateField(form);
    }
    mode = m_selected;
    g_system->getOutput()->setDirty();
  }
}

bool FormLineInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  bool result = FormInput::overlaps(pt, scrollX, scrollY);
  if (result) {
    int x = pt.x - (_x + scrollX);
    int charWidth = g_system->getOutput()->getCharWidth();
    int selected = x / charWidth;
    int len = strlen(_buffer);
    if (selected <= len && selected > -1 && selected != _mark) {
      _mark = selected;
      redraw = true;
    }
  }
  return result;
}

void FormDropList::clicked(int x, int y, bool pressed) {
  if (form != NULL && !pressed && g_system->isRunning()) {
    setFocus(true);
    updateForm(form);
    _listActive = !_listActive;
    if (!_listActive) {
      mode = m_selected;
    }
    g_system->getOutput()->setDirty();
  }
}

void FormListBox::clicked(int x, int y, bool pressed) {
  if (form != NULL && !pressed && g_system->isRunning()) {
    setFocus(true);
    if (_activeIndex != -1) {
      optionSelected(_activeIndex + _topIndex);
    }
  }
  FormInput::clicked(x, y, pressed);
}

void cmd_form_close(var_s *self) {
  g_system->getOutput()->removeInputs();
  g_system->getOutput()->resetScroll();
}

void cmd_form_refresh(var_s *self) {
  var_t arg;
  v_init(&arg);
  eval(&arg);
  bool setVars = v_getint(&arg) != 0;
  v_free(&arg);
  g_system->getOutput()->updateInputs(self, setVars);
}

void cmd_form_do_events(var_s *self) {
  // apply any variable changes onto attached widgets
  if (g_system->isRunning()) {
    AnsiWidget *out = g_system->getOutput();

    form = self;

    // pump system messages until there is a widget callback
    mode = m_active;

    // clear the keyboard
    dev_clrkb();

    int charWidth = out->getCharWidth();
    int sw = out->getScreenWidth();

    // process events
    while (g_system->isRunning() && mode == m_active) {
      MAEvent event = g_system->processEvents(true);
      if (event.type == EVENT_TYPE_KEY_PRESSED) {
        if (event.key == SB_KEY_TAB) {
          dev_clrkb();
          focusInput = out->getNextField(focusInput);
          out->setDirty();
        } else if (focusInput != NULL &&
                   event.key != SB_KEY_MENU &&
                   focusInput->edit(event.key, sw, charWidth)) {
          dev_clrkb();
          out->setDirty();
        } else if (event.key == SB_KEY_MK_PUSH || event.key == SB_KEY_MK_RELEASE ||
                   SB_KEY_CTRL(SB_KEY_UP) || SB_KEY_CTRL(SB_KEY_DOWN)) {
          // no exit on mouse events
          dev_clrkb();
        } else {
          break;
        }
      }
    }

    form = NULL;
  }
}

int get_selected_index(var_p_t v_field) {
  var_p_t value = map_get(v_field, FORM_INPUT_INDEX);
  int result = 0;
  if (value == NULL) {
    value = map_add_var(v_field, FORM_INPUT_INDEX, result);
  } else {
    result = v_getint(value);
  }
  return result;
}

FormInput *create_input(var_p_t v_field) {
  int x = map_get_int(v_field, FORM_INPUT_X, -1);
  int y = map_get_int(v_field, FORM_INPUT_Y, -1);
  int w = map_get_int(v_field, FORM_INPUT_W, -1);
  int h = map_get_int(v_field, FORM_INPUT_H, -1);

  const char *label = map_get_str(v_field, FORM_INPUT_LABEL);
  const char *type = map_get_str(v_field, FORM_INPUT_TYPE);

  if (label == NULL) {
    label = "Label";
  }

  var_p_t value = map_get(v_field, FORM_INPUT_VALUE);
  if (value == NULL) {
    value = map_add_var(v_field, FORM_INPUT_VALUE, 0);
    v_setstr(value, label);
  }

  FormInput *widget = NULL;
  if (type != NULL) {
    if (strcasecmp("button", type) == 0) {
      widget = new FormButton(label, x, y, w, h);
    } else if (strcasecmp("label", type) == 0) {
      widget = new FormLabel(label, x, y, w, h);
    } else if (strcasecmp("link", type) == 0) {
      widget = new FormLink(label, x, y, w, h);
    } else if (strcasecmp("tab", type) == 0) {
      widget = new FormTab(label, x, y, w, h);
    } else if (strcasecmp("listbox", type) == 0 ||
               strcasecmp("list", type) == 0) {
      ListModel *model = new ListModel(get_selected_index(v_field), value);
      widget = new FormListBox(model, x, y, w, h);
    } else if (strcasecmp("choice", type) == 0 ||
               strcasecmp("dropdown", type) == 0) {
      ListModel *model = new ListModel(get_selected_index(v_field), value);
      widget = new FormDropList(model, x, y, w, h);
    } else if (strcasecmp("text", type) == 0) {
      int maxSize = map_get_int(v_field, FORM_INPUT_LENGTH, -1);
      int charHeight = g_system->getOutput()->getCharHeight();
      int charWidth = g_system->getOutput()->getCharWidth();
      if (maxSize < 1 || maxSize > 1024) {
        maxSize = 100;
      }
      const char *text = NULL;
      if (value->type == V_STR) {
        text = value->v.p.ptr;
      }
      if (h * 2 >= charHeight) {
        widget = new TextEditInput(text, charWidth, charHeight, x, y, w, h);
      } else {
        widget = new FormLineInput(text, maxSize, false, x, y, w, h);
      }
    } else if (strcasecmp("image", type) == 0) {
      const char *name = map_get_str(v_field, FORM_INPUT_NAME);
      ImageDisplay *image = create_display_image(v_field, name);
      if (image != NULL) {
        widget = new FormImage(image, x, y);
      }
    }
  }
  if (widget == NULL) {
    widget = new FormButton(label, x, y, w, h);
  }
  return widget;
}

// creates a new form using the given map
extern "C" void v_create_form(var_p_t var) {
  bool hasInputs = false;
  var_p_t arg;
  AnsiWidget *out = g_system->getOutput();

  if (code_isvar()) {
    arg = code_getvarptr();
    if (arg->type == V_MAP) {
      var_p_t inputs = map_get(arg, FORM_INPUTS);
      if (inputs != NULL && inputs->type == V_ARRAY) {
        for (int i = 0; i < inputs->v.a.size; i++) {
          var_p_t elem = v_elem(inputs, i);
          if (elem->type == V_MAP) {
            hasInputs = true;
          }
        }
      }
    }
  }

  if (hasInputs) {
    map_set(var, arg);
    var_p_t v_focus = map_get(var, FORM_FOCUS);
    int i_focus = v_focus != NULL ? v_getint(v_focus) : -1;
    var_p_t inputs = map_get(var, FORM_INPUTS);
    for (int i = 0; i < inputs->v.a.size; i++) {
      var_p_t elem = v_elem(inputs, i);
      if (elem->type == V_MAP) {
        FormInput *widget = create_input(elem);
        widget->construct(var, elem, i);
        out->addInput(widget);
        if (i_focus == i || inputs->v.a.size == 1) {
          widget->setFocus(true);
        }
      }
    }

    out->setDirty();
    v_zerostr(map_add_var(var, FORM_VALUE, 0));
    create_func(var, "doEvents", cmd_form_do_events);
    create_func(var, "close", cmd_form_close);
    create_func(var, "refresh", cmd_form_refresh);
  } else {
    err_form_input();
  }
}

