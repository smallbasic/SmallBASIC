// -*- c-file-style: "java" -*-
// $Id: Browser.cpp,v 1.1 2004-04-12 00:25:03 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
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

#include <pkg.h>
#include <ebjlib.h>
#include <FontOptions.h>
#include <FontDlg.h>
#include <HTMLWindow.h>
#include <FileList.h>
#include <Editor.h>
#include "Browser.h"
#include "resource.h"

#define MMC_ACCESS true
#define MSG_REFRESH   ((enum MSG_TYPE)(MSG_USER+12))
#define MSG_RUN       ((enum MSG_TYPE)(MSG_USER+13))
#define MSG_SHOW_OUT  ((enum MSG_TYPE)(MSG_USER+14))

const char aboutHTML[] =
    "<font size=2>SmallBASIC for <u>e</u><i>Book</i><b>Man</b>"
    "<br>Version 0.9.0g<br><br><font size=1>"
    "Copyright (c) 2000-2004 Nicholas Christopoulos, "
    "Chris Warren-Smith<br>"
    "<u>http://smallbasic.sourceforge.net</u><br>"
    "<p>SmallBASIC comes with ABSOLUTELY NO "
    "WARRANTY. This program is free software; "  
    "you can use it redistribute it and/or modify "
    "it under the terms of the GNU General "
    "Public License version 2 as published by "
    "the Free Software Foundation.";

const char title[] = "SmallBASIC v0.9.0g";

CViewable* browseWnd=0;
CViewable* editor = 0;

int helpIndex = 1;
int helpScroll = 0;
bool launching = 0;

//--HelpWindow------------------------------------------------------------------

struct HelpWindow : public HTMLWindow {
    HelpWindow();
    ~HelpWindow();
    CMenu* menu;
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
};

HelpWindow::HelpWindow() : 
    HTMLWindow("SmallBASIC.help not found", title, 
               0, 0, LCD_QueryWidth(), LCD_QueryHeight()) {
    ebo_name_t pkgHelp = {"", "", "SmallBASIC", "help"};
    if (hostIO_is_simulator()) {
        strcpy(pkgHelp.publisher, "sim");
    }
    ebo_enumerator_t ee;
    if (ebo_locate_object(&pkgHelp, &ee) == EBO_OK) {
        pkg = PKG_Open(&pkgHelp);
        loadAnchors(PKG_String(pkg, 0));
        htmlView->loadPage(PKG_String(pkg, helpIndex));
        if (helpScroll != 0) {
            GUI_UpdateNow();
            htmlView->navigateTo(helpScroll);
        }
        menu = createMenu(this, ID_HELP);
    }
}

HelpWindow::~HelpWindow() {
    helpScroll = htmlView->getScroll();
}

S32 HelpWindow::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
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

        switch ((U16)data) {
        case mnuClose:
            Close();
        }
        return 1;

    case MSG_HYPERLINK:
        helpIndex = data;
        break;

    default:
        break;
    }

    return HTMLWindow::MsgHandler(type, from, data);
}

//--CodeEditor------------------------------------------------------------------

struct CodeEditor : public Editor {
    CodeEditor() : Editor("SmallBASIC") {
        menuBar = createMenubar(this, ID_EDITOR);
    }
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
};

