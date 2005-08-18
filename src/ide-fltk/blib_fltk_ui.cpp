// -*- c-file-style: "java" -*-
// $Id: blib_fltk_ui.cpp,v 1.12 2005-08-18 23:15:55 zeeb90au Exp $
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

#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/Group.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/RadioButton.h>
#include <fltk/Rectangle.h>
#include <fltk/events.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "Fl_Ansi_Window.h"

C_LINKAGE_BEGIN

#include "blib_ui.h"

extern MainWindow *wnd;

struct FormGroup : public Group {
    FormGroup(int x1, int x2, int y1, int y2) : Group(x1,x2,y1,y2) {}
    void draw(); // avoid drawing over the tab-bar
};

void FormGroup::draw() {
    int numchildren = children();
    Rectangle r(w(), h());
    if (box() == NO_BOX) {
        setcolor(color());
        fillrect(r);
    } else {
        draw_box();
        box()->inset(r);
    }
    push_clip(r);
    for (int n = 0; n < numchildren; n++) {
        Widget& w = *child(n);
        draw_child(w);
        draw_outside_label(w);
    }
    pop_clip();
}

FormGroup* form = 0;

enum ControlType {
    ctrl_button,
    ctrl_link,
    ctrl_radio,
    ctrl_check,
    ctrl_text,
    ctrl_label,
    ctrl_list
};

struct WidgetInfo {
    ControlType type;
    var_t* var;
};

struct AnchorLink : public Button {
    AnchorLink(int x, int y, int w, int h) : Button(x, y, w, h) {}
    void draw() {
        int bx = 0;
        int by = 0;
        int bw = w();
        int bh = h();
        Rectangle r(bx,by,bw,bh);
        box()->inset(r);
        draw_background();
        setcolor(labelcolor());
        draw_label(Rectangle(bx, by-2, bw, bh), 
                   ALIGN_INSIDE|ALIGN_BOTTOMLEFT);
        drawline(2, bh-2, 2+(int)getwidth(label()), bh-2);
        focusbox()->draw(Rectangle(bx+1, by+1, bw-2, bh-2), style(), 0);
        // current_flags_highlight());
    }
};

void closeButton(Widget* w, void* v) {
    WidgetInfo* inf = (WidgetInfo*)v;
    v_setstrn(inf->var, "1", 1);
    wnd->setModal(false);
}

void closeModeless(Widget* w, void* v) {
    WidgetInfo* inf = (WidgetInfo*)v;
    v_setstrn(inf->var, "1", 1);
    wnd->outputGroup->remove(w);
    wnd->outputGroup->redraw();
    delete w;
}

void createForm() {
    if (form == 0) {
        wnd->outputGroup->begin();
        form = new FormGroup(wnd->out->x()+1, 
                             wnd->out->y()+1, 
                             wnd->out->w()-2, 
                             wnd->out->h()-2);
        form->resizable(0);
        form->color(color(152,152,152));
        wnd->outputGroup->end(); 
    }
    form->begin();
}

void ui_reset() {
    if (form != 0) {
        form->hide();
        int n = form->children();
        for (int i=0; i<n; i++) {
            WidgetInfo* inf = (WidgetInfo*)form->child(i)->user_data();
            delete inf;
        }

        form->clear();
        wnd->outputGroup->remove(form);
        form->parent(0);
        delete form;
        form = 0;
    }
    wnd->out->show();
    wnd->out->take_focus();
    wnd->out->redraw();
}

