// -*- c-file-style: "java" -*-
// $Id: Fl_Help_Widget.cpp,v 1.1 2005-03-07 22:34:17 zeeb90au Exp $
//
// Copyright(C) 2001-2005 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
// Based on my ebookman HTMLWindow.cpp with some methods borrowed
// from the fltk HelpView
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <errno.h>
#include <fltk/draw.h>
#include <fltk/layout.h>
#include <fltk/Rectangle.h>
#include <fltk/Font.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#include <fltk/damage.h>

#include "HelpWidget.h"
#include "MainWindow.h"

// uncomment for unit testing and then run:
// make Fl_Ansi_Window.exe
#define UNIT_TEST 1

static const char* bullet = " * ";
#define DEFAULT_INDENT 2
#define BULLET_INDENT  5
#define FOREGROUND_COLOR fltk::color(0,0,128)
#define BACKGROUND_COLOR fltk::color(230,230,230)
#define FONT_SIZE 11
#define SCROLL_W 17

Color getColor(const char *n);
void lineBreak(const char* s, int slen, int width, int& stlen, int& pxlen);
struct AnchorNode;

//--MetaNode--------------------------------------------------------------------

struct MetaNode {
    int x;
    int y;
    int height;
    int width;
    int lineHeight;
    int indent;
    bool uline;
    bool center;
    int fontSize;
    Font* font;
    Color color;
    Color background;
    Group* wnd;
    AnchorNode* anchor;
};

//--BaseNode--------------------------------------------------------------------

struct BaseNode : public Object {
    virtual void display(MetaNode* mn) {}
    virtual int indexOf(const char* sFind, bool matchCase) {return -1;}
};

//--FontNode--------------------------------------------------------------------

struct FontNode : public BaseNode {
    FontNode(Font* font,
             int fontSize,
             Color color, 
             bool bold, 
             bool italic) {
        this->font = font;
        this->fontSize = fontSize;
        this->color = color;
        if (this->font && bold) {
            this->font = this->font->bold();
        }
        if (this->font && italic) {
            this->font = this->font->italic();
        }
    }
    
    void display(MetaNode* mn) {
        if (font) {
            setfont(font, fontSize);
        }
        if (color) {
            setcolor(color);
        }

        int oldLineHeight = mn->lineHeight;
        mn->lineHeight = (int)(getascent()+getdescent());
        if (mn->lineHeight > oldLineHeight) {
            mn->y += (mn->lineHeight - oldLineHeight);
        }
        mn->font = font;
        mn->color = color;
        mn->fontSize = fontSize;
    }

    Font* font; // covers face,bold,italic
    int fontSize;
    Color color;
};

//--BrNode---------------------------------------------------------------------

struct BrNode : public BaseNode {
    BrNode(U16 indent, U16 newline) {
        this->indent = indent;
        this->newline = newline;
    }
    
    void display(MetaNode* mn) {
        if (newline) {
            mn->x = mn->indent+indent;
            mn->y += (mn->lineHeight*newline);
            mn->lineHeight = (int)(getascent()+getdescent());
        }
    }
    
    U16 newline;
    U16 indent;
};

//--AnchorNode--------------------------------------------------------------

struct AnchorNode : public BaseNode {
    AnchorNode(Properties& p) : BaseNode() {
        String* prop;
        prop = p.get("name");
        if (prop != null) {
            name.append(prop->toString());
        }
        prop = p.get("href");
        if (prop != null) {
            href.append(prop->toString());
        }
        wrapxy = 0;
        pushed = 0;
    }

    void display(MetaNode* mn) {
        if (pushed) {
            mn->uline = true;
        }
        mn->anchor = this;
        x1 = mn->x;
        y1 = mn->y - mn->lineHeight;
        wrapxy = 0;
    }

    bool ptInSegment(int x, int y) {
        if (y >= y1 && y <= y2) {
            // found row
            if ((x < x1 && y < y1+lineHeight) ||
                (x > x2 && y > y2-lineHeight)) {
                // outside row start or end
                return 0;
            }
            return this;
        }
        return 0;
    }

    String name;
    String href;
    int x1,x2,y1,y2;
    int lineHeight;
    bool wrapxy; // begin on page boundary
    bool pushed;
};

AnchorNode* selectedAnchor = 0;

