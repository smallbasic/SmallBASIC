// -*- c-file-style: "java" -*-
// $Id: input.cpp,v 1.6 2005-01-08 23:51:14 zeeb90au Exp $
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

#include <ebjlib.h>
#include "sys.h"
#include "device.h"
#include "smbas.h"
#include "ebm_main.h"

#include "resource.h"
#include <FontOptions.h>
#include <HTMLWindow.h>

EXTERN_C_BEGIN

static Properties env;
extern SBWindow out; // in ebm_main

int putenv(const char *s) {
    Properties p;
    p.load(s);
    String* key = p.getKey(0);
    if (key == null) {
        return 0;
    }

    String* value = env.get(key->toString());
    if (value != null) {
        // property already exists
        String* newValue = p.get(key->toString());
        value->empty();
        value->append(newValue->toString());
    } else {
        // new property
        env.load(s);
    }
    return 1;
}

char* getenv(const char *s) {
    String* str = env.get(s);
    return (str ? (char*)str->toString() : null);
}

int dev_env_count() {
    return env.length() / 2;
}

char *dev_getenv_n(int n) {
    String* s = env.get(n);
    return (char*)(s == 0 || s->toString() == 0 ? "" : s->toString());
}

struct InputHTMLWindow : public HTMLWindow {
    InputHTMLWindow(const char* s, const char* title, RECT rc) :
        HTMLWindow(s, title, rc) {
        menu = createMenu(this, SB_RUN_WND);
    }
    InputHTMLWindow(const char* s, const char* title, 
                    S16 x, S16 y, U16 w, U16 h) :
        HTMLWindow(s, title, x, y, w, h) {
        menu = createMenu(this, SB_RUN_WND);
    }
    void Close();
    CMenu* menu;
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
};

S32 InputHTMLWindow::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    switch (type) {
    case MSG_KEY:
        if (data == K_MENU) {
            menu->Show();
            return 1;
        }
        break;

    case MSG_MENU_SELECT:
        if (data == -1) {
            return 1;
        }

        out.MsgHandler(type, from, data);
        if (data == mnuBreak) {
            Close();
        }
        return 1;

    default:
        break;
    }

    return HTMLWindow::MsgHandler(type, from, data);
}

void InputHTMLWindow::Close() {
    htmlView->getInputProperties(env);    
    HTMLWindow::Close();
}

void dev_html(const char* html, const char* title, int x, int y, int w, int h) {
    InputHTMLWindow* wnd;
    if (title && title[0] == 0) {
        title = 0; // make empty string null
    }
    if (x == -1 || y == -1 || w == -1 || h == -1) {
        wnd = new InputHTMLWindow(html, title, HTMLWindow::popupRect());
    } else {
        wnd = new InputHTMLWindow(html, title, x,y,w,h);
    }
    out.saveScreen();
    GUI_EventLoop(wnd); 
    out.restoreScreen();
}

#if 0
/**
 * gets a string (INPUT)
 */
char *dev_gets(char *dest, int size) {
    int code;
    word ch=0;
    word prev_x = dev_getx();
    word prev_y = dev_gety();
    word pos = 0;

    dev_clreol();
    dev_clrkb();

    *dest = '\0';

    do  {
        // wait for event
        while ((code = dev_events(1)) == 0);
        if (code < 0) { // BREAK event
            *dest = '\0';
            brun_break();
            return dest;
        }

        while (dev_kbhit()) {  // we have keys
            ch = dev_getch();
            switch (ch)   {
            case -1: 
            case -2:
                return dest;
            case 0: case 10: case 13:   // ignore
                break;
            case SB_KEY_BACKSPACE:      // backspace
                if  (pos) 
                    pos--;
                dest[pos] = '\0';
                
                // redraw
                dev_setxy(prev_x, prev_y);
                dev_clreol();
                dev_print(dest);
                break;

            case SB_KEY_LEFT:
            case SB_KEY_RIGHT:
                break;

            default:
                if ((ch & 0xFF00) != 0xFF00) { // Not an hardware key
                    // store it
                    dest[pos] = ch;
                    pos ++;
                    dest[pos] = '\0';
                }
                else
                    ch = 0;

                // check the size
                if  (pos >= (size-2))
                    break;

                // redraw
                if  (ch)  {
                    dev_setxy(prev_x, prev_y);
                    dev_clreol();
                    dev_print(dest);
                }
            }
        }   // dev_kbhit() loop
        
    } while (ch != '\n' && ch != '\r');

    dest[pos] = '\0';
    dev_setxy(prev_x, prev_y);
    dev_clreol();
    dev_print(dest);
    dev_print("\n");
    return dest;     
}

#endif

EXTERN_C_END

//--EndOfFile-------------------------------------------------------------------
