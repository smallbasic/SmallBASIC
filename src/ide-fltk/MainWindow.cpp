// -*- c-file-style: "java" -*-
// $Id: MainWindow.cpp,v 1.24 2004-12-16 22:15:13 zeeb90au Exp $
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
#include <fltk/ask.H>
#include <fltk/events.h>
#include <fltk/Window.h>
#include <fltk/Item.h>
#include <fltk/Group.h>
#include <fltk/TabGroup.h>
#include <fltk/MenuBar.h>
#include <fltk/filename.h>

#include "MainWindow.h"
#include "EditorWindow.h"
#include "sbapp.h"

extern "C" {
#include "fs_socket_client.h"
}

#ifdef __CYGWIN__
#include <fltk/win32.h>
#endif

using namespace fltk;

MainWindow* wnd;

enum ExecState {
    init_state,
    edit_state, 
    run_state, 
    modal_state,
    break_state, 
    quit_state
} runMode = init_state;

char buff[PATH_MAX];
char *runfile = 0;
const char* toolhome = "./Bas-Home/";
int px,py,pw,ph;

const char aboutText[] =
    "About SmallBASIC...\n\n"
    "Copyright (c) 2000-2004 Nicholas Christopoulos.\n\n"
    "FLTK Version 0.9.6.0\n"
    "Copyright (c) 2004 Chris Warren-Smith.\n\n"
    "http://smallbasic.sourceforge.net\n\n"
    "SmallBASIC comes with ABSOLUTELY NO WARRANTY.\n"
    "This program is free software; you can use it\n"
    "redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License version 2 as published by\n"
    "the Free Software Foundation.";

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

void about_cb(Widget*, void* v) {
    message(aboutText);
}

void busyMessage() {
    wnd->fileStatus->label("Selection unavailable while program is running.");
    wnd->fileStatus->redraw();
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
            wnd->fileStatus->copy_label(buff);
            wnd->fileStatus->redraw();
        }
    } else {
        busyMessage();
    }
}

void basicMain(const char* filename) {
    wnd->editWnd->readonly(true);
    wnd->tabGroup->selected_child(wnd->outputGroup);
    wnd->out->clearScreen();
    runMode = run_state;
    wnd->runStatus->label("RUN");
    wnd->runStatus->redraw();
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
        wnd->fileStatus->copy_label(gsb_last_errmsg);
    }

    wnd->runStatus->label(success ? " " : "ERR");
    wnd->editWnd->readonly(false);
    wnd->redraw();
    runMode = edit_state;
}

void run_cb(Widget*, void*) {
    const char* filename = wnd->editWnd->getFileName();
    if (runMode == edit_state) {
        if (wnd->editWnd->checkSave(false) && filename[0]) {
            basicMain(filename);
        }
    } else {
        busyMessage();
    }
}

// callback for editor-plug-in plug-ins. we assume the target
// program will be changing the contents of the editor buffer
void editor_cb(Widget* w, void* v) {
    char filename[256];
    strcpy(filename, wnd->editWnd->getFileName());

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
            wnd->runStatus->label("RUN");
            wnd->runStatus->redraw();
            int success = sbasic_main((const char* )v);
            wnd->tabGroup->selected_child(wnd->editGroup);
            wnd->runStatus->label(success ? " " : "ERR");
            wnd->editWnd->loadFile(filename, -1);
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
        strcpy(opt_command, toolhome);
        setTitle((const char*)v);
        basicMain((const char*)v);
        setTitle(wnd->editWnd->getFileName());
        opt_command[0] = 0;
    } else {
        busyMessage();
    }
}

//--EditWindow functions--------------------------------------------------------

