#include "resource.h"

// -*- c-file-style: "java" -*-
// Generated with rcp2ebm, version 1.10
// This file is not intended to be manually edited.
//

const FONT* getFont(int fontId) {
    const FONTGROUP* fg = GUI_GetFontGroup()+1;
    int size = fg->size;
    switch (fontId) {
    case 0: // normal
        return GUI_GetFont(size, CTRL_NORMAL);
    case 1: // bold
        return GUI_GetFont(size, CTRL_BOLD);
    case 2: // symbol
        return GUI_GetFont(size, CTRL_ITALIC);
    }

    // advance to next size
    while ((++fg)->size == size) {}

    switch (fontId) {
    case 3: // large
        return GUI_GetFont(fg->size, CTRL_NORMAL);
    case 4: // symbol 11
        return GUI_GetFont(fg->size, CTRL_BOLD);
    case 5: // symbol 7
        return GUI_GetFont(fg->size, CTRL_ITALIC);
    }
    
    // led
    return GUI_GetFont(size, CTRL_NORMAL);
}

struct Gadget : public CViewable {
    Gadget(U16 id, U16 width, U16 height) :
        CViewable(id, width, height),
        border(0) {}
    void SetFont(const FONT*) {}
    void SetBackgroundColor(const COLOR) {}
    void Draw() {
        CWindow *window = GetWindow();
        if (window != NULL) {
            switch (border) {
            case 1:
                window->DrawRect(GetX(), GetY(), GetWidth(), GetHeight());
                break;
            case 2:
                window->DrawRectFilled(GetX(), GetY(), GetWidth(), GetHeight());
                break;
            case 3:
                window->InvertRect(GetX(), GetY(), GetWidth(), GetHeight());
                break;
            }
        }
        
    }
    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
        if (type==MSG_PEN_UP) {
            GetParent()->MsgHandler(MSG_BUTTON_SELECT, from, data);
            return 1;
        }
        return CViewable::MsgHandler(type, from, data);
    }

    void setBorder(int i) {border=i;}
    private:
    int border;
};

struct CenterLabel : public CLabel {
    CenterLabel(U16 id, U16 width, U16 height, const char* label) :
        CLabel(id, label) {
        SetWidth(width);
        SetHeight(height);
        this->width=width;
    }
    void SetFont(const FONT* font) {
        CViewable::SetFont(font);
    }
    void Draw() {
        CWindow *window = GetWindow();
        if (window) {
            const FONT* font = GetFont();
            const char * label = GetLabel();
            int textWidth = GUI_TextWidth(font,label,strlen(label));
            int x = GetX() + (width-textWidth)/2;
            window->DrawText(label, x, GetY(), font);
        }
    }
    private:
    int width;
};

struct GroupBox : public CLabel {
    GroupBox(U16 id, U16 width, U16 height, const char* label) :
        CLabel(id, label) {
        SetWidth(width);
        SetHeight(height);
    }
    void SetFont(const FONT* font) {
        CViewable::SetFont(font);
    }
    void Draw() {
        CWindow *window = GetWindow();
        if (window) {
            const FONT* font = GetFont();
            int yOffs = GUI_FontHeight(font)/2;
            int x = GetX();
            int y = GetY();
            window->DrawRect(x, y+yOffs, GetWidth(), GetHeight()-yOffs);
            window->DrawText(GetLabel(), x+4, y, font);
        }
    }
};

S16 getCenter(CWindow *wnd, CButton* bn, bool left) {
    RECT rcWnd, rcBn;
    wnd->GetUsableRect(&rcWnd);
    bn->GetUsableRect(&rcBn);
    if (left) {
        return (rcWnd.width - rcBn.width) / 2;
    } else {
        return (rcWnd.height - rcBn.height) / 2;
    }
}

S16 getCenter(CWindow *wnd, CLabel* l, bool left) {
    RECT rc;
    wnd->GetUsableRect(&rc);
    if (left) {
        int tw = GUI_TextWidth(l->GetFont(),
                               l->GetLabel(),
                               strlen(l->GetLabel()));
        return (rc.width - tw) / 2;
    } else {
        return (rc.height-GUI_FontHeight(l->GetFont())) /2;
    }
}

CListWnd::CListWnd(U16 id, U16 width, U16 height, S32 numrows, 
                   const char **list)
    : CList(id,width,height,numrows,LISTOPTION_ALWAYS_HIGHLIGHT) {
    this->list = list;
    SetFont(getFont(0));
}

void CListWnd::setList(const char** list, S32 numrows) {
    this->list = list;
    SetNumRows(numrows);
}