struct HTMLAnchorEndNode : public BaseNode {
    HTMLAnchorEndNode() : BaseNode() {}
    void display(MetaNode* mn) {
        beginNode = mn->anchor;
        if (beginNode) {
            beginNode->x2 = mn->x;
            beginNode->y2 = mn->y;
            beginNode->lineHeight = mn->lineHeight;
        }
        mn->uline = false;
        mn->anchor = 0;
    }
    AnchorNode* beginNode;
};

//--TextNode--------------------------------------------------------------------

struct TextNode : public BaseNode {
    TextNode(const char* s, U16 textlen, U8 uline, U8 center) {
        this->s = s;
        this->textlen = textlen;
        this->uline = uline;
        this->center = center;
        this->width = 0;
    }

    void display(MetaNode* mn) {
        if (width == 0) {
            width = (int)getwidth(s, textlen);
        }
        if (width < mn->width-mn->x) {
            // simple non-wrapping textout
            drawtext(s, textlen, mn->x, mn->y);
            if (uline || mn->uline) {
                drawline(mn->x, mn->y+1, mn->x+width, mn->y+1);
            }
            mn->x += width;
        } else {
            int linelen, linepx;
            int len = textlen;
            const char* p = s;
            while (len > 0) {
                lineBreak(p, len, mn->width-mn->x, linelen, linepx);
                if (linepx == -1) {
                    // no break point - create new line if required
                    linepx = (int)getwidth(p, linelen);
                    if (linepx+2 > mn->width-mn->x) {
                        mn->x = mn->indent;
                        mn->y += mn->lineHeight;

                        // let anchor know where it really starts
                        if (mn->anchor && mn->anchor->wrapxy == false) {
                            mn->anchor->x1 = mn->x;
                            mn->anchor->y1 = mn->y - mn->lineHeight;
                            mn->anchor->wrapxy = true;
                        }
                    }
                }
                drawtext(p, linelen, mn->x, mn->y);
                if (uline || mn->uline) {
                    drawline(mn->x, mn->y+1, mn->x+linepx, mn->y+1);
                }
                p += linelen;
                len -= linelen;

                if (mn->anchor) {
                    mn->anchor->wrapxy = true;
                }

                if (mn->x+linepx < mn->width) {
                    mn->x += linepx;
                } else {
                    mn->x = mn->indent;
                    mn->y += mn->lineHeight;
                }
            }
        }
        mn->uline = uline;
        mn->center = center;
    }

    int indexOf(const char* sFind, bool matchCase) {
        int numMatch = 0;
        int findLen = strlen(sFind);
        for (int i=0; i<textlen; i++) {
            bool equals = matchCase ? 
                s[i] == sFind[numMatch] :
                toupper(s[i]) == toupper(sFind[numMatch]);
            numMatch = (equals ? numMatch+1 : 0);
            if (numMatch == findLen) {
                return i+1;
            }
        }
        return -1;
    }

    const char* s; // 4
    U16 textlen;   // 4
    U16 width;     // 4
    U8 uline;      // 2
    U8 center;     // 2
};

//--HrNode------------------------------------------------------------------

struct HrNode : public BaseNode {
    HrNode() : BaseNode() {}

    void display(MetaNode* mn) {
        mn->x = mn->indent;
        mn->y += mn->lineHeight;
        setcolor(GRAY45);
        drawline(mn->x, mn->y+1, mn->x+mn->width-6, mn->y+1);
        setcolor(GRAY99);        
        drawline(mn->x, mn->y+2, mn->x+mn->width-6, mn->y+2);
        setcolor(mn->color);
        mn->y+=4;
    }
};

//--TdNode------------------------------------------------------------------

struct TdNode : public BaseNode {
    TdNode(int col) : BaseNode() {
        this->col = col;
    }
    void display(MetaNode* mn) {}
    U16 col;
};

//--ImageNode---------------------------------------------------------------

struct ImageNode : public BaseNode {
    ImageNode(Image* image) : 
        BaseNode() {
        this->image = image;
    }
    void display(MetaNode* mn) {
    }
    const Image* image;
};

//--ButtonNode--------------------------------------------------------------

struct ButtonNode : public BaseNode {
    ButtonNode(Group* wnd, Properties& p, Font* font);

