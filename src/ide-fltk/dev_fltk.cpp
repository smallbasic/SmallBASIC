// -*- c-file-style: "java" -*-
// $Id: dev_fltk.cpp,v 1.8 2004-11-17 22:31:46 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"
#include "device.h"
#include "smbas.h"

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/FL_VERSION.h>

#include "MainWindow.h"

C_LINKAGE_BEGIN

#define PEN_ON  2
#define PEN_OFF 0

extern MainWindow *wnd;

int osd_devinit() {
    wnd->resetPen();
    os_graphics = 1;
    os_graf_mx = wnd->out->w();
    os_graf_my = wnd->out->h();
    os_ver = FL_MAJOR_VERSION+FL_MINOR_VERSION+FL_PATCH_VERSION;
    os_color = 1;
    os_color_depth = 16;
    setsysvar_str(SYSVAR_OSNAME, "FLTK");
    return 1;
}

void osd_setcolor(long color) {
    wnd->out->setColor(color);
}

void osd_settextcolor(long fg, long bg) {
    wnd->out->setTextColor(fg, bg);
}

void osd_refresh() {
    wnd->out->redraw();
}

int osd_devrestore() {
    return 1;
}

/**
 *   system event-loop
 *   return value:
 *     0 continue 
 *    -1 close sbpad application
 *    -2 stop running basic application
 */
int osd_events(int wait_flag) {
    if (wait_flag || (wnd->penState == PEN_ON && fltk::ready() == false)) {
        // in a "while 1" loop checking for a pen/mouse
        // event with pen(0) or executing input statement. 
        fltk::wait();
    }

    fltk::check();
    return wnd->isBreakExec() ? -2 : 0;
}

void osd_setpenmode(int enable) {
    wnd->penState = (enable ? PEN_ON : PEN_OFF);
}

int osd_getpen(int code) {
    if (wnd->penState == PEN_OFF) {
        fltk::wait();
    }

    switch (code) {
    case 0: // return true if there is a waiting pen event
    case 3: // returns true if the pen is down (and save curpos)
        if (wnd->isTurboMode() == false) {
            wait();
        }
        if (event_state() & ANY_BUTTON) {
            fltk::get_mouse(wnd->penDownX, wnd->penDownY);
            // convert mouse screen rect to out-client rect
            wnd->penDownX -= wnd->x() + wnd->out->x();
            wnd->penDownY -= wnd->y() + wnd->out->y();
            wnd->penDownY -= wnd->tabGroup->y();
            return 1;
        }
        return 0;

    case 1:  // last pen-down x
        return wnd->penX;

    case 2:  // last pen-down y
        return wnd->penY;
       
    case 4:  // cur pen-down x
        wnd->penX = wnd->penDownX;
        return wnd->penDownX;

    case 5:  // cur pen-down y
        wnd->penY = wnd->penDownY;
        return wnd->penDownY;
    }
    return 0;
}

int osd_getx() {
    return wnd->out->getX();
}

int osd_gety() {
    return wnd->out->getY();
}

void osd_setxy(int x, int y) {
    wnd->out->setXY(x,y);
}

void osd_cls() {
    wnd->out->clearScreen();
}

int osd_textwidth(const char *str) {
    return (int)wnd->out->textWidth(str);
}

int osd_textheight(const char *str) {
    return wnd->out->textHeight();
}

void osd_setpixel(int x, int y) {
    wnd->out->setPixel(x, y, dev_fgcolor);
}

long osd_getpixel(int x, int y) {
    return wnd->out->getPixel(x,y);
}

void osd_line(int x1, int y1, int x2, int y2) {
    wnd->out->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
    if (bFill) {
        wnd->out->drawFGRectFilled(x1, y1, x2-x1, y2-y1);
    } else {
        wnd->out->drawFGRect(x1, y1, x2-x1, y2-y1);
    }
}

void osd_beep() {
    //    fl_beep();
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
}

void osd_clear_sound_queue() {
}

void osd_write(const char *s) {
    wnd->out->print(s);
}

// empty implementations
void dev_html(const char* html, const char* title, int x, int y, int w, int h) {
}

void dev_image(int handle, int index, int x, int y, 
               int sx, int sy, int w, int h) {
}

int dev_image_width(int handle, int index) {
    return -1;
}

int dev_image_height(int handle, int index) {
    return -1;
}

void enter_cb(Widget*, void* v) {
    wnd->setModal(false);
}

struct LineInput : public fltk::Input {
    LineInput(int def_w) : 
        fltk::Input(wnd->out->getX()+2, 
                    wnd->out->getY()+1, 
                    def_w,
                    wnd->out->textHeight()+4) {
        this->def_w = def_w;
    }
    bool replace(int b, int e, const char* text, int ilen) {
        // grow the input box width
        if (ilen) {
            int strw = (int)getwidth(value())+def_w;
            if (strw > w()) {
                w(strw);
                redraw();
            }
        }
        return Input::replace(b, e, text, ilen);
    }
    int def_w;
};

char *dev_gets(char *dest, int size) {
    wnd->outputGroup->begin();
    LineInput* in = new LineInput(20);
    wnd->outputGroup->end();
    in->callback(enter_cb);
    in->when(WHEN_ENTER_KEY_ALWAYS);
    in->box(BORDER_BOX);
    in->color(color(220,220,220));
    in->take_focus();
    in->reserve(size);
    in->textfont(wnd->out->labelfont());
    in->textsize(wnd->out->labelsize());

    wnd->setModal(true);
    while (wnd->isModal()) {
        wait();
    }

    if (wnd->isBreakExec()) {
        brun_break();
    }

    wnd->outputGroup->remove(in);
    int len = in->size() < size ? in->size() : size;
    strncpy(dest, in->value(), len);
    dest[len] = 0;
    delete in;

    return dest;
}

C_LINKAGE_END

