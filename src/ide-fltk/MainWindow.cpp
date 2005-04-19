// -*- c-file-style: "java" -*-
// $Id: MainWindow.cpp,v 1.41 2005-04-19 23:52:19 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

#include <fltk/run.h>
#include <fltk/error.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/Window.h>
#include <fltk/Item.h>
#include <fltk/Group.h>
#include <fltk/TabGroup.h>
#include <fltk/TiledGroup.h>
#include <fltk/MenuBar.h>
#include <fltk/filename.h>

#include "MainWindow.h"
#include "EditorWindow.h"
#include "HelpWidget.h"
#include "sbapp.h"

extern "C" {
#include "fs_socket_client.h"
}

#if defined(WIN32) 
#include <fltk/win32.h>
#include <shellapi.h>
#endif

using namespace fltk;

enum ExecState {
    init_state,
    edit_state, 
    run_state, 
    modal_state,
    break_state, 
    quit_state
} runMode = init_state;

char buff[PATH_MAX];
char *startDir;
char *runfile = 0;
const char* toolhome = "./Bas-Home/";
int px,py,pw,ph;
MainWindow* wnd;

// in dev_fltk.cpp
bool cacheLink(dev_file_t* df, char* localFile);
void updateHelp(const char* s);
void getHomeDir(char* fileName);

const char aboutText[] =
    "<b>About SmallBASIC...</b><br><br>"
    "Copyright (c) 2000-2005 Nicholas Christopoulos.<br><br>"
    "FLTK Version 0.9.6.1<br>"
    "Copyright (c) 2005 Chris Warren-Smith.<br><br>"
    "<u>http://smallbasic.sourceforge.net</u><br><br>"
    "SmallBASIC comes with ABSOLUTELY NO WARRANTY. "
    "This program is free software; you can use it "
    "redistribute it and/or modify it under the terms of the "
    "GNU General Public License version 2 as published by "
    "the Free Software Foundation.<br><br>"
    "<i>Press F1 for help";

//--Menu callbacks--------------------------------------------------------------

void quit_cb(Widget*, void* v) {
    if (runMode == edit_state || runMode == quit_state) {
        if (wnd->editWnd->checkSave(true)) {
            exit(0);
        }
    } else {
        switch (choice("Terminate running program?",
                       "Exit", "Break", "Cancel")) {
        case 0:
            exit(0);
        case 1:
            dev_pushkey(SB_KEY_BREAK);
            runMode = break_state;
        }
    }
}

void statusMsg(const char* msg) {
    wnd->fileStatus->copy_label(msg);
    wnd->fileStatus->redraw();
}

void runMsg(const char * msg) {
    wnd->runStatus->copy_label(msg);
    wnd->runStatus->redraw();
}

void busyMessage() {
    statusMsg("Selection unavailable while program is running.");
}

void showEditTab() {
    wnd->tabGroup->selected_child(wnd->editGroup);
}

void showHelpTab() {
    wnd->tabGroup->selected_child(wnd->helpGroup);
}

void showOutputTab() {
    wnd->tabGroup->selected_child(wnd->outputGroup);
}

void browseFile(const char* url) {
#if defined(WIN32) 
    ShellExecute(xid(Window::first()), "open", url, 0,0, SW_SHOWNORMAL);
#else 
    statusMsg("Launching htmlview script...");
    if (fork() == 0) {
        fclose(stderr);
        fclose(stdin);
        fclose(stdout);
        execlp("htmlview", "htmlview", url, NULL);
        ::exit(0); // in case exec failed 
    }
#endif
}

void help_home_cb(Widget*, void* v) {
    strcpy(buff, "http://smallbasic.sf.net");
    browseFile(buff);
}

void help_contents_cb(Widget*, void* v) {
    snprintf(buff, sizeof(buff), "file:///%s/help/0_0.html", startDir);
    wnd->helpWnd->loadFile(buff);
    showHelpTab();
}

void help_readme_cb(Widget*, void* v) {
    snprintf(buff, sizeof(buff), "file:///%s/readme.html", startDir);
    wnd->helpWnd->loadFile(buff);
    showHelpTab();
}

