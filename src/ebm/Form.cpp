/*
 * eBookman java-ish libraries
 * Copyright(C) 2001 Chris Warren-Smith. Gawler, South Australia
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

#include "ebjlib.h"
#include "form.h"

Form::Form(U16 id) : CWindow(id, 0,0,0,0) {
    createWindow(this);
    menuBar = createMenubar(this);
}

Form::~Form() {
    if (menuBar != null) {
        delete menuBar;
    }
}

S32 Form::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    switch (type) {
    case MSG_BUTTON_SELECT:
        switch (from->GetID()) {
        case IDOK:
            Close();
            break;
        }
        return 1;

    case MSG_KEY:
        if (isModal()) {
            break;
        }

        switch (data) {
        case K_MENU:
            if (menuBar != null) {
                menuBar->Show();
            }
            return 1;
        default:
            break;
        }
    default:
        break;
    }
    return CWindow::MsgHandler(type, from, data);
}
