// -*- c-file-style: "java" -*-
// $Id: ebm_osd.cpp,v 1.6 2004-08-13 11:33:19 zeeb90au Exp $
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
#include "ebm_main.h"
#include <evnt_fun.h>

const int longSleep = 250000; // input should still be responsive
const int turboSleep = 4000;
const int menuSleep = 400000;

EXTERN_C_BEGIN
#include <piezo.h>
#include <ereader_hostio.h>

#define PEN_ON  2
#define PEN_OFF 0

extern SBWindow out;
bool needUpdate=false;
int ticks=0;
const int ticksPerRedraw=5; // update text every 1/4 second

int osd_devinit() {
    needUpdate=false;
    out.resetPen();
    return 1;
}

void osd_setcolor(long color) {
    out.setColor(color);
}

void osd_settextcolor(long fg, long bg) {
    out.setTextColor(fg, bg);
}

void osd_refresh() {
    GUIFLAGS *guiFlags = GUI_Flags_ptr();
	if (needUpdate || (*guiFlags & GUIFLAG_NEED_FLUSH)) {
        *guiFlags &= ~GUIFLAG_NEED_FLUSH;
		imgUpdate();
	}
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
        usleep(out.isTurboMode() ? turboSleep : longSleep);
    }

    GUIFLAGS *guiFlags = GUI_Flags_ptr();
    if (needUpdate) {
        needUpdate = false;
        imgUpdate(); // update graphics operation
        *guiFlags &= ~GUIFLAG_NEED_FLUSH;
    }

    if (!EVNT_IsWaiting()) {
        if ((*guiFlags & GUIFLAG_NEED_FLUSH) == 0) {
            if (wait_flag == 0 && out.penState == PEN_ON) {
                // typically in a "while 1" loop checking for a pen 
                // event with pen(0). need to conserve battery here
                usleep(out.isTurboMode() ? turboSleep : longSleep);
            }
            return out.wasBreakEv() ? -2 : 0; // no redraw required
        }
        if (wait_flag == 1 || ++ticks==ticksPerRedraw) {
            ticks=0;
            *guiFlags &= ~GUIFLAG_NEED_FLUSH;
            imgUpdate(); // defered text update
        }
        return out.wasBreakEv() ? -2 : 0;
    }

    if (out.penState == PEN_ON && PEN_IsDown()) {
        U16 x,y;
        PEN_GetPosition(&x, &y);
        if (y < out.getHeight()) {
            return 0; 
        }
        // else pump event for system messages
    }

    GUI_EventLoop(0);
    while (out.isMenuActive()) {
        while (!EVNT_IsWaiting()) {
            usleep(menuSleep);
        }
        GUI_EventLoop(0);
    }
    if (out.wasBreakEv()) {
        return -2;
    }

    // this may call osd_events() again but it will fall out before 
    // reaching here a second time since we just ate the event
    return dev_kbhit();
}

void osd_setpenmode(int enable) {
    out.penState = (enable ? PEN_ON : PEN_OFF);
}

int osd_getpen(int code) {
    if (out.penState == PEN_OFF) {
        usleep(out.isTurboMode() ? turboSleep : longSleep);
    }

    switch (code) {
    case 0:  // return true if there is a waiting pen event
        return PEN_IsDown();
    case 1:  // last pen-down x
        return out.penDownX;
    case 2:  // last pen-down y
        return out.penDownY;
    case 3:  // returns true if the pen is down (and save curpos)
        if (PEN_GetPosition(&out.penX, &out.penY)) {
            out.penDownX = out.penX;
            out.penDownY = out.penY;
            return 1;
        } else {
            return 0;
        }
        
    case 4:  // cur pen-down x
        if (PEN_GetPosition(&out.penX, &out.penY)) {
            out.penDownX = out.penX;
        }
        return out.penDownX;
    case 5:  // cur pen-down y
        if (PEN_GetPosition(&out.penX, &out.penY)) {
            out.penDownY = out.penY;
        }
        return out.penDownY;
    }
    return 0;
}

int osd_getx() {
    return out.getX();
}

int osd_gety() {
    return out.getY();
}

void osd_setxy(int x, int y) {
    out.setXY(x,y);
}

void osd_cls() {
    out.clearScreen();
}

int osd_textwidth(const char *str) {
    return out.textWidth(str);
}

int osd_textheight(const char *str) {
    return out.textHeight();
}

void osd_setpixel(int x, int y) {
    imgPixelSetColor(imgGetBase(), x, y, 15-dev_fgcolor);
    needUpdate=true;
}

long osd_getpixel(int x, int y) {
    return 15-imgPixelGetColor(imgGetBase(), x,y);
}

void osd_line(int x1, int y1, int x2, int y2) {
    out.drawLine(x1, y1, x2, y2);
    GUIFLAGS *guiFlags = GUI_Flags_ptr();
    *guiFlags &= ~GUIFLAG_NEED_FLUSH;
    needUpdate=false;
    imgUpdate(); 
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
    if (bFill) {
        out.drawFGRectFilled(x1, y1, x2-x1, y2-y1);
    } else {
        out.drawFGRect(x1, y1, x2-x1, y2-y1);
    }
    needUpdate=true;
}

void osd_beep() {
    GUI_Beep();
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
    piezo_tone t;
    t.tone = ((32768/frq)-2)/2;
    // interval in seconds = (man*4^15-exp)/1000000
    // 4^10 = 1024 -> /= 1000000 -> ~1ms
    t.man = ms;
    t.exp = 10;
    t.unused = 0;
    piezo_play_1tone(t, 0);
    // sleep while the piezo thread plays the note
    usleep(1000*ms);
}

void osd_clear_sound_queue() {
}

void osd_write(const char *s) {
    out.print(s);
}

EXTERN_C_END