void CListWnd::DrawRow(RECT *rect, S32 rownum) {
    CWindow *wnd = GetWindow();
    int len = GUI_TruncateText(GetFont(), list[rownum], rect->width, false);
    wnd->DrawText(list[rownum], rect->x+1, rect->y+1, GetFont(), 0, 0, len);
}

U16 CListWnd::GetRowHeight(S32 /*rownum*/) {
    return GUI_FontHeight(GetFont())+2;
}

const char* CListWnd::getSelectionText(S32 row) {
    return row < GetNumRows() ? list[row] : 0;
}

CPopupMenu::CPopupMenu(CPopupTrigger* trigger) : 
    CViewable(trigger->GetMenu()->GetID(), 0,0) {
    this->trigger = trigger;
    this->menu = trigger->GetMenu();
}

CPopupMenu::~CPopupMenu() {
    // menu is not deleted in the CPopupTrigger container
    delete menu; 
}
bool createWindow(CWindow* wnd)
{
    switch (wnd->GetID()) {

    case ID_FONT_DLG: {
        static const char title[] = "Font Settings";
        wnd->SetTitle(title);
        wnd->SetX(5);
        wnd->SetY(24);
        wnd->SetWidth(190);
        wnd->SetHeight(200);
        const FONT* font0 = getFont(0);
        static const char *label1Label = "Font:";
        CLabel* label1 = new CLabel(4000, label1Label, font0);
        wnd->AddChild(label1,5,5);
        static const char* list2Items[] = {""};
        CListWnd* list2 = new CListWnd(ID_FONT_LIST,100,104,1,list2Items);
        list2->SetFont(font0);
        wnd->AddChild(list2,label1->GetX(),(label1->GetY()+label1->GetHeight())+2);
        Gadget* gadget3 = new Gadget(4001,list2->GetWidth()+2,list2->GetHeight()+2);
        gadget3->setBorder(1);
        gadget3->Disable();
        wnd->AddChild(gadget3,list2->GetX()-1,list2->GetY()-1);
        static const char *label4Label = "Style:";
        CLabel* label4 = new CLabel(4002, label4Label, font0);
        wnd->AddChild(label4,(gadget3->GetX()+gadget3->GetWidth())+5,5);
        static const char* list5Items[] = {""};
        CListWnd* list5 = new CListWnd(ID_STYLE_LIST,70,45,1,list5Items);
        list5->SetFont(font0);
        wnd->AddChild(list5,label4->GetX(),(label4->GetY()+label4->GetHeight())+2);
        Gadget* gadget6 = new Gadget(4003,list5->GetWidth()+2,list5->GetHeight()+2);
        gadget6->setBorder(1);
        gadget6->Disable();
        wnd->AddChild(gadget6,list5->GetX()-1,list5->GetY()-1);
        static const char *label7Label = "Size:";
        CLabel* label7 = new CLabel(4004, label7Label, font0);
        wnd->AddChild(label7,gadget6->GetX()+1,(gadget6->GetY()+gadget6->GetHeight())+2);
        static const char* list8Items[] = {""};
        CListWnd* list8 = new CListWnd(ID_SIZE_LIST,70,45,1,list8Items);
        list8->SetFont(font0);
        wnd->AddChild(list8,label7->GetX(),(label7->GetY()+label7->GetHeight())+2);
        Gadget* gadget9 = new Gadget(4005,list8->GetWidth()+2,list8->GetHeight()+2);
        gadget9->setBorder(1);
        gadget9->Disable();
        wnd->AddChild(gadget9,list8->GetX()-1,list8->GetY()-1);
        static const char *label10Label = "Preview:";
        CLabel* label10 = new CLabel(4006, label10Label, font0);
        wnd->AddChild(label10,5,(gadget9->GetY()+gadget9->GetHeight())+2);
        Gadget* gadget11 = new Gadget(ID_FONT_SAMPLE,180,18);
        gadget11->Disable();
        wnd->AddChild(gadget11,label10->GetX(),(label10->GetY()+label10->GetHeight())+2);
        CButton* button12 = new CButton(ID_OK,40,15,"OK");
        button12->SetFont(font0);
        wnd->AddChild(button12,100,(gadget11->GetY()+gadget11->GetHeight())+12);
        CButton* button13 = new CButton(ID_CANCEL,40,15,"Cancel");
        button13->SetFont(font0);
        wnd->AddChild(button13,(button12->GetX()+button12->GetWidth())+2,button12->GetY());
        return true;
    }

    case ID_PROMPT_SAVE: {
        static const char title[] = "Save File?";
        wnd->SetTitle(title);
        wnd->SetX(4);
        wnd->SetY(36);
        wnd->SetWidth(190);
        wnd->SetHeight(105);
        const FONT* font0 = getFont(0);
        static const char *label1Label = "The text has changed.";
        CLabel* label1 = new CLabel(4007, label1Label, font0);
        wnd->AddChild(label1,26,13);
        static const char *label2Label = "Do you want to save the changes?";
        CLabel* label2 = new CLabel(4008, label2Label, font0);
        wnd->AddChild(label2,28,25);
        CButton* button3 = new CButton(ID_YES,40,15,"Yes");
        button3->SetFont(font0);
        wnd->AddChild(button3,29,50);
        CButton* button4 = new CButton(ID_NO,40,15,"No");
        button4->SetFont(font0);
        wnd->AddChild(button4,81,50);
        CButton* button5 = new CButton(ID_CANCEL,40,15,"Cancel");
        button5->SetFont(font0);
        wnd->AddChild(button5,133,50);
        return true;
    }

    case ID_FIND_DLG: {
        static const char title[] = "Find";
        wnd->SetTitle(title);
        wnd->SetX(5);
        wnd->SetY(35);
        wnd->SetWidth(190);
        wnd->SetHeight(62);
        const FONT* font0 = getFont(0);
        static const char *label1Label = "Find what:";
        CLabel* label1 = new CLabel(4009, label1Label, font0);
        wnd->AddChild(label1,5,5);
        CCheckbox* checkbox2 = new CCheckbox(ID_MATCH_CASE,66,15,"Match case");
        checkbox2->SetFont(font0);
        wnd->AddChild(checkbox2,5,21);
        CTextEdit* field3 = new 
                CTextEdit(ID_TXT_FIND,91,15,TEXTOPTION_NOUNDERLINE|TEXTOPTION_ONELINE);
        field3->SetFont(font0);
        wnd->AddChild(field3,50,5);
        CButton* button4 = new CButton(ID_FIND,40,15,"Find");
        button4->SetFont(font0);
        wnd->AddChild(button4,141,6);
        CButton* button5 = new CButton(ID_CANCEL,40,15,"Cancel");
        button5->SetFont(font0);
        wnd->AddChild(button5,141,23);
        CMenu *list6 = new CMenu(ID_DIR_LIST);
        list6->SetNumRows(2);
        list6->SetRow(0,0,"Down");
        list6->SetRow(1,1,"Up");
        CPopupTrigger *popuptrigger7 = new CPopupTrigger(ID_POPLIST,0,0,list6);
        popuptrigger7->SetCurrentRow(0);
        wnd->AddChild(popuptrigger7,70,21);
        wnd->AddChild(new CPopupMenu(popuptrigger7), 0,0);
        wnd->SetFocus(wnd->GetChild(ID_TXT_FIND));
        return true;
    }

    case ID_EDITOR: {
        wnd->SetX(-2);
        wnd->SetY(-2);
        wnd->SetWidth(204);
        wnd->SetHeight(244);
        const FONT* font0 = getFont(0);
        CTextEdit* field1 = new 
                CTextEdit(ID_EDIT,200,120,TEXTOPTION_NOUNDERLINE);
        field1->SetFont(font0);
        wnd->AddChild(field1,0,10);
        return true;
    }

    case ID_FILE_OPEN: {
        static const char title[] = "Open";
        wnd->SetTitle(title);
        wnd->SetX(5);
        wnd->SetY(34);
        wnd->SetWidth(190);
        wnd->SetHeight(182);
        const FONT* font0 = getFont(0);
        static const char* list1Items[] = {""};
        CListWnd* list1 = new CListWnd(ID_FILE_LIST,185,132,1,list1Items);
        list1->SetFont(font0);
        wnd->AddChild(list1,1,0);
        Gadget* gadget2 = new Gadget(4010,186,1);
        gadget2->setBorder(1);
        gadget2->Disable();
        wnd->AddChild(gadget2,0,133);
        CButton* button3 = new CButton(ID_OPEN,40,15,"Open");
        button3->SetFont(font0);
        wnd->AddChild(button3,101,148);
        CButton* button4 = new CButton(ID_CANCEL,40,15,"Cancel");
        button4->SetFont(font0);
        wnd->AddChild(button4,143,148);
        CMenu *list5 = new CMenu(ID_TYPE_LIST);
        list5->SetNumRows(2);
        list5->SetRow(0,0,"BAS");
        list5->SetRow(1,1,"SBU");
        CPopupTrigger *popuptrigger6 = new CPopupTrigger(ID_POPLIST,0,0,list5);
        popuptrigger6->SetCurrentRow(0);
        wnd->AddChild(popuptrigger6,28,150);
        wnd->AddChild(new CPopupMenu(popuptrigger6), 0,0);
        static const char *label7Label = "File name:";
        CLabel* label7 = new CLabel(4011, label7Label, font0);
        wnd->AddChild(label7,2,136);
        static const char *label8Label = "Type:";
        CLabel* label8 = new CLabel(4012, label8Label, font0);
        wnd->AddChild(label8,2,151);
        CTextEdit* field9 = new 
                CTextEdit(ID_FILE_NAME,185,15,TEXTOPTION_NOUNDERLINE|TEXTOPTION_ONELINE);
        field9->SetFont(font0);
        wnd->AddChild(field9,48,136);
        return true;
    }

    }
    return 0;
}

