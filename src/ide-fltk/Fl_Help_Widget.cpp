// -*- c-file-style: "java" -*-
// $Id: Fl_Help_Widget.cpp,v 1.21 2005-04-14 23:26:13 zeeb90au Exp $
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
#include <ctype.h>
#include <io.h>
#include <stdlib.h>

#include <fltk/ask.h>
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
#include <fltk/SharedImage.h>

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
#define FONT_SIZE_H1 22
#define SCROLL_W 15
#define CELL_SPACING 4
#define INPUT_SPACING 2
#define INPUT_WIDTH 90
#define SCROLL_SIZE 1000
#define HSCROLL_STEP 20
#define ELIPSE_LEN 10
#define IMG_TEXT_BORDER 25

extern "C" void trace(const char *format, ...);
Color getColor(String* s, Color def);
void lineBreak(const char* s, int slen, int width, int& stlen, int& pxlen);
const char* skipWhite(const char* s);
Image* loadImage(const char* imgSrc);
struct AnchorNode;
struct InputNode;
static char truestr[] = "true";
static char falsestr[] = "false";
static char spacestr[] = " ";
static char anglestr[] = "<";

//--Display---------------------------------------------------------------------

struct Display {
    S16 x,y;
    U16 height,width;
    U16 tabW, tabH;
    U16 lineHeight;
    U16 indent;
    U16 fontSize;
    U16 tableLevel;
    U16 nodeId;
    S16 imgY;
    U16 imgIndent;
    U8 uline;
    U8 center;
    U8 content;
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
        // flow around images
        if (imgY != -1 && y > imgY) {
            imgY = -1;
            x = indent = imgIndent;
        }
    }
    void endImageFlow() {
        // end text flow around images
        if (imgY != -1) {
            indent = imgIndent;
            x = indent;
            y = imgY+1;
            imgY = -1;
        }
    }
};

//--Attributes------------------------------------------------------------------

struct Value {
    bool relative;
    int value;
};

struct Attributes : public Properties {
    Attributes(int growSize) : Properties(growSize) {}
    String* getValue() {return get("value");}
    String* getName() {return get("name");}
    String* getHref() {return get("href");}
    String* getType() {return get("type");}
    String* getSrc() {return get("src");}
    String* getOnclick() {return get("onclick");}
    String* getBgColor() {return get("bgcolor");}
    String* getFgColor() {return get("fgcolor");}
    String* getBackground() {return get("background");}
    void getValue(String& s) {s.append(getValue());}
    void getName(String& s) {s.append(getName());}
    void getHref(String& s) {s.append(getHref());}
    void getType(String& s) {s.append(getType());}
    Value getWidth(int def=-1) {return getValue("width", def);}
    Value getHeight(int def=-1) {return getValue("height", def);}
    int getSize(int def=-1) {return getIntValue("size", def);}
    int getBorder(int def=-1) {return getIntValue("border", def);}
    int getIntValue(const char* attr, int def);
    Value getValue(const char* attr, int def);
};

int Attributes::getIntValue(const char* attr, int def) {
    String* s = get(attr);
    return (s != null ? s->toInteger() : def);
}

Value Attributes::getValue(const char* attr, int def) {
    Value val;
    val.relative = false;
    String* s = get(attr);
    if (s) {
        int ipc = s->indexOf('%', 0);
        if (ipc != -1) {
            val.relative = true;
        }
        val.value = s->toInteger();
    } else {
        val.value = def;
    }
    return val;
}

//--BaseNode--------------------------------------------------------------------

struct BaseNode : public Object {
    virtual void display(Display* out) {}
    virtual int indexOf(const char* sFind, U8 matchCase) {return -1;}
    virtual int getY() {return -1;}
};

//--FontNode--------------------------------------------------------------------

struct FontNode : public BaseNode {
    FontNode(Font* font, int fontSize, Color color, bool bold, bool italic) {
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
        out->fontSize = fontSize;
    }

    Font* font; // includes face,bold,italic
    U16 fontSize;
    Color color;
};

//--BrNode----------------------------------------------------------------------

struct BrNode : public BaseNode {
    BrNode(U8 premode) {
        this->premode = premode;
    }
    
    void display(Display* out) {
        // when <pre> is active don't flow text around images
        if (premode && out->imgY != -1) {
            out->endImageFlow();
        } else {
            out->newRow(1);
        }
        out->lineHeight = (int)(getascent()+getdescent());
    }
    U8 premode;
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
        x1 = x2 = out->x;
        y1 = y2 = out->y - out->lineHeight;
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

AnchorNode* pushedAnchor = 0;

struct AnchorEndNode : public BaseNode {
    AnchorEndNode() : BaseNode() {}
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

struct UlNode : public BaseNode {
    UlNode(bool ordered) {
        this->ordered = ordered;
    }
    void display(Display* out) {
        nextId = 0;
        out->newRow(1);
        out->indent += LI_INDENT;
    }
    bool ordered;
    int nextId;
};

struct UlEndNode : public BaseNode {
    UlEndNode() {}
    void display(Display* out) {
        out->indent -= LI_INDENT;
        out->newRow(2);
    }
};

struct LiNode : public BaseNode {
    LiNode(const Style* style, UlNode* ulNode) {
        this->style = style;
        this->ulNode = ulNode;
    }
    void display(Display* out) {
        out->content = true;
        out->x = out->indent;
        out->y += out->lineHeight;
        int x = out->x-(LI_INDENT-DEFAULT_INDENT);
        int y = out->y-(int)(getascent()-getdescent());
        if (out->measure == false) {
            if (ulNode && ulNode->ordered) {
                char t[10];
                sprintf(t, "%d.", ++ulNode->nextId);
                drawtext(t, 2, x, out->y);
            } else {
                dotImage.draw(Rectangle(x, y, 5, 5), style, OUTPUT);
            }
        }
    }
    const Style* style;
    UlNode* ulNode;
};

//--ImageNode-------------------------------------------------------------------

struct ImageNode : public BaseNode {
    ImageNode(const Style* style, String* siteHome, Attributes* a) : BaseNode() {
        this->style = style;
        path.append(siteHome);
        path.append(a->getSrc()); // <img src=blah/images/g.gif>
        // load image relative to the loaded files location
        image = loadImage(path.toString());
        image->measure(w.value, h.value);
        w = a->getWidth(w.value);
        h = a->getHeight(h.value);
        background = false;
        fixed = false;
    }