    void display(MetaNode* mn) {
        button->x(mn->x);
        button->y(mn->y);

        if (button->visible()) {
            button->redraw();
        } else {
            button->show();
        }
    }
    int getWidth() {
        return button->w();
    }
    int getHeight() {
        return button->h();
    }
    void deleteChild(Group* g) {
        g->remove(button);
        delete button;
    }
    Widget* button;
    String title;
};

ButtonNode::ButtonNode(Group* wnd, Properties& p, Font* font)
    : BaseNode() {
    String* prop;
    prop = p.get("value");
    if (prop != null) {
        title.append(prop->toString());
    }
    prop = p.get("checked");
    int checked=-1;
    if (prop != null) {
        checked = prop->toInteger();
    }
    prop = p.get("type");
    if (prop == null || prop->equals("text")) {
//         button = new CTextEdit(ID_TEXTBOX, rc.width, rc.height, 
//                                TEXTOPTION_ONELINE);
//         if (title.length()) {
//             ((CTextEdit*)button)->SetText(title);
//         }
//         button->SetFont(font);
    } else if (prop->equals("checkbox")) {
//         button = new CCheckbox(ID_CHKBOX, 0, 0, title);
//         button->SetFont(font);
//         if (checked != -1) {
//             ((CCheckbox*)button)->SetDownStatus(true);
//         }
    } else if (prop->equals("option")) {
//         button = new OptionBox(new OptionMenu(p), checked);
    } else {
 //        button = new CButton(ID_BUTTON, 0, 0, title);
//         button->SetFont(font);
    }
    prop = p.get("size");
    if (prop != null) {
        button->w(prop->toInteger());
    }
}

//--NamedInput------------------------------------------------------------------

struct NamedInput : public Object {
    NamedInput(ButtonNode* node, String* name) {
        this->input = node;
        this->name.append(name->toString());
    }
    ~NamedInput() {}
    ButtonNode* input;
    String name;
};

//--HelpWidget------------------------------------------------------------------

HelpWidget::HelpWidget(const char* str, int width, int height) :
    Group(0, 0, width, height), 
    nodeList(100), namedInputs(10), buttons(10), anchors(10) {
    user_data((void*)ID_MAIN_WND);
    scrollbar = new Scrollbar(width-SCROLL_W, 0, SCROLL_W, height);
    scrollbar->set_vertical();
    scrollbar->value(0, height, 0, 1);
    scrollbar->step(8.0);
    scrollbar->show();
    loadPage(str);
}

HelpWidget::~HelpWidget() {
    cleanup();
}

void HelpWidget::init() {
    vscroll = 0;
    pageHeight = 0;
    maxObjectHeight = 0;
    maxTextHeight = 0;
    nextPageDown = 0;
    nextPageUp = 0;
    numLines = 0;
    titleText.empty();
}

void HelpWidget::cleanup() {
    buttons.iterateInit();
    while (buttons.hasNext()) {
        ButtonNode* p = (ButtonNode*)nodeList.next();
        p->deleteChild(this);
    }

    // button/anchor items destroyed in nodeList    
    buttons.emptyList();
    anchors.emptyList();
    nodeList.removeAll();
    namedInputs.removeAll();
}

void HelpWidget::loadPage(const char* str) {
    htmlStr = str;
    reloadPage();
}

void HelpWidget::reloadPage() {
    cleanup();
    init();
    compileHTML();
}

// returns the control with the given name
Widget* HelpWidget::getInput(const char* name) {
    namedInputs.iterateInit();
    NamedInput* ni = 0;
    while (namedInputs.hasNext()) {
        ni = (NamedInput*)namedInputs.next();
        if (ni->name.equals(name)) {
            return ni->input->button;
        }
    }
    return null;
}

// return the value of the given button control
const char* HelpWidget::getInputValue(Widget* button) {
    namedInputs.iterateInit();
    NamedInput* ni = 0;
    while (namedInputs.hasNext()) {
        ni = (NamedInput*)namedInputs.next();
        if (ni->input->button == button) {
            return ni->input->title.toString();
        }
    }
    return null;
}

// return the name of the given button control
const char* HelpWidget::getInputName(Widget* button) {
    namedInputs.iterateInit();
    NamedInput* ni = 0;
    while (namedInputs.hasNext()) {
        ni = (NamedInput*)namedInputs.next();
        if (ni->input->button == button) {
            return ni->name.toString();
        }
    }
    return null;
}