CMenuBar* createMenubar(CWindow* wnd)
{
    return createMenubar(wnd, wnd->GetID());
}

CMenuBar* createMenubar(CWindow* wnd, U16 wndId)
{
    switch (wndId) {

    case ID_MAINWND: {
        CMenuBar *menuBar = new CMenuBar(0,23,wnd);
        CMenu *menu0 = new CMenu(0);
        menu0->SetNumRows(4);
        menu0->SetRow(0,mnuShowOutput,"Output");
        menu0->SetRow(1,mnuHelp,"Help");
        menu0->SetSeparatorRow(2);
        menu0->SetRow(3,mnuExit,"Exit");
        menuBar->AddButton(new CPushButton(0,0,0,"-"), menu0);
        return menuBar;
    }

    case ID_HELP: {
        CMenuBar *menuBar = new CMenuBar(0,23,wnd);
        CMenu *menu0 = new CMenu(0);
        menu0->SetNumRows(3);
        menu0->SetRow(0,mnuClose,"Close");
        menu0->SetSeparatorRow(1);
        menu0->SetRow(2,mnuExit,"Exit");
        menuBar->AddButton(new CPushButton(0,0,0,"-"), menu0);
        return menuBar;
    }

    case ID_EDITOR: {
        CMenuBar *menuBar = new CMenuBar(0,23,wnd);
        CMenu *menu0 = new CMenu(0);
        menu0->SetNumRows(9);
        menu0->SetRow(0,mnuOpen,"Open");
        menu0->SetSeparatorRow(1);
        menu0->SetRow(2,mnuSave,"Save");
        menu0->SetRow(3,mnuSaveAs,"Save As...");
        menu0->SetSeparatorRow(4);
        menu0->SetRow(5,mnuSaveRun,"Run");
        menu0->SetRow(6,mnuShowOutput,"Output");
        menu0->SetSeparatorRow(7);
        menu0->SetRow(8,mnuExit,"Exit");
        menuBar->AddButton(new CPushButton(0,0,0,"File"), menu0);
        CMenu *menu1 = new CMenu(1);
        menu1->SetNumRows(10);
        menu1->SetRow(0,mnuCut,"Cut");
        menu1->SetRow(1,mnuCopy,"Copy");
        menu1->SetRow(2,mnuPaste,"Paste");
        menu1->SetRow(3,mnuClear,"Clear");
        menu1->SetRow(4,mnuSelectAll,"Select All");
        menu1->SetSeparatorRow(5);
        menu1->SetRow(6,mnuFind,"Find");
        menu1->SetRow(7,mnuFindNext,"Find Next");
        menu1->SetSeparatorRow(8);
        menu1->SetRow(9,mnuWhatLine,"What Line");
        menuBar->AddButton(new CPushButton(1,0,0,"Edit"), menu1);
        CMenu *menu2 = new CMenu(2);
        menu2->SetNumRows(1);
        menu2->SetRow(0,mnuToggleKeypad,"Toggle Keypad");
        menuBar->AddButton(new CPushButton(2,0,0,"View"), menu2);
        CMenu *menu3 = new CMenu(3);
        menu3->SetNumRows(1);
        menu3->SetRow(0,mnuHelp,"Index");
        menuBar->AddButton(new CPushButton(3,0,0,"Help"), menu3);
        return menuBar;
    }

    case SB_RUN_WND: {
        CMenuBar *menuBar = new CMenuBar(0,23,wnd);
        CMenu *menu0 = new CMenu(0);
        menu0->SetNumRows(7);
        menu0->SetRow(0,mnuBreak,"Break");
        menu0->SetRow(1,mnuKeyboard,"Keyboard");
        menu0->SetRow(2,mnuTurbo,"Turbo");
        menu0->SetSeparatorRow(3);
        menu0->SetRow(4,mnuAboutBasFile,"About");
        menu0->SetSeparatorRow(5);
        menu0->SetRow(6,mnuExit,"Exit");
        menuBar->AddButton(new CPushButton(0,0,0,"File"), menu0);
        return menuBar;
    }

    }
    return 0;
}

