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

#include "platform/common/StringLib.h"

// width and height fudge factors for when button w+h specified as -1
#define BN_W 16
#define BN_H 12
#define LN_H 2

// control types available using the BUTTON command
enum ControlType {
  ctrl_button,
  ctrl_exit_button,
  ctrl_link,
  ctrl_exit_link,
  ctrl_text,
  ctrl_label,
  ctrl_listbox,
};

struct ListModel : IFormWidgetListModel {
  ListModel(const char *items, var_t *v);
  virtual ~ListModel() { clear(); }

  void clear();
  void create(const char *items, var_t *v);
  const char *getTextAt(int index);
  int getIndex(const char *t);
  int rows() const { return _list.length(); }
  int selected() const { return _focusIndex; }
  void selected(int index) { _focusIndex = index; }

private:
  void fromArray(const char *caption, var_t *v);
  List<String *> _list;
  int _focusIndex;
};

// binds a smallbasic variable with a form widget
struct WidgetData : public IButtonListener {
  WidgetData(ControlType type, var_t *var);
  virtual ~WidgetData();

  void arrayToString(String &s, var_t *v);
  void buttonClicked(const char *action);
  void setupWidget(IFormWidget *widget);
  bool updateGui();
  void updateVarFlag();
  void transferData();
  bool isLink() { return _type == ctrl_link || _type == ctrl_exit_link; }
  int padding() { return isLink() ? LN_H : BN_H; }

  IFormWidget *_widget;
  var_t *_var;

private:
  ControlType _type;

  // startup value used to check if
  // exec has altered a bound variable
  union {
    long i;
    byte *ptr;
  } orig;
};

typedef WidgetData *WidgetDataPtr;

struct Form {
  Form();
  virtual ~Form();

  void setupWidget(WidgetDataPtr widgetData);
  bool hasEvent() { return _mode == m_selected; }
  void invoke(WidgetDataPtr widgetData);
  bool execute();

  enum Mode {
    m_reset,
    m_init,
    m_active,
    m_selected
  };

private:
  Mode _mode;
  List<WidgetDataPtr> _items; // form child items
  var_t *_var;                // form variable contains the value of the event widget
  int _cmd;                   // doform argument by value
  bool _kbHandle;             // whether doform returns on a keyboard event
  int _prevX;
  int _prevY;
};

#endif
