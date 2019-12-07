// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Window.H>
#include <FL/filename.H>

#define Fl_HELP_WIDGET_RESOURCES
#include "platform/fltk/HelpWidget.h"

#define FOREGROUND_COLOR fl_rgb_color(0x12, 0x12, 0x12)
#define BACKGROUND_COLOR fl_rgb_color(192, 192, 192)
#define ANCHOR_COLOR fl_rgb_color(0,0,255)
#define BUTTON_COLOR fl_rgb_color(0,0,255)

#define DEFAULT_INDENT 8
#define TABLE_PADDING 4
#define CODE_PADDING 4
#define LI_INDENT 18
#define FONT_SIZE_H1 23
#define CELL_SPACING 4
#define INPUT_WIDTH 90
#define BUTTON_WIDTH 20
#define SCROLL_SIZE 10000
#define HSCROLL_STEP 20
#define ELIPSE_LEN 10
#define IMG_TEXT_BORDER 25
#define NO_COLOR 0

Fl_Color getColor(strlib::String *s, Fl_Color def);
void lineBreak(const char *s, int slen, int width, int &stlen, int &pxlen);
const char *skipWhite(const char *s);
bool unquoteTag(const char *tagBegin, const char *&tagEnd);
Fl_Image *loadImage(const char *imgSrc);
struct AnchorNode;
struct InputNode;
static char truestr[] = "true";
static char falsestr[] = "false";
static char spacestr[] = " ";
static char anglestr[] = "<";
char rangeValue[20];

//--Display---------------------------------------------------------------------

struct Display {
  strlib::String *selection;
  Fl_Group *wnd;
  AnchorNode *anchor;
  Fl_Font font;
  Fl_Color color;
  Fl_Color background;
  Fl_Color selectionColor;
  Fl_Color anchorColor;
  int16_t  x1, x2, y1, y2;
  int16_t tabW, tabH;
  int16_t lineHeight;
  int16_t indent;
  int16_t baseIndent;
  uint16_t fontSize;
  uint16_t tableLevel;
  uint16_t nodeId;
  int16_t  imgY;
  uint16_t imgIndent;
  int16_t markX, markY, pointX, pointY;
  uint8_t uline;
  uint8_t center;
  uint8_t content;
  uint8_t exposed;
  uint8_t measure;
  uint8_t selected;
  uint8_t invertedSel;
  uint8_t insideCode;
  uint8_t insideUl;

