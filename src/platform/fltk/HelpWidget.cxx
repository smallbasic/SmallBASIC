//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

// Based on my ebookman HTMLWindow.cpp with some methods borrowed
// from the fltk HelpView
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#include <fltk3/ask.h>
#include <fltk3/run.h>
#include <fltk3/draw.h>
#include <fltk3/Rectangle.h>
//#include <fltk3/XPMImage.h>
#include <fltk3/CheckButton.h>
#include <fltk3/RadioButton.h>
#include <fltk3/Choice.h>
#include <fltk3/Input.h>
#include <fltk3/Output.h>
#include <fltk3/SharedImage.h>
#include <fltk3/Slider.h>
#include <fltk3/ValueInput.h>
#include <fltk3images/all.h>

//#define FL_HELP_WIDGET_RESOURCES
#include "HelpWidget.h"

#define FOREGROUND_COLOR fltk3::default_style.color()
#define BACKGROUND_COLOR fltk3::default_style.color2()
#define ANCHOR_COLOR fltk3::rgb_color(0,0,128)
#define BUTTON_COLOR fltk3::default_style.labelcolor()
#define DEFAULT_INDENT 2
#define LI_INDENT 18
#define FONT_SIZE_H1 23
#define SCROLL_W 15
#define CELL_SPACING 4
#define INPUT_WIDTH 90
#define BUTTON_WIDTH 20
#define SCROLL_SIZE 10000
#define HSCROLL_STEP 20
#define ELIPSE_LEN 10
#define IMG_TEXT_BORDER 25

extern "C" void trace(const char *format, ...);
Color getColor(strlib::String *s, Color def);
void lineBreak(const char *s, int slen, int width, int &stlen, int &pxlen);
const char *skipWhite(const char *s);
bool unquoteTag(const char *tagBegin, const char *&tagEnd);
Image *loadImage(const char *imgSrc);
struct AnchorNode;
struct InputNode;
static char truestr[] = "true";
static char falsestr[] = "false";
static char spacestr[] = " ";
static char anglestr[] = "<";
char rangeValue[20];

//--Display---------------------------------------------------------------------

struct Display {
  S16 x1, x2, y1, y2;
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
  U8 selected;
  U8 invertedSel;
  S16 markX, markY, pointX, pointY;
  strlib::String *selection;
  Font font;
  Color color;
  Color background;
  Group *wnd;
  AnchorNode *anchor;

  void drawBackground(fltk3::Rectangle &rc) {
    if (background != NO_COLOR) {
      Color oldColor = getcolor();
      setcolor(background);
      fillrect(rc);
      setcolor(oldColor);
    }
  } 
  void endImageFlow() {
    // end text flow around images
    if (imgY != -1) {
      indent = imgIndent;
      x1 = indent;
      y1 = imgY + 1;
      imgY = -1;
    }
  }

  void newRow(U16 nrows = 1, bool doBackground = true) {
    int bgY = y1 + (int)getdescent();

    x1 = indent;
    y1 += nrows *lineHeight;
    // flow around images
    if (imgY != -1 && y1 > imgY) {
      imgY = -1;
      x1 = indent = imgIndent;
    }

    if (!measure && background != NO_COLOR && doBackground) {
      fltk3::Rectangle rc(x1, bgY, x2 - x1 + CELL_SPACING, lineHeight);
      drawBackground(rc);
    }
  }

  // restore previous colors
  void restoreColors() {
    setcolor(color);
    background = oldBackground;
  }

  void setColors(Color nodeForeground, Color nodeBackground) {
    setcolor(nodeForeground != NO_COLOR ? nodeForeground : color);

    oldBackground = background;
    if (nodeBackground != NO_COLOR) {
      background = nodeBackground;
    }
  }

private:
  Color oldBackground;
};

//--Attributes------------------------------------------------------------------

struct Value {
  bool relative;
  int value;
};

struct Attributes : public Properties {
  Attributes(int growSize) : Properties(growSize) {
  }
  strlib::String *getValue() {
    return get("value");
  }
  strlib::String *getName() {
    return get("name");
  }
  strlib::String *getHref() {
    return get("href");
  }
  strlib::String *getType() {
    return get("type");
  }
  strlib::String *getSrc() {
    return get("src");
  }
  strlib::String *getOnclick() {
    return get("onclick");
  }
  strlib::String *getBgColor() {
    return get("bgcolor");
  }
  strlib::String *getFgColor() {
    return get("fgcolor");
  }
  strlib::String *getBackground() {
    return get("background");
  }
  strlib::String *getAlign() {
    return get("align");
  }
  bool isReadonly() {
    return get("readonly") != 0;
  }
  void getValue(strlib::String &s) {
    s.append(getValue());
  }
  void getName(strlib::String &s) {
    s.append(getName());
  }
  void getHref(strlib::String &s) {
    s.append(getHref());
  }
  void getType(strlib::String &s) {
    s.append(getType());
  }
  Value getWidth(int def = -1) {
    return getValue("width", def);
  }
  Value getHeight(int def = -1) {
    return getValue("height", def);
  }
  int getSize(int def = -1) {
    return getIntValue("size", def);
  }
  int getBorder(int def = -1) {
    return getIntValue("border", def);
  }
  int getRows(int def = 1) {
    return getIntValue("rows", def);
  }
  int getCols(int def = 20) {
    return getIntValue("cols", def);
  }
  int getMax(int def = 1) {
    return getIntValue("min", def);
  }
  int getMin(int def = 1) {
    return getIntValue("max", def);
  }
  int getColSpan(int def = 1) {
    return getIntValue("colspan", def);
  }
  int getIntValue(const char *attr, int def);
  Value getValue(const char *attr, int def);
};

int Attributes::getIntValue(const char *attr, int def) {
  strlib::String *s = get(attr);
  return (s != null ? s->toInteger() : def);
}

