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

#ifndef DIALOG_H
#define DIALOG_H

struct Dialog {
    Dialog() {exitCode = ID_CANCEL;}
    int show(U16 formId);
    void setExitCode(U16 code) {exitCode=code;}
    int getExitCode() {return exitCode;}
    protected:
    int exitCode;
};

#endif
