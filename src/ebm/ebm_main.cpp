// -*- c-file-style: "java" -*-
// $Id: ebm_main.cpp,v 1.6 2004-07-03 23:06:57 zeeb90au Exp $
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

#include "ebm_main.h"
#include "device.h"
#include "smbas.h"
#include "sbapp.h"
#include "keys.h"
#include "ebm.h"
#include <evnt_fun.h>

#undef open
#undef close
#undef read

#include "ebjlib.h"
#include "resource.h"
#include "FontOptions.h"
#include "AnsiWindow.h"
#include "HTMLWindow.h"
#include "Browser.h"

SBWindow out;

S32 GUI_main(MSG_TYPE type, CViewable *object, S32 data) {
    switch (type) {
    case MSG_APP_START:
        out.doShell();
        GUI_Exit();
        return 1;

    case MSG_PROXY:
        // Clicked a seb'ed .bas file the following attribute:
        // _PROXY|global|"!SmallBasic!fxe"
        if (strcmp("bas", ((ebo_name_t*)data)->extension) == 0 ||
            strcmp("sbx", ((ebo_name_t*)data)->extension)) {
            String file;
            file.append(((ebo_name_t*)data)->name);
            file.append(".");
            file.append(((ebo_name_t*)data)->extension);
            out.run(file);
            GUI_Exit();
        }
        return 1;
    default:
        return 0;
    }
}

extern "C" void dev_exit() {
    GUI_Exit();
}

SBWindow::SBWindow() : AnsiWindow(SB_RUN_WND) {
    menu = createMenu(this, SB_RUN_WND);

    isTurbo = false;
    opt_nosave = true;
    opt_quite = true;

    File f;
    if (f.open("SmallBASIC.ini")) {
        Properties p;
        p.load(f.getPtr());
        String* prop;

        prop = p.get("genexe");
        if (prop != null && prop->equals("0")) {
            opt_nosave = false;
        }
        prop = p.get("quiet");
        if (prop != null && prop->equals("0")) {
            opt_quite = false;
        }
    }
}

SBWindow::~SBWindow() {
    File f;
    if (f.open("SmallBASIC.ini", File::writeMode)) {
        f.writeLine(opt_nosave?"genexe=0\n":"genexe=1\n");
        f.writeLine(opt_quite?"quite=1\n":"quite=0\n");
    }
}

void SBWindow::run(const char* fileName) {
    isBreak = false;
    menuActive = false;
    sbasic_main(fileName);
    GUI_UpdateNow();
}

void SBWindow::doShell() {
    Browser dlg(fontOptions);
    dlg.verbose = !opt_quite;
    dlg.save = !opt_nosave;
    dlg.pause = false;

    while (true) {
        dlg.turbo = isTurbo;
        dlg.show();

        if (dlg.quit) {
            break;
        }

        if (dlg.reset) {
            clearScreen();
        } else {
            restoreScreen();
        }

        opt_quite = !dlg.verbose;
        opt_nosave = !dlg.save;
        isTurbo = dlg.turbo;
        
        if (dlg.fileName.length() > 0) {
            if (dlg.fileName.startsWith("mmc:")) {
                opt_nosave = true;
            }
            run(dlg.fileName);
            saveScreen();
        }

        if (dlg.pause || dlg.fileName.length() == 0) {
            while (EVNT_IsWaiting() == false) {
                usleep(100000);
            }

            // eat event
            event_t event;
            EVNT_GetEvent(&event);
        }
    }
}

void SBWindow::sendKeys(const char* s) {
    int len = strlen(s);
    for (int i=0; i<len; i++) {
        dev_pushkey(s[i]);
    }
    dev_pushkey('\n');
}

void SBWindow::doAboutBasFile() {
    char* about = getenv("about");
    if (about != null) {
        GUI_Alert(ALERT_INFO, about);
    } else {
        GUI_Alert(ALERT_INFO, "ENV(\"about=program\") statement not found");
    }
}

void SBWindow::doKey(S32 key) {
    // don't accept keystrokes while showing the menu
    if (isMenuActive() == false) {
        dev_pushkey(key);
    }
}

void SBWindow::doKeyboard() {
    saveScreen();
    GUI_EventLoop(new CLatin1Keyboard(0,10, 0));
    restoreScreen();
    dev_pushkey('\n');
}

S32 SBWindow::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    switch (type) {
    case MSG_MENU_SELECT:
        menuActive = false;
        if (data == -1) {
            return 1;
        }

        switch ((U16)data) {
        case mnuBreak:
            dev_pushkey(SB_KEY_BREAK);
            isBreak = true;
            break;
        case mnuExit:
            GUI_Exit();
            break;
        case mnuKeyboard:
            doKeyboard();
            break;
        case mnuTurbo:
            isTurbo = !isTurbo;
            break;
        case mnuAboutBasFile:
            doAboutBasFile();
            break;
        }
        return 1;
        
    case MSG_KEY:
        switch (data) {
        case K_MENU: 
            menuActive = true;
            menu->SetRow(2, mnuTurbo, isTurbo ? "Turbo Off" : "Turbo On");
            menu->Show();
            break;

        case K_KEYBOARD:
            return 0;
        case K_JOG_UP:
        case K_PAGE_UP:
            doKey(SB_KEY_PGUP);
            break;
        case K_UP:
            doKey(SB_KEY_UP);
            break;
        case K_JOG_DOWN:
        case K_PAGE_DOWN:
            doKey(SB_KEY_PGDN);
            break;
        case K_DOWN:
            doKey(SB_KEY_DN);
            break;
        case K_LEFT:
            doKey(SB_KEY_LEFT);
            break;
        case K_RIGHT:
            doKey(SB_KEY_RIGHT);
            break;
        case K_BACKSPACE: 
        case K_DELETE:
            doKey(SB_KEY_BACKSPACE);
            break;
        case K_JOG_ENTER:
            doKey(K_ENTER);
            break;

        default:
            if (data >= 0xf700 && data <= 0xf900) {
                return 0; // Ignore any remaining function keys
            }
            doKey(data);
            break;
        }
        return 1;
    default:
        break;
    }

    return CWindow::MsgHandler(type, from, data);
}