Value Attributes::getValue(const char *attr, int def) {
  Value val;
  val.relative = false;
  strlib::String *s = get(attr);
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

struct BaseNode : public strlib::Object {
  virtual void display(Display *out) {
  } 
  virtual int indexOf(const char *sFind, U8 matchCase) {
    return -1;
  }
  virtual void getText(strlib::String *s) {
  }
  virtual int getY() {
    return -1;
  }
};

//--FontNode--------------------------------------------------------------------

struct FontNode : public BaseNode {
  FontNode(Font font, int fontSize, Color color, bool bold, bool italic);
  void display(Display *out);

  Font font;                   // includes face,bold,italic
  U16 fontSize;
  Color color;
};

FontNode::FontNode(Font font, int fontSize, Color color, bool bold, bool italic) :
  BaseNode(),
  font(font),
  fontSize(fontSize),
  color(color) {
  if (!this->font) {
    this->font = fltk3::COURIER;
  }
  if (bold) {
    this->font += HELVETICA_BOLD;
  }
  if (italic) {
    this->font += HELVETICA_ITALIC;
  }
}

void FontNode::display(Display *out) {
  if (font) {
    setfont(font, fontSize);
  }
  if (color == (Color) - 1) {
    setcolor(out->color);       // </font> restores color
  } else if (color != 0) {
    setcolor(color);
  }
  int oldLineHeight = out->lineHeight;
  out->lineHeight = (int)(getascent() + getdescent());
  if (out->lineHeight > oldLineHeight) {
    out->y1 += (out->lineHeight - oldLineHeight);
  }
  out->font = font;
  out->fontSize = fontSize;
}

//--BrNode----------------------------------------------------------------------

struct BrNode : public BaseNode {
  BrNode(U8 premode) {
    this->premode = premode;
  } 
  void display(Display *out) {
    // when <pre> is active don't flow text around images
    if (premode && out->imgY != -1) {
      out->endImageFlow();
    } else {
      out->newRow(1);
    }
    out->lineHeight = (int)(getascent() + getdescent());
  }
  U8 premode;
};

//--AnchorNode------------------------------------------------------------------

struct AnchorNode : public BaseNode {
  AnchorNode(Attributes &p) : BaseNode() {
    p.getName(name);
    p.getHref(href);
    wrapxy = 0;
    pushed = 0;
  }
  void display(Display *out) {
    if (pushed) {
      out->uline = true;
    }
    out->anchor = this;
    x1 = x2 = out->x1;
    y1 = y2 = out->y1 - out->lineHeight;
    wrapxy = 0;
    if (href.length() > 0) {
      setcolor(ANCHOR_COLOR);
    }
  }

  bool ptInSegment(int x, int y) {
    if (y > y1 && y < y2) {
      // found row
      if ((x < x1 && y < y1 + lineHeight) || 
          (x > x2 && y > y2 - lineHeight)) {
        // outside row start or end
        return false;
      }
      return true;
    }
    return false;
  }

  int getY() {
    return y1;
  }

  strlib::String name;
  strlib::String href;
  S16 x1, x2, y1, y2;
  U16 lineHeight;
  U8 wrapxy;                    // begin on page boundary
  U8 pushed;
};

AnchorNode *pushedAnchor = 0;

struct AnchorEndNode : public BaseNode {
  AnchorEndNode() : BaseNode() {
  }
  void display(Display *out) {
    AnchorNode *beginNode = out->anchor;
    if (beginNode) {
      beginNode->x2 = out->x1;
      beginNode->y2 = out->y1;
      beginNode->lineHeight = out->lineHeight;
    }
    out->uline = false;
    out->anchor = 0;
    setcolor(out->color);
  }
};

//--StyleNode-------------------------------------------------------------------

struct StyleNode : public BaseNode {
  StyleNode(U8 uline, U8 center) {
    this->uline = uline;
    this->center = center;
  } 
  void display(Display *out) {
    out->uline = uline;
    out->center = center;
  }
  U8 uline;                     // 2
  U8 center;                    // 2
};

//--LiNode----------------------------------------------------------------------

struct UlNode : public BaseNode {
  UlNode(bool ordered) {
    this->ordered = ordered;
  } 
  void display(Display *out) {
    nextId = 0;
    out->newRow(1);
    out->indent += LI_INDENT;
  }
  bool ordered;
  int nextId;
};

struct UlEndNode : public BaseNode {
  UlEndNode() {
  } 
  void display(Display *out) {
    out->indent -= LI_INDENT;
    out->newRow(2);
  }
};

struct LiNode : public BaseNode {
  LiNode(UlNode *ulNode) {
    this->ulNode = ulNode;
  } 
  void display(Display *out) {
    out->content = true;
    out->x1 = out->indent;
    out->y1 += out->lineHeight;
    int x = out->x1 - (LI_INDENT - DEFAULT_INDENT);
    int y = out->y1 - (int)(getascent() - getdescent());
    if (out->measure == false) {
      if (ulNode && ulNode->ordered) {
        char t[10];
        sprintf(t, "%d.", ++ulNode->nextId);
        drawtext(t, 2, x, out->y1);
      } else {
        //TODO: fixme dotImage.draw(x, y, 5, 5);
        // draw messes with the current font - restore
        setfont(out->font, out->fontSize);
      }
    }
  }
  UlNode *ulNode;
};

//--ImageNode-------------------------------------------------------------------

struct ImageNode : public BaseNode {
  ImageNode(const Style *style, strlib::String *docHome, Attributes *a);
  ImageNode(const Style *style, strlib::String *docHome, strlib::String *src, bool fixed);
  ImageNode(const Style *style, const Image *image);
  void makePath(strlib::String *src, strlib::String *docHome);
  void reload();
  void display(Display *out);
  const Image *image;
  const Style *style;
  strlib::String path, url;
  Value w, h;
  U8 background, fixed;
  U8 valign;                    // 0=top, 1=center, 2=bottom
};

ImageNode::ImageNode(const Style *style, strlib::String *docHome, Attributes *a) : 
  BaseNode(), style(style) {
  makePath(a->getSrc(), docHome);
  image = loadImage(path.toString());
  w = a->getWidth(image->w());
  h = a->getHeight(image->h());
  background = false;
  fixed = false;
  valign = 0;
}

ImageNode::ImageNode(const Style *style, strlib::String *docHome, strlib::String *src, bool fixed) : 
  BaseNode(), style(style), fixed(fixed) {
  makePath(src, docHome);
  image = loadImage(path.toString());
  w.value = image->w();
  h.value = image->h();
  w.relative = 0;
  h.relative = 0;
  background = true;
  valign = 0;
}

ImageNode::ImageNode(const Style *style, const Image *image) : 
  BaseNode(), image(image), style(style), fixed(true) {
  w.value = image->w();
  h.value = image->h();
  w.relative = 0;
  h.relative = 0;
  background = false;
  valign = 2;
}

void ImageNode::makePath(strlib::String *src, strlib::String *docHome) {
  // <img src=blah/images/g.gif>
  url.append(src);              // html path
  path.append(docHome);         // local file system path
  if (src) {
    if ((*src)[0] == '/') {
      path.append(src->substring(1));
    } else {
      path.append(src);
    }
  }
}

void ImageNode::reload() {
  image = loadImage(path.toString());
  if (w.relative == 0) {
    w.value = image->w();
  }
  if (h.relative == 0) {
    h.value = image->h();
  }
}

void ImageNode::display(Display *out) {
  if (image == 0) {
    return;
  }
  int iw = w.relative ? (w.value *(out->x2 - out->x1) / 100) : w.value < out->x2 ? w.value : out->x2;
  int ih = h.relative ? (h.value *(out->wnd->h() - out->y1) / 100) : h.value;
  if (out->measure == false) {
    if (background) {
      // tile image inside rect x,y,tabW,tabH
      int x = out->x1 - 1;
      int y = fixed ? 0 : out->y1 - (int)getascent();
      int y1 = y;
      int x1 = x;
      int numHorz = out->tabW / w.value;
      int numVert = out->tabH / h.value;
      for (int iy = 0; iy <= numVert; iy++) {
        x1 = x;
        for (int ix = 0; ix <= numHorz; ix++) {
          if (x1 + w.value > x + out->tabW) {
            iw = out->tabW - (x1 - x);
          } else {
            iw = w.value;
          }
          if (y1 + h.value > y + out->tabH) {
            ih = out->tabH - (y1 - y);
          } else {
            ih = h.value;
          }
          //image->draw(x1, y1, iw, ih);
          // TODO: fixme
          x1 += w.value;
        }
        y1 += h.value;
      }
    } else {
      int x = out->x1 + DEFAULT_INDENT;
      int y = out->y1;
      switch (valign) {
      case 0:                  // top
        y -= (int)getascent();
        break;
      case 1:                  // center
        break;
      case 2:                  // bottom
        break;
      }
      if (out->anchor && out->anchor->pushed) {
        x += 1;
        y += 1;
      }
      // image->draw(x, y, iw, ih);
      // TODO: fixme
    }
  }
  if (background == 0) {
    out->content = true;
    if (iw + IMG_TEXT_BORDER > out->x2) {
      out->x1 = out->indent;
      out->y1 += ih;
      out->imgY = -1;
    } else {
      out->imgY = out->y1 + ih;
      out->imgIndent = out->indent;
      out->x1 += iw + DEFAULT_INDENT;
      out->indent = out->x1;
    }
  }
  setfont(out->font, out->fontSize);    // restore font
}

//--TextNode--------------------------------------------------------------------

struct TextNode : public BaseNode {
  TextNode(const char *s, U16 textlen);
  void display(Display *out);
  void drawSelection(const char *s, U16 len, U16 width, Display *out);
  int indexOf(const char *sFind, U8 matchCase);
  void getText(strlib::String *s);

  int getY();

  const char *s;                // 4
  U16 textlen;                  // 4
  U16 width;                    // 4
  S16 ybegin;                   // 4
};

TextNode::TextNode(const char *s, U16 textlen) : BaseNode() {
  this->s = s;
  this->textlen = textlen;
  this->width = 0;
  this->ybegin = 0;
}

void TextNode::getText(strlib::String *s) {
  s->append(this->s, this->textlen);
}

void TextNode::drawSelection(const char *s, U16 len, U16 width, Display *out) {
  int out_y = out->y1 - (int)getascent();
  if (out->pointY < out_y) {
    return;                     // selection above text
  }
  if (out->markY > out_y + out->lineHeight) {
    return;                     // selection below text
  }

  fltk3::Rectangle rc(out->x1, out_y, width, out->lineHeight);
  int selBegin = 0;             // selection index into the draw string
  int selEnd = len;

  if (out->markY > out_y) {
    if (out->pointY < out_y + out->lineHeight) {
      // paint single row selection
      S16 leftX, rightX;
      if (out->markX < out->pointX) {
        leftX = out->markX;
        rightX = out->pointX;
      } else {                  // invert selection
        leftX = out->pointX;
        rightX = out->markX;
      }

      if (leftX > out->x1 + width || rightX < out->x1) {
        return;                 // selection left or right of text
      }

      bool left = true;
      int x = out->x1;
      // find the left+right margins
      if (leftX == rightX) {
        // double click - select word
        for (int i = 0; i < len; i++) {
          int width = (int)getwidth(s + i, 1);
          x += width;
          if (left) {
            if (s[i] == ' ') {
              rc.set_x(x);
              selBegin = i + 1;
            }
            if (x > leftX) {
              left = false;
            }
          } else if (s[i] == ' ' && x > rightX) {
            rc.w(x - rc.x() - width);
            selEnd = i;
            break;
          }
        }
      } else {
        // drag row - draw around character boundary
        for (int i = 0; i < len; i++) {
          x += (int)getwidth(s + i, 1);
          if (left) {
            if (x < leftX) {
              rc.set_x(x);
              selBegin = i + 1;
            } else {
              left = false;
            }
          } else if (x > rightX) {
            rc.w(x - rc.x());
            selEnd = i + 1;
            break;
          }
        }
      }
    } else {
      // top row multiline - find the left margin
      S16 leftX = out->invertedSel ? out->pointX : out->markX;
      if (leftX > out->x1 + width) {
        return;                 // selection left of text
      }
      int x = out->x1;
      for (int i = 0; i < len; i++) {
        x += (int)getwidth(s + i, 1);
        if (x < leftX) {
          rc.set_x(x);
          selBegin = i;
        } else {
          break;
        }
      }
    }
  } else {
    if (out->pointY < out_y + out->lineHeight) {
      // bottom row multiline - find the right margin
      S16 rightX = out->invertedSel ? out->markX : out->pointX;
      if (rightX < out->x1) {
        return;
      }
      int x = out->x1;
      for (int i = 0; i < len; i++) {
        x += (int)getwidth(s + i, 1);
        if (x > rightX) {
          rc.w(x - rc.x());
          selEnd = i + 1;
          break;
        }
      }
    }
    // else middle row multiline - fill left+right
  }

  if (selEnd > selBegin && out->selection != 0) {
    // capture the selected text
    out->selection->append(s + selBegin, selEnd - selBegin);
  }
  setcolor(GRAY80);
  fillrect(rc);
  setcolor(out->color);
}

void TextNode::display(Display *out) {
  ybegin = out->y1;
  out->content = true;

  if (width == 0) {
    width = (int)getwidth(s, textlen);
  }
  if (width < out->x2 - out->x1) {
    // simple non-wrapping textout
    if (out->center) {
      int xctr = ((out->x2 - out->x1) - width) / 2;
      if (xctr > out->x1) {
        out->x1 = xctr;
      }
    }
    if (out->measure == false) {
      if (out->selected) {
        drawSelection(s, textlen, width, out);
      }
      drawtext(s, textlen, out->x1, out->y1);
      if (out->uline) {
        drawline(out->x1, out->y1 + 1, out->x1 + width, out->y1 + 1);
      }
    }
    out->x1 += width;
  } else {
    int linelen, linepx, cliplen;
    int len = textlen;
    const char *p = s;
    while (len > 0) {
      lineBreak(p, len, out->x2 - out->x1, linelen, linepx);
      cliplen = linelen;
      if (linepx > out->x2 - out->x1) {
        // no break point - create new line if not already on one
        if (out->x1 != out->indent) {
          out->newRow();
          // anchor now starts on a new line
          if (out->anchor && out->anchor->wrapxy == false) {
            out->anchor->x1 = out->x1;
            out->anchor->y1 = out->y1 - out->lineHeight;
            out->anchor->wrapxy = true;
          }
        }
        // clip long text - leave room for elipses
        int cellW = out->x2 - out->indent - ELIPSE_LEN;
        if (linepx > cellW) {
          linepx = 0;
          cliplen = 0;
          do {
            linepx += (int)getwidth(p + cliplen, 1);
            cliplen++;
          }
          while (linepx < cellW);
        }
      }
      if (out->measure == false) {
        if (out->selected) {
          drawSelection(p, cliplen, linepx, out);
        }
        drawtext(p, cliplen, out->x1, out->y1);
        if (out->uline) {
          drawline(out->x1, out->y1 + 1, out->x1 + linepx, out->y1 + 1);
        }
        if (cliplen != linelen) {
          drawpoint(out->x1 + linepx, out->y1);
          drawpoint(out->x1 + linepx + 2, out->y1);
          drawpoint(out->x1 + linepx + 4, out->y1);
        }
      }
      p += linelen;
      len -= linelen;

      if (out->anchor) {
        out->anchor->wrapxy = true;
      }
      if (out->x1 + linepx < out->x2) {
        out->x1 += linepx;
      } else {
        out->newRow();
      }
    }
  }
}

int TextNode::indexOf(const char *sFind, U8 matchCase) {
  int numMatch = 0;
  int findLen = strlen(sFind);
  for (int i = 0; i < textlen; i++) {
    U8 equals = matchCase ? s[i] == sFind[numMatch] : toupper(s[i]) == toupper(sFind[numMatch]);
    numMatch = (equals ? numMatch + 1 : 0);
    if (numMatch == findLen) {
      return i + 1;
    }
  }
  return -1;
}

int TextNode::getY() {
  return ybegin;
}

//--HrNode----------------------------------------------------------------------

struct HrNode : public BaseNode {
  HrNode() : BaseNode() {
  }
  void display(Display *out) {
    if (out->imgY != -1) {
      out->endImageFlow();
      out->y1 -= out->lineHeight;
    }
    out->y1 += 4;
    out->x1 = out->indent;
    if (out->measure == false) {
      setcolor(GRAY45);
      drawline(out->x1, out->y1 + 1, out->x2 - 6, out->y1 + 1);
      setcolor(GRAY99);
      drawline(out->x1, out->y1 + 2, out->x2 - 6, out->y1 + 2);
      setcolor(out->color);
    }
    out->y1 += out->lineHeight + 2;
  }
};

//--Table Support---------------------------------------------------------------

struct TableNode;

struct TrNode : public BaseNode {
  TrNode(TableNode *tableNode, Attributes *a);
  void display(Display *out);

  TableNode *table;
  U16 cols;
  S16 y1, height;
  Color background, foreground;
};

struct TrEndNode : public BaseNode {
  TrEndNode(TrNode *trNode);
  void display(Display *out);

  TrNode *tr;
};

struct TdNode : public BaseNode {
  TdNode(TrNode *trNode, Attributes *a);
  void display(Display *out);

  TrNode *tr;
  Color background, foreground;
  Value width;
  U16 colspan;
};

struct TdEndNode : public BaseNode {
  TdEndNode(TdNode *tdNode);
  void display(Display *out);

  TdNode *td;
};

struct TableNode : public BaseNode {
  TableNode(Attributes *a);
  ~TableNode();
  void display(Display *out);
  void doEndTD(Display *out, TrNode *tr, Value *tdWidth);
  void doEndTable(Display *out);
  void setColWidth(Value *width);
  void cleanup();

  U16 *columns;
  S16 *sizes;
  U16 rows, cols;
  U16 nextCol;
  U16 nextRow;
  U16 width;
  U16 nodeId;
  U16 initX, initY;             // start of table
  S16 maxY;                     // end of table
  S16 border;
};

struct TableEndNode : public BaseNode {
  TableEndNode(TableNode *tableNode);
  void display(Display *out);

  TableNode *table;
};

//--TableNode-------------------------------------------------------------------

TableNode::TableNode(Attributes *a) : BaseNode() {
  rows = 0;
  cols = 0;
  columns = 0;
  sizes = 0;
  border = a->getBorder();
}

TableNode::~TableNode() {
  cleanup();
}

void TableNode::display(Display *out) {
  nextCol = 0;
  nextRow = 0;
  out->endImageFlow();
  width = out->x2 - out->indent;
  initX = out->indent;
  initY = maxY = out->y1;
  nodeId = out->nodeId;

  if (out->content) {
    // update table initial row position- we remember content
    // state on re-visiting the table via the value of initY
    initY += out->lineHeight;
    maxY = initY;
  }
  out->content = false;

  if (cols && (out->exposed || columns == 0)) {
    // prepare default column widths
    if (out->tableLevel == 0) {
      // traverse the table structure to determine widths
      out->measure = true;
    }
    cleanup();
    columns = (U16 *) malloc(sizeof(U16) *cols);
    sizes = (S16 *) malloc(sizeof(S16) *cols);
    int cellW = width / cols;
    for (int i = 0; i < cols; i++) {
      columns[i] = cellW * (i + 1);
      sizes[i] = 0;
    }
  }
  int lineHeight = (int)(getascent() + getdescent());
  if (lineHeight > out->lineHeight) {
    out->lineHeight = lineHeight;
  }
  out->tableLevel++;
}

// called from </td> to prepare for wrapping and resizing
void TableNode::doEndTD(Display *out, TrNode *tr, Value *tdWidth) {
  int index = nextCol - 1;
  if (out->y1 > maxY || tdWidth->value != -1) {
    // veto column changes - wrapped or fixed-width cell
    sizes[index] = -1;
  } else if (out->y1 == tr->y1 && out->x1 < columns[index] && out->x1 > sizes[index] && sizes[index] != -1) {
    // largest <td></td> on same line, less than the default width
    // add CELL_SPACING*2 since <td> reduces width by CELL_SPACING
    sizes[index] = out->x1 + (CELL_SPACING * 3);
  }

  if (out->y1 > maxY) {
    maxY = out->y1;             // new max table height
  }
  // close image flow to prevent bleeding into previous cell
  out->imgY = -1;
}

void TableNode::doEndTable(Display *out) {
  out->x2 = width;
  out->indent = initX;
  out->x1 = initX;
  out->y1 = maxY;
  if (out->content) {
    out->newRow(1, false);
  }
  out->content = false;
  out->tableLevel--;
  out->tabH = out->y1 - initY;
  out->tabW = width;

  if (cols && columns && out->exposed) {
    // adjust columns for best fit (left align)
    int delta = 0;
    for (int i = 0; i < cols - 1; i++) {
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

void TableNode::setColWidth(Value *colw) {
  // set user specified column width
  int tdw = colw->relative ? colw->value * width / 100 : colw->value < width ? colw->value : width;
  int delta = columns[nextCol] - tdw;
  columns[nextCol] = tdw;
  for (int i = nextCol + 1; i < cols - 1; i++) {
    columns[i] -= delta;
  }
}

void TableNode::cleanup() {
  if (columns) {
    free(columns);
  }
  if (sizes) {
    free(sizes);
  }
}

TableEndNode::TableEndNode(TableNode *tableNode) : BaseNode() {
  table = tableNode;
}

void TableEndNode::display(Display *out) {
  if (table) {
    table->doEndTable(out);
  }
}

//--TrNode----------------------------------------------------------------------

TrNode::TrNode(TableNode *tableNode, Attributes *a) : BaseNode() {
  table = tableNode;
  y1 = height = cols = 0;
  if (table) {
    table->rows++;
  }
  foreground = getColor(a->getFgColor(), NO_COLOR);
  background = getColor(a->getBgColor(), NO_COLOR);
}

void TrNode::display(Display *out) {
  out->setColors(foreground, background);

  if (table == 0) {
    return;
  }

  if (out->content) {
    // move bottom of <tr> to next line
    table->maxY += out->lineHeight + DEFAULT_INDENT;
  }
  out->content = false;
  y1 = table->maxY;
  table->nextCol = 0;
  table->nextRow++;

  if (background && out->measure == false) {
    fltk3::Rectangle rc(table->initX, y1 - (int)getascent(), table->width, out->lineHeight);
    out->drawBackground(rc);
  }
}

TrEndNode::TrEndNode(TrNode *trNode) : BaseNode() {
  tr = trNode;
  if (tr && tr->table && tr->cols > tr->table->cols) {
    tr->table->cols = tr->cols;
  }
}

void TrEndNode::display(Display *out) {
  out->restoreColors();

  if (tr && tr->table) {
    tr->height = tr->table->maxY - tr->y1 + out->lineHeight;
  }
}

//--TdNode----------------------------------------------------------------------

TdNode::TdNode(TrNode *trNode, Attributes *a) : BaseNode() {
  tr = trNode;
  if (tr) {
    tr->cols++;
  }
  foreground = getColor(a->getFgColor(), NO_COLOR);
  background = getColor(a->getBgColor(), NO_COLOR);
  width = a->getWidth();
  colspan = a->getColSpan(1) - 1;       // count 1 for each additional col
}

void TdNode::display(Display *out) {
  out->setColors(foreground, background);

  if (tr == 0 || tr->table == 0 || tr->table->cols == 0) {
    return;                     // invalid table model
  }

  TableNode *table = tr->table;
  if (out->measure && table->nextRow == 1 && width.value != -1) {
    table->setColWidth(&width);
  }

  out->x1 = table->initX + DEFAULT_INDENT + (table->nextCol == 0 ? 0 : table->columns[table->nextCol - 1]);

  out->y1 = tr->y1;             // top+left of next cell

  // adjust for colspan attribute
  if (colspan) {
    table->nextCol += colspan;
  }
  if (table->nextCol > table->cols - 1) {
    table->nextCol = table->cols - 1;
  }

  out->indent = out->x1;
  out->x2 = out->x1 + table->columns[table->nextCol] - CELL_SPACING;
  if (out->x2 > out->tabW) {
    out->x2 = out->tabW - CELL_SPACING; // stay within table bounds
  }
  table->nextCol++;

  if (out->measure == false) {
    fltk3::Rectangle rc(out->indent - CELL_SPACING,
                        tr->y1 - (int)getascent(), out->x2 - out->indent + (CELL_SPACING * 2), 
                        out->lineHeight);
    out->drawBackground(rc);
    if (table->border > 0) {
      Color oldColor = getcolor();
      setcolor(BLACK);
      fillrect(rc);
      setcolor(oldColor);
    }
  }

}

TdEndNode::TdEndNode(TdNode *tdNode) : BaseNode() {
  td = tdNode;
}

void TdEndNode::display(Display *out) {
  out->restoreColors();

  if (td && td->tr && td->tr->table) {
    td->tr->table->doEndTD(out, td->tr, &td->width);
  }
}

//--NamedInput------------------------------------------------------------------

struct NamedInput : public strlib::Object {
  NamedInput(InputNode *node, strlib::String *name) {
    this->input = node;
    this->name.append(name->toString());
  }
  ~NamedInput() {
  }
  InputNode *input;
  strlib::String name;
};

//--InputNode-------------------------------------------------------------------

static void onclick_callback(Widget *button, void *buttonId) {
  ((HelpWidget *) button->parent())->onclick(button);
}

static void def_button_callback(Widget *button, void *buttonId) {
  // supply "onclick=fff" to make it do something useful
  // check for parent of HelpWidget
  if (fltk3::modal() == button->parent()->parent()) {
    // fltk3::exit_modal();
    // TODO: fixme
  }
}

struct InputNode : public BaseNode {
  InputNode(Group *parent);
  InputNode(Group *parent, Attributes *a, const char *v, int len);
  InputNode(Group *parent, Attributes *a);
  void update(strlib::List *namedInputs, Properties *p, Attributes *a);
  void display(Display *out);

  Widget *button;
  strlib::String onclick;
  U16 rows, cols;
};

// creates either a text, checkbox, radio, hidden or button control
InputNode::InputNode(Group *parent, Attributes *a) : BaseNode() {
  parent->begin();
  strlib::String *type = a->getType();
  if (type != null && type->equals("text")) {
    button = new Input(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_TEXTBOX);
  } else if (type != null && type->equals("readonly")) {
    button = new Widget(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_READONLY);
  } else if (type != null && type->equals("checkbox")) {
    button = new CheckButton(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_CHKBOX);
  } else if (type != null && type->equals("radio")) {
    button = new RadioButton(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_RADIO);
  } else if (type != null && type->equals("slider")) {
    button = new Slider(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_RANGEVAL);
  } else if (type != null && type->equals("valueinput")) {
    button = new ValueInput(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_RANGEVAL);
  } else if (type != null && type->equals("thumbwheel")) {
    //    button = new ThumbWheel(0, 0, BUTTON_WIDTH, 0);
    //    button->argument(ID_RANGEVAL);
    // TODO: fixme
  } else if (type != null && type->equals("hidden")) {
    button = new Widget(0, 0, 0, 0);
    button->argument(ID_HIDDEN);
  } else {
    button = new Button(0, 0, 0, 0);
    button->argument(ID_BUTTON);
    button->callback(def_button_callback);
  }
  parent->end();
}

InputNode::InputNode(Group *parent, Attributes *a, const char *s, int len) : BaseNode() {
  // creates a textarea control
  parent->begin();
  if (a->isReadonly()) {
    strlib::String str;
    str.append(s, len);
    button = new Widget(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_READONLY);
    button->copy_label(str.toString());
  } else {
    button = new Input(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_TEXTAREA);
    ((Input *) button)->value(s, len);
  }
  parent->end();
}

InputNode::InputNode(Group *parent) : BaseNode() {
  // creates a select control
  parent->begin();
  button = new Choice(0, 0, INPUT_WIDTH, 0);
  button->argument(ID_SELECT);
  parent->end();
}

void createDropList(InputNode *node, strlib::List *options) {
  //  Choice *menu = (Choice *) node->button;
  //  menu->begin();
  //strlib::Object **list = options->getList();
  int len = options->length();
  for (int i = 0; i < len; i++) {
    //strlib::String *s = (strlib::String *) list[i];
    //Item *item = new Item();
    //item->copy_label(s->toString());
    // TODO: fixme
  }
  //  menu->end();
}

void InputNode::update(strlib::List *names, Properties *env, Attributes *a) {
  Valuator *valuator;
  Input *input;
  Color color;
  strlib::String *name = a->getName();
  strlib::String *value = a->getValue();
  strlib::String *align = a->getAlign();

  if (name != null) {
    names->add(new NamedInput(this, name));
  }

  if (button == 0) {
    return;
  }
  // value uses environment/external attributes
  if (value == 0 && name != 0 && env) {
    value = env->get(name->toString());
  }

  switch (button->argument()) {
  case ID_READONLY:
    button->align(ALIGN_LEFT | ALIGN_CLIP);
    if (value && value->length()) {
      button->copy_label(value->toString());
    }
    // fallthru
  case ID_TEXTAREA:
    button->box(NO_BOX);
    rows = a->getRows();
    cols = a->getCols();
    if (rows > 1) {
      button->type(fltk3::MULTILINE_INPUT);
    }
    break;
  case ID_RANGEVAL:
    valuator = (Valuator *) button;
    valuator->minimum(a->getMin());
    valuator->maximum(a->getMax());
    valuator->step(1);
    valuator->align(ALIGN_LEFT);
    if (value && value->length()) {
      valuator->value(value->toInteger());
    }
    break;
  case ID_TEXTBOX:
    button->box(NO_BOX);
    input = (Input *) button;
    if (value && value->length()) {
      input->value(value->toString());
    }
    break;
  case ID_BUTTON:
    if (value && value->length()) {
      button->copy_label(value->toString());
    } else {
      button->copy_label(" ");
    }
    break;
  case ID_HIDDEN:
    if (value && value->length()) {
      button->copy_label(value->toString());
    }
    break;
  }

  // size
  int size = a->getSize();
  if (size != -1) {
    button->w(size);
  }
  // set callback
  onclick.append(a->getOnclick());
  if (onclick.length()) {
    button->callback(onclick_callback);
  }
  // set colors
  color = getColor(a->getBgColor(), 0);
  if (color) {
    button->color(color);       // background
  } else {
    button->color(BUTTON_COLOR);
  }
  color = getColor(a->getFgColor(), 0);
  if (color) {
    button->labelcolor(color);  // foreground
    button->textcolor(color);
  } else {
    button->labelcolor(ANCHOR_COLOR);
    button->textcolor(ANCHOR_COLOR);
  }

  // set alignment
  if (align != 0) {
    if (align->equals("right")) {
      button->align(ALIGN_RIGHT | ALIGN_CLIP);
    } else if (align->equals("center")) {
      button->align(ALIGN_CENTER | ALIGN_CLIP);
    } else if (align->equals("top")) {
      button->align(ALIGN_TOP | ALIGN_CLIP);
    } else if (align->equals("bottom")) {
      button->align(ALIGN_BOTTOM | ALIGN_CLIP);
    } else {
      button->align(ALIGN_LEFT | ALIGN_CLIP);
    }
  }
  // set border
  switch (a->getBorder(0)) {
  case 1:
    button->box(BORDER_BOX);
    break;
  case 2:
    button->box(SHADOW_BOX);
    break;
  case 3:
    button->box(ENGRAVED_BOX);
    break;
  case 4:
    button->box(THIN_DOWN_BOX);
    break;
  }
}

void InputNode::display(Display *out) {
  if (button == 0 || ID_HIDDEN == button->argument()) {
    return;
  }

  int height = 4 + (int)(getascent() + getdescent());
  switch (button->argument()) {
  case ID_SELECT:
    height += 4;
    break;
  case ID_BUTTON:
    if (button->w() == 0 && button->label()) {
      button->w(12 + (int)getwidth(button->label()));
    }
    break;
  case ID_TEXTAREA:
  case ID_READONLY:
    button->w(4 + ((int)getwidth("$") * cols));
    height = 4 + ((int)(getascent() + getdescent()) * rows);
    break;
  default:
    break;
  }
  if (out->x1 != out->indent && button->w() > out->x2 - out->x1) {
    out->newRow();
  }
  out->lineHeight = height;
  button->x(out->x1);
  button->y(out->y1 - (int)getascent());
  button->h(out->lineHeight - 2);
  button->labelfont(out->font);
  button->textfont(out->font);
  button->textsize(out->fontSize);
  button->labelsize(out->fontSize);
  if (button->y() + button->h() < out->y2 && button->y() >= 0) {
    // button->clear_flag(INVISIBLE);
    // TODO: fixme
  } else {
    // draw a fake control in case partially visible
    setcolor(button->color());
    rectf(button->x(), button->y(), button->w(), button->h());
    setcolor(out->color);
  }
  out->x1 += button->w();
  out->content = true;
}

//--EnvNode---------------------------------------------------------------------

struct EnvNode : public TextNode {
  EnvNode(Properties *p, const char *s, U16 textlen) : 
    TextNode(0, 0) {
    strlib::String var;
    var.append(s, textlen);
    var.trim();
    if (p) {
      strlib::String *s = p->get(var.toString());
      value.append(s);
    }
    if (value.length() == 0) {
      value.append(::getenv(var.toString()));
    }
    this->s = value.toString();
    this->textlen = value.length();
  }
  // here to provide value cleanup
  strlib::String value;
};

//--HelpWidget------------------------------------------------------------------

static void scrollbar_callback(Widget *scrollBar, void *helpWidget) {
  ((HelpWidget *) helpWidget)->scrollTo(((Scrollbar *) scrollBar)->value());
}

static void anchor_callback(Widget *helpWidget, void *target) {
  ((HelpWidget *) helpWidget)->navigateTo((const char *)target);
}

HelpWidget::HelpWidget(int x, int y, int width, int height, int defsize) :
Group(x, y, width, height),
  nodeList(100), namedInputs(5), inputs(5), anchors(5), images(5) {
  begin();
  scrollbar = new Scrollbar(width - SCROLL_W, 0, SCROLL_W, height);
  // scrollbar->set_vertical();
  // TODO: fixme
  scrollbar->value(0, 1, 0, SCROLL_SIZE);
  scrollbar->user_data(this);
  scrollbar->callback(scrollbar_callback);
  scrollbar->show();
  end();
  callback(anchor_callback);    // default callback
  init();
  cookies = 0;
  docHome.empty();
  labelsize(defsize);
  mouseMode = mm_select;
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
  endSelection();
}

void HelpWidget::endSelection() {
  markX = pointX = -1;
  markY = pointY = -1;
  selection.empty();
}

void HelpWidget::setFontSize(int i) {
  labelsize(i);
  reloadPage();
}

void HelpWidget::cleanup() {
  int len = inputs.length();
  strlib::Object **list = inputs.getList();
  for (int i = 0; i < len; i++) {
    InputNode *p = (InputNode *) list[i];
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
  title.empty();
}

void HelpWidget::reloadPage() {
  cleanup();
  init();
  compile();
  damage(DAMAGE_ALL | DAMAGE_CONTENTS);
  pushedAnchor = 0;
}

// returns the control with the given name
Widget *HelpWidget::getInput(const char *name) {
  strlib::Object **list = namedInputs.getList();
  int len = namedInputs.length();
  for (int i = 0; i < len; i++) {
    NamedInput *ni = (NamedInput *) list[i];
    if (ni->name.equals(name)) {
      return ni->input->button;
    }
  }
  return null;
}

// return the value of the given control
const char *HelpWidget::getInputValue(Widget *widget) {
  if (widget == 0) {
    return null;
  }
  switch (widget->argument()) {
  case ID_TEXTBOX:
  case ID_TEXTAREA:
    return ((Input *) widget)->value();
  case ID_RADIO:
  case ID_CHKBOX:
    return ((RadioButton *) widget)->value()? truestr : falsestr;
  case ID_SELECT:
    //    widget = ((Choice *) widget)->item();
    // TODO: fixme
    return widget ? widget->label() : null;
  case ID_RANGEVAL:
    sprintf(rangeValue, "%f", ((Valuator *) widget)->value());
    return rangeValue;
  case ID_HIDDEN:
  case ID_READONLY:
    return widget->label();
  }
  return null;
}

// return the nth form value
const char *HelpWidget::getInputValue(int i) {
  int len = namedInputs.length();
  if (i < len) {
    strlib::Object **list = namedInputs.getList();
    NamedInput *ni = (NamedInput *) list[i];
    return getInputValue(ni->input->button);
  }
  return 0;
}

// return the name of the given button control
const char *HelpWidget::getInputName(Widget *button) {
  strlib::Object **list = namedInputs.getList();
  int len = namedInputs.length();
  for (int i = 0; i < len; i++) {
    NamedInput *ni = (NamedInput *) list[i];
    if (ni->input->button == button) {
      return ni->name.toString();
    }
  }
  return null;
}

// return all of the forms names and values - except hidden ones
void HelpWidget::getInputProperties(Properties *p) {
  if (p == 0) {
    return;
  }
  strlib::Object **list = namedInputs.getList();
  int len = namedInputs.length();
  for (int i = 0; i < len; i++) {
    NamedInput *ni = (NamedInput *) list[i];
    const char *value = getInputValue(ni->input->button);
    if (value) {
      p->put(ni->name.toString(), value);
    }
  }
}

// update a widget's display value using the given string based
// assignment statement, eg val=1000
bool HelpWidget::setInputValue(const char *assignment) {
  strlib::String s = assignment;
  strlib::String name = s.lvalue();
  strlib::String value = s.rvalue();
  strlib::Object **list = namedInputs.getList();
  int len = namedInputs.length();
  Choice *choice;
  Widget *item;

  if (value.length() == 0) {
    return false;
  }

  for (int i = 0; i < len; i++) {
    NamedInput *ni = (NamedInput *) list[i];
    if (ni->name.equals(name)) {
      Widget *button = ni->input->button;

      switch (button->argument()) {
      case ID_TEXTBOX:
      case ID_TEXTAREA:
        ((Input *) button)->value(value.toString());
        break;
      case ID_RADIO:
      case ID_CHKBOX:
        ((RadioButton *) button)->value(value.equals(truestr) || value.equals("1"));
        break;
      case ID_SELECT:
        choice = (Choice *) button;
        //item = choice->find(value.toString());
        // TODO: fixme
        if (item) {
          //choice->set_focus(item);
        }
        break;
      case ID_RANGEVAL:
        ((Valuator *) button)->value(value.toNumber());
        break;
      case ID_READONLY:
        button->copy_label(value.toString());
        break;
      }
      return true;
    }
  }
  return false;
}

void HelpWidget::scrollTo(const char *anchorName) {
  int len = anchors.length();
  strlib::Object **list = anchors.getList();
  for (int i = 0; i < len; i++) {
    AnchorNode *p = (AnchorNode *) list[i];
    if (p->name.equals(anchorName)) {
      if (p->getY() > scrollHeight) {
        vscroll = -scrollHeight;
      } else {
        vscroll = -p->getY();
      }
      damage(DAMAGE_ALL | DAMAGE_CONTENTS);
      return;
    }
  }
}

void HelpWidget::scrollTo(int scroll) {
  // called from the scrollbar using scrollbar units
  if (vscroll != scroll) {
    vscroll = -(scroll * scrollHeight / SCROLL_SIZE);
    damage(DAMAGE_ALL);
  }
}

void HelpWidget::layout() {
  scrollbar->resize(w() - SCROLL_W, 0, SCROLL_W, h());
  endSelection();
}

void HelpWidget::draw() {
  if (damage() == DAMAGE_CHILD) {
    Group::draw();
    return;
  }

  BaseNode *p = 0;
  Display out;
  out.uline = false;
  out.center = false;
  out.wnd = this;
  out.anchor = 0;
  out.font = HELVETICA;
  out.fontSize = (int)labelsize();
  out.color = foreground;
  out.background = background;
  out.y2 = h();
  out.indent = DEFAULT_INDENT + hscroll;
  out.x1 = out.indent;
  out.x2 = w() - (DEFAULT_INDENT + SCROLL_W) + hscroll;
  out.content = false;
  out.measure = false;
  out.exposed = (damage() & DAMAGE_EXPOSE) ? 1 : 0;
  out.tableLevel = 0;
  out.imgY = -1;
  out.imgIndent = out.indent;
  out.tabW = out.x2;
  out.tabH = out.y2;
  out.selection = 0;
  out.selected = (markX != pointX || markY != pointY);
  if (event_clicks() == 1 && damage() == DAMAGE_HIGHLIGHT) {
    // double click
    out.selected = true;
  }
  if (out.selected) {
    out.markX = markX;
    out.pointX = pointX;
    if (markY < pointY) {
      out.markY = markY;
      out.pointY = pointY;
      out.invertedSel = false;
    } else {
      out.markY = pointY;
      out.pointY = markY;
      out.invertedSel = true;
    }
    out.markX += hscroll;
    out.markY += vscroll;
    out.pointX += hscroll;
    out.pointY += vscroll;

    if (damage() == DAMAGE_HIGHLIGHT) {
      // capture new selection text
      out.selection = &selection;
      out.selection->empty();
    }
  }
  // must call setfont() before getascent() etc
  setfont(out.font, out.fontSize);
  out.y1 = (int)getascent();
  out.lineHeight = out.y1 + (int)getdescent();
  out.y1 += vscroll;

  push_clip(0, 0, w(), h());
  bool havePushedAnchor = false;
  if (pushedAnchor && (damage() == DAMAGE_PUSHED)) {
    // just draw the anchor-push
    int h = (pushedAnchor->y2 - pushedAnchor->y1) + pushedAnchor->lineHeight;
    push_clip(0, pushedAnchor->y1, out.x2, h);
    havePushedAnchor = true;
  }
  // draw the background
  setcolor(out.background);
  fillrect(fltk3::Rectangle(0, 0, w() - SCROLL_W, out.y2));
  setcolor(out.color);

  out.background = NO_COLOR;

  // hide any inputs
  int len = inputs.length();
  strlib::Object **list = inputs.getList();
  for (int i = 0; i < len; i++) {
    InputNode *p = (InputNode *) list[i];
    if (p->button) {
      p->button->set_flag(INVISIBLE);
    }
  }

  list = nodeList.getList();
  len = nodeList.length();
  for (int i = 0; i < len; i++) {
    p = (BaseNode *) list[i];
    out.nodeId = i;
    p->display(&out);

    if (out.nodeId < i) {
      // perform second pass on previous outer table
      TableNode *table = (TableNode *) list[out.nodeId];
      out.x1 = table->initX;
      out.y1 = table->initY;
      out.exposed = false;
      for (int j = out.nodeId; j <= i; j++) {
        p = (BaseNode *) list[j];
        out.nodeId = j;
        p->display(&out);
      }
      out.exposed = (damage() & DAMAGE_EXPOSE) ? 1 : 0;
    }
    if (out.exposed == false && out.tableLevel == 0 && out.y1 - out.lineHeight > out.y2) {
      // clip remaining content
      break;
    }
  }

  if (out.exposed) {
    // size has changed or need to recombob scrollbar
    int pageHeight = (out.y1 - vscroll) + out.lineHeight;
    int height = h() - out.lineHeight;
    int scrollH = pageHeight - height;
    if (scrollH < 1) {
      // nothing to scroll
      scrollHeight = height;
      scrollbar->deactivate();
      scrollbar->slider_size(10);
      scrollbar->value(0, 1, 0, SCROLL_SIZE);
      vscroll = 0;
    } else {
      int value = SCROLL_SIZE * -vscroll / scrollH;
      int sliderH = height * height / pageHeight;
      scrollHeight = scrollH;
      scrollbar->activate();
      scrollbar->value(value, 1, 0, SCROLL_SIZE);
      //scrollbar->pagesize(SCROLL_SIZE * height / scrollH);
      // TODO: fixme
      scrollbar->linesize(SCROLL_SIZE * out.lineHeight / scrollH);
      scrollbar->slider_size(max(10, min(sliderH, height - 40)));
      if (height - vscroll > pageHeight) {
        vscroll = -(pageHeight - height);
      }
    }
  }
  // draw child controls
  draw_child(*scrollbar);

  // prevent other child controls from drawing over the scrollbar
  push_clip(0, 0, w() - SCROLL_W, h());
  int numchildren = children();
  for (int n = 0; n < numchildren; n++) {
    Widget & w = *child(n);
    if (&w != scrollbar) {
      draw_child(w);
    }
  }
  pop_clip();

  if (havePushedAnchor) {
    pop_clip();
  }
  pop_clip();
}

void HelpWidget::compile() {
  U8 pre = !isHtmlFile();
  U8 bold = false;
  U8 italic = false;
  U8 center = false;
  U8 uline = false;

  Color color = 0;
  Font font = fltk3::HELVETICA;
  int fontSize = (int)labelsize();
  int taglen = 0;
  int textlen = 0;
  U8 padlines = false;          // padding between line-breaks

  strlib::Stack tableStack(5);
  strlib::Stack trStack(5);
  strlib::Stack tdStack(5);
  strlib::Stack olStack(5);
  strlib::List options(5);
  Attributes p(5);
  strlib::String *prop;
  BaseNode *node;
  InputNode *inputNode;

  const char *text = htmlStr.toString();
  const char *tagBegin = text;
  const char *tagEnd = text;
  const char *tag;
  const char *tagPair = 0;

#define ADD_PREV_SEGMENT                        \
  prevlen = i-pindex;                           \
  if (prevlen > 0) {                            \
    nodeList.add(new TextNode(p, prevlen));     \
    padlines = true;                            \
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
        break;                  // no tag closure
      }
      text = tagEnd + 1;        // adjoining tags
    }
    // process open text leading to the found tag
    if (tagBegin > text && tagPair == 0) {
      textlen = tagBegin - text;
      const char *p = text;
      int pindex = 0;
      int prevlen, ispace;
      for (int i = 0; i < textlen; i++) {
        switch (text[i]) {
        case '&':
          // handle entities
          if (text[i + 1] == '#') {
            ADD_PREV_SEGMENT;
            int ch = 0;
            i += 2;
            while (isdigit(text[i])) {
              ch = (ch * 10) + (text[i++] - '0');
            }
            if (text[i] == ';') {
              i++;
            }
            if (ch == 133) {
              // TODO: fixme              node = new ImageNode(style(), &ellipseImage);
              nodeList.add(node);
            } else if (ch > 129 && ch < 256) {
              // TODO fixme node = new TextNode(&entityMap[ch].xlat, 1);
              nodeList.add(node);
            }
            pindex = i;
            p = text + pindex;
          } else
            /*
            for (int j = 0; j < entityMapLen; j++) {
              if (0 == strncasecmp(text + i + 1, entityMap[j].ent, entityMap[j].elen - 1)) {
                ADD_PREV_SEGMENT;
                // save entity replacement
                node = new TextNode(&entityMap[j].xlat, 1);
                nodeList.add(node);
                // skip past entity
                i += entityMap[j].elen;
                pindex = i;
                p = text + pindex;
                i--;
                // stop searching
                break;
              }
            }
            */
          break;

        case '\r':
        case '\n':
          ADD_PREV_SEGMENT;
          if ((prevlen && text[i - 1] == ' ')) {
            padlines = false;
          }
          if (pre) {
            nodeList.add(new BrNode(pre));
          } else if (padlines == true) {
            nodeList.add(new TextNode(spacestr, 1));
            padlines = false;   // don't add consequtive spacestrs
          }
          // skip white space
          while (i < textlen && (isWhite(text[i + 1]))) {
            i++;                // ends on final white-char
          }

          // skip white-char character
          pindex = i + 1;
          p = text + pindex;
          break;

        case '-':
        case '~':
        case ':':
          // break into separate segments to cause line-breaks
          prevlen = i - pindex + 1;
          if (prevlen > 0) {
            nodeList.add(new TextNode(p, prevlen));
            padlines = true;
          }
          pindex = i + 1;
          p = text + pindex;
          break;

        case ' ':
        case '\t':
          if (pre) {
            continue;
          }
          // skip multiple whitespaces
          ispace = i;
          while (text[ispace + 1] == ' ' || text[ispace + 1] == '\t') {
            ispace++;
            if (ispace == textlen) {
              break;
            }
          }
          if (ispace > i) {
            ADD_PREV_SEGMENT;
            pindex = i = ispace;
            p = text + pindex;
          }
          break;
        }
      }                         // end for (int i=0; i<textlen...

      int len = textlen - pindex;
      if (len) {
        nodeList.add(new TextNode(p, len));
        padlines = true;
      }
    }
    // move to after end tag
    text = *tagEnd == 0 ? 0 : tagEnd + 1;

    // process the tag
    taglen = tagEnd - tagBegin - 1;
    if (taglen > 0) {
      tag = tagBegin + 1;
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
                   0 == strncasecmp(tag, "code", 4) || 
                   0 == strncasecmp(tag, "h", 1)) {     // </h1>
          if (0 == strncasecmp(tag, "h", 1)) {
            if (bold > 0) {
              bold--;
            }
            nodeList.add(new BrNode(pre));
            padlines = false;
          }
          color = (0 == strncasecmp(tag, "f", 1) ? (Color) - 1 : 0);
          font = fltk3::HELVETICA;
          node = new FontNode(font, fontSize, color, bold, italic);
          nodeList.add(node);
        } else if (0 == strncasecmp(tag, "pre", 3)) {
          pre = false;
          node = new FontNode(font, fontSize, 0, bold, italic);
          nodeList.add(node);
          nodeList.add(new BrNode(pre));
        } else if (0 == strncasecmp(tag, "a", 1)) {
          nodeList.add(new AnchorEndNode());
        } else if (0 == strncasecmp(tag, "ul", 2) || 0 == strncasecmp(tag, "ol", 2)) {
          nodeList.add(new UlEndNode());
          olStack.pop();
          padlines = false;
        } else if (0 == strncasecmp(tag, "u", 1)) {
          uline = false;
          nodeList.add(new StyleNode(uline, center));
        } else if (0 == strncasecmp(tag, "td", 2)) {
          nodeList.add(new TdEndNode((TdNode *) tdStack.pop()));
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "tr", 2)) {
          node = new TrEndNode((TrNode *) trStack.pop());
          nodeList.add(node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "table", 5)) {
          node = new TableEndNode((TableNode *) tableStack.pop());
          nodeList.add(node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "textarea", 8) && tagPair) {
          inputNode = new InputNode(this, &p, tagPair, tagBegin - tagPair);
          nodeList.add(inputNode);
          inputs.add(inputNode);
          inputNode->update(&namedInputs, cookies, &p);
          tagPair = 0;
          p.removeAll();
        } else if (0 == strncasecmp(tag, "select", 6) && tagPair) {
          inputNode = new InputNode(this);
          createDropList(inputNode, &options);
          nodeList.add(inputNode);
          inputs.add(inputNode);
          inputNode->update(&namedInputs, cookies, &p);
          tagPair = 0;
          p.removeAll();
        } else if (0 == strncasecmp(tag, "option", 6) && tagPair) {
          strlib::String *option = new strlib::String();
          option->append(tagPair, tagBegin - tagPair);
          options.add(option);  // continue scan for more options
        } else if (0 == strncasecmp(tag, "title", 5) && tagPair) {
          title.empty();
          title.append(tagPair, tagBegin - tagPair);
          tagPair = 0;
        } else if (0 == strncasecmp(tag, "script", 6) || 
                   0 == strncasecmp(tag, "style", 5)) {
          tagPair = 0;
        }
      } else if (isalpha(tag[0]) || tag[0] == '!') {
        // process the start of the tag
        if (0 == strncasecmp(tag, "br", 2) || 
            0 == strncasecmp(tag, "p>", 2)) {
          nodeList.add(new BrNode(pre));
          padlines = false;
          text = skipWhite(tagEnd + 1);
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
          padlines = false;
        } else if (0 == strncasecmp(tag, "title", 5)) {
          tagPair = text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "pre", 3)) {
          pre = true;
          node = new FontNode(NULL, fontSize, 0, bold, italic);
          nodeList.add(node);
          nodeList.add(new BrNode(pre));
        } else if (0 == strncasecmp(tag, "code", 4)) {
          node = new FontNode(NULL, fontSize, 0, bold, italic);
          nodeList.add(node);
        } else if (0 == strncasecmp(tag, "td", 2)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new TdNode((TrNode *) trStack.peek(), &p);
          nodeList.add(node);
          tdStack.push(node);
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "tr", 2)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new TrNode((TableNode *) tableStack.peek(), &p);
          nodeList.add(node);
          trStack.push(node);
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "table", 5)) {
          p.removeAll();
          p.load(tag + 5, taglen - 5);
          node = new TableNode(&p);
          nodeList.add(node);
          tableStack.push(node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
          // continue the font in case we resize
          node = new FontNode(font, fontSize, 0, bold, italic);
          nodeList.add(node);
          prop = p.getBackground();
          if (prop != null) {
            node = new ImageNode(style(), &docHome, prop, false);
            nodeList.add(node);
            images.add(node);
          }
        } else if (0 == strncasecmp(tag, "ul>", 3) || 
                   0 == strncasecmp(tag, "ol>", 3)) {
          node = new UlNode(tag[0] == 'o' || tag[0] == 'O');
          olStack.push(node);
          nodeList.add(node);
          padlines = false;
        } else if (0 == strncasecmp(tag, "u>", 2)) {
          uline = true;
          nodeList.add(new StyleNode(uline, center));
        } else if (0 == strncasecmp(tag, "li>", 3)) {
          node = new LiNode((UlNode *) olStack.peek());
          nodeList.add(node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "a ", 2)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new AnchorNode(p);
          nodeList.add(node);
          anchors.add(node);
        } else if (0 == strncasecmp(tag, "font ", 5)) {
          p.removeAll();
          p.load(tag + 5, taglen - 5);
          color = getColor(p.get("color"), 0);
          prop = p.get("font-size");
          if (prop != null) {
            // convert from points to pixels
            //const fltk3::Monitor & monitor = fltk3::Monitor::all();
            //fontSize = (int)(prop->toInteger() * monitor.dpi_y() / 72.0);
            // TODO: fixme
          } else {
            prop = p.get("size");
            if (prop != null) {
              fontSize = 7 + (prop->toInteger() * 2);
            }
          }
          prop = p.get("face");
          if (prop != null) {
            // TODO: fixme
            // font = fltk3::font(prop->toString());
          }
          node = new FontNode(font, fontSize, color, bold, italic);
          nodeList.add(node);
        } else if (taglen == 2 && 0 == strncasecmp(tag, "h", 1)) {
          // H1-H6 from large to small
          int size = FONT_SIZE_H1 - ((tag[1] - '1') * 2);
          node = new FontNode(font, size, 0, ++bold, italic);
          nodeList.add(new BrNode(pre));
          nodeList.add(node);
          padlines = false;
        } else if (0 == strncasecmp(tag, "input ", 6)) {
          // check for quoted values including '>'
          if (unquoteTag(tagBegin + 6, tagEnd)) {
            taglen = tagEnd - tagBegin - 1;
            text = *tagEnd == 0 ? 0 : tagEnd + 1;
          }
          p.removeAll();
          p.load(tag + 6, taglen - 6);
          inputNode = new InputNode(this, &p);
          nodeList.add(inputNode);
          inputs.add(inputNode);
          inputNode->update(&namedInputs, cookies, &p);
        } else if (0 == strncasecmp(tag, "textarea", 8)) {
          p.load(tag + 8, taglen - 8);
          tagPair = text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "select", 6)) {
          p.load(tag + 6, taglen - 6);
          tagPair = text = skipWhite(tagEnd + 1);
          options.removeAll();
        } else if (0 == strncasecmp(tag, "option", 6)) {
          tagPair = text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "img ", 4)) {
          p.removeAll();
          p.load(tag + 4, taglen - 4);
          node = new ImageNode(style(), &docHome, &p);
          nodeList.add(node);
          images.add(node);
        } else if (0 == strncasecmp(tag, "body ", 5)) {
          p.removeAll();
          p.load(tag + 5, taglen - 5);
          text = skipWhite(tagEnd + 1);
          foreground = getColor(p.getFgColor(), foreground);
          background = getColor(p.getBgColor(), background);
          prop = p.getBackground();
          if (prop != null) {
            node = new ImageNode(style(), &docHome, prop, true);
            nodeList.add(node);
            images.add(node);
          }
        } else if (0 == strncasecmp(tag, "script", 6) || 
                   0 == strncasecmp(tag, "style", 5)) {
          tagPair = text;
        } else {
          // unknown tag
          text = skipWhite(tagEnd + 1);
        }
      } else if (tag[0] == '?') {
        nodeList.add(new EnvNode(cookies, tag + 1, taglen - 1));
      } else {
        // '<' is a literal character
        nodeList.add(new TextNode(anglestr, 1));
        tagEnd = tagBegin;
        text = tagBegin + 1;
      }                         // end if-start, else-end tag
    }                           // if found a tag
    tagBegin = *tagEnd == 0 ? tagEnd : tagEnd + 1;
  }

  // prevent nodes from being auto-deleted
  olStack.emptyList();
  tdStack.emptyList();
  trStack.emptyList();
  while (tableStack.peek()) {
    node = new TableEndNode((TableNode *) tableStack.pop());
    nodeList.add(node);
  }
}

