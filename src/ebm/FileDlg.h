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

#ifndef _FILE_DLG
#define _FILE_DLG

#include "Dialog.h"

struct FileDlg : public Dialog {
    FileDlg() {}
    ~FileDlg() {}

    int show(bool saveAs);

    void setFileName(const char* f) {
        fileName.append(f);
    }
    const char* getFileName() {
        return fileName;
    }
    
    private:
    String fileName;
};

#endif
