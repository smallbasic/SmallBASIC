// -*- c-file-style: "java" -*-
// $Id: Fl_Help_Widget.cpp,v 1.4 2005-03-10 22:39:55 zeeb90au Exp $
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
#include <fltk/xpmImage.h>

#include "HelpWidget.h"
#include "MainWindow.h"

// uncomment for unit testing and then run:
// make Fl_Ansi_Window.exe
#define UNIT_TEST 1

#define DEFAULT_INDENT 2
#define FOREGROUND_COLOR fltk::color(32,32,32)
#define BACKGROUND_COLOR fltk::color(230,230,230)
#define ANCHOR_COLOR fltk::color(0,0,128)
#define FONT_SIZE 11
#define SCROLL_W 17
#define CELL_SPACING 4

Color getColor(const char *n);
void lineBreak(const char* s, int slen, int width, int& stlen, int& pxlen);
const char* skipWhite(const char* s);
struct AnchorNode;

static char* dot_xpm[] = {
    "5 5 3 1",
    "   c None",
    ".  c #F4F4F4",
    "+  c #000000",
    ".+++.",
    "+++++",
    "+++++",
    "+++++",
    ".+++."};

static xpmImage dotImage(dot_xpm);

//--MetaNode--------------------------------------------------------------------

struct MetaNode {
    S16 x,y;
    U16 height;
    U16 width;
    U16 lineHeight;
    U16 indent;
    U16 lines;
    U16 fontSize;
    U8 uline;
    U8 center;
    U8 inTR;
    U8 textOut;
    Font* font;
    Color color;
    Color background;
    Group* wnd;
    AnchorNode* anchor;

    void newRow(U16 nrows=1) {
        x = indent;
        y += nrows*lineHeight;
        lines += nrows;
    }
};

//--Attributes------------------------------------------------------------------

struct Attributes : public Properties {
    Attributes(int growSize) : Properties(growSize) {}
    String* getValue() {
        return get("value");
    }
    String* getName() {
        return get("name");
    }
    String* getHref() {
        return get("href");
    }
    String* getType() {
        return get("type");
    }
    int getSize() {
        String* s = get("size");
        return (s != null ? s->toInteger() : -1);
    }
};

//--BaseNode--------------------------------------------------------------------

struct BaseNode : public Object {
    virtual void display(MetaNode* mn) {}
    virtual int indexOf(const char* sFind, U8 matchCase) {return -1;}
    virtual int getY() {return -1;}
};

//--FontNode--------------------------------------------------------------------

struct FontNode : public BaseNode {
    FontNode(Font* font,
             int fontSize,
             Color color, 
             U8 bold, 
             U8 italic) {
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
            mn->lines++;
        }
        mn->font = font;
        mn->color = color;
        mn->fontSize = fontSize;
    }

    Font* font; // includes face,bold,italic
    U16 fontSize;
    Color color;
};

//--BrNode----------------------------------------------------------------------

struct BrNode : public BaseNode {
    BrNode(U16 indent, U16 nrows=1) {
        this->indent = indent;
        this->nrows = nrows;
    }
    
    void display(MetaNode* mn) {
        if (indent) {
            mn->indent = indent;
        }
        mn->newRow(nrows);
        mn->lineHeight = (int)(getascent()+getdescent());
    }
    
    U16 indent;
    U16 nrows;
};

//--AnchorNode------------------------------------------------------------------

struct AnchorNode : public BaseNode {
    AnchorNode(Attributes& p) : BaseNode() {
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
        setcolor(ANCHOR_COLOR);
    }

    bool ptInSegment(int x, int y) {
        if (y >= y1 && y <= y2) {
            // found row
            if ((x < x1 && y < y1+lineHeight) ||
                (x > x2 && y > y2-lineHeight)) {
                // outside row start or end
                return false;
            }
            return true;
        }
        return false;
    }

    int getY() {return y1;}