void help_about_cb(Widget*, void* v) {
    wnd->helpWnd->loadBuffer(aboutText);
    showHelpTab();
}

void break_cb(Widget*, void* v) {
    if (runMode == run_state || runMode == modal_state) {
        runMode = break_state;
    }
}

void fullscreen_cb(Widget *w, void* v) {
    if (w->value()) {
        // store current geometry of the window
        px = wnd->x(); 
        py = wnd->y();
        pw = wnd->w(); 
        ph = wnd->h();
        wnd->fullscreen();
    } else {
        // restore geometry to the window and turn fullscreen off
        wnd->fullscreen_off(px,py,pw,ph);
    }
}

void next_tab_cb(Widget* w, void* v) {
    Widget* current = wnd->tabGroup->selected_child();
    if (current == wnd->helpGroup) {
        wnd->tabGroup->selected_child(wnd->outputGroup);
    } else if (current == wnd->outputGroup) {
        wnd->tabGroup->selected_child(wnd->editGroup);
    } else {
        wnd->tabGroup->selected_child(wnd->helpGroup);
    }
}

void turbo_cb(Widget* w, void* v) {
    wnd->isTurbo = w->value();
}

void goto_cb(Widget* w, void* v) {
    buff[0] = 0;
    const char *val = input("Goto Line:", buff);
    if (val != NULL) {
        wnd->editWnd->gotoLine(atoi(val));
    }
}

void font_size_cb(Widget* w, void* v) {
    if (runMode == edit_state) {
        buff[0] = 0;
        sprintf(buff, "%d", wnd->out->fontSize());
        const char *val = input("Output Size:", buff);
        if (val != NULL) {
            wnd->out->fontSize(min(30, max(1, atoi(val))));
            sprintf(buff, "Size %d", wnd->out->fontSize());
            statusMsg(buff);
        }
    } else {
        busyMessage();
    }
}

void basicMain(const char* filename) {
    wnd->editWnd->readonly(true);
    showOutputTab();
    wnd->out->clearScreen();
    runMode = run_state;

    statusMsg("Choose Program/Break to end");
    runMsg("RUN");

    int success = sbasic_main(filename);

    if (runMode == quit_state) {
        exit(0);
    }

    if (success == false && gsb_last_line) {
        wnd->editWnd->gotoLine(gsb_last_line);
        int len = strlen(gsb_last_errmsg);
        if (gsb_last_errmsg[len-1] == '\n') {
            gsb_last_errmsg[len-1] = 0;
        }
        statusMsg(gsb_last_errmsg);
        runMsg("ERR");
    } else {
        statusMsg("Ready");
        runMsg("");
    }

    wnd->editWnd->readonly(false);
    runMode = edit_state;
}

void run_cb(Widget*, void*) {
    const char* filename = wnd->editWnd->getFilename();
    if (runMode == edit_state) {
        if (filename == 0 || filename[0] == 0) {
            getHomeDir(buff);
            strcat(buff, "untitled.bas");
            filename = buff;
            wnd->editWnd->doSaveFile(filename, false);
        } else {
            wnd->editWnd->doSaveFile(filename, true);
        }
        basicMain(filename);
    } else {
        busyMessage();
    }
}

// callback for editor-plug-in plug-ins. we assume the target
// program will be changing the contents of the editor buffer
void editor_cb(Widget* w, void* v) {
    char filename[256];
    strcpy(filename, wnd->editWnd->getFilename());

    if (runMode == edit_state) {
        if (wnd->editWnd->checkSave(false) && filename[0]) {
            int pos = wnd->editWnd->position();
            int row,col,s1r,s1c,s2r,s2c;
            wnd->editWnd->getRowCol(&row, &col);
            wnd->editWnd->getSelStartRowCol(&s1r, &s1c);
            wnd->editWnd->getSelEndRowCol(&s2r, &s2c);
            sprintf(opt_command, "%s|%d|%d|%d|%d|%d|%d",
                    filename, row-1, col, s1r-1, s1c, s2r-1, s2c);
            runMode = run_state;
            runMsg("RUN");
            strcpy(buff, startDir);
            strcat(buff, (const char*)v);
            int success = sbasic_main(buff);
            showEditTab();
            runMsg(success ? " " : "ERR");
            wnd->editWnd->loadFile(filename, -1, true);
            wnd->editWnd->position(pos);
            wnd->editWnd->take_focus();
            runMode = edit_state;
            opt_command[0] = 0;
        }
    } else {
        busyMessage();
    }
}