// BUTTON x, y, w, h, variable, caption [,type] 
//
// type can optionally be 'radio' | 'checkbox' | 'link' | 'choice'
// variable is set to 1 if a button or link was pressed (which 
// will have closed the form, or if a radio or checkbox was 
// selected when the form was closed
// 
void cmd_button() {
    int x, y, w, h;
    var_t* v = 0;
    char* caption = 0;
    char* type = 0;

    if (-1 != par_massget("IIIIPSs", &x, &y, &w, &h, &v, &caption, &type)) {
        Button* widget = 0;
        WidgetInfo* inf = new WidgetInfo();
        inf->var = v;

        if (type && strncmp("modeless", type, 8) == 0) {
            wnd->outputGroup->begin();
            widget = new Button(x, y, w, h);
            widget->callback(closeModeless);
            widget->copy_label(caption);
            widget->user_data(inf);
            wnd->outputGroup->end();
            inf->type = ctrl_button;
            pfree2(caption, type);
            return;
        }

        createForm();
        if (type) {
            if (strncmp("radio", type, 5) == 0) {
                widget = new RadioButton(x, y, w, h);
                inf->type = ctrl_radio;
            } else if (strncmp("checkbox", type, 8) == 0) {
                widget = new CheckButton(x, y, w, h);
                inf->type = ctrl_check;
            } else if (strncmp("label", type, 8) == 0) {
                widget = (Button*)new Widget(x, y, w, h);
                widget->box(NO_BOX);
                inf->type = ctrl_label;
            } else if (strncmp("link", type, 4) == 0) {
                widget = (Button*)new AnchorLink(x, y, w, h);
                widget->callback(closeButton);
                widget->box(NO_BOX);
                widget->labelcolor(color(150,0,0));
                inf->type = ctrl_link;
            } else if (strncmp("choice", type, 6) == 0) {
                Choice* choice = new Choice(x, y, w, h);
                choice->begin();
                // "Easy|Medium|Hard"
                int itemIndex = 0;
                inf->type = ctrl_list;
                int len = caption ? strlen(caption) : 0;
                for (int i=0; i<len; i++) {
                    char* c = strchr(caption+i, '|');
                    int endIndex = c ? c-caption : len;
                    String s(caption+i, endIndex-i);
                    Item* item = new Item();
                    item->copy_label(s.toString());
                    i = endIndex;
                    if (v->type == V_STR && v->v.p.ptr &&
                        strcmp((const char*)v->v.p.ptr, s.toString()) == 0) {
                        choice->focus_index(itemIndex);
                    }
                    itemIndex++;
                }
                choice->user_data(inf);
                choice->end();
                form->end();
                pfree2(caption, type);
                return;
            }
        }
        if (widget == 0) {
            widget = new Button(x, y, w, h);
            widget->callback(closeButton);
            inf->type = ctrl_button;
        }

        // prime input field from variable
        if (v->type == V_STR && v->v.p.ptr &&
            strcmp((const char*)v->v.p.ptr, "1") == 0) {
            if (inf->type == ctrl_check || 
                inf->type == ctrl_radio) {
                widget->value(true);
            } else if (inf->type != ctrl_button) {
                widget->value((const char*)v->v.p.ptr);
            }
        }

        widget->copy_label(caption);
        widget->user_data(inf);
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

        // prime field from var_t
        if (v->type == V_STR && v->v.p.ptr) {
            widget->value((const char*)v->v.p.ptr);
        }

        WidgetInfo* inf = new WidgetInfo();
        inf->var = v;
        inf->type = ctrl_text;
        widget->user_data(inf);

        form->end();
    }
}

// DOFORM [x,y,w,h [,border-style, bg-color]]
// Executes the form
void cmd_doform() {
    int x, y, w, h, box, bg;
    int numArgs;

    if (form == 0) {
        ui_reset();
        rt_raise("UI: NO FORM FIELDS DEFINED");
        return;
    } 

    x = y = w = h = box = bg = 0;
    numArgs = par_massget("iiiiii", &x, &y, &w, &h, &box, &bg);

    if (numArgs != 0 && (numArgs < 4 || numArgs > 6)) {
        ui_reset();
        rt_raise("UI: INVALID FORM ARGUMENTS: %d", numArgs);
        return;
    }

    if (numArgs > 0) {
        switch (box) {
        case 1:
            form->box(BORDER_BOX);
            break;
        case 2:
            form->box(SHADOW_BOX);
            break;
        case 3:
            form->box(ENGRAVED_BOX);
            break;
        case 4:
            form->box(THIN_DOWN_BOX);
            break;
        default: 
            form->box(NO_BOX);
        }
        if (numArgs == 6) {
            form->color(AnsiWindow::ansiToFltk(bg));
        }
        if (x < 2) {
            x = 2;
        }
        if (y < 2) {
            y = 2;
        }
        form->x(x);
        form->y(y);
        if (x+w > form->w()) {
            w = form->w()-x;
        }
        form->w(w);
        if (y+h > form->h()) {
            h = form->h()-y;
        }
        form->h(h);
    } else {
        form->box(ENGRAVED_BOX);
    }

    wnd->tabGroup->selected_child(wnd->outputGroup);
    form->take_focus();
    form->show();

    wnd->setModal(true);
    while (wnd->isModal()) {
        fltk::wait();
    }

    if (wnd->isBreakExec()) {
        brun_break();
    }
    wnd->resetPen();

    int n = form->children();
    for (int i=0; i<n; i++) {
        Widget* w = form->child(i);
        WidgetInfo* inf = (WidgetInfo*)w->user_data();
        switch (inf->type) {
        case ctrl_check:
        case ctrl_radio:
            if (((Button*)w)->value()) {
                v_setstrn(inf->var, "1", 1);
            }
            break;
        case ctrl_text:
            // copy input data into variable
            v_setstrn(inf->var, ((Input*)w)->value(), ((Input*)w)->size());
            break;
        case ctrl_list:
            // copy drop list item into variable
            v_setstr(inf->var, ((Choice*)w)->item()->label());
            break;
        default:
            break;
        }
    }

    ui_reset();
}

C_LINKAGE_END

// End of "$Id: blib_fltk_ui.cpp,v 1.12 2005-08-18 23:15:55 zeeb90au Exp $".