    String name;
    String href;
    S16 x1,x2,y1,y2;
    U16 lineHeight;
    U8 wrapxy; // begin on page boundary
    U8 pushed;
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
        setcolor(mn->color);
    }
    AnchorNode* beginNode;
};

//--StyleNode-------------------------------------------------------------------

struct StyleNode : public BaseNode {
    StyleNode(U8 uline, U8 center) {
        this->uline = uline;
        this->center = center;
    }
    void display(MetaNode* mn) {
        mn->uline = uline;
        mn->center = center;
    }
    U8 uline;   // 2
    U8 center;  // 2
};

//--LiNode----------------------------------------------------------------------

struct LiNode : public BaseNode {
    LiNode(const Style* style) {
        this->style = style;
    }
    void display(MetaNode* mn) {
        int x = DEFAULT_INDENT+3;
        int y = mn->y+ (int)(getascent()-getdescent());
        dotImage.draw(Rectangle(x, y, 5, 5), style, OUTPUT);
        mn->y += mn->lineHeight;
        mn->x = x + 10;
        mn->indent = mn->x;
        mn->lines++;
    }
    const Style* style;
};

//--TextNode--------------------------------------------------------------------

struct TextNode : public BaseNode {
    TextNode(const char* s, U16 textlen) {
        this->s = s;
        this->textlen = textlen;
        this->width = 0;
        this->ybegin = 0;
    }

    void display(MetaNode* mn) {
        mn->textOut = true;
        if (width == 0) {
            width = (int)getwidth(s, textlen);
        }
        if (width < mn->width-mn->x) {
            // simple non-wrapping textout
            drawtext(s, textlen, mn->x, mn->y);
            if (mn->uline) {
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
                    if (linepx > mn->width-mn->x) {
                        mn->newRow();

                        // let anchor know where it really starts
                        if (mn->anchor && mn->anchor->wrapxy == false) {
                            mn->anchor->x1 = mn->x;
                            mn->anchor->y1 = mn->y - mn->lineHeight;
                            mn->anchor->wrapxy = true;
                        }
                    }
                }
                drawtext(p, linelen, mn->x, mn->y);
                if (mn->uline) {
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
                    mn->newRow();
                }
            }
        }
    }

    int indexOf(const char* sFind, U8 matchCase) {
        int numMatch = 0;
        int findLen = strlen(sFind);
        for (int i=0; i<textlen; i++) {
            U8 equals = matchCase ? 
                s[i] == sFind[numMatch] :
                toupper(s[i]) == toupper(sFind[numMatch]);
            numMatch = (equals ? numMatch+1 : 0);
            if (numMatch == findLen) {
                return i+1;
            }
        }
        return -1;
    }

    int getY() {return ybegin;}

    const char* s; // 4
    U16 textlen;   // 4
    U16 width;     // 4
    S16 ybegin;    // 4
};

//--HrNode----------------------------------------------------------------------

struct HrNode : public BaseNode {
    HrNode() : BaseNode() {}

    void display(MetaNode* mn) {
        mn->x = mn->indent;
        mn->y += 4;
        setcolor(GRAY45);
        drawline(mn->x, mn->y+1, mn->x+mn->width-6, mn->y+1);
        setcolor(GRAY99);        
        drawline(mn->x, mn->y+2, mn->x+mn->width-6, mn->y+2);
        setcolor(mn->color);
        mn->y += mn->lineHeight+2;
        mn->lines++;
    }
};

//--TableNode-------------------------------------------------------------------

struct TableNode : public BaseNode {
    TableNode() : BaseNode() {
        rows = 0;
    }

    void display(MetaNode* mn) {
        nextCol = 0;
        nextRow = 0;
        mn->lineHeight = (int)(getascent()+getdescent());
        indent = mn->indent;
        width = mn->width-indent;
        x = indent;
        y = mn->y;
        rowY = y;
    }
    U16 rows;
    U16 nextCol;
    U16 nextRow;
    U16 indent;
    U16 width;
    S16 x, y, rowY;
};