S32 CodeEditor::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    switch (type) {
    case MSG_KEY:
        if (data == K_MENU) {
            menuBar->Show();
        } else if (from != activeView) {
            activeView->MsgHandler(MSG_KEY, from, data);
            SetFocus(activeView);
            if (dirty == false) {
                dirty = true;
                createTitle();
            }
        } 
        return 1;

    case MSG_TEXT_CHANGED:
        if (dirty == false) {
            dirty = true;
            createTitle();
        }
        break;

    case MSG_MENU_SELECT:
        if (data == -1) {
            return 1;
        }

        switch ((U16)data) {
        case mnuSave:
            fileSave();
            break;

        case mnuSaveAs:
            fileSaveAs();
            break;

        case mnuOpen:
            if (promptSave()) {
                launching = false;
                browseWnd->MsgHandler(MSG_REFRESH, this, 0);
                Hide();
            }
            break;

        case mnuSaveRun:
            fileSave();
            launching = true;
            Hide();
            GUI_EventMessage(MSG_RUN, browseWnd, 0);
            break;

        case mnuShowOutput:
            launching = true;
            Hide();
            GUI_EventMessage(MSG_SHOW_OUT, browseWnd, 0);
            break;

        case mnuExit:
            if (promptSave()) {
                GUI_Exit();
            }
            break;

        case mnuCut:
            onCut();
            break;

        case mnuCopy:
            onCopy();
            break;

        case mnuPaste:
            onPaste();
            break;

        case mnuClear:
            onClear();
            break;

        case mnuSelectAll:
            onSelectAll();
            break;

        case mnuFind:
            editFind(false);
            break;

        case mnuFindNext:
            editFind(true);
            break;

        case mnuWhatLine: {
            char t[20];
            sprintf(t, "Line %d", currentLine()+1);
            GUI_Alert(ALERT_INFO, t);
            break;
        }

        case mnuHelp:
            GUI_EventLoop(new HelpWindow());
            break;

        case mnuAbout:
            GUI_EventLoop(new HTMLWindow(aboutHTML, "About", 
                                         HTMLWindow::popupRect()));
        }
        return 1;
        
    default:
        break;
    }
    return Editor::MsgHandler(type, from, data);
}

//--BrowserWnd------------------------------------------------------------------

struct BrowserWnd : public HTMLWindow {
    BrowserWnd(Browser& dlg, const char* html) : 
        HTMLWindow(html, title, 0, 0, 
                   LCD_QueryWidth(), LCD_QueryHeight(), false), dlg(dlg) {
        menu = createMenu(this, ID_MAINWND);
        browseWnd = this;

        File f;
        if (f.open("_cmd.bas", File::readMode)) {
            basCommand = f.readLine();
        }
        if (dlg.resumeEdit && launching) {
            editor->Show();
        }
        launching = false;
    }        

    virtual ~BrowserWnd() {
        saveCommand();
    }
    
    void doAnchor(CViewable *from);
    void doButton(CViewable *from);
    void doRefresh();
    void setOption(const char* control, bool active);
    void Draw();
    void saveCommand();
    void updateCommand();
    CMenu* menu;
    Browser& dlg;
    String basCommand;
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
};

S32 BrowserWnd::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    switch (type) { 
    case MSG_REFRESH:
        doRefresh();
        return 1;

    case MSG_HYPERBUTTON:
        doButton(from);
        break;

    case MSG_PASTE:
        // sent from keyboard commmad
        htmlView->getInput("prog")->MsgHandler(type, from, data);
        break;

    case MSG_KEY:
        if (dlg.activeTab != tabSplash && data == K_MENU && menu != null) {
            menu->Show();
        }
        return 1;

    case MSG_MENU_SELECT:
        if (data == -1) {
            return 1;
        }
        switch ((U16)data) {
        case mnuExit:
            dlg.quit = true;
            Close();
            break;

        case mnuShowOutput:
            dlg.fileName.empty();
            Close();
            break;
            
        case mnuHelp:
            GUI_EventLoop(new HelpWindow());
            break;
        }
        return 1;

    case MSG_BUTTON_SELECT:
        switch (from->GetID()) {
        case ID_BUTTON:
            GUI_EventMessage(MSG_HYPERBUTTON, this, data);
            break;

        case ID_ANCHOR:
            doAnchor(from);
            break;

        case ID_CHKBOX:
            setOption(htmlView->getInputName(from), data);
            break;
        }
        return 1;
        
    case MSG_RUN: 
        dlg.fileName = ((CodeEditor*)editor)->getFileName();
        Close();
        break;

    case MSG_SHOW_OUT:
        dlg.fileName.empty();
        Close();
        break;
        
    default:
        break;
    }

    return HTMLWindow::MsgHandler(type, from, data);
}

void BrowserWnd::Draw() {
    if (launching == false) {
        HTMLWindow::Draw();
        
        if (dlg.activeTab == tabRun && basCommand.length()) {
            CTextEdit* te = (CTextEdit*)htmlView->getInput("prog");
            te->SetText(basCommand.toString());
        }
    }
}

void BrowserWnd::updateCommand() {
    if (dlg.activeTab == tabRun) {
        CTextEdit* te = (CTextEdit*)htmlView->getInput("prog");
        basCommand = te->GetTextHandle();
    }
}

void BrowserWnd::saveCommand() {
    File f;
    if (basCommand.length() > 0 && f.open("_cmd.bas", File::writeMode)) {
        f.writeLine(basCommand);
    }
}

