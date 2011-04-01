// $Id$
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "sys.h"
#include "var.h"

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

// binds a smallbasic variable with a QT widget
struct WidgetInfo : public QObject {
  WidgetInfo();
  WidgetInfo(const WidgetInfo& winf);
  ~WidgetInfo();

  QWidget* widget;
  ControlType type;
  var_t* var;
  bool is_group_radio;

  // startup value used to check if
  // exec has altered a bound variable
  union {
    long i;
    byte* ptr;
  } orig;

  void update_var_flag();

  public slots: 
  void invoked();
};

typedef WidgetInfo* WidgetInfoPtr;
Q_DECLARE_METATYPE(WidgetInfoPtr);

// End of "$Id$".