struct TableEndNode : public BaseNode {
    TableEndNode(TableNode* tableNode) : BaseNode() {
        this->table = tableNode;
    }
    void display(MetaNode* mn) {
        if (table == 0) {
            return;
        }
        mn->width = table->width;
        mn->indent = table->indent;
        mn->x = table->indent;
        mn->y = table->y;
        if (mn->textOut) {
            mn->newRow();
        }
        mn->textOut = false;
    }
    TableNode* table;    
};

//--TrNode----------------------------------------------------------------------

struct TrNode : public BaseNode {
    TrNode(TableNode* tableNode, U8 begin) : BaseNode() {
        this->begin = begin;
        this->table = tableNode;
        this->cols = 0;
        if (table && begin) {
            table->rows++;
        }
    }
    void display(MetaNode* mn) {
        if (table == 0) {
            return;
        }
        if (begin) {
            if (mn->textOut) {
                table->y += mn->lineHeight;
                table->x = table->indent;
            }
            mn->textOut = false;
            table->nextCol = 0;
            table->nextRow++;
            table->rowY = table->y;
        }
        mn->inTR = begin;
    }
    TableNode* table;
    U16 cols;
    U8 begin;
};

//--TdNode----------------------------------------------------------------------

struct TdNode : public BaseNode {
    TdNode(TrNode* trNode, U8 begin) : BaseNode() {
        this->begin = begin;
        this->tr = trNode;
        if (tr && begin) {
            tr->cols++;
        }
    }
    void display(MetaNode* mn) {
        if (tr == 0 || tr->table == 0) {
            return;
        }
        TableNode* table = tr->table;
        if (begin) {
            int cellWidth = table->width/tr->cols;
            mn->x = table->x + (table->nextCol*cellWidth);
            mn->y = table->rowY;
            mn->width = mn->x + cellWidth - CELL_SPACING;
            mn->indent = mn->x;
            mn->textOut = false;
            table->nextCol++;
        } else if (mn->y > table->y) {
            table->y = mn->y;
        }
    }
    TrNode* tr;
    U8 begin;
};

//--ImageNode-------------------------------------------------------------------

struct ImageNode : public BaseNode {
    ImageNode(const Style* style, Attributes& p) : BaseNode() {
        this->style = style;
        this->image = 0;
    }
    void display(MetaNode* mn) {
    }
    const Image* image;
    const Style* style;
};

//--ButtonNode------------------------------------------------------------------

struct ButtonNode : public BaseNode {
    ButtonNode(Group* wnd, Attributes& p, Font* font);

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
    String value;
};

