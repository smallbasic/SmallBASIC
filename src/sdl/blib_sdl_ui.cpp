// -*- c-file-style: "java" -*-
// $Id: blib_sdl_ui.cpp,v 1.1 2007-04-28 05:28:23 zeeb90au Exp $
//
// Copyright(C) 2001-2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
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
#include "osd.h"

#ifdef IMPL_UI

#include <map>
#include <vector>
#include <string>
#include <utility>
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "SDL.h"

// SDL surface in dev_sdl.c
extern SDL_Surface* screen;
extern int dev_w;
extern int dev_h;

extern "C" void ui_reset();

enum {m_unset, m_init, m_modeless, m_modal, m_closed} mode = m_unset;
int modeless_x;
int modeless_y;
int modeless_w;
int modeless_h;
int cursor;

using namespace gcn;

// Guichan SDL components
SDLInput* input;             // Input driver
SDLGraphics* graphics;       // Graphics driver
Gui* gui;                    // A Gui object - binds it all together
ImageFont* font;             // A font
SDLImageLoader* imageLoader; // For loading images

enum ControlType {
    ctrl_button,
    ctrl_link,
    ctrl_radio,
    ctrl_check,
    ctrl_text,
    ctrl_label,
    ctrl_listbox,
    ctrl_dropdown
};

struct WidgetInfo {
    ControlType type;
    var_t* var;
    bool is_group_radio;
};

struct DropListModel : ListModel {
    std::vector<std::string> list;
    int focus_index;

    DropListModel(const char* items, var_t* v) {
        focus_index = -1;
        // parse a string like "Easy|Medium|Hard"
        int item_index = 0;
        int len = items ? strlen(items) : 0;
        for (int i=0; i<len; i++) {
            char* c = strchr(items+i, '|');
            int end_index = c ? c-items : len;
            std::string s(items+i, end_index-i);
            list.push_back(s);
            i = end_index;
            if (v->type == V_STR && v->v.p.ptr &&
                strcmp((const char*)v->v.p.ptr, s.c_str()) == 0) {
                focus_index = item_index;
            }
            item_index++;
        }
    }
    
    int getNumberOfElements() {
        return list.size();
    }

    std::string getElementAt(int i) {
        return list.at(i);
    }
};

typedef std::map<gcn::Widget*, WidgetInfo*>::iterator WI;

struct Form : Container, ActionListener {
    void update();
    void action(const ActionEvent& actionEvent);
    void add_widget(gcn::Widget* widget, WidgetInfo* inf, int x, int y, int w, int h);
    void transfer_data(gcn::Widget* w, WidgetInfo* inf);
    bool set_radio_group(var_t* v, RadioButton* radio);
    std::map<gcn::Widget*, WidgetInfo*> widget_map;
    ~Form();
};

Form* form = 0;

Form::~Form() {
    Container::clear();
    for (WI iter = widget_map.begin(); iter != widget_map.end(); iter++) {
        delete iter->first; // cleanup widget
        delete iter->second; // cleanup widgetinfo
    }
}

void Form::add_widget(gcn::Widget* widget, WidgetInfo* inf, int x, int y, int w, int h) {
    if (w != -1 && h != -1) {
        widget->setSize(w, h);
    }
    widget->addActionListener(form);
    widget_map.insert(std::make_pair(widget, inf));
    add(widget, x, y);
}

void Form::action(const gcn::ActionEvent& actionEvent) {
    // find the WidgetInfo for the event's widget
    WI iter = widget_map.find(actionEvent.getSource());
    if (iter != widget_map.end()) {
        transfer_data(iter->first, iter->second);
    }
    if (iter->second->type == ctrl_button) {
        // submit button - close modeless form
        if (mode == m_modeless) {
            ui_reset();
        }
        if (mode != m_unset) {
            mode = m_closed;
        }
    }
}

// copy all modeless widget fields into variables
void Form::update() {
    for (WI iter = widget_map.begin(); iter != widget_map.end(); iter++) {
        transfer_data(iter->first, iter->second);
    }
}

// transfer widget data in variables
void Form::transfer_data(gcn::Widget* w, WidgetInfo* inf) {
    DropDown* dropdown;
    ListBox* listbox;
    DropListModel* model;

    switch (inf->type) {
    case ctrl_button:
        v_setstr(inf->var, ((Button*)w)->getCaption().c_str());
        break;

    case ctrl_check:
        if (((CheckBox*)w)->isMarked()) {
            v_setstr(inf->var, ((CheckBox*)w)->getCaption().c_str());
        } else {
            v_zerostr(inf->var);
        }
        break;

    case ctrl_radio:
        if (((RadioButton*)w)->isMarked()) {
            v_setstr(inf->var, ((RadioButton*)w)->getCaption().c_str());
        } else if (!inf->is_group_radio) {
            // reset radio variable for radio that is not part of a group
            v_zerostr(inf->var);
        }
        break;

    case ctrl_text:
        v_setstr(inf->var, ((TextBox*)w)->getText().c_str());
        break;

    case ctrl_dropdown:
        dropdown = (DropDown*)w;
        model = (DropListModel*)dropdown->getListModel();
        if (dropdown->getSelected() != -1) {
            std::string s = model->getElementAt(dropdown->getSelected());
            v_setstr(inf->var, s.c_str());
        }
        break;

    case ctrl_listbox:
        listbox = (ListBox*)w;
        model = (DropListModel*)dropdown->getListModel();
        if (dropdown->getSelected() != -1) {
            std::string s = model->getElementAt(dropdown->getSelected());
            v_setstr(inf->var, s.c_str());
        }
        break;

    default:
        break;
    }
}

