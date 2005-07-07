// -*- c-file-style: "java" -*-
// $Id: MainWindow.cpp,v 1.60 2005-07-07 23:13:52 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2005 Chris Warren-Smith. Gawler, South Australia
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
#include <sys/stat.h>

#include <fltk/Choice.h>
#include <fltk/Group.h>
#include <fltk/Item.h>
#include <fltk/MenuBar.h>
#include <fltk/TabGroup.h>
#include <fltk/TiledGroup.h>
#include <fltk/ValueInput.h>
#include <fltk/Window.h>
#include <fltk/ask.h>
#include <fltk/error.h>
#include <fltk/events.h>
#include <fltk/filename.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "EditorWindow.h"
#include "HelpWidget.h"
#include "sbapp.h"
#include "StringLib.h"

#if defined(WIN32) 
#include <fltk/win32.h>
#include <shellapi.h>
#endif

extern "C" {
#include "fs_socket_client.h"
}

using namespace fltk;

enum ExecState {
    init_state,
    edit_state, 
    run_state, 
    modal_state,
    break_state, 
    quit_state
} runMode = init_state;

char path[MAX_PATH];
char *startDir;
char *runfile = 0;
int px,py,pw,ph;
int completionIndex = 0;
MainWindow* wnd;

#define MIN_FONT_SIZE 11
#define MAX_FONT_SIZE 22
#define DEF_FONT_SIZE 12
#define SCAN_LABEL "-[ Refresh ]-"

const char* bashome = "./Bas-Home/";
const char untitledFile[] = "untitled.bas";
const char lasteditFile[] = "lastedit.txt";
const char historyFile[] = "history.txt";
const char aboutText[] =
    "<b>About SmallBASIC...</b><br><br>"
    "Copyright (c) 2000-2005 Nicholas Christopoulos.<br><br>"
    "FLTK Version 0.9.6.3<br>"
    "Copyright (c) 2002-2005 Chris Warren-Smith.<br><br>"
    "<a href=http://smallbasic.sourceforge.net>"
    "http://smallbasic.sourceforge.net</a><br><br>"
    "SmallBASIC comes with ABSOLUTELY NO WARRANTY. "
    "This program is free software; you can use it "
    "redistribute it and/or modify it under the terms of the "
    "GNU General Public License version 2 as published by "
    "the Free Software Foundation.<br><br>"
    "<i>Press F1 for help";

// in dev_fltk.cpp
void getHomeDir(char* fileName);
bool cacheLink(dev_file_t* df, char* localFile);
void updateForm(const char* s);
void closeForm();
bool isFormActive();

//--Menu callbacks--------------------------------------------------------------

void quit_cb(Widget*, void* v) {
    if (runMode == edit_state || runMode == quit_state) {

        // auto-save scratchpad        
        const char* filename = wnd->editWnd->getFilename();
        int offs = strlen(filename)-strlen(untitledFile);
        if (filename[0] == 0 || 
            (offs > 0 && strcasecmp(filename+offs, untitledFile) == 0)) {
            getHomeDir(path);
            strcat(path, untitledFile);
            wnd->editWnd->doSaveFile(path, 0);
            exit(0);
        }

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
    const char* filename = wnd->editWnd->getFilename();
    wnd->fileStatus->copy_label(msg && msg[0] ? msg : 
                                filename && filename[0] ? filename : 
                                untitledFile);
    wnd->fileStatus->labelcolor(wnd->rowStatus->labelcolor());
    wnd->fileStatus->redraw();

#if defined(WIN32) 
    ::SetFocus(xid(Window::first()));
#endif
}

void runMsg(const char * msg) {
    wnd->runStatus->copy_label(msg && msg[0] ? msg : "");
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
        if (execlp("htmlview", "htmlview", url, NULL) == 0) {
            execlp("mozilla", "mozilla", url, NULL);
        }
        ::exit(0); // in case exec failed 
    }
#endif
}

void help_home_cb(Widget*, void* v) {
    strcpy(path, "http://smallbasic.sf.net");
    browseFile(path);
}

