// -*- c-file-style: "java" -*-
// $Id: Fl_Help_Widget.cpp,v 1.8 2005-03-17 22:30:33 zeeb90au Exp $
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
#include <fltk/CheckButton.h>
#include <fltk/RadioButton.h>
#include <fltk/Choice.h>
#include <fltk/Item.h>
#include <fltk/Input.h>

#define FL_HELP_WIDGET_RESOURCES
#include "HelpWidget.h"

// uncomment for unit testing and then run:
// make Fl_Ansi_Window.exe
//#define UNIT_TEST 1

#define FOREGROUND_COLOR fltk::color(32,32,32)
#define BACKGROUND_COLOR fltk::color(230,230,230)
#define ANCHOR_COLOR fltk::color(0,0,128)
#define BUTTON_COLOR fltk::color(220,220,220)
#define DEFAULT_INDENT 2
#define LI_INDENT 18
#define FONT_SIZE 11
#define FONT_SIZE_H1 17
#define SCROLL_W 17
#define CELL_SPACING 4
#define INPUT_SPACING 2
#define TH_SPACING 5
#define INPUT_WIDTH 90
#define SCROLL_SIZE 1000

Color getColor(const char *n);
void lineBreak(const char* s, int slen, int width, int& stlen, int& pxlen);
const char* skipWhite(const char* s);
struct AnchorNode;

//--Display---------------------------------------------------------------------

struct Display {
    S16 x,y;
    U16 height;
    U16 width;
    U16 lineHeight;
    U16 indent;
    U16 fontSize;
    U16 tableLevel;
    U16 nodeId;
    U8 uline;
    U8 center;
    U8 inTR;
    U8 textOut;
    U8 exposed;
    U8 measure;
    Font* font;
    Color color;
    Color background;
    Group* wnd;
    AnchorNode* anchor;

    void newRow(U16 nrows=1) {
        x = indent;
        y += nrows*lineHeight;
    }
};

//--Attributes------------------------------------------------------------------

struct Attributes : public Properties {
    Attributes(int growSize) : Properties(growSize) {}
    String* getValue() {return get("value");}
    String* getName() {return get("name");}
    String* getHref() {return get("href");}
    String* getType() {return get("type");}
    void getValue(String& s) {s.append(getValue());}
    void getName(String& s) {s.append(getName());}
    void getHref(String& s) {s.append(getHref());}
    void getType(String& s) {s.append(getType());}
    int getSize() {return getIntValue("size");}
    int getWidth() {return getIntValue("width");}
    int getHeight() {return getIntValue("height");}
    int getIntValue(const char* attr) {
        String* s = get(attr);
        return (s != null ? s->toInteger() : -1);
    }
};

//--BaseNode--------------------------------------------------------------------

struct BaseNode : public Object {
    virtual void display(Display* out) {}
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
    
    void display(Display* out) {
        if (font) {
            setfont(font, fontSize);
        }
        if (color) {
            setcolor(color);
        }

        int oldLineHeight = out->lineHeight;
        out->lineHeight = (int)(getascent()+getdescent());
        if (out->lineHeight > oldLineHeight) {
            out->y += (out->lineHeight - oldLineHeight);
        }
        out->font = font;
        out->color = color;
        out->fontSize = fontSize;
    }

    Font* font; // includes face,bold,italic
    U16 fontSize;
    Color color;
};

//--BrNode----------------------------------------------------------------------

struct BrNode : public BaseNode {
    BrNode(S16 indent, U16 nrows=1) {
        this->indent = indent;
        this->nrows = nrows;
    }
    
    void display(Display* out) {
        if (indent != 0) {
            out->indent += indent;
        }
        out->newRow(nrows);
        out->lineHeight = (int)(getascent()+getdescent());
    }
    
    S16 indent;
    U16 nrows;
};

//--AnchorNode------------------------------------------------------------------