void form_begin() {
    if (form == 0) {
        // initialize and create the Guichan gui
        form = new Form();
        graphics = new gcn::SDLGraphics();

        // Set the target for the graphics object to be the screen.
        // In other words, we will draw to the screen.
        // Note, any surface will do, it doesn't have to be the screen.
        graphics->setTarget(screen);

        // Set the dimension of the top container to match the screen.
        gui = new gcn::Gui();

        // Set gui to use the SDLGraphics object.
        gui->setGraphics(graphics);

        // Set gui to use the SDLInput object
        input = new gcn::SDLInput();
        gui->setInput(input);

        // Set the top container
        gui->setTop(form);

        imageLoader = new gcn::SDLImageLoader();
        // The ImageLoader in use is static and must be set to be
        // able to load images
        gcn::Image::setImageLoader(imageLoader);

        // Load the image font.
        const char* c =
            " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        try {
            font = new gcn::ImageFont("fixedfont.bmp", c);
            // The global font is static and must be set.
            gcn::Widget::setGlobalFont(font);
        } catch (gcn::Exception e) {
            rt_raise("UI: Failed to load fixedfont.bmp: %s", 
                     e.getMessage().c_str());
        }
        cursor = SDL_ShowCursor(SDL_ENABLE);
    }
}

void form_iteration() {
    try {
        // Poll SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_c) {
                    if (event.key.keysym.mod & KMOD_CTRL) {
                        mode = m_closed;
                        return;
                    }
                }
            }
            input->pushInput(event); // handle the event in guichan
        }

        gui->logic(); // Let the gui perform it's logic (like handle input)
        gui->draw();  // Draw the gui
        SDL_Flip(graphics->getTarget()); // Update the screen
        if (mode != m_closed) {
            SDL_WaitEvent(NULL);
        }
    } catch (gcn::Exception e) {
        rt_raise("UI: Event failed: %s", e.getMessage().c_str());
        //std::cerr << "Std exception: " << e.what() << std::endl;
        //std::cerr << "Unknown exception" << std::endl;
        mode = m_closed;
    }
}

// the radio control belong to the same group when they share 
// a common basic variable
bool Form::set_radio_group(var_t* v, RadioButton* radio) {
    if (v == 0 || v->type != V_STR) {
        return false;
    }

    for (WI iter = widget_map.begin(); iter != widget_map.end(); iter++) {
        gcn::Widget* widget = iter->first;
        WidgetInfo* inf = iter->second;
        if (inf->type == ctrl_radio &&
            inf->var->type == V_STR &&
            (inf->var == v || inf->var->v.p.ptr == v->v.p.ptr)) {
            // another ctrl_radio is linked to the same variable
            radio->setGroup(((RadioButton*)widget)->getGroup());
            inf->is_group_radio = true;
            return true;
        }
    }
    return false;
}

extern "C" void ui_reset() {
    if (form) {
        delete form;
        delete font;
        delete gui;
        delete input;
        delete graphics;
        delete imageLoader;
        form = 0;
    }
    osd_cls();
    SDL_ShowCursor(cursor);
    SDL_Flip(screen);
}