void help_contents_cb(Widget*, void* v) {
    char helpFile[250];
    strcpy(helpFile, "0_0.html"); // default to index page

    if (event_key() != 0) {
        // scan for help context
        FILE* fp;
        char buffer[MAX_PATH];

        TextEditor *editor = wnd->editWnd->editor;
        TextBuffer* tb = editor->buffer();
        int pos = editor->insert_position();
        int start = tb->word_start(pos);
        int end = tb->word_end(pos);
        const char* selection = tb->text_range(start, end);
        int lenSelection = strlen(selection);

        snprintf(path, sizeof(path), "%s/help/help.idx", startDir);
        fp = fopen(path, "r");

        if (fp) {
            while (feof(fp) == 0) {
                if (fgets(buffer, sizeof(buffer), fp) &&
                    strncasecmp(selection, buffer, lenSelection) == 0 &&
                    buffer[lenSelection] == ':') {
                    strcpy(helpFile, buffer+lenSelection+1);
                    helpFile[strlen(helpFile)-1] = 0; // trim \n
                    break;
                }
            }
            fclose(fp);
        }
        // cleanup
        free((void*)selection);
    }

    showHelpTab();
    snprintf(path, sizeof(path), "%s/help/", startDir);
    wnd->helpWnd->setDocHome(path);
    strcat(path, helpFile);
    wnd->helpWnd->loadFile(path);
}

void help_app_cb(Widget*, void* v) {
    const char* helpFile = dev_getenv("APP_HELP");
    if (helpFile) {
        wnd->helpWnd->loadFile(helpFile);
    } else {
        wnd->helpWnd->loadBuffer("APP_HELP env variable not found");
    }
    showHelpTab();
}

void help_readme_cb(Widget*, void* v) {
    snprintf(path, sizeof(path), "file:///%s/readme.html", startDir);
    wnd->helpWnd->loadFile(path);
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
    if (runMode == edit_state) {
        // toggle between edit and help
        if (current == wnd->helpGroup) {
            wnd->tabGroup->selected_child(wnd->editGroup);
        } else if (current == wnd->outputGroup) {
            wnd->tabGroup->selected_child(wnd->editGroup);
        } else {
            wnd->tabGroup->selected_child(wnd->helpGroup);
        }
    } else {
        // toggle between edit and output
        if (current == wnd->helpGroup) {
            wnd->tabGroup->selected_child(wnd->outputGroup);
        } else if (current == wnd->outputGroup) {
            wnd->tabGroup->selected_child(wnd->editGroup);
        } else {
            wnd->tabGroup->selected_child(wnd->outputGroup);
        }
    }
}

void turbo_cb(Widget* w, void* v) {
    wnd->isTurbo = w->value();
}

void find_cb(Widget* w, void* v) {
    bool found = wnd->editWnd->findText(wnd->findText->value(), (int)v);
    wnd->findText->textcolor(found ? BLACK : RED);
    wnd->findText->redraw();
}

void goto_cb(Widget* w, void* v) {
    Input* gotoLine = (Input*)v;
    wnd->editWnd->gotoLine(atoi(gotoLine->value()));
    wnd->editWnd->take_focus();
}

void font_size_cb(Widget* w, void* v) {
    ValueInput* sizeBn = (ValueInput*)v;
    int value = (int)sizeBn->value();
    value = value > MAX_FONT_SIZE ? MAX_FONT_SIZE : 
        value < MIN_FONT_SIZE ? MIN_FONT_SIZE : value;
    wnd->out->fontSize(value);
    wnd->editWnd->fontSize(value);
    sizeBn->value(value);
}

void func_list_cb(Widget* w, void* v) {
    const char* label = wnd->funcList->item()->label();
    if (label) {
        if (strcmp(label, SCAN_LABEL) == 0) {
            wnd->funcList->clear();
            wnd->funcList->begin();
            wnd->editWnd->createFuncList();
            new Item(SCAN_LABEL);
            wnd->funcList->end();
        } else {
            wnd->editWnd->findFunc(label);
            wnd->editWnd->take_focus();
        }
    }
}

void basicMain(const char* filename) {
    wnd->editWnd->readonly(true);
    showOutputTab();
    runMode = run_state;

    runMsg("RUN");
    wnd->copy_label("SmallBASIC");

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
        closeForm(); // unhide the error
        statusMsg(gsb_last_errmsg);
        wnd->fileStatus->labelcolor(RED);
        runMsg("ERR");
    } else {
        statusMsg(wnd->editWnd->getFilename());
        runMsg(0);
    }

    wnd->editWnd->readonly(false);
    if (isFormActive() == false) {
        wnd->editWnd->take_focus();
    }
    runMode = edit_state;
}