void BrowserWnd::setOption(const char* control, bool active) {
    if (strcmp("verbose", control) == 0) {
        dlg.verbose = active;
    } else if (strcmp("save", control) == 0) {
        dlg.save = active;
    } else if (strcmp("turbo", control) == 0) {
        dlg.turbo = active;
    } else if (strcmp("pause", control) == 0) {
        dlg.pause = active;
    } else if (strcmp("resume", control) == 0) {
        dlg.resumeEdit = active;
    }
}

void BrowserWnd::doRefresh() {
    dlg.emptyFileList();
    dlg.updateFileList();
    dlg.makeActivePage();
    htmlView->loadPage(dlg.html.toString());
}

void BrowserWnd::doAnchor(CViewable *from) {
    String anchor = getAnchorLink(from);

    if (anchor.startsWith("T")) {
        updateCommand();
        switch (anchor.charAt(1)) {
        case '0':
            dlg.activeTab = tabPrograms;
            break;
        case '1':
            dlg.activeTab = tabRun;
            break;
        case '2':
            dlg.activeTab = tabSettings;
            break;
        case '3':
            dlg.activeTab = tabAbout;
            break;
        }

        dlg.makeActivePage();
        htmlView->loadPage(dlg.html.toString());

    } else {
        switch (dlg.activeTab) {
        case tabPrograms:
            if (anchor.startsWith("R")) {
                // refresh
                doRefresh();
            } else if (anchor.startsWith("E")) {
                // edit
                dlg.editFile(anchor.toString()+1);
            } else if (anchor.startsWith("N")) {
                // new
                dlg.newFile();
            } else {
                // launch
                dlg.fileName = anchor.toString()+1;
                Close();
            }
            return;

        case tabRun:
            basCommand.empty();
            dlg.doKeyboard((CTextEdit*)htmlView->getInput("prog"), basCommand);
            break;

        case tabSettings:
            dlg.doSetFont();
            break;

        default:
            break;
        }
    }

    GUI_NeedUpdate();            
}

void BrowserWnd::doButton(CViewable *from) {
    switch (dlg.activeTab) {
    case tabSplash:
        dlg.activeTab = tabPrograms;
        dlg.makeActivePage();
        htmlView->loadPage(dlg.html.toString());
        break;

    case tabRun:
        updateCommand();
        saveCommand();
        if (basCommand.length()) {
            dlg.fileName = "_cmd.bas";
            Close();
        }
        return;

    default:
        return;
    }

    GUI_NeedUpdate();            
}

//--Browser---------------------------------------------------------------------

Browser::Browser(FontOptions& fontOptions) : fontOptions(fontOptions) {
    activeTab = tabSplash;
    quit = false;
    resumeEdit = true;
    editor = new CodeEditor();
}

Browser::~Browser() {
    delete editor;
}

void Browser::editFile(const char* fileName) {
    CodeEditor* ed = (CodeEditor*)editor;
    if (ed->isLoaded(fileName) == false) {
        if (ed->promptSave() == false) {
            return;
        }
        ed->doLoad(fileName);
    }
    launching = false;
    ed->Show();
}

void Browser::newFile() {
    CodeEditor* ed = (CodeEditor*)editor;
    if (ed->promptSave() == false) {
        return;
    }
    ed->fileNew();
    ed->Show();
}

void Browser::doSetFont() {
    FontDlg dlg(fontOptions);
    dlg.show();
    if (dlg.getExitCode() == ID_OK) {
        fontOptions.openFontGroup();
        reset = true;
    }
}

void Browser::doKeyboard(CTextEdit* te, String& cmd) {
    te->SetSelection(0, te->TextLength());
    GUI_EventLoop(new CLatin1Keyboard(0, 42, te->GetTextHandle()));
    cmd = te->GetTextHandle();
}