struct AnchorNode : public BaseNode {
    AnchorNode(Attributes& p) : BaseNode() {
        p.getName(name);
        p.getHref(href);
        wrapxy = 0;
        pushed = 0;
    }

    void display(Display* out) {
        if (pushed) {
            out->uline = true;
        }
        out->anchor = this;
        x1 = out->x;
        y1 = out->y - out->lineHeight;
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
    void display(Display* out) {
        beginNode = out->anchor;
        if (beginNode) {
            beginNode->x2 = out->x;
            beginNode->y2 = out->y;
            beginNode->lineHeight = out->lineHeight;
        }
        out->uline = false;
        out->anchor = 0;
        setcolor(out->color);
    }
    AnchorNode* beginNode;
};

//--StyleNode-------------------------------------------------------------------

struct StyleNode : public BaseNode {
    StyleNode(U8 uline, U8 center) {
        this->uline = uline;
        this->center = center;
    }
    void display(Display* out) {
        out->uline = uline;
        out->center = center;
    }
    U8 uline;   // 2
    U8 center;  // 2
};

//--LiNode----------------------------------------------------------------------

struct LiNode : public BaseNode {
    LiNode(const Style* style) {
        this->style = style;
    }
    void display(Display* out) {
        out->x = out->indent;
        out->y += out->lineHeight;
        int x = out->x-(LI_INDENT-DEFAULT_INDENT);
        int y = out->y-(int)(getascent()-getdescent());
        if (out->measure == false) {
            dotImage.draw(Rectangle(x, y, 5, 5), style, OUTPUT);
        }
    }
    const Style* style;
};

struct UlNode : public BaseNode {
    UlNode() {}
    void display(Display* out) {
        out->newRow(1);
        out->indent += LI_INDENT;
    }
};

struct UlEndNode : public BaseNode {
    UlEndNode() {}
    void display(Display* out) {
        out->indent -= LI_INDENT;
        out->newRow(2);
    }
};

//--TextNode--------------------------------------------------------------------

struct TextNode : public BaseNode {
    TextNode(const char* s, U16 textlen) {
        this->s = s;
        this->textlen = textlen;
        this->width = 0;
        this->ybegin = 0;
    }