CMenu* createMenu(CWindow* wnd, U16 wndId)
{
    switch (wndId) {

    case ID_MAINWND: {
        CMenu * mnu = new CMenu(ID_MAINWND,0,-1,0,wnd);
        mnu->SetNumRows(4);
        mnu->SetRow(0,mnuShowOutput,"Output");
        mnu->SetRow(1,mnuHelp,"Help");
        mnu->SetSeparatorRow(2);
        mnu->SetRow(3,mnuExit,"Exit");
        return mnu;
    }

    case ID_HELP: {
        CMenu * mnu = new CMenu(ID_HELP,0,-1,0,wnd);
        mnu->SetNumRows(3);
        mnu->SetRow(0,mnuClose,"Close");
        mnu->SetSeparatorRow(1);
        mnu->SetRow(2,mnuExit,"Exit");
        return mnu;
    }

    case ID_EDITOR: {
        CMenu * mnu = new CMenu(ID_EDITOR,0,-1,0,wnd);
        mnu->SetNumRows(9);
        mnu->SetRow(0,mnuOpen,"Open");
        mnu->SetSeparatorRow(1);
        mnu->SetRow(2,mnuSave,"Save");
        mnu->SetRow(3,mnuSaveAs,"Save As...");
        mnu->SetSeparatorRow(4);
        mnu->SetRow(5,mnuSaveRun,"Run");
        mnu->SetRow(6,mnuShowOutput,"Output");
        mnu->SetSeparatorRow(7);
        mnu->SetRow(8,mnuExit,"Exit");
        return mnu;
    }

    case SB_RUN_WND: {
        CMenu * mnu = new CMenu(SB_RUN_WND,0,-1,0,wnd);
        mnu->SetNumRows(7);
        mnu->SetRow(0,mnuBreak,"Break");
        mnu->SetRow(1,mnuKeyboard,"Keyboard");
        mnu->SetRow(2,mnuTurbo,"Turbo");
        mnu->SetSeparatorRow(3);
        mnu->SetRow(4,mnuAboutBasFile,"About");
        mnu->SetSeparatorRow(5);
        mnu->SetRow(6,mnuExit,"Exit");
        return mnu;
    }

    }
    return 0;
}

