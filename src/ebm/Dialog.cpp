/*
 * TapWrite - edit and save text files on your eBookman!
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
#include "resource.h"
#include "Dialog.h"

struct DialogWnd : public CWindow {
    DialogWnd(Dialog &dialog, U16 formId) :
        CWindow(formId, 0,0,0,0),
        dialog(dialog) {
        createWindow(this);
    }

    Dialog &dialog;
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
};

int Dialog::show(U16 formId) {
    GUI_EventLoop(new DialogWnd(*this, formId)); 
    return exitCode;
}

S32 DialogWnd::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    if (type == MSG_BUTTON_SELECT) {
        dialog.setExitCode(from->GetID());
        Close();
        return 1;
    }
    return CWindow::MsgHandler(type, from, data);
}

