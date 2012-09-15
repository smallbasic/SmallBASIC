// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FORM_UI_H
#define FORM_UI_H

#include "config.h"
#include "common/sys.h"
#include "common/var.h"

#include "ansiwidget.h"

// control types available using the BUTTON command
enum ControlType {
  ctrl_button,
  ctrl_text,
  ctrl_label,
  ctrl_listbox,
};

// binds a smallbasic variable with a form widget
struct WidgetData  : public IButtonListener {
  WidgetData(IFormWidget *widget, ControlType type, var_t *var);
  virtual ~WidgetData();

  IFormWidget *widget;
  ControlType type;
  var_t *var;
  bool is_group_radio;

  // startup value used to check if
  // exec has altered a bound variable
  union {
    long i;
    byte *ptr;
  } orig;

  void buttonClicked(const char *action);
  void setupWidget();
  bool updateGui();
  void updateVarFlag();
  void transferData();
};

typedef WidgetData *WidgetDataPtr;

struct Form {
  Form();
  virtual ~Form();

  void add(WidgetDataPtr widgetData) { items.add(widgetData); }
  bool hasEvent() { return mode == m_selected; }
  void invoke(WidgetDataPtr widgetData);
  void execute();
  void update();

  enum Mode { 
    m_reset, 
    m_init, 
    m_active, 
    m_selected 
  };

private:
  Mode mode;
  Vector<WidgetDataPtr> items; // form child items
  var_t *var;                  // form variable contains the value of the event widget
  int cmd;                     // doform argument by value
  bool kb_handle;              // whether doform returns on a keyboard event
  int prev_x;
  int prev_y;
};

#endif
