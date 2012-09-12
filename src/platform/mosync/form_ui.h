// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "common/sys.h"
#include "common/var.h"

#include "ansiwidget.h"

// whether a widget event has fired
bool form_event();

// control types available using the BUTTON command
enum ControlType {
  ctrl_button,
  ctrl_text,
  ctrl_label,
  ctrl_listbox,
};

// binds a smallbasic variable with a form widget
struct WidgetInfo {
  WidgetInfo(FormWidget *widget, ControlType type, var_t *var);
  WidgetInfo(const WidgetInfo &winf);

  FormWidget *widget;
  ControlType type;
  var_t *var;
  bool is_group_radio;

  // startup value used to check if
  // exec has altered a bound variable
  union {
    long i;
    byte *ptr;
  } orig;

  bool updateGui();
  void updateVarFlag();
  void invoked();
  void transferData();
};

typedef WidgetInfo *WidgetInfoPtr;

enum Mode { m_reset, m_init, m_active, m_selected };

struct Form {
  Form();
  virtual ~Form();

  void update();

  Mode mode;
  Vector<WidgetInfoPtr> items; // form child items
  var_t *var;                  // form variable contains the value of the event widget
  int cmd;                     // doform argument by value
  bool kb_handle;              // whether doform returns on a keyboard event
  int prev_x;
  int prev_y;
};