void setTitle(const char* filename) {
    if (filename && filename[0]) {
        wnd->fileStatus->copy_label(filename);
        char* slash = strrchr(filename, '/');
        sprintf(buff, "%s - SmallBASIC", (slash?slash+1:filename));
        wnd->copy_label(buff);
    } else {
        wnd->fileStatus->label("Untitled");
        wnd->label("SmallBASIC");
    }
    wnd->redraw();
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

void getHomeDir(char* fileName) {
    sprintf(fileName, "%s/.smallbasic/", dev_getenv("HOME"));
	mkdir(fileName, 0777);
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
        if (stricmp(filename+len-4, ".bas") == 0) {
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
                strcpy(buffer, toolhome);
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
    if (stricmp(s+len-4, ".bas") == 0 && 
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

#ifdef __CYGWIN__
    wnd->icon((char *)LoadIcon(xdisplay, MAKEINTRESOURCE(101)));
#endif

    check();
    switch (runMode) {
    case run_state:
        wnd->editWnd->loadFile(runfile, -1);
        basicMain(runfile);
        break;
    case edit_state:
        wnd->editWnd->loadFile(runfile, -1);
        break;
    default:
        setTitle(0);
        runMode = edit_state;
    }

    return run();
}

struct TabPage : public Group {
    TabPage(int x, int y, int w, int h, const char * s) : 
        Group(x, y, w, h, s) {}
    int handle(int event) {
        // TextDisplay::layout() does nothing when not visible
        if (event == SHOW) {
            Widget* w = child(0);
            w->layout();
            w->take_focus();
        }
        return Group::handle(event);
    }
};

//--MainWindow methods----------------------------------------------------------

MainWindow::MainWindow(int w, int h) : Window(w, h, "SmallBASIC") {
    int mnuHeight = 20;
    int statusHeight = mnuHeight;
    int groupHeight = h-mnuHeight-statusHeight-3;
    int tabHeight = mnuHeight;
    int tabBegin = 0; // =mnuHeight for top position tabs
    int pageHeight = groupHeight-tabHeight;

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
    m->add("&Edit/Find A&gain",   CTRL+'a', (Callback*)EditorWindow::find2_cb);
    m->add("&Edit/&Replace...",   0,        (Callback*)EditorWindow::replace_cb);
    m->add("&Edit/_Replace &Again",CTRL+'t',(Callback*)EditorWindow::replace2_cb);
    m->add("&Edit/&Goto Line...", 0,        (Callback*)goto_cb);
    m->add("&Edit/Output Size...",0,        (Callback*)font_size_cb);
    m->add("&View/&Full Screen",  0,        (Callback*)fullscreen_cb)->type(Item::TOGGLE);
    m->add("&View/&Turbo",        0,        (Callback*)turbo_cb)->type(Item::TOGGLE);
    scanPlugIns(m);
    m->add("&Program/&Run",       CTRL+'r', (Callback*)run_cb);
    m->add("&Program/&Break",     CTRL+'b', (Callback*)break_cb);
    m->add("&Help/About...",      0,        (Callback*)about_cb);

    callback(quit_cb);
    shortcut(0); // remove default EscapeKey shortcut

    tabGroup = new TabGroup(0, mnuHeight, w, groupHeight);
    tabGroup->begin();

    TabPage* eg = new TabPage(0, tabBegin, w, pageHeight, "Editor");
    editGroup = eg;
    editGroup->begin();
    editWnd = new EditorWindow(2, 2, w-4, pageHeight-4);
    m->user_data(editWnd); // the EditorWindow is callback user data (void*)

    editGroup->resizable(editWnd);
    editGroup->end();
    tabGroup->resizable(editGroup);

    TabPage* og = new TabPage(0, tabBegin, w, pageHeight, "Output");
    outputGroup = og;
    outputGroup->hide();
    outputGroup->begin();
    out = new Fl_Ansi_Window(2, 2, w-4, pageHeight-4);
    outputGroup->resizable(out);
    outputGroup->end();
    tabGroup->end();
    resizable(tabGroup);

    Group* statusBar = new Group(0, h-mnuHeight+1, w, mnuHeight-2);
    statusBar->begin();
    statusBar->box(NO_BOX);
    fileStatus = new Widget(0,0, w-122, mnuHeight-2);
    modStatus = new Widget(w-120, 0, 28, mnuHeight-2);
    runStatus = new Widget(w-90, 0, 28, mnuHeight-2);
    rowStatus = new Widget(w-60, 0, 28, mnuHeight-2);
    colStatus = new Widget(w-30, 0, 28, mnuHeight-2);

    for (int n=0; n<statusBar->children(); n++) {
        Widget* w = statusBar->child(n);
        w->labelfont(HELVETICA);
        w->box(THIN_DOWN_BOX);
        w->color(GRAY75);
    }

    fileStatus->align(ALIGN_INSIDE_LEFT);
    statusBar->resizable(fileStatus);
    statusBar->end();
    end();
}

MainWindow::~MainWindow() {}

bool MainWindow::isBreakExec(void) {
    return (runMode == break_state || runMode == quit_state);
}

void MainWindow::setModal(bool modal) {
    runMode = modal ? modal_state : run_state;
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
    // execute a link from the html window
    if (0 == strncmpi(file, "http://", 7)) {
        char line[1024];
        char localFile[PATH_MAX];
        dev_file_t df;
        FILE* fp;

        getHomeDir(localFile);
        strcat(localFile, "download.bas");
        fp = fopen(localFile, "w");
        if (fp == 0) {
            return;
        }

        strcpy(df.name, file);
        if (http_open(&df) == 0) {
            fclose(fp);
            return;
        }

        while (!sockcl_eof(&df)) {
            sockcl_read(&df, (unsigned char*)line, sizeof(line));
            fwrite(line, strlen(line), 1, fp);
        }
        
        // cleanup
        sockcl_close(&df);
        fclose(fp);

        // run the remote program
        wnd->editWnd->loadFile(localFile, -1);
        setTitle(file);
        addHistory(file);
        basicMain(localFile);
        return;
    }

    char* colon = strrchr(file, ':');
    if (colon && colon-1 != file) {
        file = colon+1; // clean 'file:' but not 'c:'
    }

    if (access(file, 0) == 0) {
        wnd->editWnd->loadFile(file, -1);
        setTitle(file);
        basicMain(file);
    } else {
        sprintf(buff, "Failed to open %s", file);
        wnd->fileStatus->copy_label(buff);
        wnd->fileStatus->redraw();
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

#if defined(__CYGWIN__)
// see http://www.sysinternals.com/ntw2k/utilities.shtml
// for the free DebugView program
#include <windows.h>
void trace(const char *format, ...) {
    char    buf[4096], *p = buf;
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
#endif

//--EndOfFile-------------------------------------------------------------------
