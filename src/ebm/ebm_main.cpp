/**
 * -*- c-file-style: "java" -*-
 * SmallBASIC for eBookMan
 * Copyright(C) 2001-2002 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 *                  _.-_:\
 *                 /      \
 *                 \_.--*_/
 *                       v
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 * 
 */

#include "ebm_main.h"

#include "device.h"
#include "smbas.h"
#include "keys.h"
#include "ebm.h"

#undef open
#undef close
#undef read

#include "ebjlib.h"
#include "resource.h"
#include "AnsiWindow.h"
#include "HTMLWindow.h"
#include "FileDlg.h"

SBWindow out;
extern byte opt_quite; // quiet

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

SBWindow::SBWindow() : AnsiWindow(SB_MAIN_WND) {
    shellMenu = createMenubar(this);
    runMenu = createMenubar(this, SB_RUN_WND);

    ebo_name_t pkgHelp = {"", "", "SmallBASIC", "help"};
    if (hostIO_is_simulator()) {
        strcpy(pkgHelp.publisher, "Developer");
    }
    ebo_enumerator_t ee;
    if (ebo_locate_object(&pkgHelp, &ee) == EBO_OK) {
        pkg = PKG_Open(&pkgHelp);
    }

    runState = rsIdle;
    bIsTurbo = true;
    bGenExe = true;
    opt_quite = true;

    File f;
    if (f.open("SmallBASIC.ini")) {
        Properties p;
        p.load(f.getPtr());
        String* prop;

        prop = p.get("genexe");
        if (prop != null && prop->equals("0")) {
            bGenExe = false;
        }
        prop = p.get("quiet");
        if (prop != null && prop->equals("0")) {
            opt_quite = false;
        }
    }
}

void SBWindow::run(const char* file) {
    if (runState != rsBrun) {
        runState = rsBrun;
        bIsTurbo = false;
        menuActive = false;
        char fileName[EBO_LEN_NAME+EBO_LEN_EXT+5];
        strcpy(fileName, file);
        brun(fileName, bGenExe);
        runState = rsIdle;
    }
}

void SBWindow::doShell() {
    write("Welcome to SmallBASIC\n\n");
    doAbout();
    const char prompt[] = "sb:>";
    char buffer[100];
    while(true) {
        write(prompt);
        runState = rsShell;
        bIsTurbo = true;
        char *cmd = dev_gets(buffer, sizeof(buffer));
        if (cmd == 0 || *cmd == 0) {
            continue;
        }
        switch (runState) {
        case rsSelFile:
            run(cmd);
            break;
        case rsAbout:
            clearScreen();
            doAbout();
            break;
        default:
            File f;
            f.open("SmallBASIC.cmd", File::writeMode);
            f.write(cmd, strlen(cmd));
            f.close();
            int quiet = opt_quite;
            int genExe = bGenExe;
            opt_quite = 1;
            bGenExe = false;
            run("SmallBASIC.cmd");
            opt_quite = quiet;
            bGenExe = genExe;
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

void SBWindow::doHelp() {
    saveScreen();
    if (pkg != null) {
        GUI_EventLoop(new HTMLWindow(pkg));
    } else {
        GUI_Alert(ALERT_OK, "SmallBASIC.help not found");
    }
    restoreScreen();
}

void SBWindow::doAbout() {
    #define EBOOKMAN "\033[4me\033[0m\337\033[3mook\033[0;1mMan\033[0m\256"
    const char about[] = 
        "SmallBASIC for " EBOOKMAN " v0.8.3d\n"
        "Written by Nicholas Christopoulos\n" EBOOKMAN
        " edition by Chris Warren-Smith\n" 
        "\033[4mhttp://smallbasic.sourceforge.net\033[0m\n\n"
        "SmallBASIC comes with ABSOLUTELY NO\n" 
        "WARRANTY. This program is free software;\n" 
        "you can use it redistribute it and/or modify\n" 
        "it under the terms of the GNU General\n"
        "Public License version 2 as published by\n" 
        "the Free Software Foundation.\n\n";
    write(about);
}

void SBWindow::doAboutBasFile() {
    char* about = getenv("about");
    if (about != null) {
        GUI_Alert(ALERT_INFO, about);
    } else {
        GUI_Alert(ALERT_INFO, 
                  "ENV(\"about=program info\") statement not found");
    }
}

void SBWindow::doKeyboard() {
    File f;
    char buffer[100];
    buffer[0] = 0;
    if (f.open("SmallBASIC.cmd", File::readMode)) {
        f.read(buffer, sizeof(buffer));
        f.close();
    }

    saveScreen();
    GUI_EventLoop(new CLatin1Keyboard(0,10, buffer));
    restoreScreen();
    dev_pushkey('\n');
}

void SBWindow::doKey(S32 key) {
    // don't accept keystrokes while showing the menu
    if (isMenuActive() == false) {
        dev_pushkey(key);
    }
}

void SBWindow::doList() {
    if (runState == rsShell) {
        FileDlg dlg;
        saveScreen();
        if (dlg.show(false) != ID_CANCEL) {
            sendKeys(dlg.getFileName());
            runState = rsSelFile;
        }
        restoreScreen();        
    }
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
            if (runState == rsBrun) {
                dev_pushkey(SB_KEY_BREAK);
                runState = rsBreak;
            }
            break;
        case mnuClose:
            GUI_Exit();
            break;
        case mnuHelp:
            doHelp();
            break;
        case mnuList:
            doList();
            break;
        case mnuAbout:
            sendKeys("about");
            runState = rsAbout;
            break;
        case mnuKeyboard:
            doKeyboard();
            break;
        case mnuTurbo:
            bIsTurbo = !bIsTurbo;
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
            if (runState == rsBrun) {
                CMenu *menu = runMenu->GetMenu(0);
                menu->SetRow(2, mnuTurbo, bIsTurbo ? "Turbo Off" : "Turbo On");
                runMenu->Show();
            } else {
                shellMenu->Show();
            }
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

    case MSG_PEN_DOWN:
    case MSG_PEN_DOUBLECLICK:
    case MSG_PEN_TRACK:
        penX = data >> 16;
        penY = data;
        penUpdate = true;
        if (type == MSG_PEN_DOWN) {
            penDownX = penX;
            penDownY = penY;
            penDown = true;
        }
        return 1;
    case MSG_PEN_UP:
        penUpdate = true;
        penDown = false;
        return 1;

    default:
        break;
    }

    return CWindow::MsgHandler(type, from, data);
}
