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
#include "FileDlg.h"

struct FileDlgWnd : public CWindow {
    FileDlgWnd(FileDlg &fileDlg, bool saveAs) :
        CWindow(ID_FILE_OPEN, 0,0,0,0),
        fileDlg(fileDlg) {
        this->fileList = null;
        this->fileIndex = -1;
        this->extnIndex = 0;
        this->saveAs = saveAs;
        createWindow(this);
        editFileName = (CTextEdit*)GetChildID(ID_FILE_NAME);
        listFiles();

        if (saveAs) {
            SetTitle("Save As");
            ((CButton*)GetChildID(ID_OPEN))->SetLabel("Save");
            SetFocus(editFileName);
        }
    }

    ~FileDlgWnd() {
        emptyList();
    }

    FileDlg &fileDlg;
    int fileIndex; 
    int extnIndex; // file extension index
    List list; // underlying string container
    const char **fileList; // list iterator
    CTextEdit* editFileName;
    bool saveAs;

    void listFiles();
    void emptyList();
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
};

int FileDlg::show(bool saveAs) {
    // FileDlgWnd deleted inside event loop
    GUI_EventLoop(new FileDlgWnd(*this, saveAs)); 
    return exitCode;
}

S32 FileDlgWnd::MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
    switch (type) {

    case MSG_BUTTON_SELECT:
        switch (from->GetID()) {
        case ID_OPEN:
            fileDlg.setFileName(editFileName->GetText());
            if (fileDlg.getFileName() == null ||
                strlen(fileDlg.getFileName()) == 0) {
                return 1;
            }
            if (saveAs && File::exists(fileDlg.getFileName())) {
                const char* msg = 
                    "File already exists.\nDo you want to replace it?";
                if (!GUI_Alert(ALERT_OK, msg)) {
                    return 1;
                }
            }
            // fall through

        case ID_CANCEL:
            fileDlg.setExitCode(from->GetID());
            Close();
            break;
        }
        return 1;

    case MSG_ROW_SELECT:
        // file name selection
        if (list.length() > 0 && data != -1) {
            fileIndex = data;
            editFileName->SetText(fileList[fileIndex]);
        }
        return 1;

    case MSG_MENU_SELECT:
        // file type selection
        if (data != -1) {
            extnIndex = data;
            listFiles();
        }
        return 1;

    default:break;
    }
    return CWindow::MsgHandler(type, from, data);
}

void FileDlgWnd::emptyList() {
    if (fileList != null) {
        delete fileList;
        fileList = null;
    }
    list.removeAll();
    fileIndex = -1;
}

int compareNames(const void* p1, const void* p2) {
    char* file1 = *((char**)p1);
    char* file2 = *((char**)p2);
    return stricmp(file1, file2);
}

void FileDlgWnd::listFiles() {
    CMenu* filter = ((CPopupTrigger*)GetChildID(ID_POPLIST))->GetMenu();
    const char* extn = filter->GetRowName(extnIndex);
    bool allFiles = (strcmp("-ALL-", extn) == 0);
    emptyList();

    // scan for main ram files
    ebo_enumerator_t ebo_enum;
    int i = ebo_first_object(&ebo_enum);
    while (i == EBO_OK) {
        if (allFiles || stricmp(extn, ebo_enum.name.extension)==0) {
            String *s = new String();
            s->append(ebo_enum.name.name);
            s->append(".");
            s->append(ebo_enum.name.extension);
            list.append(s);
        }
        i = ebo_next_object(&ebo_enum);
    }

    // scan for MMC files
    i = ebo_first_xobject(&ebo_enum);
    while (i == EBO_OK) {
        if (allFiles || stricmp(extn, ebo_enum.name.extension)==0) {
            String *s = new String();
            s->append("mmc:");
            s->append(ebo_enum.name.name);
            s->append(".");
            s->append(ebo_enum.name.extension);
            list.append(s);
        }
        i = ebo_next_xobject(&ebo_enum);
    }

    CListWnd* listWnd = (CListWnd*)GetChildID(ID_FILE_LIST);    
    i=0;

    if (list.length()) {
        fileList = new const char*[list.length()];
        list.iterateInit();
        while (list.hasMoreElements()) {
            fileList[i++] = ((String*)list.nextElement())->toString();
        }
        qsort(fileList, list.length(), sizeof(char*), compareNames);
    }        

    listWnd->setList(fileList, i);
    listWnd->SetOptions(LISTOPTION_ALWAYS_HIGHLIGHT);
    SetFocus(listWnd);
    listWnd->Draw();
}