// BUTTON x, y, w, h, variable, caption [,type] 
//
// type can optionally be 'radio' | 'checkbox' | 'link' | 'choice'
// variable is set to 1 if a button or link was pressed (which 
// will have closed the form, or if a radio or checkbox was 
// selected when the form was closed
// 
extern "C" void cmd_button() {
    int x, y, w, h;
    var_t* v = 0;
    char* caption = 0;
    char* type = 0;
    
    if (-1 != par_massget("IIIIPSs", &x, &y, &w, &h, &v, &caption, &type)) {
        WidgetInfo* inf = new WidgetInfo();
        inf->var = v;

        form_begin();
        if (type) {
            if (strncmp("radio", type, 5) == 0) {
                inf->type = ctrl_radio;
                inf->is_group_radio = false;
                RadioButton* widget = new RadioButton();
                widget->setCaption(caption);
                widget->setGroup(caption);
                form->add_widget(widget, inf, x, y, w, h);
                inf->is_group_radio = form->set_radio_group(v, widget);
                if (v->type == V_STR && v->v.p.ptr &&
                    strcmp((const char*)v->v.p.ptr, caption) == 0) {
                    widget->setMarked(true);
                }
            } else if (strncmp("checkbox", type, 8) == 0) {
                inf->type = ctrl_check;
                CheckBox* widget = new CheckBox();
                widget->setCaption(caption);
                form->add_widget(widget, inf, x, y, w, h);
                if (v->type == V_STR && v->v.p.ptr &&
                    strcmp((const char*)v->v.p.ptr, caption) == 0) {
                    widget->setMarked(true);
                }
            } else if (strncmp("button", type, 6) == 0) {
                inf->type = ctrl_button;
                Button* widget = new Button();
                widget->setCaption(caption);
                form->add_widget(widget, inf, x, y, w, h);
            } else if (strncmp("label", type, 5) == 0) {
                inf->type = ctrl_label;
                Label* widget = new Label();
                widget->setCaption(caption);
                form->add_widget(widget, inf, x, y, w, h);
            } else if (strncmp("listbox", type, 7) == 0) {
                inf->type = ctrl_listbox;
                ListBox* widget = new ListBox();
                DropListModel* model = new DropListModel(caption, v);
                widget->setListModel(model);
                widget->setBorderSize(1);
                if (model->focus_index != -1) {
                    widget->setSelected(model->focus_index);
                }
            } else if (strncmp("dropdown", type, 8) == 0) {
                inf->type = ctrl_dropdown;
                DropDown* widget = new DropDown();
                DropListModel* model = new DropListModel(caption, v);
                widget->setListModel(model);
                if (model->focus_index != -1) {
                    widget->setSelected(model->focus_index);
                }
                form->add_widget(widget, inf, x, y, w, h);
            } else {
                ui_reset();
                rt_raise("UI: UNKNOWN TYPE: %s", type);
            }
        } else {
            inf->type = ctrl_button;
            Button* widget = new Button();
            widget->setCaption(caption);
            form->add_widget(widget, inf, x, y, w, h);
        }
    }
    pfree2(caption, type);
}

// TEXT x, y, w, h, variable
// When DOFORM returns the variable contains the user entered value
//
extern "C" void cmd_text() {
    int x, y, w, h;
    var_t* v = 0;

    if (-1 != par_massget("IIIIP", &x, &y, &w, &h, &v)) {
        form_begin();
        TextBox* widget = new TextBox();

        // prime field from var_t
        if (v->type == V_STR && v->v.p.ptr) {
            widget->setText((const char*)v->v.p.ptr);
        }

        WidgetInfo* inf = new WidgetInfo();
        inf->var = v;
        inf->type = ctrl_text;
        form->add_widget(widget, inf, x, y, w, h);
    }
}

// DOFORM [x,y,w,h]
//
// Modal syntax:
//   BUTTON ...
//   DOFORM ...
//
// Modeless syntax:
//   DOFORM 'begin modeless form
//   BUTTON ....
//   DOFORM 'continue modeless form
//
extern "C" void cmd_doform() {
    int x, y, w, h;
    int num_args;

    x = y = w = h = 0;
    num_args = par_massget("iiii", &x, &y, &w, &h);

    if (form == 0) {
        // begin modeless state - m_unset, m_init, m_modeless
        mode = m_init;
        modeless_x = x;
        modeless_y = y;
        modeless_w = w;
        modeless_h = h;
        return;
    }

    if (mode != m_unset) {
        // continue modeless state
        if (form == 0) {
            rt_raise("UI: FORM HAS CLOSED");
            return;
        }

        // set form position in initial iteration
        if (mode == m_init) {
            mode = m_modeless;
            if (num_args == 0) {
                // apply coordinates from inital doform call
                x = modeless_x;
                y = modeless_y;
                w = modeless_w;
                h = modeless_h;
            }
        } else {
            // pump system messages until button is clicked
            while (mode == m_modeless) { // && output.break_exec == 0) {
                form_iteration();
            }
            mode = m_modeless;
            form->update();
            return;
        }
    }

    switch (num_args) {
    case 0:
    case 2:
    case 4:
        break;
    default:
        ui_reset();
        rt_raise("UI: INVALID FORM ARGUMENTS: %d", num_args);
        return;
    }

    if (w < 1 || x+w > dev_w) {
        w = dev_w-x;
    }
    if (h < 1 || y+h > dev_h) {
        h = dev_h-y;
    }

    form->setDimension(gcn::Rectangle(x, y, w, h));

    if (mode == m_unset) {
        mode = m_modal;
        while (mode == m_modal) {//  && output.break_exec == 0) {
            form_iteration();
        }
        form->update();
        ui_reset();
    }
}

#endif

// End of "$Id: blib_sdl_ui.cpp,v 1.1 2007-04-28 05:28:23 zeeb90au Exp $".