void HelpWidget::getInputProperties(Properties& p) {
    namedInputs.iterateInit();
    NamedInput* ni = 0;

    while (namedInputs.hasNext()) {
        ni = (NamedInput*)namedInputs.next();
        switch ((int)ni->input->button->user_data()) {
        case ID_TEXTBOX:
            //            p.put(ni->name.toString(), (const char*)
            //                  ((CTextEdit*)ni->input->button)->GetTextHandle());
            break;
        case ID_CHKBOX:
            //            p.put(ni->name.toString(), (const char*)
            //                  ((CButton*)ni->input->button)->GetDownStatus() ? 
            //                  "true" : "false");
            break;
        case ID_OPTION:
            //            p.put(ni->name.toString(), (const char*)
            //  ((OptionBox*)ni->input->button)->getOption());
            break;
        }
    }
}

void HelpWidget::home() {
    if (vscroll != 0) {
        vscroll = 0;
        redraw();
        nextPageUp = 0;
    }
}
    
void HelpWidget::end() {
    int height = h();
    if (pageHeight > height) {
        vscroll = (-pageHeight+height)-(maxTextHeight*2);
        redraw();
        nextPageUp = 0;
    }
}

void HelpWidget::pageUp() { 
    int height = h();
    int y = 0;
    BaseNode* p = 0;

    if (vscroll < 0) {
        if (nextPageUp != 0) {
            vscroll = nextPageUp;
        } else {
            // find the start of the previous page
            nodeList.iterateInit();
            //height = h();
            while (nodeList.hasNext()) {
                p = (BaseNode*)nodeList.next();
                //y = p->getY();
                if (y >= -vscroll-height) {
                    vscroll = -y;
                    break;
                }
            }
           
        }
        nextPageUp = 0;
        if (vscroll > 0) {
            vscroll = 0;
        }
        redraw();
        nextPageUp = 0;
    }
}

void HelpWidget::lineUp() {
    if (vscroll < 0) {
        vscroll += maxTextHeight;
        if (vscroll > 0) {
            vscroll = 0;
        }
        redraw();
        nextPageUp = 0;
    }
}

void HelpWidget::pageDown() {
    int height = h();
    if (pageHeight > height) {
        if (vscroll-height >= -pageHeight+(height/4)) {
            nextPageUp = vscroll;
            if (nextPageDown != 0) {
                vscroll = nextPageDown;
            } else {
                // scroll down
                vscroll -= height-maxTextHeight;
            }
        } else {
            // goto the end
            vscroll = (-pageHeight+height)-(maxTextHeight*2);
        }
        redraw();
    }
}

void HelpWidget::lineDown() {
    int height = h();
    if (vscroll-height >= -pageHeight-maxTextHeight) {
        vscroll -= maxTextHeight;
        redraw();
    }
}

void HelpWidget::navigateTo(const char* anchorName) {
}

void HelpWidget::navigateTo(int scroll) {
    if (scroll < 1 && scroll > -pageHeight && vscroll != scroll) {
        vscroll = scroll;
        redraw();
    }
}

void HelpWidget::layout() {
    scrollbar->resize(w()-SCROLL_W, 0, SCROLL_W, h());
}

void HelpWidget::draw() {
    BaseNode* p = 0;
    MetaNode mn;
    mn.uline = false;
    mn.center = false;
    mn.wnd = this;
    mn.anchor = 0;

    mn.font = HELVETICA;
    mn.fontSize = FONT_SIZE;
    mn.color = FOREGROUND_COLOR;
    mn.background = BACKGROUND_COLOR;
    mn.height = h();
    mn.width = w()-SCROLL_W;
    mn.x = DEFAULT_INDENT;
    mn.indent = DEFAULT_INDENT;

    // must call setfont() before getascent()
    setfont(mn.font, mn.fontSize);
    mn.y = (int)getascent()+vscroll;
    mn.lineHeight = mn.y + (int)+getdescent();

    if (selectedAnchor && (damage()&DAMAGE_PUSHED)) {
        int h = (selectedAnchor->y2-selectedAnchor->y1)+mn.lineHeight;
        push_clip(Rectangle(0, selectedAnchor->y1, mn.width, h));
    }

    update_child(*scrollbar);
    setcolor(mn.background);
    fillrect(Rectangle(w(), h()));
    setcolor(mn.color);

    nodeList.iterateInit();
    while (nodeList.hasNext()) {
        p = (BaseNode*)nodeList.next();
        p->display(&mn);
        if (mn.y > mn.height) {
            break;
        }
    }
    pop_clip();
}