    ImageNode(const Style* style, String* siteHome, String* src, bool fixed)
        : BaseNode() {
        this->style = style;
        this->fixed = fixed;
        path.append(siteHome);
        path.append(src->toString());
        image = loadImage(path.toString());
        image->measure(w.value, h.value);
        w.relative = 0;
        h.relative = 0;
        background = true;
    }

    void reload() {
        int iw,ih;
        image = loadImage(path.toString());
        image->measure(iw, ih);
        if (w.relative == 0) {
            w.value = iw;
        }
        if (h.relative == 0) {
            h.value = ih;
        }
    }

    void display(Display* out) {
        if (image == 0) {
            return;
        }
        int iw = w.relative ? (w.value*(out->width-out->x)/100) : 
            w.value < out->width ? w.value : out->width;
        int ih = h.relative ? (h.value*(out->wnd->h()-out->y)/100) : h.value;
        if (out->measure == false) {
            if (background) {
                // tile image inside rect x,y,tabW,tabH
                int x = out->x-1;
                int y = fixed ? 0 : out->y-(int)getascent();
                int y1 = y;
                int x1 = x;
                int numHorz = out->tabW/w.value;
                int numVert = out->tabH/h.value;
                for (int iy=0; iy<=numVert; iy++) {
                    x1 = x;
                    for (int ix=0; ix<=numHorz; ix++) {
                        if (x1+w.value > x+out->tabW) {
                            iw = out->tabW-(x1-x);
                        } else {
                            iw = w.value;
                        }
                        if (y1+h.value > y+out->tabH) {
                            ih = out->tabH-(y1-y);
                        } else {
                            ih = h.value;
                        }
                        Rectangle rc(x1, y1, iw, ih);
                        image->draw(rc, style, OUTPUT);
                        x1 += w.value;
                    }
                    y1 += h.value;
                }
            } else {
                int x = out->x+DEFAULT_INDENT;
                int y = out->y - (int)getascent();
                if (out->anchor && out->anchor->pushed) {
                    x += 1;
                    y += 1;
                }
                image->draw(Rectangle(x, y, iw, ih), style, OUTPUT);
            }
        }
        if (background == 0) {
            out->content = true;
            if (iw+IMG_TEXT_BORDER > out->width) {
                out->x = out->indent;
                out->y += ih;
                out->imgY = -1;
            } else {
                out->imgY = out->y+ih;
                out->imgIndent = out->indent;
                out->indent = out->x;
                out->x += iw+DEFAULT_INDENT;
            }
        }
    }
    const Image* image;
    const Style* style;
    String path;
    Value w,h;
    U8 background, fixed;
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
        out->content = true;

