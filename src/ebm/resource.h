/** resource.h - created with rcp2ebm, rev 1.6. */
#ifndef RCP2EBM_VER
#define RCP2EBM_VER 1_6

bool createWindow(CWindow* wnd);
CMenuBar* createMenubar(CWindow* wnd);
CMenuBar* createMenubar(CWindow* wnd, U16 id);
bool showAlert(int resourceId, char* txt1, char* txt2, char* txt3);
const FONT* getFont(int fontId);

struct CListWnd : public CList {
    CListWnd(U16 id, U16 width, U16 height, S32 numrows, 
             const char **list) : CList(id,width,height,numrows) {
        this->list = list;
    }
    void setList(const char** list, S32 numrows) {
        this->list = list;
        SetNumRows(numrows);
    }
    private:
    void DrawRow(RECT *rect, S32 rownum) {
        CWindow *wnd = GetWindow();
        wnd->DrawText(list[rownum], rect->x, rect->y+1, GetFont());
    }
    U16 GetRowHeight(S32 /*rownum*/) {
        return GUI_FontHeight(GetFont())+2;
    }
    const char** list;
};

#define ID_CANCEL                     1
#define ID_FILE_LIST                  2
#define ID_FILE_NAME                  3
#define ID_FILE_OPEN                  4
#define ID_LIST_WND                   5
#define ID_OPEN                       6
#define ID_POPLIST                    7
#define ID_TYPE_LIST                  8
#define SB_MAIN_WND                   9
#define SB_RUN_WND                    10
#define mnuAbout                      11
#define mnuAboutBasFile               12
#define mnuBreak                      13
#define mnuClose                      14
#define mnuHelp                       15
#define mnuKeyboard                   16
#define mnuList                       17
#define mnuTurbo                      18
#endif