bool HelpWidget::find(const char* s, bool matchCase) {
    nodeList.iterateInit();
    BaseNode* p = 0;
    while (nodeList.hasNext()) {
        p = (BaseNode*)nodeList.next();
//         if (p->getY() > -vscroll) {
//             // start looking from this point onwards
//             if (p->indexOf(s, matchCase) != -1) {
//                 navigateTo(-p->getY());
//                 return true;
//             }
//         }
    }
    return false;
}

void HelpWidget::onMove(int event) {
    if (selectedAnchor && event == fltk::DRAG) {
        bool pushed = selectedAnchor->ptInSegment(fltk::event_x(), 
                                                  fltk::event_y());
        if (selectedAnchor->pushed != pushed) {
            Widget::cursor(fltk::CURSOR_HAND);
            selectedAnchor->pushed = pushed;
            redraw(DAMAGE_PUSHED);
        }
    } else {
        anchors.iterateInit();
        AnchorNode* p = 0;
        while (anchors.hasNext()) {
            p = (AnchorNode*)anchors.next();
            if (p->ptInSegment(fltk::event_x(), fltk::event_y())) {
                Widget::cursor(fltk::CURSOR_HAND);
                return;
            }
        }
        Widget::cursor(fltk::CURSOR_DEFAULT);
    }
}

void HelpWidget::onPush(int event) {
    selectedAnchor = 0;
    anchors.iterateInit();
    AnchorNode* p = 0;
    while (anchors.hasNext()) {
        p = (AnchorNode*)anchors.next();
        if (p->ptInSegment(fltk::event_x(), fltk::event_y())) {
            selectedAnchor = p;
            selectedAnchor->pushed = true;
            Widget::cursor(fltk::CURSOR_HAND);
            redraw(DAMAGE_PUSHED);
            break;
        }
    }
}

int HelpWidget::handle(int event) {
    switch (event) {
    case fltk::PUSH:
        onPush(event);
        return 1;

    case fltk::ENTER:
        return 1;

    case fltk::MOVE:
    case fltk::DRAG:
        onMove(event);
        return 1;

    case fltk::RELEASE:
        if (selectedAnchor) {
            Widget::cursor(fltk::CURSOR_DEFAULT);
            selectedAnchor->pushed = false;
            redraw(DAMAGE_PUSHED);
        }
        return 1;
    }
    return 0;
}