ButtonNode::ButtonNode(Group* wnd, Attributes& p, Font* font) : BaseNode() {
    String* prop = 0;
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
    } else if (prop->equals("radio")) {
        //         button = new OptionBox(new OptionMenu(p), checked);
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

static void scrollbar_callback(Widget* s, void *wnd) {
    ((HelpWidget*)wnd)->navigateTo(((Scrollbar*)s)->value());
}

HelpWidget::HelpWidget(const char* str, int width, int height) :
    Group(0, 0, width, height), 
    nodeList(100), namedInputs(10), buttons(10), anchors(10) {
    user_data((void*)ID_MAIN_WND);
    scrollbar = new Scrollbar(width-SCROLL_W, 0, SCROLL_W, height);
    scrollbar->set_vertical();
    scrollbar->value(0, 1, 0, 100);
    scrollbar->pagesize(10);
    scrollbar->user_data(this);
    scrollbar->callback(scrollbar_callback);
    scrollbar->show();
    loadPage(str);
}

HelpWidget::~HelpWidget() {
    cleanup();
}

void HelpWidget::init() {
    vscroll = 0;
    numRows = 0;

    pageHeight = 0;
    maxTextHeight = 0;
}

void HelpWidget::cleanup() {
    int len = buttons.length();
    Object** list = buttons.getList();
    for (int i=0; i<len; i++) {
        ButtonNode* p = (ButtonNode*)list[i];        
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
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    NamedInput* ni = 0;

    for (int i=0; i<len; i++) {
        ni = (NamedInput*)list[i];
        if (ni->name.equals(name)) {
            return ni->input->button;
        }
    }
    return null;
}

// return the value of the given button control
const char* HelpWidget::getInputValue(Widget* button) {
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    NamedInput* ni = 0;

    for (int i=0; i<len; i++) {
        ni = (NamedInput*)list[i];
        if (ni->input->button == button) {
            return ni->input->value.toString();
        }
    }
    return null;
}

// return the name of the given button control
const char* HelpWidget::getInputName(Widget* button) {
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    NamedInput* ni = 0;

    for (int i=0; i<len; i++) {
        ni = (NamedInput*)list[i];
        if (ni->input->button == button) {
            return ni->name.toString();
        }
    }
    return null;
}

void HelpWidget::getInputProperties(Properties& p) {
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    NamedInput* ni = 0;

    for (int i=0; i<len; i++) {
        ni = (NamedInput*)list[i];
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
    }
}
    
void HelpWidget::end() {
    int height = h();
    if (pageHeight > height) {
        vscroll = (-pageHeight+height)-(maxTextHeight*2);
        redraw();
    }
}

void HelpWidget::pageUp() { 
    int height = h();
    int y = 0;
    BaseNode* p = 0;
    Object** list = nodeList.getList();
    int len = nodeList.length();

    if (vscroll < 0) {
        // find the start of the previous page
        for (int i=0; i<len; i++) {
            p = (BaseNode*)list[i];
            y = p->getY();
            if (y >= -vscroll-height) {
                vscroll = -y;
                break;
            }
        }
        
        if (vscroll > 0) {
            vscroll = 0;
        }
        redraw();
    }
}

void HelpWidget::lineUp() {
    if (vscroll < 0) {
        vscroll += maxTextHeight;
        if (vscroll > 0) {
            vscroll = 0;
        }
        redraw();
    }
}

void HelpWidget::pageDown() {
    int height = h();
    if (pageHeight > height) {
        if (vscroll-height >= -pageHeight+(height/4)) {
            // scroll down
            vscroll -= height-maxTextHeight;
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
    if (vscroll != scroll) {
        vscroll = -scroll;
        redraw(DAMAGE_ALL);
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
    mn.x = DEFAULT_INDENT;
    mn.width = w()-(SCROLL_W+DEFAULT_INDENT);
    mn.indent = DEFAULT_INDENT;
    mn.inTR = false;
    mn.textOut = false;
    mn.lines = 0;

    // must call setfont() before getascent() etc
    setfont(mn.font, mn.fontSize);
    mn.y = (int)getascent();
    mn.lineHeight = mn.y+(int)getdescent();
    mn.y += vscroll;

    if (selectedAnchor && (damage()&DAMAGE_PUSHED)) {
        int h = (selectedAnchor->y2-selectedAnchor->y1)+mn.lineHeight;
        push_clip(Rectangle(0, selectedAnchor->y1, mn.width, h));
    }
    
    setcolor(mn.background);
    fillrect(Rectangle(mn.x+mn.width, mn.height));
    setcolor(mn.color);

    Object** list = nodeList.getList();
    int len = nodeList.length();

    for (int i=0; i<len; i++) {
        p = (BaseNode*)list[i];
        p->display(&mn);
        if (mn.y > mn.height && mn.inTR == false) {
            break;
        }
    }
    //scrollbar->value(0, -vscroll, 0, mn.lines);
    update_child(*scrollbar);
    pop_clip();
}

U8 HelpWidget::find(const char* s, U8 matchCase) {
    BaseNode* p = 0;
    Object** list = nodeList.getList();
    int len = nodeList.length();

    for (int i=0; i<len; i++) {
        p = (BaseNode*)list[i];
        if (p->getY() > -vscroll) {
            // start looking from this point onwards
            if (p->indexOf(s, matchCase) != -1) {
                navigateTo(-p->getY());
                return true;
            }
        }
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
        int len = anchors.length();
        Object** list = anchors.getList();
        AnchorNode* p = 0;

        for (int i=0; i<len; i++) {
            p = (AnchorNode*)list[i];
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

    Object** list = anchors.getList();
    int len = anchors.length();
    AnchorNode* p = 0;

    for (int i=0; i<len; i++) {
        p = (AnchorNode*)list[i];
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
    U8 pre = false;
    U8 bold = false;
    U8 italic = false;
    U8 center = false;
    U8 uline = false;

    Font *font = fltk::HELVETICA;
    Color color = FOREGROUND_COLOR;
    int fontSize = FONT_SIZE;
    int taglen = 0;
    int textlen = 0;
    U8 newline = false;

    Stack tableStack(10);
    Stack trStack(10);
    Attributes p(10);
    String* prop;
    BaseNode* node;

    const char* text = htmlStr.toString();
    const char* tagBegin = text;
    const char* tagEnd = text;
    const char* tag;

#define ADD_PREV_SEGMENT                            \
    prevlen = i-pindex;                             \
    if (prevlen > 0) {                              \
        newline = false;                            \
        nodeList.append(new TextNode(p, prevlen));  \
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
                            node = new TextNode(&entityMap[j].xlat, 1);
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
                        nodeList.append(new BrNode(0));
                    }
                    if (isWhite(text[i+1]) == false && 
                        text[i+1] != '<' && newline == false) {
                        // replace with single space
                        nodeList.append(new TextNode(" ", 1));
                        newline = false;
                    }
                    // skip newline character
                    pindex = i+1;
                    p = text+pindex;
                    break;
                    
                case ' ':
                case '\t':
                    // skip multiple whitespaces
                    ispace = i;
                    while (ispace<textlen && (text[ispace+1] == ' ' || 
                                              text[ispace+1] == '\t')) {
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
                newline = false;
                nodeList.append(new TextNode(p, len));
            }
            text = *tagEnd == 0 ? 0 : tagEnd+1;
        }

        // process the tag
        newline = false;
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
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "font", 4)) {
                    font = fltk::HELVETICA;
                    fontSize = FONT_SIZE;
                    color = FOREGROUND_COLOR;
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "pre", 3)) {
                    pre = false;
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "p", 1)) {
                    // end paragraph
                } else if (0 == strnicmp(tag, "a", 1)) {
                    nodeList.append(new HTMLAnchorEndNode());
                } else if (0 == strnicmp(tag, "ul", 2)) {
                    nodeList.append(new BrNode(DEFAULT_INDENT, 2));
                    newline = true;
                    numRows++;
                } else if (0 == strnicmp(tag, "u", 1)) {
                    uline = false;
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "td", 2)) {
                    nodeList.append(new TdNode((TrNode*)trStack.peek(), false));
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "tr", 2)) {
                    node = new TrNode((TableNode*)tableStack.peek(), false);
                    nodeList.append(node);
                    trStack.pop();
                    newline = true;
                    numRows++;
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "table", 5)) {
                    node = new TableEndNode((TableNode*)tableStack.pop());
                    nodeList.append(node);
                    newline = true;
                    numRows++;
                    text = skipWhite(tagEnd+1);
                }
            } else {
                if (0 == strnicmp(tag, "br", 2)) {
                    nodeList.append(new BrNode(0));
                    newline = true;
                    numRows++;
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
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "hr", 2)) {
                    nodeList.append(new HrNode());
                    newline = true;
                    numRows++;
                } else if (0 == strnicmp(tag, "title", 5)) {
                    tagEnd = strchr(tagEnd+1, '>'); // skip past </title>
                    text = tagEnd+1;
                } else if (0 == strnicmp(tag, "pre", 3)) {
                    pre = true;
                    node = new FontNode(COURIER, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "p", 1)) {
                    // paragraph
                } else if (0 == strnicmp(tag, "td", 2)) {
                    nodeList.append(new TdNode((TrNode*)trStack.peek(), true));
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "tr", 2)) {
                    node = new TrNode((TableNode*)tableStack.peek(), true);
                    nodeList.append(node);
                    trStack.push(node);
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "table", 5)) {
                    node = new TableNode();
                    nodeList.append(node);
                    tableStack.push(node);
                    newline = true;
                    numRows++;
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "ul>", 3)) {
                    nodeList.append(new BrNode(0));
                    newline = true;
                    numRows++;
                } else if (0 == strnicmp(tag, "u>", 2)) {
                    uline = true;
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "li>", 3)) {
                    nodeList.append(new LiNode(style()));
                    newline = true;
                    numRows++;
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
                    p.load(tag+4, taglen-4);
                    nodeList.append(new ImageNode(style(), p));
                    p.removeAll();
                }
            } // end if-start, else-end tag
        } // if found a tag
        tagBegin = *tagEnd == 0 ? 0 : tagEnd+1;
    }
    
    // don't delete nodes
    trStack.emptyList();
    tableStack.emptyList();
}

