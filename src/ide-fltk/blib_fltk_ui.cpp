// -*- c-file-style: "java" -*-
// $Id: blib_fltk_ui.cpp,v 1.1 2004-11-18 23:00:37 zeeb90au Exp $
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "sys.h"
#include "var.h"
#include "kw.h"
#include "pproc.h"
#include "device.h"
#include "smbas.h"

#include <fltk/run.h>
#include <fltk/Group.h>
#include <fltk/Input.h>
#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/RadioButton.h>
#include <fltk/events.h>

#include "MainWindow.h"

C_LINKAGE_BEGIN

#include "blib_ui.h"

void enter_cb(Widget*, void* v); // in dev_fltk
extern MainWindow *wnd;

Group* form = 0;

enum ControlType {
    ctrl_button,
    ctrl_link,
    ctrl_radio,
    ctrl_check,
    ctrl_text
};

struct WidgetInfo {
    ControlType type;
	var_t* var;    
};

void createForm() {
    if (form == 0) {
        wnd->outputGroup->begin();
        form = new Group(wnd->out->x(), 
                         wnd->out->y(), 
                         wnd->out->w(), 
                         wnd->out->h());
        wnd->outputGroup->end(); 
    }
    form->begin();
}

void ui_reset() {
    if (form != 0) {

        int n = form->children();
        for (int i=0; i<n; i++) {
            WidgetInfo* widgetInf = (WidgetInfo*)form->child(i)->user_data();
            delete widgetInf;
        }

        form->clear();
        wnd->outputGroup->remove(form);
        form->parent(0);
        delete form;
        form = 0;
    }    
}

// BUTTON x, y, w, h, variable, caption [,type] 
//
// type can optionally be 'radio' | 'checkbox' | 'link'
// variable is set to 1 is a button or link was pressed (which 
// will have closed the form, or if a radio or checkbox was 
// selected when the form was closed
// 
void cmd_button() {
    int x, y, w, h;
	var_t* v = 0;
    char* caption = 0;
    char* type = 0;

    if (-1 != par_massget("IIIIPSs", &x, &y, &w, &h, &v, &caption, &type)) {
        createForm();
        Widget* widget = 0;
        WidgetInfo* widgetInf = new WidgetInfo();
        widgetInf->var = v;

        // TODO prime field from var_t

        if (type) {
            if (strncmp("radio", type, 5) == 0) {
                widget = (Widget*)new RadioButton(x, y, w, h);
                widgetInf->type = ctrl_radio;
            } else if (strncmp("checkbox", type, 8) == 0) {
                widget = (Widget*)new CheckButton(x, y, w, h);
                widgetInf->type = ctrl_check;
            } else if (strncmp("link", type, 4) == 0) {
                widget = (Widget*)new Button(x, y, w, h);
                widget->callback(enter_cb);
                widget->box(NO_BOX);
                widgetInf->type = ctrl_link;
            }
        }
        if (widget == 0) {
            widget = (Widget*)new Button(x, y, w, h);
            widget->callback(enter_cb);
            widgetInf->type = ctrl_button;
        }

        widget->copy_label(caption);
        widget->user_data(widgetInf);
        form->end();
    }
	pfree2(caption, type);
}

// TEXT x, y, w, h, variable
// When DOFORM returns the variable contains the user entered value
//
void cmd_text() {
    int x, y, w, h;
	var_t* v = 0;

    if (-1 != par_massget("IIIIP", &x, &y, &w, &h, &v)) {
        createForm();
        Input* widget = new Input(x, y, w, h);
        widget->box(BORDER_BOX);
        widget->color(color(220,220,220));

        // TODO prime field from var_t

        WidgetInfo* widgetInf = new WidgetInfo();
        widgetInf->var = v;
        widgetInf->type = ctrl_text;
        widget->user_data(widgetInf);

        form->end();
    }
}

// DOFORM
// Executes the form
// 
void cmd_doform() {
    if (form == 0) {
        rt_raise("UI: NO FIELDS DEFINED");
        return;
    } 

    wnd->out->hide();
    form->take_focus();
    form->show();

    wnd->setModal(true);
    while (wnd->isModal()) {
        wait();
    }

    if (wnd->isBreakExec()) {
        brun_break();
    }

    int n = form->children();
    for (int i=0; i<n; i++) {
        Widget* w = form->child(i);
        WidgetInfo* widgetInf = (WidgetInfo*)w->user_data();
        switch (widgetInf->type) {
        case ctrl_button:
            break;
        case ctrl_link:
            break;
        case ctrl_check:
            break;
        case ctrl_radio:
            break;
        case ctrl_text:  {
            // copy input data into variable
            Input* in = (Input*)w;
            int size = in->size();
            widgetInf->var->type = V_STR;
            widgetInf->var->v.p.ptr = (unsigned char*)tmp_alloc(size+1);
            strncpy((char*)widgetInf->var->v.p.ptr, in->value(), size);
            widgetInf->var->v.p.ptr[size] = 0;  }
            break;
        }
    }

    form->hide();
    wnd->out->show();

    ui_reset();
}

C_LINKAGE_END
