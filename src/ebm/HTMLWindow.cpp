/**
 * HTMLWindow version 1.0 Chris Warren-Smith 12/12/2001
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
#include "HTMLWindow.h"

#define ID_MAIN_WND 1
#define ID_ANCHOR   2
#define ID_BUTTON   3

////////////////////////////////////////////////////////////////////////////////
// class HTMLNode

struct HTMLNode : public Object {
    virtual ~HTMLNode() {}
    HTMLNode(int x, 
             int y, 
             const char* s,
             int slen,
             const FONT* font) {
        this->x=x;
        this->y=y;
        this->s=s;
        this->textLength=slen;
        this->font=font;
    }

    void drawText(CWindow* wnd, const FONT* font, int yScroll) {
        RECT rc;
        wnd->GetUsableRect(&rc);
        GUI_BeginOffscreen(rc.width-x);
        for (int i=0; i<textLength; i++) {
            if (s[i] == '&') {
                if (strncmp(s+i, "&lt;", 4)==0) {
                    GUI_DrawCharOffscreen(font, '<');
                    i+=3;
                    continue;
                }
                if (strncmp(s+i, "&gt;", 4)==0) {
                    GUI_DrawCharOffscreen(font, '>');
                    i+=3;
                    continue;
                }
            }
            GUI_DrawCharOffscreen(font, s[i]);
        }
        wnd->DrawImage(x, y+yScroll, GUI_EndOffscreen(), 
                       COLOR_BLACK, COLOR_WHITE,
                       0, 0, 65535, 65535, IMG_CMB_SRC);
    }

    virtual void paint(CWindow* wnd, int yScroll) {
        drawText(wnd, font, yScroll);
    }

    void paint(CWindow* wnd) {
        drawText(wnd, font, 0);
    }

    virtual void deleteChild(CViewable* v) {}

    int getY() const {return y;}
    int getX() const {return x;}

protected:
    HTMLNode() {}
    const char* s;
    int x;
    int y;
    int textLength;
    const FONT* font;
};

////////////////////////////////////////////////////////////////////////////////
// class HTMLHrNode

struct HTMLHrNode : public HTMLNode {
    HTMLHrNode(int y) : HTMLNode(0, y, 0, 0, 0) {}
    void paint(CWindow* wnd, int yScroll) {
        int width = LCD_QueryWidth()-6;
        wnd->DrawLine(2, y+yScroll+1, width, y+yScroll+1, COLOR_GRAY53);
        wnd->DrawLine(2, y+yScroll+2, width, y+yScroll+2, COLOR_BLACK);
    }
};

////////////////////////////////////////////////////////////////////////////////
// class HTMLButtonNode

struct HTMLButtonNode : public HTMLNode {
    HTMLButtonNode(CWindow *wnd, 
                   int x, 
                   int y, 
                   bool center, 
                   const char* tag) : 
        HTMLNode(x, y, 0, 0, 0) {

        Properties p;
        p.load(tag);
        String *name = p.get("name");
        if (name != null) {
            title.append(*name->toString());
        } else {
            title.append("OK");
        }
        button = new CButton(ID_BUTTON, 0, 0, title);
        if (center) {
            this->x = 100-(getWidth()/2);
        }
        wnd->AddChild(button, this->x,y);
        GUI_CancelUpdate();
    }
    void paint(CWindow* wnd, int yScroll) {
        button->SetY(y+yScroll);
        if (button->IsHidden()) {
            button->Show();
        } else {
            button->Draw();
        }
    }
    int getWidth() {
        return button->GetWidth();
    }
    int getHeight() {
        return button->GetWidth();
    }
    void deleteChild(CViewable* v) {
        v->DeleteChild(button);
        delete button;
    }
    private:
    CButton* button;
    String title;
};

////////////////////////////////////////////////////////////////////////////////
// class HTMLAnchorNode

struct AnchorLink : public CViewable {
    AnchorLink(const char *label, 
               int labelLen, 
               const char* link,
               int linkLen,
               const FONT* font,
               U16 width, 
               U16 height) 
        : CViewable(ID_ANCHOR, width, height) {
        this->label = label;
        this->link = link;
        this->labelLen = labelLen;
        this->linkLen = linkLen;
        this->font = font;
        this->bnDown = false;
    }
    void Draw() {
        CWindow *wnd = GetWindow();
        int y = GetY();
        if (wnd && y >= 0) {
            int fontHeight = GUI_FontHeight(font); 
            U16 x = GetX();
            int xpos = wnd->DrawText(label, x, y, font, COLOR_BLACK,0,labelLen,
                                     (bnDown ? IMG_CMB_SRC_INV : IMG_CMB_SRC));
            wnd->DrawLine(x, y+fontHeight-1, xpos, y+fontHeight-1);
        }
    }

    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data) {
        RECT rect;
        bool inRect;
        switch (type) {
        case MSG_PEN_DOWN:
        case MSG_PEN_DOUBLECLICK:
            onBnDown(true);
            break;
        case MSG_PEN_UP:
            if (bnDown) {
                onBnDown(false);
                GetParent()->MsgHandler(MSG_BUTTON_SELECT, from, 1);
            }
            break;
        case MSG_PEN_TRACK:
            GetRect(&rect);
            inRect = (GUI_PointInRect(&rect, (data) >> 16, data));
            if (inRect != bnDown) {
                onBnDown(inRect);
            }
            break;
        default:
            return CViewable::MsgHandler( type, from, data );
        }
        return 1;
    }

    String getLink() {
        String s;
        s.append(link, linkLen);
        return s;
    }
    private:
    void onBnDown(bool down) {
        bnDown = down;
        Draw();
    }
    bool bnDown;
    const FONT* font;
    const char* label;
    const char* link;
    int labelLen;
    int linkLen;
};

struct HTMLAnchorNode : public HTMLNode {
    HTMLAnchorNode(CWindow *wnd, 
                   RECT &rc,
                   const char* s,
                   int slen,
                   const char* link,
                   int linkLen,
                   const FONT* font)
        : HTMLNode(rc.x, rc.y, s, slen, font) {
        button = new AnchorLink(s, slen, link, linkLen, font, 
                                rc.width, rc.height);
        wnd->AddChild(button, this->x,y);
        GUI_CancelUpdate();
    }
    void paint(CWindow* wnd, int yScroll) {
        button->SetY(y+yScroll);
        if (button->IsHidden()) {
            button->Show();
        } else {
            button->Draw();
        }
    }
    void deleteChild(CViewable* v) {
        v->DeleteChild(button);
        delete button;
    }
    private:
    AnchorLink* button;
};

////////////////////////////////////////////////////////////////////////////////
// class HTMLViewable

HTMLViewable::HTMLViewable(const char* str, int width, int height) :
    CViewable(ID_MAIN_WND, width, height),
    html(str) {
    init();
}

void HTMLViewable::loadPage(const char* str) {
    html = str;
    CWindow *wnd = GetWindow();
    nodeList.iterateInit();
    while (nodeList.hasMoreElements()) {
        HTMLNode* p = (HTMLNode*)nodeList.nextElement();
        p->deleteChild(wnd);
    }
    GUI_CancelUpdate();
    init();
    nodeList.removeAll();
    RECT rect;
    GetRect(&rect);
    wnd->DrawRectFilled(&rect, wnd->GetBackgroundColor());
    parseAndPaint();
}

void HTMLViewable::home() {
    if (vScroll != 0) {
        vScroll = 0;
        redraw();
    }
}
    
void HTMLViewable::end() {
    vScroll = (-pageHeight+getHeight())-(maxLineHeight*2);
    redraw();
}

void HTMLViewable::pageUp() { 
    if (vScroll < 0) {
        vScroll += getHeight()-maxLineHeight;
        if (vScroll > 0) {
            vScroll = 0;
        }
        redraw();
    }
}

void HTMLViewable::lineUp() {
    if (vScroll < 0) {
        vScroll += maxLineHeight;
        if (vScroll > 0) {
            vScroll = 0;
        }
        redraw();
    }
}

void HTMLViewable::pageDown() {
    int height = getHeight();
    if (vScroll-height >= -pageHeight-maxLineHeight) {
        vScroll -= height-maxLineHeight;
        if (-vScroll > pageHeight-height-maxLineHeight) {
            // hit bottom
            vScroll = (-pageHeight+height)-(maxLineHeight*2);
        }
        redraw();
    }
}

void HTMLViewable::lineDown() {
    int height = getHeight();
    if (vScroll-height >= -pageHeight-maxLineHeight) {
        vScroll -= maxLineHeight;
        redraw();
    }
}

void HTMLViewable::Draw() {
    if (nodeList.length() == 0) {
        parseAndPaint();
    } else {
        paint();
    }
}

int HTMLViewable::getStyle(bool bold, bool italic) const {
    if (italic && bold)
        return CTRL_BOLDITALIC;
    if (italic)
        return CTRL_ITALIC;
    if (bold) 
        return CTRL_BOLD;
    return CTRL_NORMAL;
}

void HTMLViewable::redraw() {
    CWindow *wnd = GetWindow();
    if (!wnd || IsHidden()) {
        return;
    }

    // disable buttons
    CViewable *child;
    for (int n = 0; (child = wnd->GetChild(n)); ++n) {
        U16 id = child->GetID();
        if (id == ID_ANCHOR || id == ID_BUTTON) {
            child->Hide();
        }
    }

    RECT rect;
    GetRect(&rect);
    wnd->DrawRectFilled(&rect, wnd->GetBackgroundColor());
    paint();
    imgUpdate();
}

void HTMLViewable::paint() {
    CWindow *wnd = GetWindow();
    nodeList.iterateInit();
    int height = getHeight();

    while (nodeList.hasMoreElements()) {
        HTMLNode* p = (HTMLNode*)nodeList.nextElement();
        int y = p->getY()+vScroll;
        if (y > -maxLineHeight) {
            p->paint(wnd, vScroll);
        }
        if (y > height) {
            return;
        }
    }
}

int HTMLViewable::getHeight() {
    RECT rc;
    GetWindow()->GetUsableRect(&rc);
    return rc.height;
}

void HTMLViewable::parseAndPaint() {
    const int size1 = 9;
    const int size2 = 12;
    const int size3 = 16;
    bool italic = false;
    bool bold = false;
    bool center = false;
    bool title = false;
    bool paragraph = false;
    bool preMode = true;
    int size = size2;
    int iBegin = 0;
    int xBegin = 2;
    int yBegin = 0;
    int len = (html == 0 ? 0 : strlen(html));
    int i = 0;
    CWindow *wnd = GetWindow();
    bool imgDirty = true;
    int style = getStyle(bold, italic);
    const FONT *font = GUI_GetFont(size, style);
    int lineHeight = GUI_FontHeight(font)+1;
    int anchorLink = 0;
    int anchorLinkLen= 0;
    int height = getHeight();

    while (i < len) {
        // find the start of the next tag
        bool preModeNL = false;
        while (i<len) {
            if (preMode && html[i] == '\n') {
                if (i>iBegin && html[i-1] == '\r') {
                    i--;
                }
                preModeNL = true;
                break;
            } else if (html[i] == '<') {
                break;
            }
            i++;
        }

        // process open text leading to the found tag 
        if (i > iBegin) {
            if (title) {
                titleText.empty();
                titleText.append(html+iBegin, i-iBegin);
                wnd->SetTitle(titleText);
            } else {
                const char *text = html+iBegin;
                int textLength = i-iBegin;
                int width = preModeNL ? GetWidth() : 
                    GUI_TextWidth(font, text, textLength);
                if (center) {
                    xBegin = 2+((GetWidth() - width)/2);
                }
                if (anchorLink > 0) {
                    // process anchorlink
                    RECT rc = {xBegin, yBegin, width, lineHeight};
                    HTMLAnchorNode *node = new 
                        HTMLAnchorNode(wnd, rc, text, textLength, 
                                       html+anchorLink, anchorLinkLen, font);
                    nodeList.append((Object*)node);
                    xBegin += width;
                    if (yBegin < height) {
                        node->paint(wnd,0);
                    }
                } else if (paragraph) {
                    newLine(xBegin, yBegin, lineHeight);
                    while (textLength > 0) {
                        int len = paraRow(text, textLength, font);
                        HTMLNode *node = new 
                            HTMLNode(xBegin, yBegin, text, len, font);
                        nodeList.append((Object*)node);
                        if (yBegin < height) {
                            node->paint(wnd);
                        }
                        newLine(xBegin, yBegin, lineHeight);
                        text += len;
                        textLength -= len;
                    }
                } else {
                    // use a paragraph to avoid clipping
                    HTMLNode *node = new 
                        HTMLNode(xBegin,yBegin,text,textLength,font);
                    nodeList.append((Object*)node);
                    if (yBegin < height) {
                        node->paint(wnd);
                    }
                    xBegin += width;
                }
            }
        }

        if (imgDirty && yBegin > height) {
            imgUpdate();
            imgDirty = false;
        }

        // find the end tag marker and process tag
        if (preModeNL) {
            newLine(xBegin, yBegin, lineHeight);
            if (html[++i] == '\n') {
                ++i; // skip past \r\n pair
            }
            iBegin = i;
            continue;
        }

        int tagBegin = i;
        while (i<len) {
            if (html[++i] == '>') {
                break;
            }
        }

        iBegin = ++i;
        int tagLength = iBegin-tagBegin-2;
        char tag[30];
        if (tagLength > 0 && tagLength < 30) {
            strncpy(tag, html+tagBegin+1, tagLength);
            tag[tagLength] = '\0';
            bool newFont = false;
            if (tag[0] == '/') {
                if (0 == stricmp(tag, "/b")) {
                    bold = false;
                    newFont = true;
                } else if (0 == stricmp(tag, "/i")) {
                    italic = false;
                    newFont = true;
                } else if (0 == stricmp(tag, "/center")) {
                    center = false;
                } else if (0 == stricmp(tag, "/font")) {
                    size = size2;
                    newFont = true;
                } else if (0 == stricmp(tag, "/p")) {
                    paragraph = false;
                } else if (0 == stricmp(tag, "/title")) {
                    title = false;
                } else if (0 == stricmp(tag, "/pre")) {
                    preMode = false;
                } else if (0 == stricmp(tag, "/a")) {
                    anchorLink = anchorLinkLen = 0;
                }
            } else {
                if (0 == stricmp(tag, "b")) {
                    bold = true;
                    newFont = true;
                } else if (0 == stricmp(tag, "i")) {
                    italic = true;
                    newFont = true;
                } else if (0 == stricmp(tag, "p")) {
                    paragraph = true;
                } else if (0 == stricmp(tag, "center")) {
                    center = true;
                } else if (0 == stricmp(tag, "br")) {
                    newLine(xBegin, yBegin, lineHeight);
                    lineHeight = GUI_FontHeight(font)+1;
                } else if (0 == stricmp(tag, "hr")) {
                    newLine(xBegin, yBegin, lineHeight);
                    HTMLHrNode *node = new HTMLHrNode(yBegin);
                    nodeList.append((Object*)node);
                    if (yBegin < height) {
                        node->paint(wnd, 0);
                    }
                    yBegin+=4;
                } else if (0 == strnicmp(tag, "a href=", 7)) {
                    anchorLink = tagBegin + 8; // includes count of < char
                    anchorLinkLen = tagLength-7;
                } else if (0 == stricmp(tag, "font size=1")) {
                    size = size1;
                    newFont = true;
                } else if (0 == stricmp(tag, "font size=2")) {
                    size = size2;
                    newFont = true;
                } else if (0 == stricmp(tag, "font size=3")) {
                    size = size3;
                    newFont = true;
                } else if (0 == stricmp(tag, "title")) {
                    title = true;
                } else if (0 == stricmp(tag, "pre")) {
                    preMode = true;
                } else if (0 == strnicmp(tag, "input type=button",17)) {
                    HTMLButtonNode *node = 
                        new HTMLButtonNode(wnd, xBegin, yBegin, center, tag+6);
                    nodeList.append((Object*)node);
                    if (yBegin < height) {
                        node->paint(wnd,0);
                    }
                    if (node->getHeight() > lineHeight) {
                        lineHeight = node->getHeight();
                    }
                    xBegin = node->getX() + node->getWidth();
                }
            }
            if (newFont) {
                style = getStyle(bold, italic);
                font = GUI_GetFont(size, style);
                int fnHeight = GUI_FontHeight(font); 
                if (fnHeight > lineHeight) {
                    lineHeight = fnHeight;
                }
                if (lineHeight > maxLineHeight) {
                    maxLineHeight = lineHeight; // save max line height
                }
            }                
        }
    }
    pageHeight = yBegin;
    if (imgDirty) {
        imgUpdate();
    }
    GUI_CancelUpdate(); // drawing completed
}

int HTMLViewable::paraRow(const char* text, int textLength, const FONT *font) {
    int width = GUI_TextWidth(font, text, textLength);
    RECT rc;
    GetWindow()->GetUsableRect(&rc);
    rc.width -= 5;
    
    if (width <= rc.width) {
        return textLength;
    }
    
    int ibreak = -1;
    int i = 0;
    while (i<textLength) {
        if (text[i++] == ' ') {
            ibreak = i;
            break;
        }
    }

    if (ibreak == -1) {
        return GUI_TruncateText(font, text, rc.width, false);
    }

    // scan forwards to find the break point
    while (i<textLength && GUI_TextWidth(font, text, i) < rc.width) {
        ibreak = i;
        while (i<textLength) {
            if (text[i++] == ' ') {
                break;
            }
        }
    }

    return ibreak;
}

////////////////////////////////////////////////////////////////////////////////
// class HTMLWindow

HTMLWindow::HTMLWindow(const char* s) :
    CWindow(0, 0, 0, 200, 240, "") {
    RECT rect;
    GetUsableRect(&rect);
    htmlView = new HTMLViewable(s, rect.width, rect.height);
    AddChild(htmlView, 0, 0);
}

HTMLWindow::HTMLWindow(const PKG *pkg) :
    CWindow(0, 0, 0, 200, 240, "") {
    this->pkg = pkg;
    loadAnchors(PKG_String(pkg, 0));
    // anchorMap maps between indexes and <a href=ff> file names
    htmlView = new HTMLViewable(PKG_String(pkg, 1), 200,240);
    AddChild(htmlView, 0, 0);
}

void HTMLWindow::Draw() {
    DrawBackground();
    DrawTitle();
    htmlView->Draw();
}

S32 HTMLWindow::MsgHandler(MSG_TYPE type, CViewable *object, S32 data) {
    switch (type) { 
    case MSG_BUTTON_SELECT: {
        U16 id = object->GetID(); 
        switch (id) {
        case ID_BUTTON:
            Close();
            break;
        case ID_ANCHOR:
            AnchorLink* anchorLink = (AnchorLink*)object;
            String* anchor = anchorMap.get(anchorLink->getLink());
            if (anchor != null) {
                GUI_EventMessage(MSG_USER, this, anchor->toInteger());
            }
        }
        return 1;
    }

    case MSG_USER:
        htmlView->loadPage(PKG_String(pkg, data));
        return 1;

    case MSG_KEY:
        switch (data) {
        case K_JOG_PAGE_UP:
        case K_JOG_UP:
        case K_PAGE_UP:
        case 'u':
        case 'U':
            htmlView->pageUp();
            break;
        case K_UP:
            htmlView->lineUp();
            break;
        case K_JOG_PAGE_DOWN:
        case K_JOG_DOWN:
        case K_PAGE_DOWN:
        case 'd':
        case 'D':
            htmlView->pageDown();
            break;
        case K_DOWN:
            htmlView->lineDown();
            break;
        case K_LEFT:
        case 'h':
        case 'H':
            htmlView->home();
            break;
        case K_RIGHT:
        case 'e':
        case 'E':
            htmlView->end();
            break;
        }
        return 1; // message handled
    default:
        break;
    }
    return CWindow::MsgHandler(type, object, data);
}