    void display(Display* out) {
        ybegin = out->y;
        out->textOut = true;

        if (width == 0) {
            width = (int)getwidth(s, textlen);
        }
        if (width < out->width-out->x) {
            // simple non-wrapping textout
            if (out->measure == false) {
                drawtext(s, textlen, out->x, out->y);
                if (out->uline) {
                    drawline(out->x, out->y+1, out->x+width, out->y+1);
                }
            }
            out->x += width;
        } else {
            int linelen, linepx;
            int len = textlen;
            const char* p = s;
            while (len > 0) {
                lineBreak(p, len, out->width-out->x, linelen, linepx);
                if (linepx == -1) {
                    // no break point - create new line if required
                    linepx = (int)getwidth(p, linelen);
                    if (linepx > out->width-out->x) {
                        out->newRow();
                        // let anchor know where it really starts
                        if (out->anchor && out->anchor->wrapxy == false) {
                            out->anchor->x1 = out->x;
                            out->anchor->y1 = out->y - out->lineHeight;
                            out->anchor->wrapxy = true;
                        }
                    }
                }
                if (out->measure == false) {
                    drawtext(p, linelen, out->x, out->y);
                    if (out->uline) {
                        drawline(out->x, out->y+1, out->x+linepx, out->y+1);
                    }
                }
                p += linelen;
                len -= linelen;

                if (out->anchor) {
                    out->anchor->wrapxy = true;
                }
                if (out->x+linepx < out->width) {
                    out->x += linepx;
                } else {
                    out->newRow();
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

    void display(Display* out) {
        out->x = out->indent;
        out->y += 4;
        if (out->measure == false) {
            setcolor(GRAY45);
            drawline(out->x, out->y+1, out->x+out->width-6, out->y+1);
            setcolor(GRAY99);        
            drawline(out->x, out->y+2, out->x+out->width-6, out->y+2);
            setcolor(out->color);
        }
        out->y += out->lineHeight+2;
    }
};

//--TableNode-------------------------------------------------------------------

struct TableNode : public BaseNode {
    TableNode() : BaseNode() {
        rows = 0;
        cols = 0;
        columns = 0;
        sizes = 0;
        tagWidth = 0;
    }

    ~TableNode() {
        cleanup();
    }

    void display(Display* out) {
        nextCol = 0;
        nextRow = 0;
        width = out->width-out->indent;
        initX = out->indent;
        initY = ryt = ryb = out->y;
        nodeId = out->nodeId;

        if (out->textOut) {
            initY += out->lineHeight;
        }
        if (cols && (out->exposed || columns == 0)) {
            // prepare default column widths
            if (out->tableLevel == 0 && tagWidth == 0) {
                // traverse the table structure to determine widths
                out->measure = true;
            }
            cleanup();
            columns = (U16*)malloc(sizeof(U16)*cols);
            sizes = (S16*)malloc(sizeof(S16)*cols);
            int cellW = width/cols;
            for (int i=0; i<cols; i++) {
                columns[i] = cellW*(i+1);
                sizes[i] = 0;
            }
        }
        out->lineHeight = (int)(getascent()+getdescent());
        out->tableLevel++;
    }

    // called from </td> to prepare for wrapping and resizing
    void doEndTD(Display* out) {
        if (out->y > ryb) {
            ryb = out->y; // wrapped cell
            sizes[nextCol-1] = -1; // veto column changes
        } else if (ryt == out->y &&
                   out->x < columns[nextCol-1] &&
                   out->x > sizes[nextCol-1] &&
                   sizes[nextCol-1] != -1) {
            // largest <td></td> on same line, less than the default width
            // add CELL_SPACING*2 since <td> reduces width by CELL_SPACING
            sizes[nextCol-1] = out->x+CELL_SPACING+CELL_SPACING;
        }
    }

    void doEndTable(Display* out) {
        out->width = width;
        out->indent = initX;
        out->x = initX;
        out->y = ryb;
        if (out->textOut) {
            out->newRow();
        }
        out->textOut = false;
        out->tableLevel--;

        if (cols && columns && out->exposed && tagWidth == 0) {
            // adjust columns for best fit (left align)
            int delta = 0;
            for (int i=0; i<cols-1; i++) {
                if (sizes[i] > 0) {
                    int spacing = columns[i] - sizes[i];
                    columns[i] = sizes[i] - delta;
                    delta += spacing;
                } else {
                    // apply delta only to wrapped column
                    columns[i] -= delta;
                }
            }
            // redraw outer tables
            if (out->tableLevel == 0) {
                out->measure = false;
                out->nodeId = nodeId;
            }
        }
    }

    void cleanup() {
        if (columns) {
            free(columns);
        }
        if (sizes) {
            free(sizes);
        }
    }
    
    U16* columns;
    S16* sizes;
    U16 rows, cols;
    U16 nextCol;
    U16 nextRow;
    U16 width;
    U16 tagWidth; // <table width=100>
    U16 nodeId;
    U16 initX, initY; // start of table
    S16 ryt, ryb; // current row y top+bottom
};

struct TableEndNode : public BaseNode {
    TableEndNode(TableNode* tableNode) : BaseNode() {
        table = tableNode;
    }

    void display(Display* out) {
        if (table) {
            table->doEndTable(out);
        }
    }
    TableNode* table;    
};

//--TrNode----------------------------------------------------------------------

struct TrNode : public BaseNode {
    TrNode(TableNode* tableNode) : BaseNode() {
        this->table = tableNode;
        this->cols = 0;
        if (table) {
            table->rows++;
        }
    }

    void display(Display* out) {
        if (table == 0) {
            return;
        }
        if (out->textOut) {
            // move bottom of <tr> to next line
            table->ryb += out->lineHeight;
        }
        table->nextCol = 0;
        table->nextRow++;
        table->ryt = table->ryb;
        out->inTR = true;
        out->textOut = false;
    }
    TableNode* table;
    U16 cols;
};

struct TrEndNode : public BaseNode {
    TrEndNode(TrNode* tr) : BaseNode() {
        if (tr && tr->table && tr->cols > tr->table->cols) {
            tr->table->cols = tr->cols;
        }
    }

    void display(Display* out) {
        out->inTR = false;
    }
};

//--TdNode----------------------------------------------------------------------

struct TdNode : public BaseNode {
    TdNode(TrNode* trNode) : BaseNode() {
        this->tr = trNode;
        if (tr) {
            tr->cols++;
        }
    }

    void display(Display* out) {
        if (tr == 0 || tr->table == 0) {
            return;
        }
        TableNode* table = tr->table;
        out->x = table->initX + (table->nextCol == 0 ? 0 : 
                                 table->columns[table->nextCol-1]);
        out->y = table->ryt; // top+left of next cell
        out->width = table->columns[table->nextCol]-CELL_SPACING;
        out->indent = out->x;
        table->nextCol++;
    }
    TrNode* tr;
};

struct TdEndNode : public BaseNode {
    TdEndNode(TdNode* tdNode) : BaseNode() {
        this->td = tdNode;
    }

    void display(Display* out) {
        if (td && td->tr && td->tr->table) {
            td->tr->table->doEndTD(out);
        }
    }
    TdNode* td;
};

//--ImageNode-------------------------------------------------------------------

struct ImageNode : public BaseNode {
    ImageNode(const Style* style, Attributes& p) : BaseNode() {
        this->style = style;
        this->image = 0;
    }

    void display(Display* out) {
    }
    const Image* image;
    const Style* style;
};

//--InputNode-------------------------------------------------------------------

struct InputNode : public BaseNode {
    InputNode(Group* parent, Font* font);
    InputNode(Group* parent, Attributes& p, Font* font);

    void display(Display* out) {
        bool select = false;
        switch ((int)button->user_data()) {
        case ID_OPTION:
            select = true;
            break;
        case ID_BUTTON:
            if (button->w() == 0) {
                button->w(10+(int)getwidth(button->label()));
            }
            break;
        }
        if (out->x + button->w() > out->width) {
            out->newRow();
        }
        out->lineHeight = (select?8:4)+(int)(getascent()+getdescent());
        button->hide();
        button->x(out->x);
        button->y(out->y-(int)getascent()-(select?1:0));
        button->h(out->lineHeight-2);
        button->textfont(out->font);
        button->show();
        out->x += button->w() + INPUT_SPACING;
        out->textOut = true;
    }
    Widget* button;
    String value;
};

InputNode::InputNode(Group* parent, Attributes& p, Font* font) : BaseNode() {
    parent->begin();
    p.getValue(value);
    String* type = p.getType();
    if (type != null && type->equals("text")) {
        button = new Input(0, 0, INPUT_WIDTH, 0);
        button->box(NO_BOX);
        if (value.length()) {
            ((Input*)button)->value(value.toString());
        }
        button->user_data((void*)ID_TEXTBOX);
    } else if (type != null && type->equals("checkbox")) {
        button = new CheckButton(0,0,18,0);
        button->user_data((void*)ID_CHKBOX);
    } else if (type != null && type->equals("radio")) {
        button = new RadioButton(0,0,18,0);
        button->user_data((void*)ID_RADIO);
    } else {
        button = new Button(0,0,0,0);
        if (value.length()) {
            button->copy_label(value.toString());
        }
        button->user_data((void*)ID_BUTTON);
        button->labelcolor(ANCHOR_COLOR);
    }
    int size = p.getSize();
    if (size != -1) {
        button->w(size);
    }
    if (button) {
        button->color(BUTTON_COLOR);
        button->textcolor(ANCHOR_COLOR);
        button->textfont(font);
    }
    parent->end();
}

InputNode::InputNode(Group* parent, Font* font) {
    parent->begin();
    button = new Choice(0, 0, INPUT_WIDTH, 0);
    button->color(BUTTON_COLOR);
    button->textcolor(ANCHOR_COLOR);
    button->textfont(font);
    button->user_data((void*)ID_OPTION);
    parent->end();
}

InputNode* createDropList(Group* parent, Font* font, const char*& tagEnd) {
    InputNode* node = new InputNode(parent, font);
    Choice* menu = (Choice*)node->button;
    menu->begin();

    const char* t = tagEnd+1;
    const char* selectEnd = strstr(t, "</select>");
    tagEnd = selectEnd+8;

    while (t && t[0]) {
        const char* option = strstr(t, "<option>");
        if (option == 0 || *(option+8) == 0 || option > selectEnd) {
            break; // no more options
        }
        option += 8; // <option>
        const char* optionEnd = strstr(option, "</option>");
        if (optionEnd == 0 || optionEnd < option) {
            break; // invalid end tag
        }
        t = optionEnd+9; // </option>

        String s;
        s.append(option, optionEnd-option);
        Item* item = new Item();
        item->copy_label(s.toString());
    }

    menu->end();
    return node;
}

//--NamedInput------------------------------------------------------------------

struct NamedInput : public Object {
    NamedInput(InputNode* node, String* name) {
        this->input = node;
        this->name.append(name->toString());
    }
    ~NamedInput() {}
    InputNode* input;
    String name;
};

//--HelpWidget------------------------------------------------------------------

static void scrollbar_callback(Widget* s, void *wnd) {
    ((HelpWidget*)wnd)->navigateTo(((Scrollbar*)s)->value());
}

HelpWidget::HelpWidget(int x, int y, int width, int height) :
    Group(x, y, width, height), 
    nodeList(100), namedInputs(10), inputs(10), anchors(10) {
    user_data((void*)ID_MAIN_WND);
    begin();
    scrollbar = new Scrollbar(width-SCROLL_W, 0, SCROLL_W, height);
    scrollbar->set_vertical();
    scrollbar->value(0, 1, 0, SCROLL_SIZE);
    scrollbar->user_data(this);
    scrollbar->callback(scrollbar_callback);
    scrollbar->show();
}

HelpWidget::~HelpWidget() {
    cleanup();
}

void HelpWidget::init() {
    vscroll = 0;
    scrollHeight = h();
    background = BACKGROUND_COLOR;
}

void HelpWidget::cleanup() {
    int len = inputs.length();
    Object** list = inputs.getList();

    for (int i=0; i<len; i++) {
        InputNode* p = (InputNode*)list[i];
        if (p->button) {
            remove(p->button);
            delete p->button;
            p->button = 0;
        }
    }

    // button/anchor items destroyed in nodeList    
    inputs.emptyList();
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
    for (int i=0; i<len; i++) {
        NamedInput* ni = (NamedInput*)list[i];
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
    for (int i=0; i<len; i++) {
        NamedInput* ni = (NamedInput*)list[i];
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
    for (int i=0; i<len; i++) {
        NamedInput* ni = (NamedInput*)list[i];
        if (ni->input->button == button) {
            return ni->name.toString();
        }
    }
    return null;
}

void HelpWidget::getInputProperties(Properties& p) {
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    Widget* w;

    for (int i=0; i<len; i++) {
        NamedInput* ni = (NamedInput*)list[i];
        switch ((int)ni->input->button->user_data()) {
        case ID_TEXTBOX:
            p.put(ni->name.toString(), (const char*)
                  ((Input*)ni->input->button)->value());
            break;
        case ID_RADIO:
        case ID_CHKBOX:
            p.put(ni->name.toString(), (const char*)
                  ((RadioButton*)ni->input->button)->value() ? 
                  "true" : "false");
            break;
        case ID_OPTION:
            w = ((Choice*)ni->input->button)->item();
            if (w) {
                p.put(ni->name.toString(), (const char*)w->label());
            }
            break;
        }
    }
}

void HelpWidget::navigateTo(const char* anchorName) {
}

void HelpWidget::navigateTo(int scroll) {
    if (vscroll != scroll) {
        vscroll = -(scroll*scrollHeight/SCROLL_SIZE);
        redraw(DAMAGE_ALL);
    }
}

void HelpWidget::layout() {
    scrollbar->resize(w()-SCROLL_W, 0, SCROLL_W, h());
}

void HelpWidget::draw() {
    if (damage() == DAMAGE_CHILD) {
        Group::draw();
        return;
    }

    BaseNode* p = 0;
    Display out;
    out.uline = false;
    out.center = false;
    out.wnd = this;
    out.anchor = 0;
    out.font = HELVETICA;
    out.fontSize = FONT_SIZE;
    out.color = FOREGROUND_COLOR;
    out.background = background;
    out.height = h();
    out.x = DEFAULT_INDENT;
    out.width = w()-(SCROLL_W+DEFAULT_INDENT);
    out.indent = DEFAULT_INDENT;
    out.inTR = false;
    out.textOut = false;
    out.measure = false;
    out.exposed = (damage()&DAMAGE_EXPOSE) ? 1:0;
    out.tableLevel = 0;
    
    // must call setfont() before getascent() etc
    setfont(out.font, out.fontSize);
    out.y = (int)getascent();
    out.lineHeight = out.y+(int)getdescent();
    out.y += vscroll;

    push_clip(Rectangle(w(), h()));
    if (selectedAnchor && (damage()&DAMAGE_PUSHED)) {
        // draw anchor push
        int h = (selectedAnchor->y2-selectedAnchor->y1)+out.lineHeight;
        push_clip(Rectangle(0, selectedAnchor->y1, out.width, h));
    }
    
    setcolor(out.background);
    fillrect(Rectangle(out.x+out.width, out.height));
    setcolor(out.color);

    Object** list = nodeList.getList();
    int len = nodeList.length();
    for (int i=0; i<len; i++) {
        p = (BaseNode*)list[i];
        out.nodeId = i;
        p->display(&out);

        if (out.nodeId < i) {
            // redraw the previous outer table
            TableNode* table = (TableNode*)list[out.nodeId];
            out.x = table->initX;
            out.y = table->initY;
            out.exposed = false;
            for (int j=out.nodeId; j<=i; j++) {
                p = (BaseNode*)list[j];
                out.nodeId = j;
                p->display(&out);
            }
            out.exposed = (damage()&DAMAGE_EXPOSE) ? 1:0;
            out.indent = DEFAULT_INDENT;
        }
        if (out.exposed == false && out.inTR == false &&
            out.y-out.lineHeight > out.height) {
            break;
        }
    }

    if (out.exposed) {
        // size has changed
        int pageHeight = (out.y-vscroll)+out.lineHeight;
        int height = h();
        int scrollH = pageHeight-height;
        if (scrollH < 1) {
            scrollHeight = height;
            scrollbar->deactivate();
            scrollbar->slider_size(10);
            scrollbar->value(0, 1, 0, SCROLL_SIZE);
            vscroll = 0;
        } else {
            int value = SCROLL_SIZE* -vscroll/scrollH;
            int sliderH = height* height/pageHeight;
            scrollHeight = scrollH;
            scrollbar->activate();
            scrollbar->value(value, 1, 0, SCROLL_SIZE);
            scrollbar->pagesize(SCROLL_SIZE* height/scrollH);
            scrollbar->linesize(SCROLL_SIZE* out.lineHeight/scrollH);
            scrollbar->slider_size(max(10, min(sliderH, height-40)));
            if (height-vscroll > pageHeight) {
                vscroll = -(pageHeight-height);
            }
        }
    }
    
    // draw child controls
    draw_child(*scrollbar);

    // prevent other child controls from drawing over the scrollbar
    push_clip(Rectangle(0,0, w()-SCROLL_W, h()));
    Group::draw();
    pop_clip();

    if (selectedAnchor && (damage()&DAMAGE_PUSHED)) {
        pop_clip();
    }
    pop_clip();
}

U8 HelpWidget::find(const char* s, U8 matchCase) {
    Object** list = nodeList.getList();
    int len = nodeList.length();
    for (int i=0; i<len; i++) {
        BaseNode* p = (BaseNode*)list[i];
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

int HelpWidget::onMove(int event) {
    if (selectedAnchor && event == fltk::DRAG) {
        bool pushed = selectedAnchor->ptInSegment(fltk::event_x(), 
                                                  fltk::event_y());
        if (selectedAnchor->pushed != pushed) {
            Widget::cursor(fltk::CURSOR_HAND);
            selectedAnchor->pushed = pushed;
            redraw(DAMAGE_PUSHED);
            return 1;
        }
    } else {
        int len = anchors.length();
        Object** list = anchors.getList();

        for (int i=0; i<len; i++) {
            AnchorNode* p = (AnchorNode*)list[i];
            if (p->ptInSegment(fltk::event_x(), fltk::event_y())) {
                Widget::cursor(fltk::CURSOR_HAND);
                return 1;
            }
        }
        Widget::cursor(fltk::CURSOR_DEFAULT);
    }
    return 0;
}

int HelpWidget::onPush(int event) {
    Object** list = anchors.getList();
    int len = anchors.length();
    selectedAnchor = 0;

    for (int i=0; i<len; i++) {
        AnchorNode* p = (AnchorNode*)list[i];
        if (p->ptInSegment(fltk::event_x(), fltk::event_y())) {
            selectedAnchor = p;
            selectedAnchor->pushed = true;
            Widget::cursor(fltk::CURSOR_HAND);
            redraw(DAMAGE_PUSHED);
            return 1;
        }
    }
    return 0;
}

int HelpWidget::handle(int event) {
    int handled = Group::handle(event);
    if (handled) {
        return handled;
    }
        
    switch (event) {
    case fltk::FOCUS:
        return 2; // aquire focus
        
    case fltk::PUSH:
        return onPush(event);

    case fltk::ENTER:
        return 1;

    case fltk::MOVE:
    case fltk::DRAG:
        return onMove(event);

    case fltk::RELEASE:
        if (selectedAnchor) {
            Widget::cursor(fltk::CURSOR_DEFAULT);
            bool pushed = selectedAnchor->pushed;
            selectedAnchor->pushed = false;
            redraw(DAMAGE_PUSHED);
            if (pushed) {
                anchor.empty();
                anchor.append(selectedAnchor->href.toString());
                do_callback();
            }
            return 1;
        }
    }
    return scrollbar->active() ? scrollbar->handle(event) : 0;
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
    Stack tdStack(10);
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
                } else if (0 == strnicmp(tag, "font", 4) ||
                           0 == strnicmp(tag, "h", 1)) { // </h1>
                    if (0 == strnicmp(tag, "h", 1)) {
                        nodeList.append(new BrNode(0, 2));
                        newline = true;
                        bold--;
                    }
                    font = fltk::HELVETICA;
                    fontSize = FONT_SIZE;
                    color = FOREGROUND_COLOR;
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "pre", 3)) {
                    pre = false;
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "a", 1)) {
                    nodeList.append(new HTMLAnchorEndNode());
                } else if (0 == strnicmp(tag, "ul", 2)) {
                    nodeList.append(new UlEndNode());
                    newline = true;
                } else if (0 == strnicmp(tag, "u", 1)) {
                    uline = false;
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "td", 2)) {
                    nodeList.append(new TdEndNode((TdNode*)tdStack.pop()));
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "tr", 2)) {
                    node = new TrEndNode((TrNode*)trStack.pop());
                    nodeList.append(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "table", 5)) {
                    node = new TableEndNode((TableNode*)tableStack.pop());
                    nodeList.append(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                }
            } else {
                if (0 == strnicmp(tag, "br", 2)) {
                    nodeList.append(new BrNode(0));
                    newline = true;
                } else if (0 == strnicmp(tag, "b>", 2)) {
                    bold = true;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "i>", 2)) {
                    italic = true;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "center", 6)) {
                    center = true;
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "hr", 2)) {
                    nodeList.append(new HrNode());
                    newline = true;
                } else if (0 == strnicmp(tag, "title", 5)) {
                    tagEnd = strchr(tagEnd+1, '>'); // skip past </title>
                    text = tagEnd+1;
                } else if (0 == strnicmp(tag, "pre", 3)) {
                    pre = true;
                    node = new FontNode(COURIER, fontSize, color, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "td", 2)) {
                    node = new TdNode((TrNode*)trStack.peek());
                    nodeList.append(node);
                    tdStack.push(node);
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "tr", 2)) {
                    node = new TrNode((TableNode*)tableStack.peek());
                    nodeList.append(node);
                    trStack.push(node);
                    text = skipWhite(tagEnd+1);
                } else if (0 == strnicmp(tag, "table", 5)) {
                    node = new TableNode();
                    nodeList.append(node);
                    tableStack.push(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                    // continue the font in case we resize
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.append(node);
                } else if (0 == strnicmp(tag, "ul>", 3)) {
                    nodeList.append(new UlNode());
                    newline = true;
                } else if (0 == strnicmp(tag, "u>", 2)) {
                    uline = true;
                    nodeList.append(new StyleNode(uline, center));
                } else if (0 == strnicmp(tag, "li>", 3)) {
                    nodeList.append(new LiNode(style()));
                    newline = true;
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
                } else if (0 == strnicmp(tag, "h1", 2) ||
                           0 == strnicmp(tag, "h2", 2) ||
                           0 == strnicmp(tag, "h3", 2)) {
                    fontSize = FONT_SIZE_H1;
                    node = new FontNode(font, fontSize, color, ++bold, italic);
                    nodeList.append(node);
                    nodeList.append(new BrNode(0));
                    newline = true;
                } else if (0 == strnicmp(tag, "input ", 6)) {
                    p.load(tag+6, taglen-6);
                    InputNode *node = new InputNode(this, p, font);
                    nodeList.append(node);
                    inputs.append(node);
                    prop = p.getName();
                    if (prop != null) {
                        namedInputs.append(new NamedInput(node, prop));
                    }
                    p.removeAll();
                } else if (0 == strnicmp(tag, "select", 6)) {
                    InputNode *node = createDropList(this, font, tagEnd);
                    nodeList.append(node);
                    inputs.append(node);
                    p.load(tag+6, taglen-6);
                    prop = p.getName();
                    if (prop != null) {
                        namedInputs.append(new NamedInput(node, prop));
                    }
                    p.removeAll();
                    text = tagEnd+1;                    
                } else if (0 == strnicmp(tag, "img ", 4)) {
                    p.load(tag+4, taglen-4);
                    nodeList.append(new ImageNode(style(), p));
                    p.removeAll();
                } else if (0 == strnicmp(tag, "body ", 5)) {
                    p.load(tag+5, taglen-5);
                    prop = p.get("bgcolor");
                    if (prop != null) {
                        background = getColor(prop->toString());
                    }
                    p.removeAll();
                }
            } // end if-start, else-end tag
        } // if found a tag
        tagBegin = *tagEnd == 0 ? 0 : tagEnd+1;
    }
    
    // prevent nodes from being auto-deleted
    tdStack.emptyList();
    trStack.emptyList();
    while (tableStack.peek()) {
        node = new TableEndNode((TableNode*)tableStack.pop());
        nodeList.append(node);
    }
}

void HelpWidget::copyText(int begin, int end) {
    String out;
    TextNode* p = 0;
    if (begin > end) {
        int i = end;
        end = begin;
        begin = i;
    }
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
    HelpWidget out(0, 0, w, h);
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