void HelpWidget::compileHTML() {
    bool pre = false;
    bool bold = false;
    bool italic = false;
    U8 center = false;
    U8 uline = false;

    Font *font = fltk::HELVETICA;
    Color color = FOREGROUND_COLOR;
    int fontSize = FONT_SIZE;
    int taglen = 0;
    int textlen = 0;

    Properties p(10);
    String* prop;
    BaseNode* node;

    const char* text = htmlStr.toString();
    const char* tagBegin = text;
    const char* tagEnd = text;
    const char* tag;

#define ADD_PREV_SEGMENT                                \
    prevlen = i-pindex;                                 \
    if (prevlen > 0) {                                  \
        node = new TextNode(p, prevlen, uline, center); \
        nodeList.append(node);                          \
    }

    while (text && *text) {
        // find the start of the next tag
        while (*tagBegin != 0 && *tagBegin != '<') {
            tagBegin++;
        }

        tagEnd = tagBegin;
        while (*tagEnd != 0 && *tagEnd != '>') {
            tagEnd++;
        }
        
        if (tagBegin == text) {
            if (*tagEnd == 0) {
                break; // no tag closure
            }
            text = tagEnd+1; // adjoining tags
        }

        // process open text leading to the found tag 
        if (tagBegin > text) {
            textlen = tagBegin - text;

            const char* p = text;
            int pindex = 0;
            int prevlen,ispace;
            
            for (int i=0; i<textlen; i++) {
                switch (text[i]) {
                case '&':
                    // handle entities
                    for (int j=0; j<entityMapLen; j++) {
                        if (0 == strnicmp(text+i+1, entityMap[j].ent, 
                                          entityMap[j].elen-1)) {
                            ADD_PREV_SEGMENT;
                            // save entity replacement
                            node = new TextNode(&entityMap[j].xlat, 1,
                                                uline, center);
                            nodeList.append(node);
                            // skip past entity
                            i += entityMap[j].elen;
                            pindex = i;
                            p = text+pindex;
                            i--;
                            // stop searching
                            break;
                        }
                    }
                    break;
                    
                case '\r':
                case '\n':
                    ADD_PREV_SEGMENT;
                    if (pre) {
                        node = new BrNode(0, 1);
                        nodeList.append(node);
                    }
                    // skip newline character
                    pindex = i+1;
                    p = text+pindex;
                    break;
                    
                case ' ':
                    // skip multiple whitespaces
                    ispace = i;
                    while (ispace<textlen && text[ispace+1] == ' ') {
                        ispace++;
                    }
                    if (ispace > i) {
                        ADD_PREV_SEGMENT;
                        pindex = i = ispace;
                        p = text+pindex;
                    }
                    break;
                }
            } // end for (int i=0; i<textlen...

            int len = textlen-pindex;
            if (len) {
                node = new TextNode(p, len, uline, center);
                nodeList.append(node);
            }
            text = *tagEnd == 0 ? 0 : tagEnd+1;
        }

        // process the tag
        taglen = tagEnd - tagBegin - 1;
        if (taglen > 0) {
            tag = tagBegin+1;
            if (tag[0] == '/') {
                tag++;
                if (0 == strnicmp(tag, "b", 1)) {
                    bold = false;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "i", 1)) {
                    italic = false;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "center", 6)) {
                    center = false;
                } else if (0 == strnicmp(tag, "font", 4)) {
                    font = fltk::HELVETICA;
                    fontSize = FONT_SIZE;
                    color = FOREGROUND_COLOR;
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "pre", 3)) {
                    pre = false;
                } else if (0 == strnicmp(tag, "p", 1)) {
                    // end paragraph
                } else if (0 == strnicmp(tag, "a", 1)) {
                    nodeList.append(new HTMLAnchorEndNode());
                } else if (0 == strnicmp(tag, "ul", 2)) {
                    nodeList.append(new BrNode(0, 2));
                } else if (0 == strnicmp(tag, "u", 1)) {
                    uline = false;
                } else if (0 == strnicmp(tag, "td", 2)) {
                    // end td
                } else if (0 == strnicmp(tag, "tr", 2)) {
                    nodeList.append(new BrNode(0, 1));
                } else if (0 == strnicmp(tag, "table", 5)) {
                }
            } else {
                if (0 == strnicmp(tag, "br", 2)) {
                    nodeList.append(new BrNode(0, 1));
                } else if (0 == strnicmp(tag, "b", 1)) {
                    bold = true;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "i", 1)) {
                    italic = true;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "center", 6)) {
                    center = true;
                } else if (0 == strnicmp(tag, "hr", 2)) {
                    nodeList.append(new HrNode());
                } else if (0 == strnicmp(tag, "title", 5)) {
                    // skip past </title>
                    tagEnd = strchr(tagEnd+1, '>');
                    text = tagEnd+1;
                } else if (0 == strnicmp(tag, "pre", 3)) {
                    pre = true;
                } else if (0 == strnicmp(tag, "p", 1)) {
                    // paragraph
                } else if (0 == strnicmp(tag, "table", 5)) {
                    nodeList.append(new BrNode(0, 1));
                } else if (0 == strnicmp(tag, "tr", 2)) {
                    // tr
                } else if (0 == strnicmp(tag, "td", 2)) {
                    nodeList.append(new TdNode(0));
                } else if (0 == strnicmp(tag, "ul>", 3)) {
                    nodeList.append(new BrNode(0, 1));
                } else if (0 == strnicmp(tag, "u>", 2)) {
                    uline = true;
                } else if (0 == strnicmp(tag, "li>", 3)) {
                    nodeList.append(new BrNode(BULLET_INDENT, 1));
                    node = new TextNode(bullet, strlen(bullet), uline, center);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "a ", 2)) {
                    p.load(tag+2, taglen-2);
                    node = new AnchorNode(p);
                    nodeList.append(node);
                    anchors.append(node);
                    p.removeAll();
                } else if (0 == strnicmp(tag, "font ", 5)) {
                    p.load(tag+5, taglen-5);
                    prop = p.get("size");
                    if (prop != null) {
                        fontSize = prop->toInteger();
                    }
                    prop = p.get("color");
                    if (prop != null) {
                        color = getColor(prop->toString());
                    }
                    prop = p.get("face");
                    if (prop != null) {
                        font = fltk::font(*prop->toString());
                    }
                    p.removeAll();
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "input ", 6)) {
                    p.load(tag+6, taglen-6);
                    ButtonNode *node = new ButtonNode(this, p, font);
                    nodeList.append(node);
                    buttons.append(node);
                    prop = p.get("name");
                    if (prop != null) {
                        namedInputs.append(new NamedInput(node, prop));
                    }
                    p.removeAll();
                } else if (0 == strnicmp(tag, "img ", 4)) {
                    // image tag
                }
            } // end if-start, else-end tag
        } // if found a tag
        tagBegin = *tagEnd == 0 ? 0 : tagEnd+1;
    }
}

