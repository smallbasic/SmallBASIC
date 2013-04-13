// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "sys.h"
#include "var.h"
#include "kw.h"
#include "pproc.h"
#include "device.h"
#include "smbas.h"
#include "keymap.h"
#include "mainwindow.h"
#include "form_ui.h"

#include <QApplication>
#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QListView>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QWidget>

extern "C" {
#include "blib_ui.h"
} 

struct Form : public QWidget {
  Form(int x1, int x2, int y1, int y2) : QWidget() {
    setGeometry(x1, x2, y1, y2);
    this->_cmd = 0;
    this->var = 0;
    this->kb_handle = false;
  } 
  ~Form() {}
  var_t *var;                   // form variable contains the value of the event widget
  int _cmd;                      // doform argument by value
  bool kb_handle;               // whether doform returns on a keyboard event
  int prev_x;
  int prev_y;
};

Form *form = 0;

enum Mode { m_reset, m_init, m_active, m_selected } mode = m_reset;

// whether a widget event has fired
bool form_event() {
  return mode == m_selected;
}

// width and height fudge factors for when button w+h specified as -1
#define BN_W  16
#define BN_H   8
#define RAD_W 22
#define RAD_H  0

// forward declared for invoked()
void transfer_data(QWidget *w, WidgetInfo *inf);

// default constructor for Q_DECLARE_METATYPE
WidgetInfo::WidgetInfo() : QObject() {}

// copy constructor for Q_DECLARE_METATYPE
WidgetInfo::WidgetInfo(const WidgetInfo &winf) : QObject() {
  widget = winf.widget;
  type = winf.type;
  var = winf.var;
  is_group_radio = winf.is_group_radio;
  orig = winf.orig;
}

// public destructor for Q_DECLARE_METATYPE  
WidgetInfo::~WidgetInfo() {}

// update the smallbasic variable
void WidgetInfo::update_var_flag() {
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

// slot/callback for the widget info called when the widget has been invoked
void WidgetInfo::invoked() {
  if (wnd->isRunning()) {
    transfer_data(widget, this);

    mode = m_selected;

    if (form->var) {
      // array type cannot be used in program select statement
      if (this->var->type == V_ARRAY) {
        v_zerostr(form->var);
      } else {
        // set the form variable from the widget var
        v_set(form->var, this->var);
      }
    }
  }
}

// implements abstract StringList as a list of strings
struct DropListModel : QAbstractItemModel {
  QVariantList list;
  int focus_index;

  DropListModel(const char *items, var_t *v) : 
    QAbstractItemModel() {
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
        QString s = QString::fromUtf8(items + i, end_index - i);
        list.append(s);
        i = end_index;
        if (v != 0 && v->type == V_STR && v->v.p.ptr &&
            strcasecmp((const char *)v->v.p.ptr, s.toUtf8().data()) == 0) {
          focus_index = item_index;
        }
        item_index++;
      }
    }
  }

  virtual ~DropListModel() {}

  // construct from an array of values
  void fromArray(const char *caption, var_t *v) {
    for (int i = 0; i < v->v.a.size; i++) {
      var_t *el_p = (var_t *)(v->v.a.ptr + sizeof(var_t) * i);
      if (el_p->type == V_STR) {
        list << (const char *)el_p->v.p.ptr;
        if (caption && strcasecmp((const char *)el_p->v.p.ptr, caption) == 0) {
          focus_index = i;
        }
      } else if (el_p->type == V_INT) {
        char buff[40];
        sprintf(buff, VAR_INT_FMT, el_p->v.i);
        list << buff;
      } else if (el_p->type == V_ARRAY) {
        fromArray(caption, el_p);
      }
    }
  }

  // returns the index corresponding to the given string
  int getPosition(const char *t) {
    int size = list.count();
    for (int i = 0; i < size; i++) {
      if (!strcasecmp(list.at(i).toString().toUtf8().data(), t)) {
        return i;
      }
    }
    return -1;
  }

  // return the text at the given index
  const char *getTextAt(int index) {
    const char *s = 0;
    if (index < list.count()) {
      s = list.at(index).toString().toUtf8().data();
    }
    return s;
  }

  // returns the model index corresponding to the given string
  QModelIndex getIndex(const char *t) {
    return createIndex(getPosition(t), 0);
  }

  // index of the item in the model 
  QModelIndex index(int row, int column, const QModelIndex &/* index */ )const {
    return createIndex(row, column);
  }

  // parent of the model item with the given index
  QModelIndex parent(const QModelIndex& /*index*/) const {
    return createIndex(-1, -1);
  }

  // return the number of rows under the given parent
  int rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : list.count();
  }

  // return the number of columns
  int columnCount(const QModelIndex& /*parent*/) const {
    return 1;
  }

  // return the data at the given index
  QVariant data(const QModelIndex& index, int role) const {
    return list.at(index.row());
  }
};