void tool_cb(Widget* w, void* v) {
    if (runMode == edit_state) {
        strcpy(opt_command, startDir);
        strcat(opt_command, toolhome+1);
        setTitle((const char*)v);
        strcpy(buff, startDir);
        strcat(buff, (const char*)v);
        basicMain(buff);
        setTitle(wnd->editWnd->getFilename());
        opt_command[0] = 0;
    } else {
        busyMessage();
    }
}

//--EditWindow functions--------------------------------------------------------

void setTitle(const char* filename) {
    if (filename && filename[0]) {
        statusMsg(filename);
        char* slash = strrchr(filename, '/');
        sprintf(buff, "%s - SmallBASIC", (slash?slash+1:filename));
        wnd->copy_label(buff);
    } else {
        statusMsg("Untitled");
        wnd->label("SmallBASIC");
    }

#if defined(WIN32) 
    ::SetFocus(xid(Window::first()));
#endif
}

void setRowCol(int row, int col) {
    sprintf(buff, "%d", row);
    wnd->rowStatus->copy_label(buff);
    wnd->rowStatus->redraw();
    sprintf(buff, "%d", col);
    wnd->colStatus->copy_label(buff);
    wnd->colStatus->redraw();
}

void setModified(bool dirty) {
    wnd->modStatus->label(dirty?"MOD":"");
    wnd->modStatus->redraw();
}

