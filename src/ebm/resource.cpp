/*
 * -*- c-file-style: "java" -*-
 * Generated with rcp2ebm, version 1.5
 * This file is not intended to be manually edited.
 */

#include <gui.h>
#include "resource.h"

const FONT* getFont(int fontId) {
    int style = CTRL_NORMAL;
    int size = 9;
    switch (fontId) {
    case 0: // normal
        style = CTRL_NORMAL;
        size = 9;
        break;
    case 1: // bold
        size = 9;
        style = CTRL_BOLD;
        break;
    case 2: // large
        size = 12;
        style = CTRL_NORMAL;
        break;
    case 3: // symbol
        size = 9;
        style = CTRL_ITALIC;
        break;
    case 4: // symbol 11
        size = 12;
        style = CTRL_BOLDITALIC;
        break;
    case 5: // symbol 7
        size = 16;
        style = CTRL_NORMAL;
        break;
    case 6: // led
        size = 16;
        style = CTRL_BOLDITALIC;
        break;
    case 7: // bold/large
        size = 12;
        style = CTRL_BOLD;
        break;

    default:
        break;
    }
    return GUI_GetFont(size, style);
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
            if (border == 1) {
                window->DrawRect(GetX(), GetY(), GetWidth(), GetHeight());
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

S16 getCenter(const FONT* font, const char* label, bool left) {
    if (left) {
        return 100-(GUI_TextWidth(font,label,strlen(label))/2);
    } else {
        return 120-(GUI_FontHeight(font)/2);
    }
}

S16 getCenter(CButton* b, bool left) {
    return getCenter(b->GetFont(), b->GetLabel(), left);
}

S16 getCenter(CLabel* l, bool left) {
    return getCenter(l->GetFont(), l->GetLabel(), left);
}

bool createWindow(CWindow* wnd)
{
    switch(wnd->GetID()) {

    case ID_FILE_OPEN: {
        const char title[] = "Run";
        wnd->SetTitle(title);
        wnd->SetX(5);
        wnd->SetY(34);
        wnd->SetWidth(190);
        wnd->SetHeight(182);
        const char* list1Items[] = {""};
        CListWnd* list1 = new CListWnd(ID_FILE_LIST,185,132,1,list1Items);
        list1->SetFont(getFont(0));
        wnd->AddChild(list1,1,0);
        Gadget* gadget2 = new Gadget(4000,186,1);
        gadget2->setBorder(1);
        wnd->AddChild(gadget2,0,133);
        CButton* button3 = new CButton(ID_OPEN,40,15,"Open");
        button3->SetFont(getFont(0));
        wnd->AddChild(button3,101,148);
        CButton* button4 = new CButton(ID_CANCEL,40,15,"Cancel");
        button4->SetFont(getFont(0));
        wnd->AddChild(button4,143,148);
        CMenu *list5 = new CMenu(ID_TYPE_LIST);
        list5->SetNumRows(2);
        list5->SetRow(0,0,"BAS");
        list5->SetRow(1,1,"SBX");
        CPopupTrigger *popuptrigger6 = new CPopupTrigger(ID_POPLIST,0,0,list5);
        popuptrigger6->SetCurrentRow(0);
        wnd->AddChild(popuptrigger6,28,150);
        const char *label7Label = "File name:";
        CLabel* label7 = new CLabel(4001, label7Label, getFont(0));
        wnd->AddChild(label7,2,136);
        const char *label8Label = "Type:";
        CLabel* label8 = new CLabel(4002, label8Label, getFont(0));
        wnd->AddChild(label8,2,151);
        CTextEdit* field9 = new 
                CTextEdit(ID_FILE_NAME,185,15,TEXTOPTION_NOUNDERLINE|TEXTOPTION_ONELINE);
        field9->SetFont(getFont(0));
        wnd->AddChild(field9,48,136);
        return true;
    }

    case ID_LIST_WND: {
        const char title[] = "Run";
        wnd->SetTitle(title);
        wnd->SetX(10);
        wnd->SetY(10);
        wnd->SetWidth(180);
        wnd->SetHeight(215);
        const char* list1Items[] = {};
        CListWnd* list1 = new CListWnd(ID_FILE_LIST,176,197,0,list1Items);
        list1->SetFont(getFont(0));
        wnd->AddChild(list1,0,0);
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
    switch(wndId) {

    case SB_MAIN_WND: {
        CMenuBar *menuBar = new CMenuBar(0,23,wnd);
        CMenu *menu0 = new CMenu(0);
        menu0->SetNumRows(4);
        menu0->SetRow(0,mnuList,"Run");
        menu0->SetRow(1,mnuKeyboard,"Keyboard");
        menu0->SetSeparatorRow(2);
        menu0->SetRow(3,mnuClose,"Close");
        menuBar->AddButton(new CPushButton(0,0,0,"File"), menu0);
        CMenu *menu1 = new CMenu(1);
        menu1->SetNumRows(2);
        menu1->SetRow(0,mnuHelp,"Index");
        menu1->SetRow(1,mnuAbout,"About SB");
        menuBar->AddButton(new CPushButton(1,0,0,"Help"), menu1);
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
        menu0->SetRow(6,mnuClose,"Close");
        menuBar->AddButton(new CPushButton(0,0,0,"File"), menu0);
        return menuBar;
    }

    }
    return 0;
}
bool showAlert(int resourceId, char* txt1, char* txt2, char* txt3)
{
    char buffer[250];
    buffer[0]='0';
    switch(resourceId) {

    }
    return 0;
}
