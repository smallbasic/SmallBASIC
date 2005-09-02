// -*- c-file-style: "java" -*-
// $Id: blib_fltk_ui.cpp,v 1.14 2005-09-02 06:26:37 zeeb90au Exp $
//
// Copyright(C) 2001-2005 Chris Warren-Smith. Gawler, South Australia
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
#include "StringLib.h"

extern "C" { 
#include "blib_ui.h"
}

extern MainWindow *wnd;

struct FormGroup : public Group {
    FormGroup(int x1, int x2, int y1, int y2) : Group(x1,x2,y1,y2) {}
    void draw(); // avoid drawing over the tab-bar
};

FormGroup* form = 0; // modal form
strlib::List* widgets = 0; // modeless form widgets

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

void button_cb(Widget* w, void* v) {
    WidgetInfo* inf = (WidgetInfo*)v;
    v_setstrn(inf->var, "1", 1);
    wnd->setModal(false);
    wnd->penState = 2;
}

void radio_cb(Widget* w, void* v) {
    WidgetInfo* inf = (WidgetInfo*)v;
    v_setstrn(inf->var, "1", 1);
}

// transfer widget data in variables
void updateVars(Widget* w) {
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

// copy all modeless widget fields into variables
void updateForm() {
    if (widgets) {
        Object** list = widgets->getList();
        int len = widgets->length();
        for (int i=0; i<len; i++) {
            updateVars((Widget*)list[i]);
        }
    }
}

// close the modeless widgets
void closeModeless() {
    if (widgets) {
        Object** list = widgets->getList();
        int len = widgets->length();
        for (int i=0; i<len; i++) {
            Widget* w = (Widget*)list[i];
            WidgetInfo* inf = (WidgetInfo*)w->user_data();
            w->parent()->remove(w);
            w->parent(0);
            delete inf;
            delete w;
        }
        widgets->emptyList();
        delete widgets;
        widgets = 0;
    }
    wnd->out->redraw();
}

void formBegin() {
    if (widgets) {
        ui_reset();
        wnd->outputGroup->begin();
        return;
    }
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

void formEnd(Widget* w) {
    if (widgets) {
        wnd->outputGroup->end();
        widgets->add((Object*)w);
    } else {
        form->end();
    }
}

C_LINKAGE_BEGIN

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

        wnd->out->show();
        wnd->out->take_focus();
        wnd->out->redraw();
    }
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

        formBegin();
        if (type) {
            if (strncmp("radio", type, 5) == 0) {
                widget = new RadioButton(x, y, w, h);
                widget->callback(radio_cb);
                inf->type = ctrl_radio;
            } else if (strncmp("checkbox", type, 8) == 0) {
                widget = new CheckButton(x, y, w, h);
                widget->callback(radio_cb);
                inf->type = ctrl_check;
            } else if (strncmp("button", type, 6) == 0) {
                widget = new Button(x, y, w, h);
                widget->callback(radio_cb);
                inf->type = ctrl_button;
            } else if (strncmp("label", type, 8) == 0) {
                widget = (Button*)new Widget(x, y, w, h);
                widget->box(NO_BOX);
                inf->type = ctrl_label;
            } else if (strncmp("link", type, 4) == 0) {
                widget = (Button*)new AnchorLink(x, y, w, h);
                widget->callback(button_cb);
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
                formEnd(choice);
                pfree2(caption, type);
                return;
            } else {
                ui_reset();
                rt_raise("UI: UNKNOWN TYPE: %s", type);
            }
        }
        if (widget == 0) {
            widget = new Button(x, y, w, h);
            widget->callback(button_cb);
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
        formEnd(widget);
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
        formBegin();
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

        formEnd(widget);
    }
}

// DOFORM [x,y,w,h [,border-style, bg-color]]
// Executes the form
void cmd_doform() {
    int x, y, w, h, box, bg;
    int numArgs;

    x = y = w = h = box = bg = 0;
    numArgs = par_massget("iiiiii", &x, &y, &w, &h, &box, &bg);

    if (numArgs != 0 && numArgs != 1 && (numArgs < 4 || numArgs > 6)) {
        ui_reset();
        rt_raise("UI: INVALID FORM ARGUMENTS: %d", numArgs);
        return;
    }

    if (numArgs == 1) {
        // modeless operations
        ui_reset();
        switch (x) {
        case 0: // begin modeless state
            widgets = new strlib::List(10);
            break;
        case 1: // end modeless state
            updateForm();
            closeModeless();
            break;
        case 2: // transfer form variables
            updateForm();
            break;
        case 3: // close only
            closeModeless();
            break;
        }
        return;
    } else if (numArgs > 0) {
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
        updateVars(form->child(i));
    }

    ui_reset();
}

C_LINKAGE_END

// End of "$Id: blib_fltk_ui.cpp,v 1.14 2005-09-02 06:26:37 zeeb90au Exp $".