bool showAlert(int resourceId, const char* txt1, const char* txt2, const char* txt3)
{
    char buffer[250];
    buffer[0]='0';
    switch (resourceId) {

    }
    return 0;
}

const char* frmHelpText(int resourceId)
{
    switch(resourceId) {

    }
    return 0;
}

const char* getFormHelpText(CWindow* wnd)
{
    switch(wnd->GetID()) {
    }
    return 0;
}

ControlStyleType getControlStyleType(CViewable* child)
{
    switch (child->GetID()) {

    case ID_CANCEL: {
        return buttonCtl;
    }

    case ID_DIR_LIST: {
        return popupmenuCtl;
    }

    case ID_EDIT: {
        return fieldCtl;
    }

    case ID_FILE_LIST: {
        return listCtl;
    }

    case ID_FILE_NAME: {
        return fieldCtl;
    }

    case ID_FIND: {
        return buttonCtl;
    }

    case ID_FONT_LIST: {
        return listCtl;
    }

    case ID_FONT_SAMPLE: {
        return gadgetCtl;
    }

    case ID_MATCH_CASE: {
        return checkboxCtl;
    }

    case ID_NO: {
        return buttonCtl;
    }

    case ID_OK: {
        return buttonCtl;
    }

    case ID_OPEN: {
        return buttonCtl;
    }

    case ID_POPLIST: {
        return popuplistCtl;
    }

    case ID_SIZE_LIST: {
        return listCtl;
    }

    case ID_STYLE_LIST: {
        return listCtl;
    }

    case ID_TXT_FIND: {
        return fieldCtl;
    }

    case ID_TYPE_LIST: {
        return popupmenuCtl;
    }

    case ID_YES: {
        return buttonCtl;
    }

    }
    return voidCtl;
}