void Browser::createLink(const char* fileName, bool basFile, bool sbxFile) {
    if (strcmp(fileName, "_cmd") == 0) {
        return;
    }

    String basName;
    basName.append(fileName);
    basName.append(".bas");
    bool loaded = ((CodeEditor*)editor)->isLoaded(basName);

    html.append("<tr><td>");
    if (loaded) {
        html.append("<b>");
        html.append(fileName);
        html.append("</b>");
    } else {
        html.append(fileName);
    }
    
    html.append("</td><td offs=18>[ ");

    if (sbxFile) {
        html.append("<a href=F");
        html.append(fileName);
        html.append(".sbx>sbx</a>");
    } else {
        html.append("sbx");
    }

    html.append(" | ");

    if (basFile) {
        // show edit link
        html.append("<a href=F");
        html.append(fileName);
        html.append(".bas>bas</a> | <a href=E");
        html.append(fileName);
        html.append(".bas>edit</a>");
    } else {
        html.append("bas");
    }

    html.append(" ]</td></tr>");
}

void Browser::createProgramsPage() {
    const char* sbxItem;
    const char* basItem;

    int basLen = basFiles.length();
    int sbxLen = sbxFiles.length();
    int basi = 0;

    for (int sbxi=0; sbxi<sbxLen; sbxi++) {
        sbxItem = sbxFiles.get(sbxi);
        int basCmp = 0;
        
        // process bas files prior to the next sbx file
        while (basi < basLen) {
            basItem = basFiles.get(basi);
            basCmp = strcmp(basItem, sbxItem);
            if (basCmp >= 0) {
                // bas file same or after sbx file
                break;
            }
            createLink(basItem, true, false);
            basi++;
        }

        // process the sbx file
        if (basi < basLen) {
            createLink(sbxItem, (basCmp==0), true);
            if (basCmp == 0) {
                basi++;
            }
        } else {
            createLink(sbxItem, false, true);            
        }
    }

    // process remaining bas files
    while (basi < basLen) {
        createLink(basFiles.get(basi++), true, false);
    }

    html.append("<br>[ <a href=N>New</a> | <a href=R>Refresh</a> ]");
}

void Browser::createRunPage() {
    html.append("Program statement:<br><input type=text name=prog ");
    html.append(" size=185><br><br>[ <a href=kb>Keyboard...</a> ]");
    html.append("<br><br><input type=button value=Run>");
}

void Browser::createSettingsPage() {
    createCheckBox("Turbo mode", "turbo", turbo);
    createCheckBox("Pause on completion", "pause", pause);
    createCheckBox("Verbose output", "verbose", verbose);
    createCheckBox("Save output", "save", save);
    createCheckBox("Resume edit on completion", "resume", resumeEdit);
    html.append("[ <a href=font>Font...</a> ]<br>");
}

void Browser::createCheckBox(const char* label, const char* name, bool active) {
    html.append(label);
    html.append(": <input type=checkbox name=");
    html.append(name);
    if (active) {
        html.append(" checked=true");
    }
    html.append("></a><br>");
}

void Browser::makeActivePage() {
    html.empty();

    if (activeTab == tabSplash) {
        html.append("<br>");
        html.append(aboutHTML);
        html.append("<br><br><br><center><input type=button value=OK>");
        return;
    }
    
    const char* tabs[] = {
        "Open", "Run", "Setup", "About", 0
    };

    // create tabs
    int i=0;
    html.append("<font size=2>[ ");
    while (true) {
        if (i == activeTab) {
            html.append("<b>");
            html.append(tabs[i]);
            html.append("</b>");
        } else {
            html.append("<a href=T");
            html.append(i);
            html.append(">");
            html.append(tabs[i]);
            html.append("</a>");
        }
        if (tabs[++i] == 0) {
            break;
        }
        html.append(" | ");
    }
    html.append(" ]<hr>");
    html.append("<font size=1><br>");
    // create page
    switch (activeTab) {
    case tabPrograms:
        createProgramsPage();
        break;
        
    case tabRun:
        createRunPage();
        break;

    case tabSettings:
        createSettingsPage();
        break;
        
    case tabAbout:
        html.append(aboutHTML);
        break;
    default:
        break;
    }
}

void Browser::updateFileList() {
    if (sbxFiles.length() == 0 || save) {
        sbxFiles.create("sbx", MMC_ACCESS, false);
    }
    
    if (basFiles.length() == 0) {
        basFiles.create("bas", MMC_ACCESS, false);
    }
}

void Browser::emptyFileList() {
    sbxFiles.empty();
    basFiles.empty();
}

void Browser::show() {
    updateFileList();
    makeActivePage();
    fileName.empty();
    reset = false;
    //launching = false;
    GUI_EventLoop(new BrowserWnd(*this, html));
}

//--EndOfFile-------------------------------------------------------------------
