// -*- c-file-style: "java" -*-
// $Id: dev_fltk.cpp,v 1.2 2004-11-08 22:22:51 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"
#include "device.h"
#include "smbas.h"

#include <fltk/run.h>

#include "MainWindow.h"
#include "cplusplus.h"

C_LINKAGE_BEGIN

#define PEN_ON  2
#define PEN_OFF 0

extern MainWindow *wnd;

const int longSleep = 250000; // input should still be responsive
const int turboSleep = 4000;
const int menuSleep = 400000;

int osd_devinit() {
    wnd->resetPen();
	setsysvar_str(SYSVAR_OSNAME, "Unix/FLTK");
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
    if (wait_flag == 1) {
        sleep(wnd->isTurboMode() ? turboSleep : longSleep);
    }

    fltk::check();

    if (wnd->wasBreakEv()) {
        return -2;
    }

    // this may call osd_events() again but it will fall out before 
    // reaching here a second time since we just ate the event
    return dev_kbhit();
}

void osd_setpenmode(int enable) {
    wnd->penState = (enable ? PEN_ON : PEN_OFF);
}

int osd_getpen(int code) {
    if (wnd->penState == PEN_OFF) {
        sleep(wnd->isTurboMode() ? turboSleep : longSleep);
    }

    switch (code) {
    case 0:  // return true if there is a waiting pen event
        return 0;// PEN_IsDown();
    case 1:  // last pen-down x
        return wnd->penDownX;
    case 2:  // last pen-down y
        return wnd->penDownY;
    case 3:  // returns true if the pen is down (and save curpos)
        return 0;
//         if (PEN_GetPosition(&out->penX, &out->penY)) {
//             out->penDownX = out->penX;
//             out->penDownY = out->penY;
//             return 1;
//         } else {
//             return 0;
//         }
        
    case 4:  // cur pen-down x
//         if (PEN_GetPosition(&out->penX, &out->penY)) {
//             out->penDownX = out->penX;
//         }
        return wnd->penDownX;
    case 5:  // cur pen-down y
//         if (PEN_GetPosition(&out->penX, &out->penY)) {
//             out->penDownY = out->penY;
//         }
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
}

long osd_getpixel(int x, int y) {
    return 15;
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

C_LINKAGE_END