void HelpWidget::copyText(int begin, int end) {
    String out;
    nodeList.iterateInit();
    TextNode* p = 0;
    if (begin > end) {
        int i = end;
        end = begin;
        begin = i;
    }
    begin -= maxTextHeight;
    while (nodeList.hasNext()) {
        p = (TextNode*)nodeList.next();
//         if (p->getY() > end) {
//             break;
//         } else if (p->getY() >= begin) {
//             out.append(p->s, p->textlen);
//             out.append(" ", 1);
//         }
    }

    if (out.length() > 0) {
    }
}

// O - 0 on success, -1 on error
// I - Filename to load (may also have target)
int HelpWidget::load(const char *f) {
    FILE *fp;                     // File to read from
    long len;                     // Length of file
    char *target;                 // Target in file
    //char *slash;                  // Directory separator
    const char *localname;        // Local filename
    char error[1024];             // Error buffer
    char newname[1024];           // New filename buffer
    
    strlcpy(newname, f, sizeof (newname));
    if ((target = strrchr (newname, '#')) != NULL) {
        *target++ = '\0';
    }
    //if (link_) {
        //localname = (*link_) (this, newname);
    //} else {
        //localname = filename_;
    //}

    //if (!localname) {
    //return (0);
    //}
    localname = f;
    
    //    strlcpy (filename_, newname, sizeof (filename_));
    //    strlcpy (directory_, newname, sizeof (directory_));
    
    // Note: We do not support Windows backslashes, since they are illegal
    //       in URLs...
//     if ((slash = strrchr (directory_, '/')) == NULL) {
//         directory_[0] = '\0';
//     } else if (slash > directory_ && slash[-1] != '/') {
//         *slash = '\0';
//     }

    if (strncmp(localname, "ftp:", 4) == 0 ||
        strncmp(localname, "http:", 5) == 0 ||
        strncmp(localname, "https:", 6) == 0 ||
        strncmp(localname, "ipp:", 4) == 0 ||
        strncmp(localname, "mailto:", 7) == 0 ||
        strncmp(localname, "news:", 5) == 0) {
        // remote link wasn't resolved
        snprintf (error, sizeof (error),
                  "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
                  "<BODY><H1>Error</H1>"
                  "<P>Unable to follow the link \"%s\" - "
                  "no handler exists for this URI scheme.</P></BODY>", localname);
        htmlStr = error;
    } else {
        if (strncmp (localname, "file:", 5) == 0) {
            localname += 5; // adjust for local filename
        }
        
        if ((fp = fopen (localname, "rb")) != NULL) {
            fseek (fp, 0, SEEK_END);
            len = ftell(fp);
            rewind(fp);
            htmlStr.empty();
            htmlStr.append(fp, len);
            fclose(fp);
        } else {
            snprintf (error, sizeof (error),
                      "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
                      "<BODY><H1>Error</H1>"
                      "<P>Unable to follow the link \"%s\" - "
                      "%s.</P></BODY>", localname, strerror (errno));
            htmlStr = error;
        }
    }

    reloadPage();

//     if (target)
//         topline (target);
//     else
//         topline (0);

    return 0;
}

//--Helper functions------------------------------------------------------------