  void drawBackground(Fl_Rect &rc) {
    if (background != NO_COLOR) {
      Fl_Color oldColor = fl_color();
      fl_color(background);
      fl_rectf(rc.x(), rc.y(), rc.w(), rc.h());
      fl_color(oldColor);
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

  void newRow(uint16_t nrows = 1, bool doBackground = true) {
    int bgY = y1;

    x1 = indent;
    y1 += nrows *lineHeight;
    // flow around images
    if (imgY != -1 && y1 > imgY) {
      imgY = -1;
      x1 = indent = imgIndent;
    }

    if (!measure && background != NO_COLOR && doBackground) {
      Fl_Rect rc(x1, bgY, x2 - x1 + CELL_SPACING, lineHeight);
      drawBackground(rc);
    }
  }

  // restore previous colors
  void restoreColors() {
    fl_color(color);
    background = oldBackground;
  }

  void setColors(Fl_Color nodeForeground, Fl_Color nodeBackground) {
    fl_color(nodeForeground != NO_COLOR ? nodeForeground : color);

    oldBackground = background;
    if (nodeBackground != NO_COLOR) {
      background = nodeBackground;
    }
  }

private:
  Fl_Color oldBackground;
};

//--Attributes------------------------------------------------------------------

struct Value {
  int value;
  bool relative;
};

struct Attributes : public Properties<String *> {
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
  int getStart(int def = 0) {
    return getIntValue("start", def);
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
  return (s != NULL ? s->toInteger() : def);
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

struct BaseNode {
  virtual ~BaseNode() {}
  virtual void display(Display *out) {}
  virtual int indexOf(const char *sFind, uint8_t matchCase) { return -1; }
  virtual void getText(strlib::String *s) {}
  virtual int getY() { return -1; }
};

//-- MeasureNode----------------------------------------------------------------

struct MeasureNode : public BaseNode {
  MeasureNode() : initX(0), initY(0) {}
  int16_t initX, initY;
};

//--CodeNode--------------------------------------------------------------------

struct CodeNode : public MeasureNode {
  CodeNode(int fontSize) :
    MeasureNode(),
    _font(FL_COURIER),
    _fontSize(fontSize),
    _nodeId(0),
    _xEnd(0),
    _yEnd(0),
    _sameLine(false) {
  }
  void display(Display *out);
  void doEndBlock(Display *out);
  Fl_Font _font;
  uint16_t _fontSize;
  uint16_t _nodeId;
  int16_t _xEnd;
  int16_t _yEnd;
  bool _sameLine;
};

void CodeNode::display(Display *out) {
  _sameLine = false;
  out->endImageFlow();
  fl_font(_font, _fontSize);

  if (out->exposed) {
    // defer drawing in child nodes
    out->measure = true;
    initX = out->x1;
    initY = out->y1;
    _font = out->font;
    _nodeId = out->nodeId;
  } else if (!out->measure) {
    int textHeight = fl_height() + fl_descent();
    int xpos = initX;
    int ypos = (initY - textHeight) + fl_descent() * 2;
    int width, height;
    if (_yEnd == initY + (CODE_PADDING * 2)) {
      _sameLine = true;
      width = _xEnd - (CODE_PADDING * 2);
      height = out->lineHeight;
    } else {
      width = out->x2 - (DEFAULT_INDENT * 2);
      height = _yEnd - initY + textHeight - fl_descent();
    }
    fl_color(fl_lighter(out->color));
    fl_rectf(xpos, ypos, width, height);
    fl_color(out->background);
    if (!_sameLine) {
      fl_rect(xpos, ypos, width, height);
    }
  }

  out->insideCode = true;

  if (_sameLine) {
    out->x1 += CODE_PADDING;
  } else {
    out->indent += CODE_PADDING;
    out->x1 = out->indent;
    out->y1 += CODE_PADDING;
  }
}

void CodeNode::doEndBlock(Display *out) {
  out->insideCode = false;

  if (!_sameLine) {
    out->indent -= CODE_PADDING;
    if (out->indent < out->baseIndent) {
      out->indent = out->baseIndent;
    }
    out->y1 += CODE_PADDING;
  }

  if (out->exposed) {
    out->measure = false;
    out->nodeId = _nodeId;
    _xEnd = out->x1;
    _yEnd = out->y1;
  }
}

struct CodeEndNode : public BaseNode {
  CodeEndNode(CodeNode *head) : _head(head) {}
  void display(Display *out);
  CodeNode *_head;
};

void CodeEndNode::display(Display *out) {
  fl_color(out->color);
  if (_head) {
    _head->doEndBlock(out);
  }
}

//--FontNode--------------------------------------------------------------------

struct FontNode : public BaseNode {
  FontNode(Fl_Font font, int fontSize, Fl_Color color, bool bold, bool italic);
  void display(Display *out);

  Fl_Font _font;
  Fl_Color _color;
  uint16_t _fontSize;
};

FontNode::FontNode(Fl_Font font, int fontSize, Fl_Color color, bool bold, bool italic) :
  BaseNode(),
  _font(font),
  _color(color),
  _fontSize(fontSize) {
  if (bold) {
    _font += FL_BOLD;
  }
  if (italic) {
    _font += FL_ITALIC;
  }
}

void FontNode::display(Display *out) {
  fl_font(_font, _fontSize);
  if (_color == (Fl_Color) - 1) {
    fl_color(out->color);       // </font> restores color
  } else if (_color != 0) {
    fl_color(_color);
  }
  int oldLineHeight = out->lineHeight;
  out->lineHeight = fl_height();
  if (out->lineHeight > oldLineHeight) {
    out->y1 += (out->lineHeight - oldLineHeight);
  }
  out->font = _font;
  out->fontSize = _fontSize;
}

//--BrNode----------------------------------------------------------------------

struct BrNode : public BaseNode {
  BrNode(uint8_t premode) :
    BaseNode(),
    premode(premode) {
  }

  void display(Display *out) {
    // when <pre> is active don't flow text around images
    if (premode && out->imgY != -1) {
      out->endImageFlow();
    } else {
      out->newRow(1);
    }
    out->lineHeight = fl_height() + fl_descent();
  }
  uint8_t premode;
};

//--AnchorNode------------------------------------------------------------------

struct AnchorNode : public BaseNode {
  AnchorNode(Attributes &p) :
    BaseNode(),
    x1(0),
    x2(0),
    y1(0),
    y2(0),
    lineHeight(0),
    wrapxy(0),
    pushed(0) {
    p.getName(name);
    p.getHref(href);
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
      fl_color(out->anchorColor);
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

  int16_t x1, x2, y1, y2;
  uint16_t lineHeight;
  uint8_t wrapxy;  // begin on page boundary
  uint8_t pushed;
  uint8_t __padding[4];
  strlib::String name;
  strlib::String href;
};

AnchorNode *pushedAnchor = 0;

struct AnchorEndNode : public BaseNode {
  AnchorEndNode() :
    BaseNode() {
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
    fl_color(out->color);
  }
};

//--StyleNode-------------------------------------------------------------------

struct StyleNode : public BaseNode {
  StyleNode(uint8_t uline, uint8_t center) :
    BaseNode(),
    uline(uline), center(center) {
  }

  void display(Display *out) {
    out->uline = uline;
    out->center = center;
  }
  uint8_t uline;     // 2
  uint8_t center;    // 2
};

//--LiNode----------------------------------------------------------------------

struct UlNode : public BaseNode {
  UlNode(Attributes &p, bool ordered) :
    BaseNode(),
    start(p.getStart(1)),
    ordered(ordered) {
  }
  void display(Display *out) {
    nextId = start;
    out->insideUl = true;
    out->newRow(1);
    out->indent += LI_INDENT;
  }
  int nextId;
  int start;
  bool ordered;
};

struct UlEndNode : public BaseNode {
  UlEndNode() :
    BaseNode() {
  }
  void display(Display *out) {
    out->insideUl = false;
    out->indent -= LI_INDENT;
    out->newRow(1);
  }
};

struct LiNode : public BaseNode {
  LiNode(UlNode *ulNode) :
    ulNode(ulNode) {
  }

  void display(Display *out) {
    out->content = true;
    out->x1 = out->indent;
    out->y1 += out->lineHeight;
    int x = out->x1 - (LI_INDENT - DEFAULT_INDENT);
    int y = out->y1 - fl_height() - fl_descent();
    if (out->measure == false) {
      if (ulNode && ulNode->ordered) {
        char t[10];
        sprintf(t, "%d.", ulNode->nextId++);
        fl_draw(t, 2, x - 4, out->y1);
      } else {
        dotImage.draw(x - 4, y + fl_height() - fl_descent(), 8, 8);
        // draw messes with the current font - restore
        fl_font(out->font, out->fontSize);
      }
    }
  }
  UlNode *ulNode;
};

//--ImageNode-------------------------------------------------------------------

struct ImageNode : public BaseNode {
  ImageNode(strlib::String *docHome, Attributes *a);
  ImageNode(strlib::String *docHome, strlib::String *src, bool fixed);
  ImageNode(Fl_Image *image);
  void makePath(strlib::String *src, strlib::String *docHome);
  void reload();
  void display(Display *out);
  Fl_Image *image;
  Value w, h;
  uint8_t background, fixed;
  uint8_t valign; // 0=top, 1=center, 2=bottom
  uint8_t __padding[5];
  strlib::String path, url;
};

ImageNode::ImageNode(strlib::String *docHome, Attributes *a) :
  BaseNode(),
  background(false),
  fixed(false),
  valign(0) {
  makePath(a->getSrc(), docHome);
  image = loadImage(path.c_str());
  w = a->getWidth(image->w());
  h = a->getHeight(image->h());
}

ImageNode::ImageNode(strlib::String *docHome, strlib::String *src, bool fixed) :
  BaseNode(),
  background(false),
  fixed(false),
  valign(0) {
  makePath(src, docHome);
  image = loadImage(path.c_str());
  w.value = image->w();
  h.value = image->h();
  w.relative = 0;
  h.relative = 0;
}

ImageNode::ImageNode(Fl_Image *image) :
  BaseNode(),
  image(image),
  background(false),
  fixed(false),
  valign(2) {
  w.value = image->w();
  h.value = image->h();
  w.relative = 0;
  h.relative = 0;
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
  image = loadImage(path.c_str());
  int iw = image->w();
  int ih = image->h();
  if (w.relative == 0) {
    w.value = iw;
  }
  if (h.relative == 0) {
    h.value = ih;
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
      int y = fixed ? 0 : out->y1 - fl_height();
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
          image->draw(x1, y1, iw, ih);
          x1 += w.value;
        }
        y1 += h.value;
      }
    } else {
      int x = out->x1 + DEFAULT_INDENT;
      int y = out->y1;
      switch (valign) {
      case 0:                  // top
        y -= fl_height();
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
      image->draw(x, y, iw, ih);
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
  fl_font(out->font, out->fontSize);    // restore font
}

//--TextNode--------------------------------------------------------------------

struct TextNode : public BaseNode {
  TextNode(const char *s, uint16_t textlen);
  void display(Display *out);
  void drawSelection(const char *s, uint16_t len, uint16_t width, Display *out);
  int indexOf(const char *sFind, uint8_t matchCase);
  void getText(strlib::String *s);
  int getY();

  const char *s;
  uint16_t textlen;
  uint16_t width;
  int16_t ybegin;
};

TextNode::TextNode(const char *s, uint16_t textlen) :
  BaseNode(),
  s(s),
  textlen(textlen),
  width(0),
  ybegin(0) {
}

void TextNode::getText(strlib::String *s) {
  s->append(this->s, this->textlen);
}

void TextNode::drawSelection(const char *s, uint16_t len, uint16_t width, Display *out) {
  int out_y = out->y1 - fl_height();
  if (out->pointY < out_y) {
    return;                     // selection above text
  }
  if (out->markY > out_y + out->lineHeight) {
    return;                     // selection below text
  }

  Fl_Rect rc(out->x1, out_y, width, out->lineHeight + fl_descent());
  int selBegin = 0;             // selection index into the draw string
  int selEnd = len;

  if (out->markY > out_y) {
    if (out->pointY < out_y + out->lineHeight) {
      // paint single row selection
      int16_t leftX, rightX;
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
          int width = fl_width(s + i, 1);
          x += width;
          if (left) {
            if (s[i] == ' ') {
              rc.x(x);
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
          x += fl_width(s + i, 1);
          if (left) {
            if (x < leftX) {
              rc.x(x);
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
      int16_t leftX = out->invertedSel ? out->pointX : out->markX;
      if (leftX > out->x1 + width) {
        // selection left of text
        return;
      }
      int x = out->x1;
      int char_w = 0;
      for (int i = 0; i < len; i++) {
        char_w = fl_width(s + i, 1);
        x += char_w;
        if (x < leftX) {
          rc.x(x);
          selBegin = i;
        } else {
          break;
        }
      }
      // subtract the left non-selected segement length from the right side
      rc.w(rc.w() - (x - out->x1) + char_w);
    }
  } else {
    if (out->pointY < out_y + out->lineHeight) {
      // bottom row multiline - find the right margin
      int16_t rightX = out->invertedSel ? out->markX : out->pointX;
      if (rightX < out->x1) {
        return;
      }
      int x = out->x1;
      for (int i = 0; i < len; i++) {
        x += fl_width(s + i, 1);
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

  fl_color(out->selectionColor);
  fl_rectf(rc.x(), rc.y(), rc.w(), rc.h());
  fl_color(out->color);
}

void TextNode::display(Display *out) {
  ybegin = out->y1;
  out->content = true;

  if (width == 0) {
    width = fl_width(s, textlen);
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
      fl_draw(s, textlen, out->x1, out->y1);
      if (out->uline) {
        fl_line(out->x1, out->y1 + 1, out->x1 + width, out->y1 + 1);
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
            linepx += fl_width(p + cliplen, 1);
            cliplen++;
          }
          while (linepx < cellW);
        }
      }
      if (out->measure == false) {
        if (out->selected) {
          drawSelection(p, cliplen, linepx, out);
        }
        fl_draw(p, cliplen, out->x1, out->y1);
        if (out->uline) {
          fl_line(out->x1, out->y1 + 1, out->x1 + linepx, out->y1 + 1);
        }
        if (cliplen != linelen) {
          fl_point(out->x1 + linepx, out->y1);
          fl_point(out->x1 + linepx + 2, out->y1);
          fl_point(out->x1 + linepx + 4, out->y1);
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

int TextNode::indexOf(const char *sFind, uint8_t matchCase) {
  int numMatch = 0;
  int findLen = strlen(sFind);
  for (int i = 0; i < textlen; i++) {
    uint8_t equals = matchCase ? s[i] == sFind[numMatch] : toupper(s[i]) == toupper(sFind[numMatch]);
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
  HrNode() :
    BaseNode() {
  }
  void display(Display *out) {
    if (out->imgY != -1) {
      out->endImageFlow();
      out->y1 -= out->lineHeight;
    }
    out->y1 += 4;
    out->x1 = out->indent;
    if (out->measure == false) {
      fl_color(FL_DARK1);
      fl_line(out->x1, out->y1 + 1, out->x2 - 6, out->y1 + 1);
      fl_color(FL_DARK2);
      fl_line(out->x1, out->y1 + 2, out->x2 - 6, out->y1 + 2);
      fl_color(out->color);
    }
    out->y1 += out->lineHeight + 2;
  }
};

//--ParagraphNode---------------------------------------------------------------

struct ParagraphNode : public BaseNode {
  ParagraphNode() : BaseNode() {}
  void display(Display *out) {
    if (out->imgY != -1) {
      out->endImageFlow();
    } else if (!out->insideCode && !out->insideUl) {
      out->newRow(2);
    }
  }
};

//--Table Support---------------------------------------------------------------

struct TableNode;

struct TrNode : public BaseNode {
  TrNode(TableNode *tableNode, Attributes *a);
  void display(Display *out);

  TableNode *table;
  Fl_Color background, foreground;
  uint16_t cols, __padding1;
  int16_t y1, height;
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
  Value width;
  Fl_Color background, foreground;
  uint16_t colspan;
};

struct TdEndNode : public BaseNode {
  TdEndNode(TdNode *tdNode);
  void display(Display *out);

  TdNode *td;
};

struct TableNode : public MeasureNode {
  TableNode(Attributes *a);
  ~TableNode();
  void display(Display *out);
  void doEndTD(Display *out, TrNode *tr, Value *tdWidth);
  void doEndTable(Display *out);
  void setColWidth(Value *width);
  void cleanup();

  uint16_t *columns;
  int16_t *sizes;
  uint16_t rows, cols;
  uint16_t nextCol;
  uint16_t nextRow;
  uint16_t width;
  uint16_t nodeId;
  int16_t maxY;                     // end of table
  int16_t border;
};

struct TableEndNode : public BaseNode {
  TableEndNode(TableNode *tableNode);
  void display(Display *out);

  TableNode *table;
};

//--TableNode-------------------------------------------------------------------

TableNode::TableNode(Attributes *a) :
  MeasureNode(),
  columns(0),
  sizes(0),
  rows(0),
  cols(0) {
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
    initY += out->lineHeight + CELL_SPACING;
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
    columns = (uint16_t *)malloc(sizeof(uint16_t) * cols);
    sizes = (int16_t *)malloc(sizeof(int16_t) * cols);
    int cellW = width / cols;
    for (int i = 0; i < cols; i++) {
      columns[i] = cellW * (i + 1);
      sizes[i] = 0;
    }
  }
  int lineHeight = fl_height() + fl_descent();
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

TableEndNode::TableEndNode(TableNode *tableNode) :
  BaseNode(),
  table(tableNode) {
}

void TableEndNode::display(Display *out) {
  if (table) {
    table->doEndTable(out);
  }
}

//--TrNode----------------------------------------------------------------------

TrNode::TrNode(TableNode *tableNode, Attributes *a) :
  BaseNode(),
  table(tableNode),
  cols(0),
  y1(0),
  height(0) {
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
    table->maxY += out->lineHeight + TABLE_PADDING;
  }
  out->content = false;
  y1 = table->maxY;
  table->nextCol = 0;
  table->nextRow++;

  if (background && out->measure == false) {
    Fl_Rect rc(table->initX, y1 - fl_height(), table->width, out->lineHeight);
    out->drawBackground(rc);
  }
}

TrEndNode::TrEndNode(TrNode *trNode) :
  BaseNode(),
  tr(trNode) {
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

TdNode::TdNode(TrNode *trNode, Attributes *a) :
  BaseNode(),
  tr(trNode) {
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

  out->x1 = table->initX + TABLE_PADDING + (table->nextCol == 0 ? 0 : table->columns[table->nextCol - 1]);
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
    Fl_Rect rc(out->indent - CELL_SPACING,
               tr->y1 - fl_height() + fl_descent(),
               out->x2 - out->indent + (CELL_SPACING * 2),
               out->lineHeight);
    out->drawBackground(rc);
    if (table->border > 0) {
      Fl_Color oldColor = fl_color();
      fl_color(FL_BLACK);
      fl_overlay_rect(rc.x(), rc.y(), rc.w(), rc.h());
      fl_color(oldColor);
    }
  }

}

TdEndNode::TdEndNode(TdNode *tdNode) :
  BaseNode(),
  td(tdNode) {
}

void TdEndNode::display(Display *out) {
  out->restoreColors();

  if (td && td->tr && td->tr->table) {
    td->tr->table->doEndTD(out, td->tr, &td->width);
  }
}

//--NamedInput------------------------------------------------------------------

struct NamedInput {
  NamedInput(InputNode *node, strlib::String *name) {
    this->input = node;
    this->name.append(name->c_str());
  }
  ~NamedInput() {
  }
  InputNode *input;
  strlib::String name;
};

//--InputNode-------------------------------------------------------------------

static void onclick_callback(Fl_Widget *button, void *buttonId) {
  ((HelpWidget *)button->parent())->onclick(button);
}

static void def_button_callback(Fl_Widget *button, void *buttonId) {
  // supply "onclick=fff" to make it do something useful
  // check for parent of HelpWidget
  if (Fl::modal() == (Fl_Window *)button->parent()->parent()) {
    Fl::modal()->set_non_modal();
  }
}

struct InputNode : public BaseNode {
  InputNode(Fl_Group *parent);
  InputNode(Fl_Group *parent, Attributes *a, const char *v, int len);
  InputNode(Fl_Group *parent, Attributes *a);
  void update(strlib::List<NamedInput *> *namedInputs, Properties<String *> *p, Attributes *a);
  void display(Display *out);

  Fl_Widget *button;
  uint32_t rows, cols;
  strlib::String onclick;
};

// creates either a text, checkbox, radio, hidden or button control
InputNode::InputNode(Fl_Group *parent, Attributes *a) :
  BaseNode() {
  parent->begin();
  strlib::String *type = a->getType();
  if (type != NULL && type->equals("text")) {
    button = new Fl_Input(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_TEXTBOX);
  } else if (type != NULL && type->equals("readonly")) {
    button = new Fl_Input(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_READONLY);
  } else if (type != NULL && type->equals("checkbox")) {
    button = new Fl_Check_Button(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_CHKBOX);
  } else if (type != NULL && type->equals("radio")) {
    button = new Fl_Radio_Button(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_RADIO);
  } else if (type != NULL && type->equals("slider")) {
    button = new Fl_Slider(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_RANGEVAL);
  } else if (type != NULL && type->equals("valueinput")) {
    button = new Fl_Value_Input(0, 0, BUTTON_WIDTH, 0);
    button->argument(ID_RANGEVAL);
  } else if (type != NULL && type->equals("hidden")) {
    button = new Fl_Input(0, 0, 0, 0);
    button->argument(ID_HIDDEN);
  } else {
    button = new Fl_Button(0, 0, 0, 0);
    button->argument(ID_BUTTON);
    button->callback(def_button_callback);
  }
  parent->end();
}

InputNode::InputNode(Fl_Group *parent, Attributes *a, const char *s, int len) :
  BaseNode() {
  // creates a textarea control
  parent->begin();
  if (a->isReadonly()) {
    strlib::String str;
    str.append(s, len);
    button = new Fl_Input(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_READONLY);
    button->copy_label(str.c_str());
  } else {
    button = new Fl_Input(0, 0, INPUT_WIDTH, 0);
    button->argument(ID_TEXTAREA);
    ((Fl_Input *)button)->value(s, len);
  }
  parent->end();
}

InputNode::InputNode(Fl_Group *parent) :
  BaseNode() {
  // creates a select control
  parent->begin();
  button = new Fl_Choice(0, 0, INPUT_WIDTH, 0);
  button->argument(ID_SELECT);
  parent->end();
}

void createDropList(InputNode *node, strlib::List<String *> *options) {
  Fl_Choice *menu = (Fl_Choice *)node->button;
  List_each(String *, it, *options) {
    String *s = (*it);
    menu->add(s->c_str());
  }
}

void InputNode::update(strlib::List<NamedInput *> *names, Properties<String *> *env, Attributes *a) {
  Fl_Valuator *valuator;
  Fl_Input *input = NULL;
  Fl_Color color;
  strlib::String *name = a->getName();
  strlib::String *value = a->getValue();
  strlib::String *align = a->getAlign();

  if (name != NULL) {
    names->add(new NamedInput(this, name));
  }

  if (button == 0) {
    return;
  }
  // value uses environment/external attributes
  if (value == 0 && name != 0 && env) {
    value = env->get(name->c_str());
  }

  switch (button->argument()) {
  case ID_READONLY:
    button->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
    if (value && value->length()) {
      button->copy_label(value->c_str());
    }
    // fallthru
  case ID_TEXTAREA:
    button->box(FL_NO_BOX);
    rows = a->getRows();
    cols = a->getCols();
    if (rows > 1) {
      button->type(FL_MULTILINE_INPUT);
    }
    break;
  case ID_RANGEVAL:
    valuator = (Fl_Valuator *)button;
    valuator->minimum(a->getMin());
    valuator->maximum(a->getMax());
    valuator->step(1);
    valuator->align(FL_ALIGN_LEFT);
    if (value && value->length()) {
      valuator->value(value->toInteger());
    }
    break;
  case ID_TEXTBOX:
    input = (Fl_Input *)button;
    if (value && value->length()) {
      input->value(value->c_str());
    }
    break;
  case ID_BUTTON:
    if (value && value->length()) {
      button->copy_label(value->c_str());
    } else {
      button->copy_label(" ");
    }
    break;
  case ID_HIDDEN:
    if (value && value->length()) {
      button->copy_label(value->c_str());
    }
    break;
  }

  // size
  int size = a->getSize();
  if (size != -1) {
    button->size(size, button->h());
  }
  // set callback
  onclick.append(a->getOnclick());
  if (onclick.length()) {
    button->callback(onclick_callback);
  }
  // set colors
  color = getColor(a->getBgColor(), 0);
  if (color) {
    button->color(color);
  }
  color = getColor(a->getFgColor(), 0);
  if (color) {
    button->labelcolor(color);
  }

  // set alignment
  if (align != 0) {
    if (align->equals("right")) {
      button->align(FL_ALIGN_RIGHT | FL_ALIGN_CLIP);
    } else if (align->equals("center")) {
      button->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
    } else if (align->equals("top")) {
      button->align(FL_ALIGN_TOP | FL_ALIGN_CLIP);
    } else if (align->equals("bottom")) {
      button->align(FL_ALIGN_BOTTOM | FL_ALIGN_CLIP);
    } else {
      button->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
    }
  }
  // set border
  switch (a->getBorder(0)) {
  case 1:
    button->box(FL_BORDER_BOX);
    break;
  case 2:
    button->box(FL_SHADOW_BOX);
    break;
  case 3:
    button->box(FL_ENGRAVED_BOX);
    break;
  case 4:
    button->box(FL_THIN_DOWN_BOX);
    break;
  }
}

void InputNode::display(Display *out) {
  if (button == 0 || ID_HIDDEN == button->argument()) {
    return;
  }

  int height = 4 + fl_height() + fl_descent();
  switch (button->argument()) {
  case ID_SELECT:
    height += 4;
    break;
  case ID_BUTTON:
    if (button->w() == 0 && button->label()) {
      button->size(12 + fl_width(button->label()), button->h());
    }
    break;
  case ID_TEXTAREA:
  case ID_READONLY:
    button->size(4 + (fl_width("$") * cols), button->h());
    height = 4 + (fl_height() + fl_descent() * rows);
    break;
  case ID_TEXTBOX:
    ((Fl_Input *)button)->textfont(out->font);
    ((Fl_Input *)button)->textsize(out->fontSize);
    break;
  default:
    break;
  }
  if (out->x1 != out->indent && button->w() > out->x2 - out->x1) {
    out->newRow();
  }
  out->lineHeight = height;
  button->resize(out->x1, out->y1 - fl_height(), button->w(), out->lineHeight - 2);
  button->labelfont(out->font);
  button->labelsize(out->fontSize);
  if (button->y() + button->h() < out->y2 && button->y() >= 0) {
    button->set_visible();
  } else {
    // draw a fake control in case partially visible
    fl_color(button->color());
    fl_rectf(button->x(), button->y(), button->w(), button->h());
    fl_color(out->color);
  }
  out->x1 += button->w();
  out->content = true;
}

//--EnvNode---------------------------------------------------------------------

struct EnvNode : public TextNode {
  EnvNode(Properties<String *> *p, const char *s, uint16_t textlen) :
    TextNode(0, 0) {
    strlib::String var;
    var.append(s, textlen);
    var.trim();
    if (p) {
      strlib::String *s = p->get(var.c_str());
      value.append(s);
    }
    if (value.length() == 0) {
      value.append(getenv(var.c_str()));
    }
    this->s = value.c_str();
    this->textlen = value.length();
  }
  // here to provide value cleanup
  strlib::String value;
};

//--HelpWidget------------------------------------------------------------------

static void scrollbar_callback(Fl_Widget *scrollBar, void *helpWidget) {
  ((HelpWidget *)helpWidget)->redraw();
}

static void anchor_callback(Fl_Widget *helpWidget, void *target) {
  ((HelpWidget *)helpWidget)->navigateTo((const char *)target);
}

HelpWidget::HelpWidget(Fl_Widget *rect, int defsize) :
  Fl_Group(rect->x(), rect->y(), rect->w(), rect->h()),
  background(BACKGROUND_COLOR),
  foreground(FOREGROUND_COLOR),
  scrollHeight(0),
  scrollWindowHeight(0),
  markX(0),
  markY(0),
  pointX(0),
  pointY(0),
  hscroll(0),
  scrollY(0),
  mouseMode(mm_select),
  nodeList(100),
  namedInputs(5),
  inputs(5),
  anchors(5),
  images(5),
  cookies(NULL) {
  begin();
  scrollbar = new Fl_Scrollbar(rect->w() - SCROLL_X, rect->y(), SCROLL_W, rect->h());
  scrollbar->type(FL_VERTICAL);
  scrollbar->value(0, 1, 0, SCROLL_SIZE);
  scrollbar->user_data(this);
  scrollbar->callback(scrollbar_callback);
  scrollbar->deactivate();
  scrollbar->show();
  end();
  callback(anchor_callback);    // default callback
  init();
  docHome.clear();
  labelsize(defsize);
}

HelpWidget::~HelpWidget() {
  cleanup();
}

void HelpWidget::init() {
  scrollHeight = 0;
  scrollWindowHeight = 0;
  hscroll = 0;
  endSelection();
  scrollbar->value(0);
}

void HelpWidget::setTheme(EditTheme *theme) {
  background = get_color(theme->_background);
  foreground = get_color(theme->_color);
  selection_color(get_color(theme->_selection_color));
  labelcolor(get_color(theme->_cursor_color));
}

void HelpWidget::endSelection() {
  markX = pointX = -1;
  markY = pointY = -1;
  selection.clear();
}

void HelpWidget::setFontSize(int i) {
  labelsize(i);
  reloadPage();
}

void HelpWidget::cleanup() {
  List_each(InputNode *, it, inputs) {
    InputNode *p = (*it);
    if (p->button) {
      remove(p->button);
      delete p->button;
      p->button = 0;
    }
  }

  // button/anchor items destroyed in nodeList
  inputs.clear();
  anchors.clear();
  images.clear();
  nodeList.removeAll();
  namedInputs.removeAll();
  title.clear();
}

void HelpWidget::reloadPage() {
  cleanup();
  init();
  compile();
  redraw();
  pushedAnchor = 0;
}

// returns the control with the given name
Fl_Widget *HelpWidget::getInput(const char *name) {
  List_each(NamedInput *, it, namedInputs) {
    NamedInput *ni = (*it);
    if (ni->name.equals(name)) {
      return ni->input->button;
    }
  }
  return NULL;
}

// return the value of the given control
const char *HelpWidget::getInputValue(Fl_Widget *widget) {
  if (widget == 0) {
    return NULL;
  }
  switch (widget->argument()) {
  case ID_TEXTBOX:
  case ID_TEXTAREA:
    return ((Fl_Input *)widget)->value();
  case ID_RADIO:
  case ID_CHKBOX:
    return ((Fl_Radio_Button *)widget)->value() ? truestr : falsestr;
  case ID_SELECT:
    return NULL;
  case ID_RANGEVAL:
    sprintf(rangeValue, "%f", ((Fl_Valuator *)widget)->value());
    return rangeValue;
  case ID_HIDDEN:
  case ID_READONLY:
    return widget->label();
  }
  return NULL;
}

// return the nth form value
const char *HelpWidget::getInputValue(int i) {
  int len = namedInputs.size();
  if (i < len) {
    NamedInput *ni = namedInputs[i];
    return getInputValue(ni->input->button);
  }
  return 0;
}

// return the name of the given button control
const char *HelpWidget::getInputName(Fl_Widget *button) {
  List_each(NamedInput *, it, namedInputs) {
    NamedInput *ni = (*it);
    if (ni->input->button == button) {
      return ni->name.c_str();
    }
  }
  return NULL;
}

// return all of the forms names and values - except hidden ones
void HelpWidget::getInputProperties(Properties<String *> *p) {
  if (p != 0) {
    List_each(NamedInput *, it, namedInputs) {
      NamedInput *ni = (*it);
      const char *value = getInputValue(ni->input->button);
      if (value) {
        p->put(ni->name.c_str(), value);
      }
    }
  }
}

// update a widget's display value using the given string based
// assignment statement, eg val=1000
bool HelpWidget::setInputValue(const char *assignment) {
  strlib::String s = assignment;
  strlib::String name = s.leftOf('=');
  strlib::String value = s.rightOf('=');
  if (value.length() == 0) {
    return false;
  }

  List_each(NamedInput *, it, namedInputs) {
    NamedInput *ni = (*it);
    if (ni->name.equals(name)) {
      Fl_Widget *button = ni->input->button;

      switch (button->argument()) {
      case ID_TEXTBOX:
      case ID_TEXTAREA:
        ((Fl_Input *)button)->value(value.c_str());
        break;
      case ID_RADIO:
      case ID_CHKBOX:
        ((Fl_Radio_Button *)button)->value(value.equals(truestr) || value.equals("1"));
        break;
      case ID_SELECT:
        break;
      case ID_RANGEVAL:
        ((Fl_Valuator *)button)->value(value.toNumber());
        break;
      case ID_READONLY:
        button->copy_label(value.c_str());
        break;
      }
      return true;
    }
  }
  return false;
}

void HelpWidget::scrollTo(const char *anchorName) {
  List_each(AnchorNode *, it, anchors) {
    AnchorNode *p = (*it);
    if (p->name.equals(anchorName)) {
      if (p->getY() > scrollHeight) {
        vscroll(-scrollHeight);
      } else {
        vscroll(-p->getY());
      }
      redraw();
      return;
    }
  }
}

void HelpWidget::resize(int x, int y, int w, int h) {
  Fl_Group::resize(x, y, w, h);
  scrollbar->resize(w - SCROLL_X, y, SCROLL_W, h);
  endSelection();
}

void HelpWidget::draw() {
  if (damage() == FL_DAMAGE_CHILD) {
    Fl_Group::draw();
    return;
  }

  int vscroll = -scrollbar->value();
  Display out;
  out.uline = false;
  out.center = false;
  out.wnd = this;
  out.anchor = 0;
  out.font = FL_HELVETICA;
  out.fontSize = (int)labelsize();
  out.color = foreground;
  out.background = background;
  out.selectionColor = selection_color();
  out.anchorColor = labelcolor();
  out.y2 = h();
  out.indent = DEFAULT_INDENT + hscroll;
  out.baseIndent = out.indent;
  out.x1 = x() + out.indent;
  out.x2 = w() - (DEFAULT_INDENT * 2) + hscroll;
  out.content = false;
  out.measure = false;
  out.exposed = exposed();
  out.tableLevel = 0;
  out.imgY = -1;
  out.imgIndent = out.indent;
  out.tabW = out.x2;
  out.tabH = out.y2;
  out.selection = 0;
  out.selected = (markX != pointX || markY != pointY);
  out.insideCode = false;
  out.insideUl = false;

  if (Fl::event_clicks() == 1 && damage() == DAMAGE_HIGHLIGHT) {
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
      out.selection->clear();
    }
  }
  // must call setfont() before getascent() etc
  fl_font(out.font, out.fontSize);
  out.y1 = y() + fl_height();
  out.lineHeight = fl_height() + fl_descent();
  out.y1 += vscroll;

  fl_push_clip(x(), y(), w() - SCROLL_X, h());
  bool havePushedAnchor = false;
  if (pushedAnchor && (damage() == DAMAGE_PUSHED)) {
    // just draw the anchor-push
    int h = (pushedAnchor->y2 - pushedAnchor->y1) + pushedAnchor->lineHeight;
    fl_push_clip(x(), y() + pushedAnchor->y1, out.x2, h);
    havePushedAnchor = true;
  }
  // draw the background
  fl_color(out.background);
  fl_rectf(x(), y(), w() - SCROLL_X, h());
  fl_color(out.color);

  out.background = NO_COLOR;

  // hide any inputs
  List_each(InputNode *, it, inputs) {
    InputNode *p = (*it);
    if (p->button) {
      p->button->clear_visible();
    }
  }

  int id = 0;
  List_each(BaseNode *, it, nodeList) {
    BaseNode *p = (*it);
    out.nodeId = id;
    p->display(&out);
    if (out.nodeId < id) {
      // perform second pass on previous outer table
      MeasureNode *node = (MeasureNode *)nodeList[out.nodeId];
      out.x1 = node->initX;
      out.y1 = node->initY;
      out.exposed = false;
      for (int j = out.nodeId; j <= id; j++) {
        p = nodeList[j];
        out.nodeId = j;
        p->display(&out);
      }
      out.exposed = exposed();
    }
    if (out.exposed == false && out.tableLevel == 0 && out.y1 - out.lineHeight > out.y2) {
      // clip remaining content
      break;
    }
    id++;
  }

  if (out.exposed) {
    // size has changed or need to recalculate scrollbar
    int pageHeight = (out.y1 - vscroll) + out.lineHeight;
    int windowHeight = h() - out.lineHeight;
    int scrollH = pageHeight;
    if (scrollH != scrollHeight || windowHeight != scrollWindowHeight) {
      scrollWindowHeight = windowHeight;
      scrollHeight = scrollH;
      if (scrollHeight < scrollWindowHeight) {
        // nothing to scroll
        scrollHeight = 0;
        scrollbar->deactivate();
        scrollbar->value(0, 1, 0, 1);
      } else {
        scrollbar->activate();
        scrollbar->value(-vscroll, scrollWindowHeight, 0, scrollHeight);
        scrollbar->linesize(out.lineHeight);
      }
      scrollbar->redraw();
    }
  }
  if (havePushedAnchor) {
    fl_pop_clip();
  }
  fl_pop_clip();

  // draw child controls
  draw_child(*scrollbar);

  // prevent other child controls from drawing over the scrollbar
  fl_push_clip(x(), y(), w() - SCROLL_X, h());
  int numchildren = children();
  for (int n = 0; n < numchildren; n++) {
    Fl_Widget &w = *child(n);
    if (&w != scrollbar) {
      draw_child(w);
    }
  }
  fl_pop_clip();
}

void HelpWidget::compile() {
  uint8_t pre = !isHtmlFile();
  uint8_t bold = false;
  uint8_t italic = false;
  uint8_t center = false;
  uint8_t uline = false;
  Fl_Color color = 0;
  Fl_Font font = FL_HELVETICA;
  int fontSize = (int)labelsize();
  int taglen = 0;
  int textlen = 0;
  bool padlines = false;          // padding between line-breaks

  strlib::Stack<TableNode *> tableStack(5);
  strlib::Stack<TrNode *> trStack(5);
  strlib::Stack<TdNode *> tdStack(5);
  strlib::Stack<UlNode *> olStack(5);
  strlib::Stack<CodeNode *> codeStack(5);
  strlib::List<String *> options(5);
  Attributes p(5);
  strlib::String *prop;
  BaseNode *node;
  InputNode *inputNode;

  const char *text = htmlStr.c_str();
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
              node = new ImageNode(&ellipseImage);
              nodeList.add(node);
            } else if (ch > 129 && ch < 256) {
              node = new TextNode(&entityMap[ch].xlat, 1);
              nodeList.add(node);
            }
            pindex = i;
            p = text + pindex;
          } else {
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
          }
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
          while (i < textlen && (IS_WHITE(text[i + 1]))) {
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
        if (0 == strncasecmp(tag, "b>", 2)) {
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
                   0 == strncasecmp(tag, "blockquote", 10) ||
                   0 == strncasecmp(tag, "h", 1)) {     // </h1>
          if (0 == strncasecmp(tag, "h", 1)) {
            if (bold > 0) {
              bold--;
            }
            padlines = false;
          }
          color = (0 == strncasecmp(tag, "f", 1) ? (Fl_Color) - 1 : 0);
          font = FL_HELVETICA;
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
          nodeList.add(new TdEndNode((TdNode *)tdStack.pop()));
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "tr", 2)) {
          node = new TrEndNode((TrNode *)trStack.pop());
          nodeList.add(node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "table", 5)) {
          node = new TableEndNode((TableNode *)tableStack.pop());
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
          strlib::String *option = new String();
          option->append(tagPair, tagBegin - tagPair);
          options.add(option);  // continue scan for more options
        } else if (0 == strncasecmp(tag, "title", 5) && tagPair) {
          title.clear();
          title.append(tagPair, tagBegin - tagPair);
          tagPair = 0;
        } else if (0 == strncasecmp(tag, "code", 4)) {
          node = new CodeEndNode((CodeNode *)codeStack.pop());
          nodeList.add(node);
        } else if (0 == strncasecmp(tag, "script", 6) ||
                   0 == strncasecmp(tag, "style", 5)) {
          tagPair = 0;
        }
      } else if (isalpha(tag[0]) || tag[0] == '!') {
        // process the start of the tag
        if (0 == strncasecmp(tag, "br", 2)) {
          nodeList.add(new BrNode(pre));
          padlines = false;
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "p>", 2)) {
          nodeList.add(new ParagraphNode());
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
          node = new FontNode(FL_COURIER, fontSize, 0, bold, italic);
          nodeList.add(node);
          nodeList.add(new BrNode(pre));
        } else if (0 == strncasecmp(tag, "code", 4)) {
          node = new CodeNode(fontSize);
          codeStack.push((CodeNode *)node);
          nodeList.add(node);
        } else if (0 == strncasecmp(tag, "td", 2)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new TdNode((TrNode *)trStack.peek(), &p);
          nodeList.add(node);
          tdStack.push((TdNode *)node);
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "tr", 2)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new TrNode((TableNode *)tableStack.peek(), &p);
          nodeList.add(node);
          trStack.push((TrNode *)node);
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "table", 5)) {
          p.removeAll();
          p.load(tag + 5, taglen - 5);
          node = new TableNode(&p);
          nodeList.add(node);
          tableStack.push((TableNode *)node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
          // continue the font in case we resize
          node = new FontNode(font, fontSize, 0, bold, italic);
          nodeList.add(node);
          prop = p.getBackground();
          if (prop != NULL) {
            node = new ImageNode(&docHome, prop, false);
            nodeList.add(node);
            images.add((ImageNode *)node);
          }
        } else if (0 == strncasecmp(tag, "ul>", 3) ||
                   0 == strncasecmp(tag, "ol ", 3) ||
                   0 == strncasecmp(tag, "ol>", 3)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new UlNode(p, tag[0] == 'o' || tag[0] == 'O');
          olStack.push((UlNode *)node);
          nodeList.add(node);
          padlines = false;
        } else if (0 == strncasecmp(tag, "u>", 2)) {
          uline = true;
          nodeList.add(new StyleNode(uline, center));
        } else if (0 == strncasecmp(tag, "li>", 3)) {
          node = new LiNode((UlNode *)olStack.peek());
          nodeList.add(node);
          padlines = false;
          text = skipWhite(tagEnd + 1);
        } else if (0 == strncasecmp(tag, "a ", 2)) {
          p.removeAll();
          p.load(tag + 2, taglen - 2);
          node = new AnchorNode(p);
          nodeList.add(node);
          anchors.add((AnchorNode *)node);
        } else if (0 == strncasecmp(tag, "font ", 5)) {
          p.removeAll();
          p.load(tag + 5, taglen - 5);
          color = getColor(p.get("color"), 0);
          prop = p.get("font-size");
          if (prop != NULL) {
            // convert from points to pixels
            float h, v;
            Fl::screen_dpi(h, v);
            fontSize = (int)(prop->toInteger() * v  / 72.0);
          } else {
            prop = p.get("size");
            if (prop != NULL) {
              fontSize = (int)labelsize() + (prop->toInteger() - 1);
            }
          }
          prop = p.get("face");
          if (prop != NULL) {
            font = get_font(prop->c_str());
          }
          node = new FontNode(font, fontSize, color, bold, italic);
          nodeList.add(node);
        } else if (taglen >= 2 && 0 == strncasecmp(tag, "h", 1)) {
          // H1-H6 from large to small
          int size = FONT_SIZE_H1 - ((tag[1] - '1') * 2);
          nodeList.add(new FontNode(font, size, 0, ++bold, italic));
          padlines = false;
        } else if (0 == strncasecmp(tag, "blockquote", 10)) {
          nodeList.add(new FontNode(font, fontSize, 0, true, true));
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
          node = new ImageNode(&docHome, &p);
          nodeList.add(node);
          images.add((ImageNode *)node);
        } else if (0 == strncasecmp(tag, "body ", 5)) {
          p.removeAll();
          p.load(tag + 5, taglen - 5);
          text = skipWhite(tagEnd + 1);
          foreground = getColor(p.getFgColor(), foreground);
          background = getColor(p.getBgColor(), background);
          prop = p.getBackground();
          if (prop != NULL) {
            node = new ImageNode(&docHome, prop, true);
            nodeList.add(node);
            images.add((ImageNode *)node);
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
  codeStack.clear();
  olStack.clear();
  tdStack.clear();
  trStack.clear();
  while (tableStack.peek()) {
    node = new TableEndNode((TableNode *)tableStack.pop());
    nodeList.add(node);
  }
}

// handle click from form button
void HelpWidget::onclick(Fl_Widget *button) {
  List_each(InputNode *, it, inputs) {
    InputNode *p = (*it);
    if (p->button == button) {
      this->event.clear();
      this->event.append(p->onclick.c_str());
      user_data((void *)this->event.c_str());
      do_callback();
      return;
    }
  }
}

int HelpWidget::onMove(int event) {
  int ex = Fl::event_x();
  int ey = Fl::event_y();

  if (pushedAnchor && event == FL_DRAG) {
    bool pushed = pushedAnchor->ptInSegment(ex, ey);
    if (pushedAnchor->pushed != pushed) {
      fl_cursor(FL_CURSOR_HAND);
      pushedAnchor->pushed = pushed;
      damage(DAMAGE_PUSHED);
    }
    return 1;
  } else {
    List_each(AnchorNode *, it, anchors) {
      AnchorNode *p = (*it);
      if (p->ptInSegment(ex, ey)) {
        fl_cursor(FL_CURSOR_HAND);
        return 1;
      }
    }
    fl_cursor(FL_CURSOR_DEFAULT);
  }

  int vscroll = -scrollbar->value();
  if (event == FL_DRAG) {
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
        int16_t scroll = vscroll + (ey - scrollY);
        scrollY = ey;
        if (scroll > 0) {
          scroll = 0;           // too far up
        } else if (-scroll > scrollHeight) {
          scroll = -scrollHeight;       // too far down
        }
        if (scroll != vscroll) {
          vscroll = scroll;
          damage(FL_DAMAGE_EXPOSE);
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
  pushedAnchor = 0;
  int ex = Fl::event_x();
  int ey = Fl::event_y();
  int vscroll = -scrollbar->value();
  int16_t scroll = vscroll;

  List_each(AnchorNode *, it, anchors) {
    AnchorNode *p = (*it);
    if (p->ptInSegment(ex, ey)) {
      pushedAnchor = p;
      pushedAnchor->pushed = true;
      fl_cursor(FL_CURSOR_HAND);
      damage(DAMAGE_PUSHED);
      return 1;
    }
  }

  switch (mouseMode) {
  case mm_select:
    // begin/continue text selection
    if (Fl::event_state(FL_SHIFT)) {
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
    if (scroll != scrollbar->value()) {
      scrollbar->value(scroll);
      damage(FL_DAMAGE_EXPOSE);
    }
    break;
  }
  return 1;                     // return 1 to become the belowmouse
}

int HelpWidget::handleKeys() {
  int result = 0;
  switch (Fl::event_key()) {
  case FL_Right:
    if (-hscroll < w() / 2) {
      hscroll -= HSCROLL_STEP;
      redraw();
      result = 1;
    }
    break;
  case FL_Left:
    if (hscroll < 0) {
      hscroll += HSCROLL_STEP;
      redraw();
      result = 1;
    }
    break;
  default:
    break;
  }

  if (!result && Fl::event_state(FL_CTRL)) {
    result = 1;
    switch (Fl::event_key()) {
    case 'u':
      vscroll(-scrollWindowHeight);
      break;
    case 'd':
      vscroll(scrollWindowHeight);
      break;
    case 'r':
      reloadPage();
      break;
    case 'f':
      find(fl_input("Find:"), false);
      break;
    case 'a':
      selectAll();
      break;
    case FL_Insert:
    case 'c':
      copySelection();
      break;
    case 'b':
    case 'q':
      if (Fl::modal() == parent()) {
        Fl::modal()->set_non_modal();
      }
      break;
    default:
      result = 0;
      break;
    }
  }
  return result;
}

int HelpWidget::handle(int event) {
  int handled = Fl_Group::handle(event);
  if (handled && event != FL_MOVE) {
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
    find(fl_input("Find:"), false);
    return 1;

  case FL_SHOW:
    take_focus();
    break;

  case FL_FOCUS:
    return 1;                   // aquire focus

  case FL_PUSH:
    if (Fl::event_x() < w() - SCROLL_X) {
      return onPush(event);
    }
    break;

  case FL_ENTER:
    return 1;

  case FL_KEYDOWN:
    if (handleKeys()) {
      return 1;
    }
    break;

  case FL_DRAG:
  case FL_MOVE:
    return onMove(event);

  case FL_RELEASE:
    if (pushedAnchor) {
      fl_cursor(FL_CURSOR_DEFAULT);
      bool pushed = pushedAnchor->pushed;
      pushedAnchor->pushed = false;
      damage(DAMAGE_PUSHED);
      if (pushed) {
        this->event.clear();
        this->event.append(pushedAnchor->href.c_str());
        if (this->event.length()) {
          // href has been set
          user_data((void *)this->event.c_str());
          do_callback();
        }
      }
      return 1;
    }
  }

  return scrollbar->active() ? scrollbar->handle(event) : 0;
}

bool HelpWidget::find(const char *s, bool matchCase) {
  if (s == 0 || s[0] == 0) {
    return false;
  }

  int foundRow = 0;
  int lineHeight = fl_height() + fl_descent();

  List_each(BaseNode *, it, nodeList) {
    BaseNode *p = (*it);
    if (p->indexOf(s, matchCase) != -1) {
      foundRow = p->getY() - scrollbar->value();
      if (foundRow > -scrollbar->value() + lineHeight) {
        break;
      }
    }
  }

  int scroll = scrollbar->value();
  if (-scroll == foundRow) {
    return false;
  }

  scroll = foundRow;

  // check scroll bounds
  if (foundRow) {
    scroll += lineHeight;
  }
  if (-scroll > scrollHeight) {
    scroll = -scrollHeight;
  }

  scrollbar->value(scroll);
  redraw();
  return true;
}

void HelpWidget::copySelection() {
  Fl::copy(selection.c_str(), selection.length(), true);
}

void HelpWidget::selectAll() {
  markX = markY = 0;
  pointX = w();
  pointY = scrollHeight + h();
  selection.clear();
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
  loadFile(path.c_str());
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

  fileName.clear();
  htmlStr.clear();

  if (docHome.length() != 0 && useDocHome) {
    fileName.append(docHome);
  }

  if (strncasecmp(f, "file:///", 8) == 0) {
    // only supports file protocol
    f += 8;
  }

  const char *target = strrchr(f, '#');
  long len = target != NULL ? target - f : strlen(f);
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
  if ((fp = fopen(fileName.c_str(), "rb")) != NULL) {
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    rewind(fp);
    htmlStr.append(fp, len);
    fclose(fp);
  } else {
    htmlStr.append("File not found: \"");
    htmlStr.append(fileName.c_str());
    htmlStr.append("\" - ");
    htmlStr.append(strerror(errno));
  }

  reloadPage();
  if (target) {
    // draw to obtain dimensions
    Fl::flush();
    scrollTo(target + 1);
  }
}

// reload broken images
void HelpWidget::reloadImages() {
  List_each(ImageNode *, it, images) {
    ImageNode *imageNode = (*it);
    if (imageNode->image == &brokenImage) {
      imageNode->reload();
    }
  }
  redraw();
}

void HelpWidget::setDocHome(const char *s) {
  docHome.clear();
  docHome.append(s);
  if (s && s[strlen(s) - 1] != '/') {
    docHome.append("/");
  }
}

const char *HelpWidget::getAnchor(int index) {
  int len = anchors.size();
  if (index < len && index > -1) {
    return anchors[index]->href.c_str();
  }
  return NULL;
}

void HelpWidget::getText(strlib::String *s) {
  List_each(BaseNode *, it, nodeList) {
    BaseNode *p = (*it);
    p->getText(s);
  }
}

bool HelpWidget::isHtmlFile() {
  const char *filename = fileName.c_str();
  if (!fileName || !fileName[0]) {
    return false;
  }
  int len = strlen(filename);
  return (strcasecmp(filename + len - 4, ".htm") == 0 ||
          strcasecmp(filename + len - 5, ".html") == 0);
}

void HelpWidget::vscroll(int offs) {
  if (scrollbar->active()) {
    int value = scrollbar->value() + offs;
    scrollbar->value(value);
  }
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
    linepx = fl_width(s, slen);
    return;
  }
  // find the last break-point within the available width
  txtWidth = fl_width(s, i);
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
    txtWidth += fl_width(s + ibreak, i - ibreak);
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
  while (IS_WHITE(*s)) {
    s++;
  }
  return s;
}

Fl_Color getColor(strlib::String *s, Fl_Color def) {
  Fl_Color result;
  if (s != NULL && s->length()) {
    result = get_color(s->c_str(), def);
  } else {
    result = def;
  }
  return result;
}

// image factory based on file extension
Fl_Shared_Image *loadImage(const char *name, uchar *buff) {
  return Fl_Shared_Image::get(name);
}

Fl_Image *loadImage(const char *imgSrc) {
  if (imgSrc == 0 || access(imgSrc, 0) != 0) {
    return &brokenImage;
  }
  Fl_Image *image = loadImage(imgSrc, 0);
  return image != 0 ? image : &brokenImage;
}

void browseFile(const char *url) {
  fl_open_uri(url, nullptr, 0);
}
