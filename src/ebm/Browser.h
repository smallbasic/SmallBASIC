// -*- c-file-style: "java" -*-
// $Id: Browser.h,v 1.3 2004-04-30 23:40:27 zeeb90au Exp $
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

#ifndef BROWSER_H
#define BROWSER_H

#include "FileList.h"

enum ActiveTab { 
    tabPrograms=0,
    tabRun=1,
    tabSettings=2,
    tabHelp=3,
    tabSplash=4
};

struct Browser {
    Browser(FontOptions& fontOptions);
    ~Browser();

    String fileName;
    String html;
    ActiveTab activeTab;
    FontOptions& fontOptions;
    bool verbose;
    bool save;
    bool quit;
    bool reset;
    bool pause;
    bool turbo;
    bool resumeEdit;

    void show();
    void makeActivePage();
    void doSetFont();
    void doKeyboard(CTextEdit* te, String& cmd);
    void updateFileList();
    void emptyFileList();
    void editFile(const char* fileName);
    void newFile();

    private:
    void createLink(const char* fileName, bool basFile, bool sbxFile);
    void createProgramsPage();
    void createRunPage();
    void createSettingsPage();
    void createCheckBox(const char* label, const char* name, bool active);

    FileList basFiles;
    FileList sbxFiles;
    String openPage;
};

#endif