#ifdef UNIT_TEST
#include <fltk/run.h>
#include <stdarg.h>
#include <windows.h>
int main(int argc, char **argv) {
    int w = 210; // must be > 104
    int h = 200;
    Window window(w, h, "Browse");
    window.begin();
    const char html[] = "hello<p>rrr&bull;<i><pre><i>"
        "<HTML><HEAD><TITLE>Error</TITLE></HEAD><center>"
        "<font size=40>Big!</font>"
        "<br><font face=Webdings color=red size=20>Big!</font>"
        "Unable</i> to &bull; xxx &bull;&bull;old\nnew?"
        "Unable[     ]to <!--follow the--> link &bull;&bull; " 
        "before link<a href=gothere.html>Link</a>after link"
        "Unable[     ]to <!--follow the--> link &bull;&bull; " 
        "xxx &bull;&bull;  </P></BODY>x";
    const char html2 [] = 
        "<link href=help.css rel=stylesheet type=text/css><title>SmallBASIC Guide</title>&amp;"
        "<a href=0.html><b>Introduction</b></a> | <a href=1.html>The language</a> | <a href=2.html>Programming Tips</a>";
    //HelpWidget out(html2, w, h);
    HelpWidget out("", w, h);
    out.load("help/0.html");
    window.resizable(&out);
    window.end();
    window.show(argc,argv);
    return run();
}

void trace(const char *format, ...) {
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
#endif

/**
 * Returns the number of characters that will fit within
 * the gixen pixel width. 
 */
void lineBreak(const char* s, int slen, int width, int& linelen, int& linepx) {
    // find the end of the first word
    int ibreak = -1;
    int i = 0;
    int txtWidth = 0;
    int breakWidth = -1;

    while (i<slen) {
        if (s[i++] == ' ') {
            ibreak = i;
            break;
        }
    }

    // truncate if no break point found
    if (ibreak == -1) {
        linelen = slen;
        linepx = -1;
        return;
    }

    // scan forwards to find the break point
    txtWidth = (int)getwidth(s, i);
    while (i<slen && txtWidth < width) {
        ibreak = i;
        breakWidth = txtWidth;

        while (i<slen) {
            if (s[i++] == ' ') {
                break;
            }
        }
        txtWidth += (int)getwidth(s+ibreak, i-ibreak);
    }
    
    if (txtWidth < width) {
        linelen = slen;
        linepx = txtWidth;
    } else {
        // doesn't fit
        linelen = ibreak;
        linepx = breakWidth;
    }
}

Color getColor(const char *n) {
    if (!n || !n[0]) {
        return 0;
    }
    
    if (n[0] == '#') {
        // Do hex color lookup
        int rgb = strtol (n + 1, NULL, 16);
        int r = rgb >> 16;
        int g = (rgb >> 8) & 255;
        int b = rgb & 255;
        return (fltk::color ((uchar) r, (uchar) g, (uchar) b));
    } else if (stricmp(n, "black") == 0) {
        return (BLACK);
    } else if (stricmp(n, "red") == 0) {
        return (RED);
    } else if (stricmp(n, "green") == 0) {
        return (fltk::color (0, 0x80, 0));
    } else if (stricmp(n, "yellow") == 0) {
        return (YELLOW);
    } else if (stricmp(n, "blue") == 0) {
        return (BLUE);
    } else if (stricmp(n, "magenta") == 0 || 
               stricmp(n, "fuchsia") == 0) {
        return (MAGENTA);
    } else if (stricmp(n, "cyan") == 0 || 
               stricmp(n, "aqua") == 0) {
        return (CYAN);
    } else if (stricmp(n, "white") == 0) {
        return (WHITE);
    } else if (stricmp(n, "gray") == 0 || 
               stricmp(n, "grey") == 0) {
        return (fltk::color (0x80, 0x80, 0x80));
    } else if (stricmp(n, "lime") == 0) {
        return (GREEN);
    } else if (stricmp(n, "maroon") == 0) {
        return (fltk::color (0x80, 0, 0));
    } else if (stricmp(n, "navy") == 0) {
        return (fltk::color (0, 0, 0x80));
    } else if (stricmp(n, "olive") == 0) {
        return (fltk::color (0x80, 0x80, 0));
    } else if (stricmp(n, "purple") == 0) {
        return (fltk::color (0x80, 0, 0x80));
    } else if (stricmp(n, "silver") == 0) {
        return (fltk::color (0xc0, 0xc0, 0xc0));
    } else if (stricmp(n, "teal") == 0) {
        return (fltk::color (0, 0x80, 0x80));
    } else {
        return 0;
    }
}

//--EndOfFile-------------------------------------------------------------------