// handle click from form button
void HelpWidget::onclick(Widget *button) {
  int len = inputs.length();
  strlib::Object **list = inputs.getList();
  for (int i = 0; i < len; i++) {
    InputNode *p = (InputNode *) list[i];
    if (p->button == button) {
      this->event.empty();
      this->event.append(p->onclick.toString());
      user_data((void *)this->event.toString());
      do_callback();
      return;
    }
  }
}

int HelpWidget::onMove(int event) {
  int ex = fltk3::event_x();
  int ey = fltk3::event_y();

  if (pushedAnchor && event == fltk3::DRAG) {
    bool pushed = pushedAnchor->ptInSegment(ex, ey);
    if (pushedAnchor->pushed != pushed) {
      fltk3::cursor(fltk3::CURSOR_HAND);
      pushedAnchor->pushed = pushed;
      damage(DAMAGE_PUSHED);
    }
    return 1;
  } else {
    int len = anchors.length();
    strlib::Object **list = anchors.getList();
    for (int i = 0; i < len; i++) {
      AnchorNode *p = (AnchorNode *) list[i];
      if (p->ptInSegment(ex, ey)) {
        fltk3::cursor(fltk3::CURSOR_HAND);
        return 1;
      }
    }
    fltk3::cursor(fltk3::CURSOR_DEFAULT);
  }

  if (event == fltk3::DRAG) {
    switch (mouseMode) {
    case mm_select:
      // drag text selection
      pointX = ex - hscroll;
      pointY = ey - vscroll;
      damage(DAMAGE_HIGHLIGHT);
      break;
    case mm_scroll:
      // follow the mouse navigation
      if (scrollY != ey) {
        // scroll up (less -ve) when draged down
        S16 scroll = vscroll + (ey - scrollY);
        scrollY = ey;
        if (scroll > 0) {
          scroll = 0;           // too far up
        } else if (-scroll > scrollHeight) {
          scroll = -scrollHeight;       // too far down
        }
        if (scroll != vscroll) {
          vscroll = scroll;
          damage(DAMAGE_EXPOSE);
        }
      }
      break;
    case mm_page:
      break;
    }
    return 1;
  }

  return 0;
}