        if (width == 0) {
            width = (int)getwidth(s, textlen);
        }
        if (width < out->width-out->x) {
            // simple non-wrapping textout
            if (out->center) {
                int xctr = ((out->width-out->x)-width)/2;
                if (xctr > out->x) {
                    out->x = xctr;
                }
            }
            if (out->measure == false) {
                drawtext(s, textlen, out->x, out->y);
                if (out->uline) {
                    drawline(out->x, out->y+1, out->x+width, out->y+1);
                }
            }
            out->x += width;
        } else {
            int linelen, linepx, cliplen;
            int len = textlen;
            const char* p = s;
            while (len > 0) {
                lineBreak(p, len, out->width-out->x, linelen, linepx);
                cliplen = linelen;
                if (linepx > out->width-out->x) {
                    // no break point - create new line if not already on one
                    if (out->x != out->indent) {
                        out->newRow();
                        // anchor now starts on a new line
                        if (out->anchor && out->anchor->wrapxy == false) {
                            out->anchor->x1 = out->x;
                            out->anchor->y1 = out->y - out->lineHeight;
                            out->anchor->wrapxy = true;
                        }
                    }

                    // clip long text - leave room for elipses
                    int cellW = out->width-out->indent-ELIPSE_LEN;
                    if (linepx > cellW) {
                        linepx = 0;
                        cliplen = 0;
                        do {
                            linepx += (int)getwidth(p+cliplen, 1);
                            cliplen++;
                        } while (linepx < cellW);
                    } 
                }
                if (out->measure == false) {
                    drawtext(p, cliplen, out->x, out->y);
                    if (out->uline) {
                        drawline(out->x, out->y+1, out->x+linepx, out->y+1);
                    }
                    if (cliplen != linelen) {
                        drawpoint(out->x+linepx, out->y);
                        drawpoint(out->x+linepx+2, out->y);
                        drawpoint(out->x+linepx+4, out->y);
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
        if (out->imgY != -1) {
            out->endImageFlow();
            out->y -= out->lineHeight;
        }
        out->y += 4;
        out->x = out->indent;
        if (out->measure == false) {
            setcolor(GRAY45);
            drawline(out->x, out->y+1, out->width-6, out->y+1);
            setcolor(GRAY99);
            drawline(out->x, out->y+2, out->width-6, out->y+2);
            setcolor(out->color);
        }
        out->y += out->lineHeight+2;
    }
};

//--TableNode-------------------------------------------------------------------

struct TableNode : public BaseNode {
    TableNode(Attributes* a) : BaseNode() {
        rows = 0;
        cols = 0;
        columns = 0;
        sizes = 0;
        tagWidth = 0;
        border = a->getBorder();
    }

    ~TableNode() {
        cleanup();
    }

    void display(Display* out) {
        nextCol = 0;
        nextRow = 0;
        out->endImageFlow();
        width = out->width-out->indent;
        initX = out->indent;
        initY = ryt = ryb = out->y;
        nodeId = out->nodeId;

        if (out->content) {
            // update table initial row position- we remember content
            // state on re-visiting the table via the value of initY 
            initY += out->lineHeight;
            ryt = ryb = initY;
        }
        out->content = false;

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
        int lineHeight =  (int)(getascent()+getdescent());
        if (lineHeight > out->lineHeight) {
            out->lineHeight = lineHeight;
        }
        out->tableLevel++;
    }

    // called from </td> to prepare for wrapping and resizing
    void doEndTD(Display* out) {
        int index = nextCol-1;
        if (out->y > ryb) {
            ryb = out->y; // wrapped cell
            sizes[index] = -1; // veto column changes
        } else if (ryt == out->y &&
                   out->x < columns[index] &&
                   out->x > sizes[index] &&
                   sizes[index] != -1) {
            // largest <td></td> on same line, less than the default width
            // add CELL_SPACING*2 since <td> reduces width by CELL_SPACING
            sizes[index] = out->x+CELL_SPACING+CELL_SPACING+2;
        }
        // close image flow to prevent bleeding into previous cell
        out->imgY = -1;
    }

    void doEndTable(Display* out) {
        out->width = width;
        out->indent = initX;
        out->x = initX;
        out->y = ryb;
        if (out->content) {
            out->newRow();
        }
        out->content = false;
        out->tableLevel--;
        out->tabH = out->y-initY;
        out->tabW = width;

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
    S16 border;
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
        table = tableNode;
        y1=y2=cols = 0;
        if (table) {
            table->rows++;
        }
    }
    void display(Display* out) {
        if (table == 0) {
            return;
        }
        if (out->content) {
            // move bottom of <tr> to next line
            table->ryb += out->lineHeight;
        }
        table->nextCol = 0;
        table->nextRow++;
        table->ryt = table->ryb;
        y1 = table->ryt-(int)getascent();
        out->content = false;
    }
    TableNode* table;
    U16 cols;
    S16 y1,y2;
};

struct TrEndNode : public BaseNode {
    TrEndNode(TrNode* trNode) : BaseNode() {
        tr = trNode;
        if (tr && tr->table && tr->cols > tr->table->cols) {
            tr->table->cols = tr->cols;
        }
    }
    void display(Display* out) {
        if (tr && tr->table && out->measure) {
            // measure the row height - the table is not revisited
            // during scrolling, so need to save height on first pass
            tr->y2 = tr->table->ryb-tr->y1+(int)getdescent()+1;
        }
    }
    TrNode* tr;
};

//--TdNode----------------------------------------------------------------------

struct TdNode : public BaseNode {
    TdNode(TrNode* trNode, Attributes* a) : BaseNode() {
        tr = trNode;
        if (tr) {
            tr->cols++;
        }
        foreground = getColor(a->getFgColor(), 0);
        background = getColor(a->getBgColor(), 0);
    }

    void display(Display* out) {
        if (tr == 0 || tr->table == 0 || tr->table->cols == 0) {
            return; // invalid table model
        } 
        TableNode* table = tr->table;
        out->x = table->initX + DEFAULT_INDENT +
            (table->nextCol == 0 ? 0 : table->columns[table->nextCol-1]);
        out->y = table->ryt; // top+left of next cell
        out->width = table->columns[table->nextCol]-CELL_SPACING;
        out->indent = out->x; 
        table->nextCol++;
        if (out->measure == false) {
            int x1 = out->indent-CELL_SPACING;
            int x2 = out->width-out->indent+9;
            Rectangle rc(x1, tr->y1, x2, tr->y2);
            if (background) {
                setcolor(background);
                fillrect(rc);
            }
            if (table->border > 0) {
                setcolor(BLACK);
                strokerect(rc);
            }
        }
        setcolor(foreground ? foreground : out->color);
    }
    TrNode* tr;
    Color background, foreground;
};

struct TdEndNode : public BaseNode {
    TdEndNode(TdNode* tdNode) : BaseNode() {
        td = tdNode;
    }
    void display(Display* out) {
        if (td && td->tr && td->tr->table) {
            td->tr->table->doEndTD(out);
        }
        setcolor(out->color);
    }
    TdNode* td;
};

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

//--InputNode-------------------------------------------------------------------

static void onclick_callback(Widget* button, void *buttonId) {
    ((HelpWidget*)button->parent())->onclick(button);
}

static void def_button_callback(Widget* button, void *buttonId) {
    // supply "onclick=fff" to make it do something useful
    fltk::exit_modal(); 
}

struct InputNode : public BaseNode {
    InputNode(Group* parent);
    InputNode(Group* parent, const char* v, int len);
    InputNode(Group* parent, Properties* env, Attributes* a);
    void updateProperties(strlib::List* namedInputs, Attributes* a);

    void display(Display* out) {
        if (button == 0) {
            return;
        }
        
        bool select = false;
        switch ((int)button->user_data()) {
        case ID_SELECT:
            select = true;
            break;
        case ID_BUTTON:
            if (button->w() == 0 && button->label()) {
                button->w(12+(int)getwidth(button->label()));
            }
            break;
        }
        if (out->x != out->indent && button->w() > out->width-out->x) {
            out->newRow();
        }
        out->lineHeight = (select?8:4)+(int)(getascent()+getdescent());
        button->x(out->x);
        button->y(out->y-(int)getascent()-(select?1:0));
        button->h(out->lineHeight-2);
        button->textfont(out->font);
        button->textsize(out->fontSize);
        button->labelsize(out->fontSize);
        if (button->y()+button->h() < out->height && button->y() >= 0) {
            button->show();
        } else {
            // draw a fake control in case partially visible
            setcolor(button->color());
            fillrect(*button);
            setcolor(out->color);            
        }
        out->x += button->w() + INPUT_SPACING;
        out->content = true;
    }
    Widget* button;
    String onclick;
};

InputNode::InputNode(Group* parent, Properties* env, Attributes* a) : 
    // creates either a text, checkbox, radio, hidden or button control
    BaseNode() {
    parent->begin();
    String* value = a->getValue();
    String* name = a->getName();
    if (value == 0 && name != 0 && env) {
        // use external attributes
        value = env->get(name->toString());
    }
    String* type = a->getType();
    if (type != null && type->equals("text")) {
        button = new Input(0, 0, INPUT_WIDTH, 0);
        button->box(NO_BOX);
        if (value && value->length()) {
            ((Input*)button)->value(value->toString());
        }
        button->user_data((void*)ID_TEXTBOX);
    } else if (type != null && type->equals("checkbox")) {
        button = new CheckButton(0,0,18,0);
        button->user_data((void*)ID_CHKBOX);
    } else if (type != null && type->equals("radio")) {
        button = new RadioButton(0,0,18,0);
        button->user_data((void*)ID_RADIO);
    } else if (type != null && type->equals("hidden")) {
        button = 0;
    } else {
        button = new Button(0,0,0,0);
        if (value && value->length()) {
            button->copy_label(value->toString());
        } else {
            button->copy_label(" ");
        }
        button->user_data((void*)ID_BUTTON);
        button->labelcolor(ANCHOR_COLOR);
        button->callback(def_button_callback);
    }
    if (button) {
        int size = a->getSize();
        if (size != -1) {
            button->w(size);
        }
        button->color(BUTTON_COLOR);
        button->textcolor(ANCHOR_COLOR);
    }
    parent->end();
}

InputNode::InputNode(Group* parent, const char* v, int len) : BaseNode() {
    // creates a textarea control
    parent->begin();
    button = new Input(0, 0, INPUT_WIDTH, 0);
    button->box(NO_BOX);
    if (v && len) {
        String value;
        value.append(v, len);
        ((Input*)button)->value(value.toString());
    }
    button->user_data((void*)ID_TEXTBOX);
    button->color(BUTTON_COLOR);
    button->textcolor(ANCHOR_COLOR);
    parent->end();
}

InputNode::InputNode(Group* parent) : BaseNode() {
    // creates a select control
    parent->begin();
    button = new Choice(0, 0, INPUT_WIDTH, 0);
    button->color(BUTTON_COLOR);
    button->textcolor(ANCHOR_COLOR);
    button->user_data((void*)ID_SELECT);
    parent->end();
}

void createDropList(InputNode* node, strlib::List* options) {
    Choice* menu = (Choice*)node->button;
    menu->begin();
    Object** list = options->getList();
    int len = options->length();
    for (int i=0; i<len; i++) {
        String* s = (String*)list[i];
        Item* item = new Item();
        item->copy_label(s->toString());
    }
    menu->end();
}

void InputNode::updateProperties(strlib::List* namedInputs, Attributes* a) {
    String* name = a->getName();
    if (name != null) {
        namedInputs->add(new NamedInput(this, name));
    }
    onclick.append(a->getOnclick());
    if (button && onclick.length()) {
        button->callback(onclick_callback);
    }
}

//--EnvNode---------------------------------------------------------------------

struct EnvNode : public TextNode {
    EnvNode(Properties* p, const char* s, U16 textlen) : TextNode(0,0) {
        String var;
        var.append(s, textlen);
        var.trim();
        if (p) {
            String* s = p->get(var.toString());
            value.append(s);
        }
        if (value.length() == 0) {
            value.append(getenv(var.toString()));
        }
        this->s = value.toString();
        this->textlen = value.length();
    }
    // here to provide value cleanup
    String value;
};

//--HelpWidget------------------------------------------------------------------

static void scrollbar_callback(Widget* scrollBar, void *helpWidget) {
    ((HelpWidget*)helpWidget)->scrollTo(((Scrollbar*)scrollBar)->value());
}

static void anchor_callback(Widget* helpWidget, void *target) {
    ((HelpWidget*)helpWidget)->navigateTo((const char*)target);
}

HelpWidget::HelpWidget(int x, int y, int width, int height) :
    Group(x, y, width, height), 
    nodeList(100), namedInputs(5), inputs(5), anchors(5), images(5) {
    begin();
    scrollbar = new Scrollbar(width-SCROLL_W, 0, SCROLL_W, height);
    scrollbar->set_vertical();
    scrollbar->value(0, 1, 0, SCROLL_SIZE);
    scrollbar->user_data(this);
    scrollbar->callback(scrollbar_callback);
    scrollbar->show();
    end();
    callback(anchor_callback); // default callback
    init();
    cookies = 0;
    siteHome.empty();
}

HelpWidget::~HelpWidget() {
    cleanup();
}

void HelpWidget::init() {
    vscroll = 0;
    hscroll = 0;
    scrollHeight = h();
    background = BACKGROUND_COLOR;
    foreground = FOREGROUND_COLOR;
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
    images.emptyList();
    nodeList.removeAll();
    namedInputs.removeAll();
}

void HelpWidget::reloadPage() {
    cleanup();
    init();
    compose();
    redraw(DAMAGE_ALL | DAMAGE_CONTENTS);
    pushedAnchor = 0;
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

// return the value of the given control
const char* HelpWidget::getInputValue(Widget* widget) {
    if (widget == 0) {
        return null;
    }
    switch ((int)widget->user_data()) {
    case ID_TEXTBOX:
        return ((Input*)widget)->value();
    case ID_RADIO:
    case ID_CHKBOX:
        return ((RadioButton*)widget)->value() ? truestr : falsestr;
    case ID_SELECT:
        widget = ((Choice*)widget)->item();
        return widget ? widget->label() : null;
    }
    return null;
}

// return the nth form value
const char* HelpWidget::getInputValue(int i) {
    int len = namedInputs.length();
    if (i < len) {
        Object** list = namedInputs.getList();
        NamedInput* ni = (NamedInput*)list[i];
        return getInputValue(ni->input->button);
    }
    return 0;
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

// return all of the forms names and values
void HelpWidget::getInputProperties(Properties* p) {
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    for (int i=0; i<len; i++) {
        NamedInput* ni = (NamedInput*)list[i];
        const char* value = getInputValue(ni->input->button);
        if (value && p) {
            p->put(ni->name.toString(), value);
        }
    }
}

// update a widget's display value
bool HelpWidget::setInputValue(const char* assignment) {
    String s = assignment;
    String name = s.lvalue();
    String value = s.rvalue();
    Object** list = namedInputs.getList();
    int len = namedInputs.length();
    Choice* choice;
    Widget* item;

    if (value.length() == 0) {
        return false;
    }

    for (int i=0; i<len; i++) {
        NamedInput* ni = (NamedInput*)list[i];
        if (ni->name.equals(name)) {
            Widget* button = ni->input->button;

            switch ((int)button->user_data()) {
            case ID_TEXTBOX:
                ((Input*)button)->value(value.toString());
                break;
            case ID_RADIO:
            case ID_CHKBOX:
                ((RadioButton*)button)->value(value.equals(truestr) ||
                                              value.equals("1"));
                break;
            case ID_SELECT:
                choice = (Choice*)button;
                item = choice->find(value.toString());
                if (item) {
                    choice->set_focus(item);
                }
                break;
            }
            return true;
        }
    }
    return false;
}

void HelpWidget::scrollTo(const char* anchorName) {
    int len = anchors.length();
    Object** list = anchors.getList();
    for (int i=0; i<len; i++) {
        AnchorNode* p = (AnchorNode*)list[i];
        if (p->name.equals(anchorName)) {
            vscroll = -p->getY();
            redraw(DAMAGE_ALL | DAMAGE_CONTENTS);
            return;
        }
    }
}

void HelpWidget::scrollTo(int scroll) {
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
    out.color = foreground;
    out.background = background;
    out.height = h();
    out.indent = DEFAULT_INDENT+hscroll;
    out.x = out.indent;
    out.width = w()-(DEFAULT_INDENT+SCROLL_W)+hscroll;
    out.content = false;
    out.measure = false;
    out.exposed = (damage()&DAMAGE_EXPOSE) ? 1:0;
    out.tableLevel = 0;
    out.imgY = -1;
    out.imgIndent = out.indent;
    out.tabW = out.width;
    out.tabH = out.height;    

    // must call setfont() before getascent() etc
    setfont(out.font, out.fontSize);
    out.y = (int)getascent();
    out.lineHeight = out.y+(int)getdescent();
    out.y += vscroll;

    push_clip(Rectangle(w(), h()));
    if (pushedAnchor && (damage() == DAMAGE_PUSHED)) {
        // just draw the anchor-push
        int h = (pushedAnchor->y2-pushedAnchor->y1)+pushedAnchor->lineHeight;
        push_clip(Rectangle(0, pushedAnchor->y1, out.width, h));
    }
    
    setcolor(out.background);
    fillrect(Rectangle(0, 0, w()-SCROLL_W, out.height));
    //TODO: show selection range
    //setcolor(color());
    //fillrect(Rectangle(hscroll, 100+vscroll, w()-SCROLL_W, out.lineHeight));
    setcolor(out.color);

    // hide any inputs
    int len = inputs.length();
    Object** list = inputs.getList();
    for (int i=0; i<len; i++) {
        InputNode* p = (InputNode*)list[i];
        if (p->button) {
            p->button->hide();
        }
    }

    list = nodeList.getList();
    len = nodeList.length();
    for (int i=0; i<len; i++) {
        p = (BaseNode*)list[i];
        out.nodeId = i;
        p->display(&out);

        if (out.nodeId < i) {
            // perform second pass on previous outer table
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
        }
        if (out.exposed == false && 
            out.tableLevel == 0 &&
            out.y-out.lineHeight > out.height) {
            // clip remaining content
            break;
        }
    }

    if (out.exposed) {
        // size has changed
        int pageHeight = (out.y-vscroll)+out.lineHeight;
        int height = h();
        int scrollH = pageHeight-height;
        if (scrollH < 1) {
            // nothing to scroll
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

    if (pushedAnchor && (damage() == DAMAGE_PUSHED)) {
        pop_clip();
    }
    pop_clip();
}

int HelpWidget::onMove(int event) {
    if (pushedAnchor && event == fltk::DRAG) {
        bool pushed = pushedAnchor->ptInSegment(fltk::event_x(), 
                                                fltk::event_y());
        if (pushedAnchor->pushed != pushed) {
            Widget::cursor(fltk::CURSOR_HAND);
            pushedAnchor->pushed = pushed;
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
    pushedAnchor = 0;
    for (int i=0; i<len; i++) {
        AnchorNode* p = (AnchorNode*)list[i];
        if (p->ptInSegment(fltk::event_x(), fltk::event_y())) {
            pushedAnchor = p;
            pushedAnchor->pushed = true;
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

    case fltk::KEY:
        if (event_key_state(RightKey) && -hscroll < w()/2) {
            hscroll -= HSCROLL_STEP;
            redraw();
            return 1;
        }
        if (event_key_state(LeftKey) && hscroll < 0) {
            hscroll += HSCROLL_STEP;
            redraw();
            return 1;
        }
        if (event_key_state(RightCtrlKey) || event_key_state(LeftCtrlKey)) {
            switch (event_key()) {
            case 'r': // reload
                reloadPage();
                return 1;
            case 'f': // find
                find(fltk::input("Find:"), false);
                return 1;
            case 'b': // break popup
                fltk::exit_modal();
                break; // handle in default
            }
        }
        break;
        
    case fltk::MOVE:
    case fltk::DRAG:
        return onMove(event);

    case fltk::RELEASE:
        if (pushedAnchor) {
            Widget::cursor(fltk::CURSOR_DEFAULT);
            bool pushed = pushedAnchor->pushed;
            pushedAnchor->pushed = false;
            redraw(DAMAGE_PUSHED);
            if (pushed) {
                this->event.empty();
                this->event.append(pushedAnchor->href.toString());
                user_data((void*)this->event.toString());
                do_callback();
            }
            return 1;
        }
    }
    return scrollbar->active() ? scrollbar->handle(event) : 0;
}

// handle click from form button
void HelpWidget::onclick(Widget* button) {
    int len = inputs.length();
    Object** list = inputs.getList();
    for (int i=0; i<len; i++) {
        InputNode* p = (InputNode*)list[i];
        if (p->button == button) {
            this->event.empty();
            this->event.append(p->onclick.toString());
            user_data((void*)this->event.toString());
            do_callback();
            return;
        }
    }
}

void HelpWidget::compose() {
    U8 pre = false;
    U8 bold = false;
    U8 italic = false;
    U8 center = false;
    U8 uline = false;

    Font *font = fltk::HELVETICA;
    Color color = 0;
    int fontSize = FONT_SIZE;
    int taglen = 0;
    int textlen = 0;
    U8 newline = false;

    strlib::Stack tableStack(10);
    strlib::Stack trStack(10);
    strlib::Stack tdStack(10);
    strlib::Stack olStack(10);
    strlib::List options(10);
    Attributes p(10);
    String* prop;
    BaseNode* node;
    InputNode* inputNode;

    const char* text = htmlStr.toString();
    const char* tagBegin = text;
    const char* tagEnd = text;
    const char* tag;
    const char* tagPair = 0;

#define ADD_PREV_SEGMENT                         \
    prevlen = i-pindex;                          \
    if (prevlen > 0) {                           \
        nodeList.add(new TextNode(p, prevlen));  \
        newline = false;                         \
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
        if (tagBegin > text && tagPair == 0) {
            textlen = tagBegin - text;
            const char* p = text;
            int pindex = 0;
            int prevlen,ispace;
            for (int i=0; i<textlen; i++) {
                switch (text[i]) {
                case '&':
                    // handle entities
                    for (int j=0; j<entityMapLen; j++) {
                        if (0 == strncasecmp(text+i+1, entityMap[j].ent, 
                                             entityMap[j].elen-1)) {
                            ADD_PREV_SEGMENT;
                            // save entity replacement
                            node = new TextNode(&entityMap[j].xlat, 1);
                            nodeList.add(node);
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
                        nodeList.add(new BrNode(pre));
                    }
                    if (newline == false && text[i+1] != '<') {
                        // replace newline with single space
                        nodeList.add(new TextNode(spacestr, 1));
                        newline = false;
                        // skip white space
                        while (i<textlen && (isWhite(text[i+1]))) {
                            i++; // ends on final white-char
                        }
                    }

                    // skip white-char character
                    pindex = i+1;
                    p = text+pindex;
                    break;

                case '-':
                case '~':
                case ':':
                    // break into separate segments to cause line-breaks
                    prevlen = i-pindex+1;
                    if (prevlen > 0) {
                        nodeList.add(new TextNode(p, prevlen));
                        newline = false;
                    }
                    pindex = i+1;
                    p = text+pindex;
                    break;
                    
                case ' ':
                case '\t':
                    if (pre) {
                        continue;
                    }
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
                nodeList.add(new TextNode(p, len));
                newline = false;
            }
        }

        // move to after end tag
        text = *tagEnd == 0 ? 0 : tagEnd+1;

        // process the tag
        newline = false;
        taglen = tagEnd - tagBegin - 1;
        if (taglen > 0) {
            tag = tagBegin+1;
            if (tag[0] == '/') {
                // process the end of tag
                tag++;
                if (0 == strncasecmp(tag, "b", 1)) {
                    bold = false;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "i", 1)) {
                    italic = false;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "center", 6)) {
                    center = false;
                    nodeList.add(new StyleNode(uline, center));
                } else if (0 == strncasecmp(tag, "font", 4) ||
                           0 == strncasecmp(tag, "h", 1)) { // </h1>
                    if (0 == strncasecmp(tag, "h", 1)) {
                        nodeList.add(new BrNode(pre));
                        newline = true;
                        if (bold > 0) {
                            bold--;
                        }
                    }
                    font = fltk::HELVETICA;
                    fontSize = FONT_SIZE;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "pre", 3)) {
                    pre = false;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "a", 1)) {
                    nodeList.add(new AnchorEndNode());
                } else if (0 == strncasecmp(tag, "ul", 2) ||
                           0 == strncasecmp(tag, "ol", 2)) {
                    nodeList.add(new UlEndNode());
                    olStack.pop();
                    newline = true;
                } else if (0 == strncasecmp(tag, "u", 1)) {
                    uline = false;
                    nodeList.add(new StyleNode(uline, center));
                } else if (0 == strncasecmp(tag, "td", 2)) {
                    nodeList.add(new TdEndNode((TdNode*)tdStack.pop()));
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "tr", 2)) {
                    node = new TrEndNode((TrNode*)trStack.pop());
                    nodeList.add(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "table", 5)) {
                    node = new TableEndNode((TableNode*)tableStack.pop());
                    nodeList.add(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "textarea", 8) && tagPair) {
                    inputNode = new InputNode(this, tagPair, tagBegin-tagPair);
                    nodeList.add(inputNode);
                    inputs.add(inputNode);
                    inputNode->updateProperties(&namedInputs, &p);
                    tagPair = 0;
                    p.removeAll();
                } else if (0 == strncasecmp(tag, "select", 6) && tagPair) {
                    inputNode = new InputNode(this);
                    createDropList(inputNode, &options);
                    nodeList.add(inputNode);
                    inputs.add(inputNode);
                    inputNode->updateProperties(&namedInputs, &p);
                    tagPair = 0;
                    p.removeAll();
                } else if (0 == strncasecmp(tag, "option", 6) && tagPair) {
                    String* option = new String();
                    option->append(tagPair, tagBegin-tagPair);
                    options.add(option);
                }
            } else if (isalpha(tag[0]) || tag[0] == '!') {
                // process the start of the tag
                if (0 == strncasecmp(tag, "br", 2) ||
                    0 == strncasecmp(tag, "p>", 2)) {
                    nodeList.add(new BrNode(pre));
                    newline = true;
                } else if (0 == strncasecmp(tag, "b>", 2)) {
                    bold = true;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "i>", 2)) {
                    italic = true;
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "center", 6)) {
                    center = true;
                    nodeList.add(new StyleNode(uline, center));
                } else if (0 == strncasecmp(tag, "hr", 2)) {
                    nodeList.add(new HrNode());
                    newline = true;
                } else if (0 == strncasecmp(tag, "title", 5)) {
                    tagEnd = strchr(tagEnd+1, '>'); // skip past </title>
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "pre", 3)) {
                    pre = true;
                    node = new FontNode(COURIER, fontSize, 0, bold, italic);
                    nodeList.add(node);
                } else if (0 == strncasecmp(tag, "td", 2)) {
                    p.removeAll();
                    p.load(tag+2, taglen-2);
                    node = new TdNode((TrNode*)trStack.peek(), &p);
                    nodeList.add(node);
                    tdStack.push(node);
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "tr", 2)) {
                    node = new TrNode((TableNode*)tableStack.peek());
                    nodeList.add(node);
                    trStack.push(node);
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "table", 5)) {
                    p.removeAll();
                    p.load(tag+5, taglen-5);
                    node = new TableNode(&p);
                    nodeList.add(node);
                    tableStack.push(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                    // continue the font in case we resize
                    node = new FontNode(font, fontSize, 0, bold, italic);
                    nodeList.add(node);
                    prop = p.getBackground();
                    if (prop != null) {
                        node = new ImageNode(style(), &siteHome, prop, false);
                        nodeList.add(node);
                        images.add(node);
                    }
                } else if (0 == strncasecmp(tag, "ul>", 3) ||
                           0 == strncasecmp(tag, "ol>", 3)) {
                    node = new UlNode(tag[0]=='o'||tag[0]=='O');
                    olStack.push(node);
                    nodeList.add(node);
                    newline = true;
                } else if (0 == strncasecmp(tag, "u>", 2)) {
                    uline = true;
                    nodeList.add(new StyleNode(uline, center));
                } else if (0 == strncasecmp(tag, "li>", 3)) {
                    node = new LiNode(style(), (UlNode*)olStack.peek());
                    nodeList.add(node);
                    newline = true;
                    text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "a ", 2)) {
                    p.removeAll();
                    p.load(tag+2, taglen-2);
                    node = new AnchorNode(p);
                    nodeList.add(node);
                    anchors.add(node);
                } else if (0 == strncasecmp(tag, "font ", 5)) {
                    p.removeAll();
                    p.load(tag+5, taglen-5);
                    color = getColor(p.get("color"),0);
                    prop = p.get("size");
                    if (prop != null) {
                        fontSize = prop->toInteger();
                    }
                    prop = p.get("face");
                    if (prop != null) {
                        font = fltk::font(*prop->toString());
                    }
                    node = new FontNode(font, fontSize, color, bold, italic);
                    nodeList.add(node);
                } else if (taglen == 2 && 0 == strncasecmp(tag, "h", 1)) {
                    fontSize = FONT_SIZE_H1-(tag[1]-'1'); // <h1> etc
                    node = new FontNode(font, fontSize, 0, ++bold, italic);
                    nodeList.add(new BrNode(pre));
                    nodeList.add(node);
                    newline = true;
                } else if (0 == strncasecmp(tag, "input ", 6)) {
                    p.removeAll();
                    p.load(tag+6, taglen-6);
                    inputNode = new InputNode(this, cookies, &p);
                    nodeList.add(inputNode);
                    inputs.add(inputNode);
                    inputNode->updateProperties(&namedInputs, &p);
                } else if (0 == strncasecmp(tag, "textarea", 8)) {
                    p.load(tag+8, taglen-8);
                    tagPair = text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "select", 6)) {
                    p.load(tag+6, taglen-6);
                    tagPair = text = skipWhite(tagEnd+1);
                    options.removeAll();
                } else if (0 == strncasecmp(tag, "option", 6)) {
                    tagPair = text = skipWhite(tagEnd+1);
                } else if (0 == strncasecmp(tag, "img ", 4)) {
                    p.removeAll();
                    p.load(tag+4, taglen-4);
                    node = new ImageNode(style(), &siteHome, &p);
                    nodeList.add(node);
                    images.add(node);
                } else if (0 == strncasecmp(tag, "body ", 5)) {
                    p.removeAll();
                    p.load(tag+5, taglen-5);
                    text = skipWhite(tagEnd+1);
                    foreground = getColor(p.getFgColor(), foreground);
                    background = getColor(p.getBgColor(), background);
                    prop = p.getBackground();
                    if (prop != null) {
                        node = new ImageNode(style(), &siteHome, prop, true);
                        nodeList.add(node);
                        images.add(node);
                    }
                } else {
                    // unknown tag
                    text = skipWhite(tagEnd+1);
                }
            } else if (tag[0] == '?') {
                nodeList.add(new EnvNode(cookies, tag+1, taglen-1));
            } else {
                // '<' is a literal character
                nodeList.add(new TextNode(anglestr, 1));
                tagEnd = tagBegin;
                text = tagBegin+1;
            } // end if-start, else-end tag
        } // if found a tag
        tagBegin = *tagEnd == 0 ? tagEnd : tagEnd+1;
    }

    // prevent nodes from being auto-deleted
    olStack.emptyList();
    tdStack.emptyList();
    trStack.emptyList();
    while (tableStack.peek()) {
        node = new TableEndNode((TableNode*)tableStack.pop());
        nodeList.add(node);
    }
}

bool HelpWidget::find(const char* s, bool matchCase) {
    if (s == 0 || s[0] == 0) {
        return false;
    }

    Object** list = nodeList.getList();
    int len = nodeList.length();
    int foundRow = 0;
    int lineHeight = (int)(getascent()+getdescent());
    for (int i=0; i<len; i++) {
        BaseNode* p = (BaseNode*)list[i];
        if (p->indexOf(s, matchCase) != -1) {
            foundRow = p->getY()-vscroll;
            if (foundRow > -vscroll+lineHeight) {
                break;
            }
        }
    }
    if (-vscroll == foundRow) {
        return false;
    }

    vscroll = -foundRow;
    // check scroll bounds
    if (foundRow) {
        vscroll += lineHeight;
    }
    if (-vscroll > scrollHeight) {
        vscroll = -scrollHeight;
    }
    
    redraw(DAMAGE_ALL | DAMAGE_CONTENTS);
    return true;
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

void HelpWidget::navigateTo(const char* s) {
    String path;
    path.append(siteHome);
    path.append(s);
    loadFile(path.toString());
}

void HelpWidget::loadBuffer(const char* str) {
    if (strnicmp("file:", str, 5) == 0) {
        loadFile(str+5);
    } else {
        htmlStr = str;
        reloadPage();
    }
}

void HelpWidget::loadFile(const char *f) {
    FILE *fp;
    long len;

    fileName.empty();
    htmlStr.empty();

    if (strncasecmp(f, "file:///", 8) == 0) {
        // only supports file protocol
        f += 8;
    }
    
    if (strstr(f, "://") != 0) {
        htmlStr.append("Invalid path syntax: ");
        htmlStr.append(f);
        reloadPage();
        return;
    }

    const char* target = strrchr(f, '#');
    len = target != null ? target-f : strlen(f);
    fileName.append(f, len);
    fileName.replaceAll('\\', '/');

    // update siteHome using the given file-name
    if (siteHome.length() == 0) {
        int i = fileName.lastIndexOf('/', 0);
        if (i != -1) {
            siteHome = fileName.substring(0, i+1);
        } else {
            siteHome.append("./");
        }
        if (siteHome[siteHome.length()-1] != '/') {
            siteHome.append("/");
        }
    }
    if ((fp = fopen(fileName.toString(), "rb")) != NULL) {
        fseek (fp, 0, SEEK_END);
        len = ftell(fp);
        rewind(fp);
        htmlStr.append(fp, len);
        fclose(fp);
    } else {
        htmlStr.append("File not found: \"");
        htmlStr.append(fileName.toString());
        htmlStr.append("\" - ");
        htmlStr.append(strerror(errno));
    }

    reloadPage();
    if (target) {
        scrollTo(target);
    }
}

void HelpWidget::getImageNames(strlib::List* nameList) {
    nameList->removeAll();
    Object** list = images.getList();
    int len = images.length();
    for (int i=0; i<len; i++) {
        ImageNode* imageNode = (ImageNode*)list[i];
        nameList->addSet(&imageNode->path);
    }
}

// reload broken images 
void HelpWidget::reloadImages() {
    Object** list = images.getList();
    int len = images.length();
    for (int i=0; i<len; i++) {
        ImageNode* imageNode = (ImageNode*)list[i];
        if (imageNode->image == &brokenImage) {
            imageNode->reload();
        }
    }
    redraw();
}

void HelpWidget::setSiteHome(const char* s) {
    siteHome.empty();
    siteHome.append(s);
}

//--Helper functions------------------------------------------------------------

/**
 * Returns the number of characters that will fit within
 * the gixen pixel width. 
 */
void lineBreak(const char* s, int slen, int width, int& linelen, int& linepx) {
    // find the end of the first word
    int i = 0;
    int txtWidth;
    int ibreak = -1;
    int breakWidth = -1;

    while (i<slen) {
        if (s[i++] == ' ') {
            ibreak = i;
            break;
        }
    }

    // no break point found
    if (ibreak == -1) {
        linelen = slen;
        linepx = (int)getwidth(s, slen);
        return;
    }

    // find the last break-point within the available width
    txtWidth = (int)getwidth(s, i);
    ibreak = i;
    breakWidth = txtWidth;

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
        // entire segment fits
        linelen = slen;
        linepx = txtWidth;
    } else {
        // first break-point is after boundary
        linelen = ibreak;
        linepx = breakWidth;
    }
}

/**
 * skip white space between tags: 
 * <table>-skip-<tr>-skip-<td></td>-skip</tr>-skip-</table>
 */
const char* skipWhite(const char* s) {
    if (s == 0 || s[0] == 0) {
        return 0;
    }
    while (isWhite(*s)) {
        s++;
    }
    return s;
}

Color getColor(String* s, Color def) {
    if (s == 0 || s->length() == 0) {
        return def; 
    }

    const char* n = s->toString();
    if (n[0] == '#') {
        // do hex color lookup
        int rgb = strtol (n + 1, NULL, 16);
        int r = rgb >> 16;
        int g = (rgb >> 8) & 255;
        int b = rgb & 255;
        return fltk::color ((uchar) r, (uchar) g, (uchar) b);
    } else if (strcasecmp(n, "black") == 0) {
        return BLACK;
    } else if (strcasecmp(n, "red") == 0) {
        return RED;
    } else if (strcasecmp(n, "green") == 0) {
        return fltk::color (0, 0x80, 0);
    } else if (strcasecmp(n, "yellow") == 0) {
        return YELLOW;
    } else if (strcasecmp(n, "blue") == 0) {
        return BLUE;
    } else if (strcasecmp(n, "magenta") == 0 || 
               strcasecmp(n, "fuchsia") == 0) {
        return MAGENTA;
    } else if (strcasecmp(n, "cyan") == 0 || 
               strcasecmp(n, "aqua") == 0) {
        return CYAN;
    } else if (strcasecmp(n, "white") == 0) {
        return WHITE;
    } else if (strcasecmp(n, "gray") == 0 || 
               strcasecmp(n, "grey") == 0) {
        return fltk::color (0x80, 0x80, 0x80);
    } else if (strcasecmp(n, "lime") == 0) {
        return GREEN;
    } else if (strcasecmp(n, "maroon") == 0) {
        return fltk::color (0x80, 0, 0);
    } else if (strcasecmp(n, "navy") == 0) {
        return fltk::color (0, 0, 0x80);
    } else if (strcasecmp(n, "olive") == 0) {
        return fltk::color (0x80, 0x80, 0);
    } else if (strcasecmp(n, "purple") == 0) {
        return fltk::color (0x80, 0, 0x80);
    } else if (strcasecmp(n, "silver") == 0) {
        return fltk::color (0xc0, 0xc0, 0xc0);
    } else if (strcasecmp(n, "teal") == 0) {
        return fltk::color (0, 0x80, 0x80);
    }
    return def;
}

// image factory based on file extension
SharedImage* loadImage(const char* name, uchar* buff) {
    int len = strlen(name);
    if (strcasecmp(name+(len-4), ".jpg") == 0 ||
        strcasecmp(name+(len-5), ".jpeg") == 0) {
        return jpegImage::get(name, buff);
    } else if (strcasecmp(name+(len-4), ".gif") == 0) {
        return gifImage::get(name, buff);
    } else if (strcasecmp(name+(len-4), ".png") == 0) {
        return pngImage::get(name, buff);
    } else if (strcasecmp(name+(len-4), ".xpm") == 0) {
        return xpmFileImage::get(name, buff);
    }
    return 0;
}

Image* loadImage(const char* imgSrc) {
    if (imgSrc == 0 || access(imgSrc, 0) != 0) {
        return &brokenImage;
    }
    Image* image = loadImage(imgSrc, 0);
    return image != 0 ? image : &brokenImage;
}

#ifdef UNIT_TEST
#include <fltk/Window.h>
#include <fltk/run.h>
#include <stdarg.h>
#include <windows.h>
int main(int argc, char **argv) {
    int w = 210; // must be > 104
    int h = 400;
    Window window(w, h, "Browse");
    window.begin();
    HelpWidget out(0, 0, w, h);
    out.loadFile("t.html#there");
    //out.loadFile("help/8_5.html");
    window.resizable(&out);
    window.end();
    window.show(argc,argv);
    return run();
}

extern "C" void trace(const char *format, ...) {
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