void run_cb(Widget*, void*) {
    const char* filename = wnd->editWnd->getFilename();
    if (runMode == edit_state) {
        // inhibit autosave on run function with environment var
        const char* noSave = dev_getenv("NO_RUN_SAVE");
        if (noSave == 0 || noSave[0] != '1') {
            if (filename == 0 || filename[0] == 0) {
                getHomeDir(path);
                strcat(path, untitledFile);
                filename = path;
                wnd->editWnd->doSaveFile(filename, false);
            } else {
                wnd->editWnd->doSaveFile(filename, true);
            }
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
    EditorWindow* editWnd = wnd->editWnd;
    TextEditor *editor = editWnd->editor;
    strcpy(filename, wnd->editWnd->getFilename());

    if (runMode == edit_state) {
        if (editWnd->checkSave(false) && filename[0]) {
            int pos = editor->insert_position();
            int row,col,s1r,s1c,s2r,s2c;
            editWnd->getRowCol(&row, &col);
            editWnd->getSelStartRowCol(&s1r, &s1c);
            editWnd->getSelEndRowCol(&s2r, &s2c);
            sprintf(opt_command, "%s|%d|%d|%d|%d|%d|%d",
                    filename, row-1, col, s1r-1, s1c, s2r-1, s2c);
            runMode = run_state;
            runMsg("RUN");
            sprintf(path, "%s%s", startDir, (const char*)v);
            int success = sbasic_main(path);
            showEditTab();
            runMsg(success ? " " : "ERR");
            editWnd->loadFile(filename, -1, true);
            editor->insert_position(pos);
            editor->show_insert_position();
            editWnd->take_focus();
            runMode = edit_state;
            opt_command[0] = 0;
        }
    } else {
        busyMessage();
    }
}

void tool_cb(Widget* w, void* filename) {
    if (runMode == edit_state) {
        strcpy(opt_command, startDir);
        strcat(opt_command, bashome+1);
        statusMsg((const char*)filename);
        sprintf(path, "%s%s", startDir, (const char*)filename);
        basicMain(path);
        statusMsg(wnd->editWnd->getFilename());
        opt_command[0] = 0;
    } else {
        busyMessage();
    }
}

void change_case_cb(Widget* w, void* v) {
    int start, end;
    TextEditor* editor = wnd->editWnd->editor;
    TextBuffer* tb = editor->buffer();
    char* selection;

    if (tb->selected()) {
        selection = (char*)tb->selection_text();
        tb->selection_position(&start, &end);
    } else {
        int pos = editor->insert_position();
        start = tb->word_start(pos);
        end = tb->word_end(pos);
        selection = (char*)tb->text_range(start, end);
    }
    int len = strlen(selection);
    enum {up, down, mixed} curcase = isupper(selection[0]) ? up : down;

    for (int i=1; i<len; i++) {
        if (isalpha(selection[i])) {
            bool isup = isupper(selection[i]);
            if ((curcase == up && isup == false) ||
                (curcase == down && isup)) {
                curcase = mixed;
                break;
            }
        }
    }

    // transform pattern: Foo -> FOO, FOO -> foo, foo -> Foo
    for (int i=0; i<len; i++) {
        selection[i] = curcase == mixed ? 
            toupper(selection[i]) : tolower(selection[i]);
    }
    if (curcase == down) {
        selection[0] = toupper(selection[0]);
        // upcase chars following non-alpha chars
        for (int i=1; i<len; i++) {
            if (isalpha(selection[i]) == false && i+1<len) {
                selection[++i] = toupper(selection[i]);
            }
        }
    }
    
    if (selection[0]) {
        tb->replace_selection(selection);
        tb->select(start, end);
    }
    free((void*)selection);
}

bool searchBackward(const char* text, int startPos,
                    const char* find, int findLen, int* foundPos) {
    int matchIndex = findLen-1;
    for (int i=startPos; i>=0; i--) {
        bool equals = toupper(text[i]) == toupper(find[matchIndex]);
        if (equals == false && matchIndex < findLen-1) {
            // partial match now fails - reset search at current index
            matchIndex = findLen-1;
            equals = toupper(text[i]) == toupper(find[matchIndex]);
        }
        matchIndex = (equals ? matchIndex-1 : findLen-1);
        if (matchIndex == -1 && (i==0 || isalpha(text[i-1]) == 0)) {
            // char prior to word is non-alpha
            *foundPos = i;
            return true;
        }
    }
    return false;
}

void expand_word_cb(Widget* w, void* v) {
    FILE* fp;
    int start, end;
    char buffer[MAX_PATH];
    const char* fullWord = 0;
    unsigned fullWordLen = 0;
    TextEditor* editor = wnd->editWnd->editor;
    TextBuffer* textbuf = editor->buffer();
    const char *text = textbuf->text();

    if (textbuf->selected()) {
        // get word before selection
        int pos1, pos2;
        textbuf->selection_position(&pos1, &pos2);
        start = textbuf->word_start(pos1-1);
        end = pos1;
        // get word from before selection to end of selection
        fullWord = text+start;
        fullWordLen = pos2-start-1;
    } else {
        // nothing selected - get word to left of cursor position
        int pos = editor->insert_position();
        end = textbuf->word_end(pos);
        start = textbuf->word_start(end-1);
        completionIndex = 0;
    }

    if (start >= end) {
        return;
    }

    const char* expandWord = text+start;
    unsigned expandWordLen = end-start;
    int wordPos = 0;

    // scan for expandWord from within the current text buffer
    if (completionIndex != -1 && 
        searchBackward(text, start-1, expandWord, expandWordLen, &wordPos)) {

        int matchPos = -1;
        if (textbuf->selected() == 0) {
            matchPos = wordPos;
            completionIndex = 1; // find next word on next call
        } else {
            // find the next word prior to the currently selected word
            int index = 1;
            while (wordPos > 0) {
                if (strncasecmp(text+wordPos, fullWord, fullWordLen) != 0 ||
                    isalpha(text[wordPos+fullWordLen+1])) {
                    // isalpha - matches fullWord but word has more chars
                    matchPos = wordPos;
                    if (completionIndex == index) {
                        completionIndex++;
                        break;
                    }
                    // count index for non-matching fullWords only
                    index++;
                }
                
                if (searchBackward(text, wordPos-1, expandWord, 
                                   expandWordLen, &wordPos) == 0) {
                    matchPos = -1;
                    break; // no more partial matches
                }
            }
            if (index == completionIndex) {
                // end of expansion sequence
                matchPos = -1;
            }
        }
        if (matchPos != -1) {
            char* word = (char*)
                textbuf->text_range(matchPos, textbuf->word_end(matchPos));
            if (textbuf->selected()) {
                textbuf->replace_selection(word+expandWordLen);
            } else {
                textbuf->insert(end, word+expandWordLen);
            }
            textbuf->select(end, end+strlen(word+expandWordLen));
            editor->insert_position(end+strlen(word+expandWordLen));
            free((void*)word);
            free((void*)text);
            return;
        }
    }

    completionIndex = -1; // no more buffer expansions

    // keywords.txt created using
    // sbasic.exe -pkw | sort | uniq  > keywords.txt
    strlib::List keywords;
    snprintf(path, sizeof(path), "%s/help/keywords.txt", startDir);
    fp = fopen(path, "r");
    if (fp == 0) {
        free((void*)text);
        return;
    }
    while (feof(fp) == 0) {
        if (fgets(buffer, sizeof(buffer), fp)) {
            int lenb = strlen(buffer)-1;
            buffer[lenb] = 0; // trim new-line
            String* s = new String;
            s->append(buffer);
            keywords.add(s);
        }
    }
    fclose(fp);

    // find the next replacement 
    int firstIndex = -1;
    int lastIndex = -1;
    int curIndex = -1;
    int numWords = keywords.length();
    for (int i=0; i<numWords; i++) {
        const char* keyword = ((String*)keywords.get(i))->toString();
        if (strncasecmp(expandWord, keyword, expandWordLen) == 0) {
            if (firstIndex == -1) {
                firstIndex = i;
            }
            if (fullWordLen == 0) {
                if (expandWordLen == strlen(keyword)) {
                    // nothing selected and word to left of cursor matches
                    curIndex = i;
                }
            } else if (strncasecmp(fullWord, keyword, fullWordLen) == 0) {
                // selection+word to left of selection matches
                curIndex = i;
            }
            lastIndex = i;
        } else if (lastIndex != -1) {
            // moved beyond matching words
            break;
        }
    }

    if (lastIndex != -1) {
        if (lastIndex == curIndex || curIndex == -1) {
            lastIndex = firstIndex; // wrap to first in subset
        } else {
            lastIndex = curIndex+1;
        }

        const char* keyword = ((String*)keywords.get(lastIndex))->toString();
        // updated the segment of the replacement text 
        // that completes the current selection
        if (textbuf->selected()) {
            textbuf->replace_selection(keyword+expandWordLen);
        } else {
            textbuf->insert(end, keyword+expandWordLen);
        }
        textbuf->select(end, end+strlen(keyword+expandWordLen));
    }

    free((void*)text);
}

//--EditWindow functions--------------------------------------------------------

void setRowCol(int row, int col) {
    char rowcol[20];
    sprintf(rowcol, "%d", row);
    wnd->rowStatus->copy_label(rowcol);
    wnd->rowStatus->redraw();
    sprintf(rowcol, "%d", col);
    wnd->colStatus->copy_label(rowcol);
    wnd->colStatus->redraw();
}

void setModified(bool dirty) {
    wnd->modStatus->label(dirty ? "MOD" : "");
    wnd->modStatus->redraw();
}

void restoreEdit() {
    FILE* fp;

    // continue editing the previous file
    getHomeDir(path);
    strcat(path, lasteditFile);
    fp = fopen(path, "r");
    if (fp) {
        fgets(path, sizeof(path), fp);
        fclose(fp);
        path[strlen(path)-1] = 0; // trim new-line
        if (access(path, 0) == 0) {
            wnd->editWnd->loadFile(path, -1, false);
            statusMsg(path);
            fileChanged(true);
            return;
        }
    }

    // continue editing scratch buffer
    getHomeDir(path);
    strcat(path, untitledFile);
    if (access(path, 0) == 0) {
        wnd->editWnd->loadFile(path, -1, false);
        statusMsg(path);
        fileChanged(true);
    }
}

void addHistory(const char* fileName) {
    FILE* fp;
    char buffer[MAX_PATH];

    // remember the last edited file
    getHomeDir(path);
    strcat(path, lasteditFile);
    fp = fopen(path, "w");
    if (fp) {
        fwrite(fileName, strlen(fileName), 1, fp);
        fwrite("\n", 1, 1, fp);
        fclose(fp);
    }

    getHomeDir(path);
    strcat(path, historyFile);

    fp = fopen(path, "r");
    if (fp) {
        // don't add the item if it already exists
        while (feof(fp) == 0) {
            if (fgets(buffer, sizeof(buffer), fp) &&
                strncmp(fileName, buffer, strlen(fileName)-1) == 0) {
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen(path, "a");
    if (fp) {
        fwrite(fileName, strlen(fileName), 1, fp);
        fwrite("\n", 1, 1, fp);
        fclose(fp);
    }
}

void fileChanged(bool loadfile) {
    FILE* fp;

    wnd->funcList->clear();
    wnd->funcList->begin();
    if (loadfile) {
        wnd->editWnd->createFuncList();
    } else {
        // remove the last edited file
        getHomeDir(path);
        strcat(path, lasteditFile);
        fp = fopen(path, "w");
        fwrite("\n", 1, 1, fp);
        fclose(fp);
    }

    new Item(SCAN_LABEL);
    wnd->funcList->end();
}

//--Startup functions-----------------------------------------------------------

void scanPlugIns(Menu* menu) {
    dirent **files;
    FILE* file;
    char buffer[MAX_PATH];
    char label[1024];

    snprintf(path, sizeof(path), "%s/Bas-Home", startDir);
    int numFiles = filename_list(path, &files);
    for (int i=0; i<numFiles; i++) {
        const char* filename = (const char*)files[i]->d_name;
        int len = strlen(filename);
        if (strcasecmp(filename+len-4, ".bas") == 0) {
            sprintf(path, "%s%s", bashome, filename);
            file = fopen(path, "r");

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
                sprintf(label, (editorTool? "&Edit/Basic/%s":"&Basic/%s"),
                        buffer+offs);
                // use an absolute path
                sprintf(path, "%s%s", bashome+1, filename);
                menu->add(label, 0, (Callback*)
                          (editorTool ? editor_cb : tool_cb), 
                          strdup(path));
            }
            fclose(file);
        }
    }
    // cleanup
    if (numFiles > 0) {
        for (int i=0; i<numFiles; i++) {
            free(files[i]);            
        }
        free(files);
    }
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
              " -m[odule]-home\n\n%s", help);
    }

    getcwd(path, sizeof(path));
    startDir = strdup(path);
    sprintf(path, "BAS_HOME=%s%s", startDir, bashome+1);
    dev_putenv(path);

    wnd = new MainWindow(600, 400);
#if defined(WIN32) 
    HICON icon = (HICON)wnd->icon();
    if (!icon) {
        icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(101),
                                IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR|LR_SHARED);
        if (!icon) icon = LoadIcon(NULL, IDI_APPLICATION);
    }
    wnd->icon((char*)icon);
#endif
    wnd->show(argc, argv);

    check();
    switch (runMode) {
    case run_state:
        wnd->editWnd->loadFile(runfile, -1, true);
        addHistory(runfile);
        basicMain(runfile);
        break;
    case edit_state:
        wnd->editWnd->loadFile(runfile, -1, true);
        break;
    default:
        restoreEdit();
        runMode = edit_state;
    }
    return run();
}

//--MainWindow methods----------------------------------------------------------

MainWindow::MainWindow(int w, int h) : Window(w, h, "SmallBASIC") {
    int mnuHeight = 22;
    int tbHeight = 30;
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
    m->add("&Edit/&Change Case",  ALT+'c',  (Callback*)change_case_cb);
    m->add("&Edit/_&Expand Word", ALT+'/',  (Callback*)expand_word_cb);
    m->add("&Edit/&Replace...",   F2Key,    (Callback*)EditorWindow::replace_cb);
    m->add("&Edit/_Replace &Again",CTRL+'t',(Callback*)EditorWindow::replace2_cb);
    m->add("&View/Toggle/&Full Screen",0,   (Callback*)fullscreen_cb)->type(Item::TOGGLE);
    m->add("&View/Toggle/&Turbo", 0,        (Callback*)turbo_cb)->type(Item::TOGGLE);
    m->add("&View/&Next Tab",     F6Key,    (Callback*)next_tab_cb);
    scanPlugIns(m);
    m->add("&Program/&Run",        F9Key,   (Callback*)run_cb);
    m->add("&Program/&Break",      CTRL+'b',(Callback*)break_cb);
    m->add("&Help/&Help Contents", F1Key,   (Callback*)help_contents_cb);
    m->add("&Help/_&Program Help", F11Key,  (Callback*)help_app_cb);
    m->add("&Help/&Release Notes", 0,       (Callback*)help_readme_cb);
    m->add("&Help/_&Home Page",    0,       (Callback*)help_home_cb);
    m->add("&Help/&About SmallBASIC",F12Key,(Callback*)help_about_cb);

    callback(quit_cb);
    shortcut(0); // remove default EscapeKey shortcut

    tabGroup = new TabGroup(0, mnuHeight, w, groupHeight);
    tabGroup->begin();

    editGroup = new Group(0, tabBegin, w, pageHeight, "Editor");
    editGroup->begin();
    editGroup->box(THIN_DOWN_BOX);

    // create the editor edit window
    editWnd = new EditorWindow(2, 3+tbHeight, w-4, pageHeight-5-tbHeight);
    m->user_data(editWnd); // the EditorWindow is callback user data (void*)
    editWnd->box(NO_BOX);
    editWnd->editor->box(NO_BOX);
    editGroup->resizable(editWnd);

    // create the editor toolbar
    Group* toolbar = new Group(2, 2, w-4, tbHeight);
    toolbar->begin();
    toolbar->box(THIN_UP_BOX);

    // find control
    findText = new Input(38, 4, 150, mnuHeight, "Find:");
    findText->align(ALIGN_LEFT|ALIGN_CLIP);
    Button* prevBn = new Button(190, 6, 18, mnuHeight-4, "@-98>;");
    Button* nextBn = new Button(210, 6, 18, mnuHeight-4, "@-92>;");
    prevBn->callback(find_cb, (void*)0);
    nextBn->callback(find_cb, (void*)1);
    findText->labelfont(HELVETICA);

    // goto-line control
    Input* gotoLine = new Input(268, 4, 40, mnuHeight, "Goto:");
    gotoLine->align(ALIGN_LEFT|ALIGN_CLIP);
    Button* gotoBn = new Button(310, 6, 18, mnuHeight-4, "@-92>;");
    gotoBn->callback(goto_cb, gotoLine);
    gotoLine->labelfont(HELVETICA);

    // sub-func jump droplist
    funcList = new Choice(339, 4, 138, mnuHeight);
    funcList->callback(func_list_cb, 0);
    funcList->labelfont(COURIER);
    funcList->begin();
    new Item();
    new Item(SCAN_LABEL);
    funcList->end();
    toolbar->resizable(funcList);

    // font-size control
    ValueInput* sizeBn = new ValueInput(542, 4, 40, mnuHeight, "Font Size:");
    sizeBn->minimum(MIN_FONT_SIZE);
    sizeBn->maximum(MAX_FONT_SIZE);
    sizeBn->value(DEF_FONT_SIZE);
    sizeBn->step(1);
    sizeBn->callback(font_size_cb, sizeBn);
    sizeBn->labelfont(HELVETICA);
    
    // close the tool-bar with a resizeable end-box
    Group* boxEnd = new Group(1000,4,0,0);
    toolbar->resizable(boxEnd);
    toolbar->end();

    editGroup->end();
    tabGroup->resizable(editGroup);

    // create the help tab
    helpGroup = new Group(0, tabBegin, w, pageHeight, "Help");
    helpGroup->box(THIN_DOWN_BOX);
    helpGroup->hide();
    helpGroup->begin();
    helpWnd = new HelpWidget(2, 2, w-4, pageHeight-4);
    helpWnd->loadBuffer(aboutText);
    helpGroup->resizable(helpWnd);
    helpGroup->end();

    // create the output tab
    outputGroup = new Group(0, tabBegin, w, pageHeight, "Output");
    outputGroup->box(THIN_DOWN_BOX);
    outputGroup->hide();
    outputGroup->begin();
    out = new AnsiWindow(2, 2, w-4, pageHeight-4, DEF_FONT_SIZE);
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
        w->color(color());
    }

    fileStatus->align(ALIGN_INSIDE_LEFT|ALIGN_CLIP);
    statusBar->resizable(fileStatus);
    statusBar->end();
    end();
    editWnd->take_focus();
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
            sprintf(localFile, "Failed to open URL: %s", file);
            statusMsg(localFile);
            return;
        }

        bool httpOK = cacheLink(&df, localFile);
        char* extn = strrchr(file, '.');

        if (httpOK && extn && 0 == strncasecmp(extn, ".bas", 4)) {
            // run the remote program
            wnd->editWnd->loadFile(localFile, -1, false);
            statusMsg(file);
            addHistory(file);
            basicMain(localFile);
        } else {
            // display as html
            int len = strlen(localFile);
            if (strcasecmp(localFile+len-4, ".gif") == 0 ||
                strcasecmp(localFile+len-4, ".jpeg") == 0 ||
                strcasecmp(localFile+len-4, ".jpg") == 0) {
                sprintf(path, "<img src=%s>", localFile);
            } else {
                sprintf(path, "file:%s", localFile);
            }
            siteHome.append(df.name, df.drv_dw[1]);
            statusMsg(siteHome.toString());
            updateForm(path);
            showOutputTab();
        }
        return;
    }

    char* colon = strrchr(file, ':');
    if (colon && colon-1 != file) {
        file = colon+1; // clean 'file:' but not 'c:'
    }

    char* extn = strrchr(file, '.');
    if (extn && (0 == strncasecmp(extn, ".bas ", 5) ||
                 0 == strncasecmp(extn, ".sbx ", 5))) {
        strcpy(opt_command, extn+5);
        extn[4] = 0; // make args available to bas program
    }

    // if the extension is .sbx and this does not exists or is older 
    // than the matching .bas then rename to .bas and set opt_nosave 
    // to false - otherwise set execFile flag to true
    if (extn && 0 == strncasecmp(extn, ".sbx", 4)) {
        struct stat st_sbx;
        struct stat st_bas;
        bool sbxExists = (stat(file, &st_sbx) == 0);
        strcpy(extn+1, "bas"); // remains .bas unless sbx valid
        opt_nosave = 0; // create/use sbx files
        if (sbxExists) {
            if (stat(file, &st_bas) == 0 &&
                st_sbx.st_mtime > st_bas.st_mtime) {
                strcpy(extn+1, "sbx");
                // sbx exists and is newer than .bas 
                execFile = true;
            }
        }
    }
    if (access(file, 0) == 0) {
        statusMsg(file);
        if (execFile) {
            addHistory(file);
            basicMain(file);
            opt_nosave = 1;
        } else {
            wnd->editWnd->loadFile(file, -1, true);
            showEditTab();
        }
    } else {
        sprintf(path, "Failed to open: %s", file);
        statusMsg(path);
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
// an alternative debug method is to use insight.exe which
// is included with cygwin. 
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