int HelpWidget::onPush(int event) {
  strlib::Object **list = anchors.getList();
  int len = anchors.length();
  pushedAnchor = 0;
  int ex = fltk3::event_x();
  int ey = fltk3::event_y();
  S16 scroll = vscroll;

  for (int i = 0; i < len; i++) {
    AnchorNode *p = (AnchorNode *) list[i];
    if (p->ptInSegment(ex, ey)) {
      pushedAnchor = p;
      pushedAnchor->pushed = true;
      fltk3::cursor(fltk3::CURSOR_HAND);
      damage(DAMAGE_PUSHED);
      return 1;
    }
  }

  switch (mouseMode) {
  case mm_select:
    // begin/continue text selection
    if (event_state(SHIFT)) {
      pointX = (ex - hscroll);
      pointY = (ey - vscroll);
    } else {
      markX = pointX = (ex - hscroll);
      markY = pointY = (ey - vscroll);
    }
    damage(DAMAGE_HIGHLIGHT);
    break;

  case mm_scroll:
    scrollY = ey;
    break;

  case mm_page:
    if (ey > h() / 2) {
      // page down/up
      scroll += ex > w() / 2 ? -h() : h();
    } else {
      // home/end
      scroll = ex > w() / 2 ? -scrollHeight : 0;
    }
    if (scroll > 0) {
      scroll = 0;               // too far up
    } else if (-scroll > scrollHeight) {
      scroll = -scrollHeight;   // too far down
    }
    if (scroll != vscroll) {
      vscroll = scroll;
      damage(DAMAGE_EXPOSE);
    }
    break;
  }
  return 1;                     // return 1 to become the belowmouse
}