void HelpWidget::copyText(int begin, int end) {
    String out;
    TextNode* p = 0;
    if (begin > end) {
        int i = end;
        end = begin;
        begin = i;
    }
    begin -= maxTextHeight;

    Object** list = nodeList.getList();
    int len = nodeList.length();

    for (int i=0; i<len; i++) {
        p = (TextNode*)list[i];
        if (p->getY() > end) {
            break;
        } else if (p->getY() >= begin) {
            out.append(p->s, p->textlen);
            out.append(" ", 1);
        }
    }

    if (out.length() > 0) {
        fltk::copy(out.toString(), out.length(), true);
    }
}

void HelpWidget::load(const char *f) {
    FILE *fp;
    long len;
    String buffer;

    const char* target = strrchr(f, '#');
    len = target != null ? target-f : strlen(f);
    buffer.append(f, len);
    buffer.replaceAll('\\', '/');

    const char* localname = buffer.toString();
    htmlStr.empty();
    if (buffer.indexOf(':', 0) != -1) {
        if (strncmp(localname, "file:", 5) == 0) {
            localname += 5; // adjust for local filename
        } else {
            htmlStr.append("Unable to follow the link \"");
            htmlStr.append(localname);
            htmlStr.append("\"");
        }
    }
        
    if ((fp = fopen (localname, "rb")) != NULL) {
        fseek (fp, 0, SEEK_END);
        len = ftell(fp);
        rewind(fp);
        htmlStr.append(fp, len);
        fclose(fp);
    } else {
        htmlStr.append("Unable to follow the link \"");
        htmlStr.append(localname);
        htmlStr.append("\" - ");
        htmlStr.append(strerror(errno));
    }

    reloadPage();
    if (target) {
        navigateTo(target);
    }
}

//--Helper functions------------------------------------------------------------

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

/**
 * skip white space between tags: 
 * <table>-skip-<tr>-skip-<td></td>-skip</tr>-skip-</table>
 */
const char* skipWhite(const char* s) {
    while (isWhite(*s)) {
        s++;
    }
    return s;
}

Color getColor(const char *n) {
    if (!n || !n[0]) {
        return 0;
    }
    
    if (n[0] == '#') {
        // do hex color lookup
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

#ifdef UNIT_TEST
#include <fltk/run.h>
#include <stdarg.h>
#include <windows.h>
int main(int argc, char **argv) {
    int w = 210; // must be > 104
    int h = 200;
    Window window(w, h, "Browse");
    window.begin();
    HelpWidget out("", w, h);
    out.load("t.html#there");
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

//--EndOfFile-------------------------------------------------------------------