void addHistory(const char* fileName) {
    getHomeDir(buff);
    strcat(buff, "history.txt");

    FILE* fp = fopen(buff, "r");
    if (fp) {
        // don't add the item if it already exists
        char buffer[1024];
        while (feof(fp) == 0) {
            if (fgets(buffer, 1024, fp) &&
                strncmp(fileName, buffer, strlen(fileName)-1) == 0) {
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen(buff, "a");
    if (fp) {
        fwrite(fileName, strlen(fileName), 1, fp);
        fwrite("\n", 1, 1, fp);
        fclose(fp);
    }
}

//--Startup functions-----------------------------------------------------------

void scanPlugIns(Menu* menu) {
    dirent **files;
    FILE* file;
    char buffer[1024];
    char label[1024];
    int numFiles = filename_list(toolhome, &files);

    for (int i=0; i<numFiles; i++) {
        const char* filename = (const char*)files[i]->d_name;
        int len = strlen(filename);
        if (strcasecmp(filename+len-4, ".bas") == 0) {
            strcpy(buffer, toolhome);
            strcat(buffer, filename);
            file = fopen(buffer, "r");

            if (!file) {
                continue;
            }
            
            if (!fgets(buffer, 1024, file)) {
                fclose(file);
                continue;
            }
            
            bool editorTool = false;
            if (strcmp("'tool-plug-in\n", buffer) == 0) {
                editorTool = true;
            } else if (strcmp("'app-plug-in\n", buffer) != 0) {
                fclose(file);
                continue;
            }
            
            if (fgets(buffer, 1024, file) && 
                strncmp("'menu", buffer, 5) == 0) {
                int offs = 6;
                buffer[strlen(buffer)-1] = 0; // trim new-line
                while (buffer[offs] && (buffer[offs] == '\t' ||
                                        buffer[offs] == ' ')) {
                    offs++;
                }
                sprintf(label, "&Basic/%s", buffer+offs);
                strcpy(buffer, toolhome+1); // use an absolute path
                strcat(buffer, filename);
                menu->add(label, 0, (Callback*)
                          (editorTool ? editor_cb : tool_cb), 
                          strdup(buffer));
            }
            fclose(file);
        }
        //this causes a stackdump
        //free(files[i]);
    }
    free(files);
}

int arg_cb(int argc, char **argv, int &i) {
    const char* s = argv[i];
    int len = strlen(s);
    if (strcasecmp(s+len-4, ".bas") == 0 && 
        access(s, 0) == 0) {
        runfile = strdup(s);
        runMode = run_state;
        i+=1;
        return 1;
    } else if (i+1 >= argc) {
        return 0;
    }

    switch (argv[i][1]) {
    case 'e':
        runfile = strdup(argv[i+1]);
        runMode = edit_state;
        i+=2;
        return 1;
    case 'r':
        runfile = strdup(argv[i+1]);
        runMode = run_state;
        i+=2;
        return 1;
    case 't':
        toolhome = strdup(argv[i+1]);
        i+=2;
        return 1;
    case 'm':
        opt_loadmod = 1;
        strcpy(opt_modlist, argv[i+1]);
        i+=2;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    int i=0;
    if (args(argc, argv, i, arg_cb) < argc) {
        fatal("Options are:\n"
              " -e[dit] file.bas\n"
              " -r[run] file.bas\n"
              " -t[ool]-home\n"
              " -m[odule]-home\n\n%s", help);
    }

    wnd = new MainWindow(600, 400);
    wnd->show(argc, argv);

#if defined(WIN32) 
    wnd->icon((char *)LoadIcon(xdisplay, MAKEINTRESOURCE(101)));
    GetCurrentDirectory(sizeof(buff), buff);
#else
    getcwd(buff, sizeof (buff));
#endif
    startDir = strdup(buff);

    check();
    switch (runMode) {
    case run_state:
        wnd->editWnd->loadFile(runfile, -1, true);
        basicMain(runfile);
        break;
    case edit_state:
        wnd->editWnd->loadFile(runfile, -1, true);
        break;
    default:
        getHomeDir(buff);
        strcat(buff, "untitled.bas");
        if (access(buff, 0) == 0) {
            // continue editing scratch buffer
            wnd->editWnd->loadFile(buff, -1, false);
        }
        setTitle(0);
        runMode = edit_state;
    }
    run();
}

//--MainWindow methods----------------------------------------------------------

MainWindow::MainWindow(int w, int h) : Window(w, h, "SmallBASIC") {
    int mnuHeight = 22;
    int statusHeight = mnuHeight;
    int groupHeight = h-mnuHeight-statusHeight-3;
    int tabBegin = 0; // =mnuHeight for top position tabs
    int pageHeight = groupHeight-mnuHeight;

    isTurbo = 0;
    opt_graphics = 1;
    opt_quite = 1;
    opt_nosave = 1;
    opt_ide = IDE_NONE; // for sberr.c
    opt_command[0] = 0;
    opt_pref_width = 0;
    opt_pref_height = 0;
    opt_pref_bpp = 0;

    int len = runfile ? strlen(runfile) : 0;
    for (int i=0; i<len; i++) {
        if (runfile[i] == '\\') {
            runfile[i] = '/';
        }
    }
    
    begin();
    MenuBar* m = new MenuBar(0, 0, w, mnuHeight);
    m->add("&File/&New File",     0,        (Callback*)EditorWindow::new_cb);
    m->add("&File/&Open File...", CTRL+'o', (Callback*)EditorWindow::open_cb);
    m->add("&File/_&Insert File...",CTRL+'i', (Callback*)EditorWindow::insert_cb);
    m->add("&File/&Save File",    CTRL+'s', (Callback*)EditorWindow::save_cb);
    m->add("&File/_Save File &As...",CTRL+SHIFT+'S', (Callback*)EditorWindow::saveas_cb);
    m->add("&File/E&xit",         CTRL+'q', (Callback*)quit_cb);
    m->add("&Edit/_&Undo",        CTRL+'z', (Callback*)EditorWindow::undo_cb);
    m->add("&Edit/Cu&t",          CTRL+'x', (Callback*)EditorWindow::cut_cb);
    m->add("&Edit/&Copy",         CTRL+'c', (Callback*)EditorWindow::copy_cb);
    m->add("&Edit/_&Paste",       CTRL+'v', (Callback*)EditorWindow::paste_cb);
    m->add("&Edit/&Find...",      CTRL+'f', (Callback*)EditorWindow::find_cb);
    m->add("&Edit/Find A&gain",   F3Key,    (Callback*)EditorWindow::find2_cb);
    m->add("&Edit/&Replace...",   F2Key,    (Callback*)EditorWindow::replace_cb);
    m->add("&Edit/_Replace &Again",CTRL+'t',(Callback*)EditorWindow::replace2_cb);
    m->add("&Edit/&Goto Line...", F4Key,    (Callback*)goto_cb);
    m->add("&Edit/Output Size...",F5Key,    (Callback*)font_size_cb);
    m->add("&View/Toggle/&Full Screen",0,   (Callback*)fullscreen_cb)->type(Item::TOGGLE);
    m->add("&View/Toggle/&Turbo", F7Key,    (Callback*)turbo_cb)->type(Item::TOGGLE);
    m->add("&View/&Next Tab",     F6Key,    (Callback*)next_tab_cb);
    scanPlugIns(m);
    m->add("&Program/&Run",       F9Key,    (Callback*)run_cb);
    m->add("&Program/&Break",     CTRL+'b', (Callback*)break_cb);
    m->add("&Help/&Help Contents",   F1Key, (Callback*)help_contents_cb);
    m->add("&Help/_&Release Notes",  F10Key,(Callback*)help_readme_cb);
    m->add("&Help/_&Home Page",      F11Key,(Callback*)help_home_cb);
    m->add("&Help/&About SmallBASIC",F12Key,(Callback*)help_about_cb);

    callback(quit_cb);
    shortcut(0); // remove default EscapeKey shortcut

    tabGroup = new TabGroup(0, mnuHeight, w, groupHeight);
    tabGroup->begin();

    editGroup = new Group(0, tabBegin, w, pageHeight, "Editor");
    editGroup->begin();
    editGroup->box(THIN_DOWN_BOX);
    editWnd = new EditorWindow(2, 2, w-4, pageHeight-4);
    m->user_data(editWnd); // the EditorWindow is callback user data (void*)
    editWnd->box(NO_BOX);
    editWnd->editor->box(NO_BOX);
    editGroup->resizable(editWnd);
    editGroup->end();
    tabGroup->resizable(editGroup);

    helpGroup = new Group(0, tabBegin, w, pageHeight, "Help");
    helpGroup->box(THIN_DOWN_BOX);
    helpGroup->hide();
    helpGroup->begin();
    helpWnd = new HelpWidget(2, 2, w-4, pageHeight-4);
    helpWnd->loadBuffer(aboutText);
    helpGroup->resizable(helpWnd);
    helpGroup->end();
    
    outputGroup = new Group(0, tabBegin, w, pageHeight, "Output");
    outputGroup->box(THIN_DOWN_BOX);
    outputGroup->hide();
    outputGroup->begin();
    out = new AnsiWindow(2, 2, w-4, pageHeight-4);
    outputGroup->resizable(out);
    outputGroup->end();

    tabGroup->end();
    resizable(tabGroup);

    Group* statusBar = new Group(0, h-mnuHeight+1, w, mnuHeight-2);
    statusBar->begin();
    statusBar->box(NO_BOX);
    fileStatus = new Widget(0,0, w-137, mnuHeight-2);
    modStatus = new Widget(w-136, 0, 33, mnuHeight-2);
    runStatus = new Widget(w-102, 0, 33, mnuHeight-2);
    rowStatus = new Widget(w-68, 0, 33, mnuHeight-2);
    colStatus = new Widget(w-34, 0, 33, mnuHeight-2);

    for (int n=0; n<statusBar->children(); n++) {
        Widget* w = statusBar->child(n);
        w->labelfont(HELVETICA);
        w->box(THIN_DOWN_BOX);
        w->color(GRAY75);
    }

    fileStatus->align(ALIGN_INSIDE_LEFT|ALIGN_CLIP);
    statusBar->resizable(fileStatus);
    statusBar->end();
    end();
}

bool MainWindow::isBreakExec(void) {
    return (runMode == break_state || runMode == quit_state);
}

void MainWindow::setModal(bool modal) {
    runMode = modal ? modal_state : run_state;
}

void MainWindow::setBreak() {
    runMode = break_state;
}

bool MainWindow::isModal() {
    return (runMode == modal_state);
}

bool MainWindow::isEdit() {
    return (runMode == edit_state);
}

void MainWindow::resetPen() {
    penDownX = 0;
    penDownY = 0;
    penState = 0;
}

void MainWindow::execLink(const char* file) {
    if (file == 0 || file[0] == 0) {
        return;
    }

    siteHome.empty();
    bool execFile = false;
    if (file[0] == '!' || file[0] == '|') {
        execFile = true; // exec flag passed with name
        file++;
    }

    // execute a link from the html window
    if (0 == strncasecmp(file, "http://", 7)) {
        char localFile[PATH_MAX];
        dev_file_t df;

        memset(&df, 0, sizeof(dev_file_t));
        strcpy(df.name, file);
        if (http_open(&df) == 0) {
            sprintf(buff, "Failed to open URL: %s", file);
            statusMsg(buff);
            return;
        }

        bool httpOK = cacheLink(&df, localFile);
        char* extn = strrchr(file, '.');

        if (httpOK && extn && 0 == strncasecmp(extn, ".bas", 4)) {
            // run the remote program
            wnd->editWnd->loadFile(localFile, -1, false);
            setTitle(file);
            addHistory(file);
            basicMain(localFile);
        } else {
            // display as html
            int len = strlen(localFile);
            if (strcasecmp(localFile+len-4, ".gif") == 0 ||
                strcasecmp(localFile+len-4, ".jpeg") == 0 ||
                strcasecmp(localFile+len-4, ".jpg") == 0) {
                sprintf(buff, "<img src=%s>", localFile);
            } else {
                sprintf(buff, "file:%s", localFile);
            }
            siteHome.append(df.name, df.drv_dw[1]);
            statusMsg(siteHome.toString());
            updateHelp(buff);
            showOutputTab();
        }
        return;
    }

    char* colon = strrchr(file, ':');
    if (colon && colon-1 != file) {
        file = colon+1; // clean 'file:' but not 'c:'
    }

    char* extn = strrchr(file, '.');
    if (extn && 0 == strncasecmp(extn, ".bas ", 5)) {
        strcpy(opt_command, extn+5);
        extn[4] = 0; // make args available to bas program
    }

    if (access(file, 0) == 0) {
        wnd->editWnd->loadFile(file, -1, true);
        setTitle(file);
        if (execFile) {
            basicMain(file);
        } else {
            showEditTab();
        }
    } else {
        sprintf(buff, "Failed to open: %s", file);
        statusMsg(buff);
    }
}

int MainWindow::handle(int e) {
    if (runMode == run_state && e == KEY && !event_key_state(RightCtrlKey)) {
        int k;
        k = event_key();
        switch (k) {
        case PageUpKey:
            dev_pushkey(SB_KEY_PGUP);
            break;
        case PageDownKey:
            dev_pushkey(SB_KEY_PGDN);
            break;
        case UpKey:
            dev_pushkey(SB_KEY_UP);
            break;
        case DownKey:
            dev_pushkey(SB_KEY_DN);
            break;
        case LeftKey:
            dev_pushkey(SB_KEY_LEFT);
            break;
        case RightKey:
            dev_pushkey(SB_KEY_RIGHT);
            break;
        case BackSpaceKey: 
        case DeleteKey:
            dev_pushkey(SB_KEY_BACKSPACE);
            break;
        case ReturnKey:
            dev_pushkey(13);
            break;
        default:
            if (k>= LeftShiftKey && k<= RightAltKey) {
                break; // ignore caps+shift+ctrl+alt
            }
            dev_pushkey(k);
            break;
        }
        return 1;
    }
    return Window::handle(e);
}

//--Debug support---------------------------------------------------------------

#if defined(WIN32)
// see http://www.sysinternals.com/ntw2k/utilities.shtml
// for the free DebugView program
#include <windows.h>
extern "C" void trace(const char *format, ...) {
    char    buf[4096],*p = buf;
    va_list args;
    
    va_start(args, format);
    p += vsnprintf(p, sizeof buf - 1, format, args);
    va_end(args);
    
    while (p > buf && isspace(p[-1])) {
        *--p = '\0';
    }
    
    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';
    OutputDebugString(buf);
}
#else
void trace(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}
#endif

//--EndOfFile-------------------------------------------------------------------