int HelpWidget::handle(int event) {
  int handled = Group::handle(event);
  if (handled && event != fltk3::MOVE) {
    return handled;
  }

  switch (event) {
  case EVENT_INCREASE_FONT:
    if (getFontSize() < MAX_FONT_SIZE) {
      setFontSize(getFontSize() + 1);
    }
    return 1;

  case EVENT_DECREASE_FONT:
    if (getFontSize() > MIN_FONT_SIZE) {
      setFontSize(getFontSize() - 1);
    }
    return 1;

  case EVENT_COPY_TEXT:
    copySelection();
    return 1;

  case EVENT_SEL_ALL_TEXT:
    selectAll();
    return 1;

  case EVENT_FIND:
    find(fltk3::input("Find:"), false);
    return 1;

  case EVENT_PG_DOWN:
    if (scrollbar->active()) {
      //scrollbar->handle_drag(scrollbar->value() + scrollbar->pagesize());
      // TODO: fixme
    }
    return 1;

  case EVENT_PG_UP:
    if (scrollbar->active()) {
      // scrollbar->handle_drag(scrollbar->value() - scrollbar->pagesize());
      // TODO: fixme
    }
    return 1;

  case fltk3::SHOW:
    focus(this);
    break;

  case fltk3::FOCUS:
    return 1;                   // aquire focus

  case fltk3::PUSH:
    return onPush(event);

  case fltk3::ENTER:
    return 1;

  case fltk3::KEYBOARD:
    if (event_state(fltk3::RightKey) && -hscroll < w() / 2) {
      hscroll -= HSCROLL_STEP;
      redraw();
      return 1;
    }
    if (event_state(fltk3::LeftKey) && hscroll < 0) {
      hscroll += HSCROLL_STEP;
      redraw();
      return 1;
    }
    if (event_state(ControlRKey) || event_state(ControlLKey)) {
      switch (event_key()) {
      case 'u':
        return handle(EVENT_PG_UP);
      case 'd':
        return handle(EVENT_PG_DOWN);
      case 'r':                // reload
        reloadPage();
        return 1;
      case 'f':                // find
        find(fltk3::input("Find:"), false);
        return 1;
      case 'a':                // select-all
        selectAll();
        return 1;
      case InsertKey:
      case 'c':                // copy
        copySelection();
        return 1;
      case 'b':                // break popup
      case 'q':
        if (fltk3::modal() == parent()) {
          //fltk3::exit_modal();
          // TODO: fixme
        }
        break;                  // handle in default
      }
    }
    break;

  case fltk3::DRAG:
  case fltk3::MOVE:
    return onMove(event);

  case fltk3::RELEASE:
    if (pushedAnchor) {
      fltk3::cursor(fltk3::CURSOR_DEFAULT);
      bool pushed = pushedAnchor->pushed;
      pushedAnchor->pushed = false;
      damage(DAMAGE_PUSHED);
      if (pushed) {
        this->event.empty();
        this->event.append(pushedAnchor->href.toString());
        if (this->event.length()) {
          // href has been set
          user_data((void *)this->event.toString());
          do_callback();
        }
      }
      return 1;
    }
  }
  return scrollbar->active()? scrollbar->handle(event) : 0;
}