// convert a basic array into a QString
void array_to_string(QString &s, var_t *v) {
  for (int i = 0; i < v->v.a.size; i++) {
    var_t *el_p = (var_t *)(v->v.a.ptr + sizeof(var_t) * i);
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

// returns the button text
const char *getText(QAbstractButton *w) {
  return w->text().toUtf8().data();
}

// set basic string variable to widget state when the variable has changed
bool update_gui(QWidget *w, WidgetInfoPtr inf) {
  QComboBox *dropdown;
  QListView *listbox;
  DropListModel *model;

  if (inf->var->type == V_INT && inf->var->v.i != inf->orig.i) {
    // update list control with new int variable
    switch (inf->type) {
    case ctrl_dropdown:
      ((QComboBox *)w)->setCurrentIndex(inf->var->v.i);
      return true;

    case ctrl_listbox:
      {
        QAbstractItemModel *model = ((QListView *)w)->model();
        ((QListView *)w)->setCurrentIndex(model->index(inf->var->v.i, 0));
      }
      return true;

    default:
      return false;
    }
  }

  if (inf->var->type == V_ARRAY && inf->var->v.p.ptr != inf->orig.ptr) {
    // update list control with new array variable
    QString s;

    switch (inf->type) {
    case ctrl_dropdown:
      delete((QComboBox *)w)->model();
      ((QComboBox *)w)->setModel(new DropListModel(0, inf->var));
      return true;

    case ctrl_listbox:
      delete((QListView *)w)->model();
      ((QListView *)w)->setModel(new DropListModel(0, inf->var));
      ((QListView *)w)->selectionModel()->clear();
      return true;

    case ctrl_label:
      array_to_string(s, inf->var);
      ((QLabel *)w)->setText(s);
      break;

    case ctrl_text:
      array_to_string(s, inf->var);
      ((QPlainTextEdit *)w)->setPlainText(s);
      break;

    default:
      return false;
    }
  }

  if (inf->var->type == V_STR && inf->orig.ptr != inf->var->v.p.ptr) {
    // update list control with new string variable
    switch (inf->type) {
    case ctrl_dropdown:
      dropdown = (QComboBox *)w;
      model = (DropListModel *)dropdown->model();
      if (strchr((const char *)inf->var->v.p.ptr, '|')) {
        // create a new list of items
        delete model;
        model = new DropListModel((const char *)inf->var->v.p.ptr, 0);
        dropdown->setModel(model);
      } else {
        // select one of the existing list items
        int selection = model->getPosition((const char *)inf->var->v.p.ptr);
        if (selection != -1) {
          dropdown->setCurrentIndex(selection);
        }
      }
      break;

    case ctrl_listbox:
      listbox = (QListView *)w;
      model = (DropListModel *)listbox->model();
      if (strchr((const char *)inf->var->v.p.ptr, '|')) {
        // create a new list of items
        delete model;
        model = new DropListModel((const char *)inf->var->v.p.ptr, 0);
        listbox->setModel(model);
      } else {
        QModelIndex selection = model->getIndex((const char *)inf->var->v.p.ptr);
        if (selection.isValid()) {
          listbox->setCurrentIndex(selection);
        }
      }
      break;

    case ctrl_check:
    case ctrl_radio:
      ((QCheckBox *)w)->setCheckState(!strcasecmp((const char *)inf->var->v.p.ptr,
                                                   getText((QCheckBox *)w))
                                       ? Qt::Checked : Qt::Unchecked);
      break;

    case ctrl_label:
      ((QLabel *)w)->setText((const char *)inf->var->v.p.ptr);
      break;

    case ctrl_text:
      ((QPlainTextEdit *)w)->setPlainText((const char *)inf->var->v.p.ptr);
      break;

    case ctrl_button:
      ((QPushButton *)w)->setText((const char *)inf->var->v.p.ptr);
      break;

    default:
      break;
    }
    return true;
  }
  return false;
}

// synchronise basic variable and widget state
void transfer_data(QWidget *w, WidgetInfoPtr inf) {
  QString s;
  QComboBox *dropdown;
  QListView *listbox;
  DropListModel *model;

  if (update_gui(w, inf)) {
    inf->update_var_flag();
    return;
  }
  // set widget state to basic variable
  switch (inf->type) {
  case ctrl_check:
    if (((QCheckBox *)w)->checkState()) {
      v_setstr(inf->var, getText((QCheckBox *)w));
    } else {
      v_zerostr(inf->var);
    }
    break;

  case ctrl_radio:
    if (((QRadioButton *)w)->isChecked()) {
      const char *label = getText((QRadioButton *)w);
      if (label) {
        v_setstr(inf->var, label);
      }
    } else if (!inf->is_group_radio) {
      // reset radio variable for radio that is not part of a group
      v_zerostr(inf->var);
    }
    break;

  case ctrl_text:
    s = ((QPlainTextEdit *)w)->toPlainText();
    if (s.length()) {
      v_setstr(inf->var, s.toUtf8().data());
    } else {
      v_zerostr(inf->var);
    }
    break;

  case ctrl_dropdown:
    dropdown = (QComboBox *)w;
    model = (DropListModel *)dropdown->model();
    if (dropdown->currentIndex() != -1) {
      const char *s = model->getTextAt(dropdown->currentIndex());
      if (s) {
        v_setstr(inf->var, s);
      }
    }
    break;

  case ctrl_listbox:
    listbox = (QListView *)w;
    model = (DropListModel *)listbox->model();
    if (listbox->selectionModel()->currentIndex().isValid()) {
      const char *s = model->getTextAt(listbox->selectionModel()->currentIndex().row());
      if (s) {
        v_setstr(inf->var, s);
      }
    }
    break;

  case ctrl_button:
    // update the basic variable with the button label
    v_setstr(inf->var, getText((QPushButton *)w));
    break;

  default:
    break;
  }

  // only update the gui when the variable is changed in basic code
  inf->update_var_flag();
}

// find the radio group of the given variable from within the parent
QButtonGroup *findButtonGroup(QWidget *parent, var_t *v) {
  QButtonGroup *radioGroup = 0;
  QObjectList children = parent->children();
  int n = children.size();

  for (int i = 0; i < n && !radioGroup; i++) {
    QObject *nextObject = children.at(i);
    if (nextObject->inherits("QButtonGroup")) {
      QList < QAbstractButton *>buttons = ((QButtonGroup *)nextObject)->buttons();
      int nButtons = buttons.size();
      for (int j = 0; j < nButtons && !radioGroup; j++) {
        QAbstractButton *nextButton = buttons.at(j);
        WidgetInfoPtr inf = nextButton->property("widgetInfo").value <WidgetInfoPtr>();
        if (inf != NULL) {
          if (inf->type == ctrl_radio &&
              inf->var->type == V_STR && (inf->var == v || inf->var->v.p.ptr == v->v.p.ptr)) {
            // another ctrl_radio is linked to the same variable
            inf->is_group_radio = true;
            radioGroup = (QButtonGroup *)nextObject;
          }
        }
      }
    }
  }
  return radioGroup;
}

// radio control's belong to the same group when they share
// a common basic variable
void update_radio_group(WidgetInfoPtr radioInf, QRadioButton *radio) {
  var_t *v = radioInf->var;

  if (v == 0 || v->type != V_STR) {
    return;
  }

  QButtonGroup *radioGroup = findButtonGroup(form, v);
  if (!radioGroup) {
    radioGroup = new QButtonGroup(form);
  } else {
    radioInf->is_group_radio = true;
  }

  radioGroup->addButton(radio);
}

void update_widget(QWidget *widget, WidgetInfoPtr inf, QRect &rect) {
  if (rect.width() != -1) {
    widget->setFixedWidth(rect.width());
  }

  if (rect._height() != -1) {
    widget->setFixedHeight(rect._height());
  }

  if (rect.x() < 0) {
    rect.setX(form->prev_x - rect.x());
  }

  if (rect.y() < 0) {
    rect.setY(form->prev_y - rect.y());
  }

  form->prev_x = rect.x() + rect.width();
  form->prev_y = rect.y() + rect._height();

  widget->setGeometry(rect);
  widget->setProperty("widgetInfo", QVariant::fromValue(inf));  
  // qVariantFromValue(inf));
  widget->setParent(form);
  inf->widget = widget;

  // allow form init to update widget from basic variable
  inf->orig.ptr = 0;
  inf->orig.i = 0;

  // copy output widget colors
  // widget->color(wnd->out->getColor());
  // widget->labelcolor(wnd->out->getBackgroundColor());
}

void update_button(QAbstractButton *widget, WidgetInfoPtr inf,
                   const char *caption, QRect &rect, int def_w, int def_h) {
  if (rect.width() < 0 && caption != 0) {
    rect.setWidth((int)wnd->out->textWidth(caption) + def_w + (-rect.width() - 1));
  }

  if (rect._height() < 0) {
    rect.setHeight((int)(wnd->out->textHeight() + def_h + (-rect._height() - 1)));
  }

  update_widget(widget, inf, rect);
  widget->setText(caption);
  widget->connect(widget, SIGNAL(clicked(bool)), inf, SLOT(invoked()));
}

// create a new form
void form_create() {
  if (form == 0) {
    form = new Form(wnd->out->x() + 2,
                    wnd->out->y() + 2, wnd->out->width() - 2, wnd->out->_height() - 2);
  }
  // form->begin();
  mode = m_init;
}

// prepare the form for display
void form_init() {
  if (form) {
    form->setGeometry(0, 0, form->width(), form->_height());
  }
}

// copy all widget fields into variables
void form_update(QWidget *group) {
  if (group && wnd->isRunning()) {
    QObjectList children = group->children();
    int n = children.size();

    for (int i = 0; i < n; i++) {
      QObject *w = children.at(i);
      if (w->isWidgetType()) {
        WidgetInfoPtr widgetInfo = w->property("widgetInfo").value<WidgetInfoPtr>();
        if (widgetInfo == NULL) {
          form_update((QWidget *)w);
        } else {
          transfer_data((QWidget *)w, widgetInfo);
        }
      }
    }
  }
}

// close the form
void form_end() {
  if (form != 0) {
    // form->end();
  }
}

// destroy the form
C_LINKAGE_BEGIN void ui_reset() {
  if (form != 0) {
    QObjectList children = form->children();
    int n = children.size();

    for (int i = 0; i < n; i++) {
      QObject *w = children.at(i);
      if (w->isWidgetType()) {
        WidgetInfoPtr widgetInfo = w->property("widgetInfo").value<WidgetInfoPtr>();
        if (widgetInfo != NULL) {
          delete widgetInfo;
        }
      }
    }

    delete form;
    form = 0;
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
    WidgetInfoPtr inf = new WidgetInfo();
    inf->var = v;
    QRect rect(x, y, w, h);

    if (prog_error) {
      return;
    }
    if (type) {
      if (strcasecmp("radio", type) == 0) {
        inf->type = ctrl_radio;
        inf->is_group_radio = false;
        form_end();             // add widget to RadioGroup
        QRadioButton *widget = new QRadioButton();
        widget->setGeometry(x, y, w, h);
        update_radio_group(inf, widget);
        update_button(widget, inf, caption, rect, RAD_W, RAD_H);
      } else if (strcasecmp("checkbox", type) == 0 || strcasecmp("check", type) == 0) {
        inf->type = ctrl_check;
        QCheckBox *widget = new QCheckBox();
        widget->setGeometry(x, y, w, h);
        update_button(widget, inf, caption, rect, RAD_W, RAD_H);
      } else if (strcasecmp("button", type) == 0) {
        inf->type = ctrl_button;
        QPushButton *widget = new QPushButton();
        widget->setGeometry(x, y, w, h);
        update_button(widget, inf, caption, rect, BN_W, BN_H);
      } else if (strcasecmp("label", type) == 0) {
        inf->type = ctrl_label;
        QLabel *widget = new QLabel();
        widget->setGeometry(x, y, w, h);
        widget->setText(caption);
        update_widget(widget, inf, rect);
      } else if (strcasecmp("listbox", type) == 0 || strcasecmp("list", type) == 0) {
        inf->type = ctrl_listbox;
        QListView *widget = new QListView();
        DropListModel *model = new DropListModel(caption, v);
        widget->setModel(model);
        widget->setGeometry(x, y, w, h);
        // if (model->focus_index != -1) {
        // widget->value(model->focus_index);
        // }
        update_widget(widget, inf, rect);
        // widget->when(WHEN_RELEASE_ALWAYS);
      } else if (strcasecmp("dropdown", type) == 0 || strcasecmp("choice", type) == 0) {
        inf->type = ctrl_dropdown;
        QComboBox *widget = new QComboBox();
        DropListModel *model = new DropListModel(caption, v);
        widget->setModel(model);
        widget->setGeometry(x, y, w, h);
        // if (model->focus_index != -1) {
        // widget->value(model->focus_index);
        // }
        update_widget(widget, inf, rect);
      } else {
        ui_reset();
        rt_raise("UI: UNKNOWN BUTTON TYPE: %s", type);
      }
    } else {
      inf->type = ctrl_button;
      QPushButton *widget = new QPushButton();
      widget->setGeometry(x, y, w, h);
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
    QPlainTextEdit *widget = new QPlainTextEdit();
    QRect rect(x, y, w, h);
    WidgetInfoPtr inf = new WidgetInfo();
    widget->setGeometry(rect);
    inf->var = v;
    inf->type = ctrl_text;
    update_widget(widget, inf, rect);
    if (rect._height() > (wnd->out->textHeight() + BN_H)) {
      // widget->type(QPlainTextEdit::MULTILINE | QPlainTextEdit::WORDWRAP);
    }
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
    form->_cmd = -1;
    form->var = 0;
    break;
  default:
    if (code_isvar()) {
      form->var = code_getvarptr();
      form->_cmd = -1;
    } else {
      var_t var;
      v_init(&var);
      eval(&var);
      form->_cmd = v_getint(&var);
      form->var = 0;
      v_free(&var);

      // apply any configuration options
      switch (form->_cmd) {
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

  if (!form->_cmd) {
    ui_reset();
  } else if (wnd->out->getMouseMode()) {
    mode = m_active;
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
  } else {
    // pump system messages until there is a widget callback
    mode = m_active;

    if (form->kb_handle) {
      dev_clrkb();
    }
    while (wnd->isRunning() && mode == m_active) {
      QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);

      if (form->kb_handle && keymap_kbhit()) {
        break;
      }
    }
    form_update(form);
  }
}

C_LINKAGE_END