bool HelpWidget::find(const char *s, bool matchCase) {
  if (s == 0 || s[0] == 0) {
    return false;
  }

  strlib::Object **list = nodeList.getList();
  int len = nodeList.length();
  int foundRow = 0;
  int lineHeight = (int)(getascent() + getdescent());
  for (int i = 0; i < len; i++) {
    BaseNode *p = (BaseNode *) list[i];
    if (p->indexOf(s, matchCase) != -1) {
      foundRow = p->getY() - vscroll;
      if (foundRow > -vscroll + lineHeight) {
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

  damage(DAMAGE_ALL | DAMAGE_CONTENTS);
  return true;
}

void HelpWidget::copySelection() {
  fltk3::copy(selection.toString(), selection.length(), true);
}

void HelpWidget::selectAll() {
  markX = markY = 0;
  pointX = w();
  pointY = scrollHeight + h();
  selection.empty();
  getText(&selection);
  redraw();
}

void HelpWidget::navigateTo(const char *s) {
  if (strncmp(s, "http://", 7) == 0) {
    // launch in real browser
    browseFile(s);
    return;
  }

  strlib::String path;
  path.append(docHome);

  int len = path.length();
  if (len && path[len - 1] == '/' && s[0] == '/') {
    // avoid adding double slashes
    path.append(s + 1);
  } else {
    path.append(s);
  }
  loadFile(path.toString());
}

void HelpWidget::loadBuffer(const char *str) {
  if (strncasecmp("file:", str, 5) == 0) {
    loadFile(str + 5);
  } else {
    htmlStr = str;
    reloadPage();
  }
}

void HelpWidget::loadFile(const char *f, bool useDocHome) {
  FILE *fp;
  long len;

  fileName.empty();
  htmlStr.empty();

  if (docHome.length() != 0 && useDocHome) {
    fileName.append(docHome);
  }

  if (strncasecmp(f, "file:///", 8) == 0) {
    // only supports file protocol
    f += 8;
  }

  const char *target = strrchr(f, '#');
  len = target != null ? target - f : strlen(f);
  fileName.append(f, len);
  fileName.replaceAll('\\', '/');

  // update docHome using the given file-name
  if (docHome.length() == 0) {
    int i = fileName.lastIndexOf('/', 0);
    if (i != -1) {
      docHome = fileName.substring(0, i + 1);
    } else {
      docHome.append("./");
    }
    if (docHome[docHome.length() - 1] != '/') {
      docHome.append("/");
    }
  }
  if ((fp = ::fopen(fileName.toString(), "rb")) != NULL) {
    fseek(fp, 0, SEEK_END);
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
    // draw to obtain dimensions
    fltk3::flush();
    scrollTo(target + 1);
  }
}

void HelpWidget::getImageNames(strlib::List *nameList) {
  nameList->removeAll();
  strlib::Object **list = images.getList();
  int len = images.length();
  for (int i = 0; i < len; i++) {
    ImageNode *imageNode = (ImageNode *) list[i];
    nameList->addSet(&imageNode->url);
  }
}

// reload broken images
void HelpWidget::reloadImages() {
  strlib::Object **list = images.getList();
  int len = images.length();
  for (int i = 0; i < len; i++) {
    ImageNode *imageNode = (ImageNode *) list[i];
    // TODO: fixme
    //if (imageNode->image == &brokenImage) {
    //      imageNode->reload();
    //    }
  }
  redraw();
}

void HelpWidget::setDocHome(const char *s) {
  docHome.empty();
  docHome.append(s);
  if (s && s[strlen(s) - 1] != '/') {
    docHome.append("/");
  }
}

const char *HelpWidget::getAnchor(int index) {
  int len = anchors.length();
  if (index < len && index > -1) {
    strlib::Object **list = anchors.getList();
    AnchorNode *p = (AnchorNode *) list[index];
    return p->href.toString();
  }
  return null;
}

void HelpWidget::getText(strlib::String *s) {
  strlib::Object **list = nodeList.getList();
  int len = nodeList.length();
  for (int i = 0; i < len; i++) {
    BaseNode *p = (BaseNode *) list[i];
    p->getText(s);
  }
}

bool HelpWidget::isHtmlFile() {
  const char *filename = fileName.toString();
  if (!fileName || !fileName[0]) {
    return false;
  }
  int len = strlen(filename);
  return (::strcasecmp(filename + len - 4, ".htm") == 0 || 
          ::strcasecmp(filename + len - 5, ".html") == 0);
}

//--Helper functions------------------------------------------------------------

/**
 * Returns the number of characters that will fit within
 * the gixen pixel width.
 */
void lineBreak(const char *s, int slen, int width, int &linelen, int &linepx) {
  // find the end of the first word
  int i = 0;
  int txtWidth;
  int ibreak = -1;
  int breakWidth = -1;

  while (i < slen) {
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

  while (i < slen && txtWidth < width) {
    ibreak = i;
    breakWidth = txtWidth;
    while (i < slen) {
      if (s[i++] == ' ') {
        break;
      }
    }
    txtWidth += (int)getwidth(s + ibreak, i - ibreak);
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

// return a new tagEnd if the current '>' is embedded in quotes
bool unquoteTag(const char *tagBegin, const char *&tagEnd) {
  bool quote = false;
  int len = tagEnd - tagBegin;
  int i = 1;
  while (i < len) {
    switch (tagBegin[i++]) {
    case '\'':
    case '\"':
      quote = !quote;
      break;
    }
  }
  // <input type="ffff>"> - move end-tag
  // <input type="ffff>text<next-tag> end-tag is unchanged
  // <input value='@>>' >
  if (quote) {
    // found unclosed quote within tag
    i = 1;
    while (true) {
      switch (tagEnd[i]) {
      case 0:
      case '<':
        return false;
      case '\'':
      case '\"':
        quote = !quote;
        break;
      case '>':
        if (quote == false) {
          tagEnd += i;
          return true;
        }
        break;
      }
      i++;
    }
  }
  return false;
}

/**
 * skip white space between tags:
 * <table>-skip-<tr>-skip-<td></td>-skip</tr>-skip-</table>
 */
const char *skipWhite(const char *s) {
  if (s == 0 || s[0] == 0) {
    return 0;
  }
  while (isWhite(*s)) {
    s++;
  }
  return s;
}

Color getColor(strlib::String *s, Color def) {
  if (s == 0 || s->length() == 0) {
    return def;
  }

  const char *n = s->toString();
  if (n[0] == '#') {
    // do hex color lookup
    int rgb = strtol(n + 1, NULL, 16);
    int r = rgb >> 16;
    int g = (rgb >> 8) & 255;
    int b = rgb & 255;
    return fltk3::rgb_color((uchar) r, (uchar) g, (uchar) b);
  } else if (::strcasecmp(n, "black") == 0) {
    return BLACK;
  } else if (::strcasecmp(n, "red") == 0) {
    return RED;
  } else if (::strcasecmp(n, "green") == 0) {
    return fltk3::rgb_color(0, 0x80, 0);
  } else if (::strcasecmp(n, "yellow") == 0) {
    return YELLOW;
  } else if (::strcasecmp(n, "blue") == 0) {
    return BLUE;
  } else if (::strcasecmp(n, "magenta") == 0 || 
             ::strcasecmp(n, "fuchsia") == 0) {
    return MAGENTA;
  } else if (::strcasecmp(n, "cyan") == 0 || 
             ::strcasecmp(n, "aqua") == 0) {
    return CYAN;
  } else if (::strcasecmp(n, "white") == 0) {
    return WHITE;
  } else if (::strcasecmp(n, "gray") == 0 || 
             ::strcasecmp(n, "grey") == 0) {
    return fltk3::rgb_color(0x80, 0x80, 0x80);
  } else if (::strcasecmp(n, "lime") == 0) {
    return GREEN;
  } else if (::strcasecmp(n, "maroon") == 0) {
    return fltk3::rgb_color(0x80, 0, 0);
  } else if (::strcasecmp(n, "navy") == 0) {
    return fltk3::rgb_color(0, 0, 0x80);
  } else if (::strcasecmp(n, "olive") == 0) {
    return fltk3::rgb_color(0x80, 0x80, 0);
  } else if (::strcasecmp(n, "purple") == 0) {
    return fltk3::rgb_color(0x80, 0, 0x80);
  } else if (::strcasecmp(n, "silver") == 0) {
    return fltk3::rgb_color(0xc0, 0xc0, 0xc0);
  } else if (::strcasecmp(n, "teal") == 0) {
    return fltk3::rgb_color(0, 0x80, 0x80);
  }
  return def;
}

// image factory based on file extension
SharedImage *loadImage(const char *name, uchar *buff) {
  return NULL;
  /*
    TODO: fixme
  int len = strlen(name);
  SharedImage *result = 0;
  if (::strcasecmp(name + (len - 4), ".jpg") == 0 || 
      ::strcasecmp(name + (len - 5), ".jpeg") == 0) {
    result = JPEGImage::get(name, buff);
  } else if (::strcasecmp(name + (len - 4), ".gif") == 0) {
    result = gifImage::get(name, buff);
  } else if (::strcasecmp(name + (len - 4), ".png") == 0) {
    result = pngImage::get(name, buff);
  } else if (::strcasecmp(name + (len - 4), ".xpm") == 0) {
    result = XPMFileImage::get(name, buff);
  }
  if (result) {
    // load the image
    ((Image *) result)->fetch();
  }
  return result;
  */
}

Image *loadImage(const char *imgSrc) {
  if (imgSrc == 0 || fltk3::access(imgSrc, 0) != 0) {
    // TODO: fixme return &brokenImage;
  }
  Image *image = loadImage(imgSrc, 0);
  //return image != 0 ? image : &brokenImage;
  return NULL;
}

#if defined(WIN32)
#include <windows.h>
#include <fltk3/window.h>
#endif

void browseFile(const char *url) {
#if defined(WIN32)
  // TODO: fixme
  //  ShellExecute(xid(fltk3::Window::first()), "open", url, 0, 0, SW_SHOWNORMAL);
#else
  if (fork() == 0) {
    fclose(stderr);
    fclose(stdin);
    fclose(stdout);
    execlp("htmlview", "htmlview", url, NULL);
    execlp("firefox", "firefox", url, NULL);
    execlp("mozilla", "mozilla", url, NULL);
    ::exit(0);                  // in case exec failed
  }
#endif
}
