// -*- c-file-style: "java" -*-
// $Id: Fl_Ansi_Window.cpp,v 1.19 2005-01-17 19:55:35 zeeb90au Exp $
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ctype.h>
#include <string.h>

#include <fltk/layout.h>
#include <fltk/Image.h>
#include <fltk/events.h>
#include <fltk/Font.h>

#include "Fl_Ansi_Window.h"

#if defined(WIN32) 
#include <fltk/win32.h>
#include <wingdi.h>
extern HDC fl_bitmap_dc;
#endif

#define INITXY 2

using namespace fltk;

// uncomment for unit testing and then run:
// make Fl_Ansi_Window.exe
//#define UNIT_TEST 1

#define begin_offscreen()   \
  initImage();              \
  ImageDraw imageDraw(img); \
  setfont(labelfont(), labelsize());

#define end_offscreen() redraw();

Fl_Ansi_Window::Fl_Ansi_Window(int x, int y, int w, int h) : 
    Widget(x, y, w, h, 0) {
    font_size = 11;
    init();
    img = 0;
}

Fl_Ansi_Window::~Fl_Ansi_Window() {
    destroyImage();
}

void Fl_Ansi_Window::destroyImage() {
    if (img) {
        img->destroy_cache();
        delete img;
        img = 0;
    }
}

void Fl_Ansi_Window::initImage() {
    // can only be called following Fl::check() or Fl::run()
    if (img == 0) {
        img = new Image(w(), h());
        ImageDraw imageDraw(img);
        setcolor(color());
        fillrect(0, 0, w(), h());
        setfont(labelfont(), labelsize());
    }
}

void Fl_Ansi_Window::init() {
    curY = INITXY; // allow for input control border
    curX = INITXY;
    tabSize = 40; // tab size in pixels (160/32 = 5)
    reset();
}

void Fl_Ansi_Window::reset() {
    curYSaved = 0;
    curXSaved = 0;
    invert = false;
    underline = false;
    bold = false;
    italic = false;
    color(WHITE); // bg
    labelcolor(BLACK); // fg
    labelfont(COURIER);
    labelsize(font_size);
}

void Fl_Ansi_Window::layout() {
    if (img && (layout_damage() & LAYOUT_WH)) { 
        int W = img->w();
        int H = img->h();
        if (w() > W) {
            W = w();
        }
        if (h() > H) {
            H = h();
        }

        Image* old = img;
        img = new Image(W, H);
        ImageDraw imageDraw(img);
        setcolor(color());
        fillrect(0, 0, W, H);
        setfont(labelfont(), labelsize());
        old->draw(0, 0, old->w(), old->h(), 0, OUTPUT);
        old->destroy_cache();
        delete old;
        redraw();
    }
    Widget::layout();
}

void Fl_Ansi_Window::draw() {
    if (img) {
        img->draw(0, 0, w(), h(), 0, OUTPUT);
    } else {
        setcolor(color());
        fillrect(0, 0, w(), h());
    }
}

void Fl_Ansi_Window::clearScreen() {
    if (img != 0) {
        init();
        begin_offscreen();
        setcolor(color());
        fillrect(0, 0, w(), h());
        end_offscreen();
    }
}

void Fl_Ansi_Window::setTextColor(long fg, long bg) {
    labelcolor(ansiToFltk(fg));
    color(ansiToFltk(bg));
}

void Fl_Ansi_Window::setColor(long fg) {
    labelcolor(ansiToFltk(fg));
}

void Fl_Ansi_Window::drawLine(int x1, int y1, int x2, int y2) {
    begin_offscreen();
    setcolor(labelcolor());
    drawline(x1, y1, x2, y2);
    end_offscreen();
}

void Fl_Ansi_Window::drawRectFilled(int x1, int y1, int x2, int y2) {
    begin_offscreen();
    setcolor(labelcolor());
    fillrect(x1, y1, x2-x1, y2-y1);
    end_offscreen();
}

void Fl_Ansi_Window::drawRect(int x1, int y1, int x2, int y2) {
    begin_offscreen();
    setcolor(labelcolor());
    drawline(x1, y1, x1, y2);
    drawline(x1, y2, x2, y2);
    drawline(x2, y2, x2, y1);
    drawline(x2, y1, x1, y1);
    end_offscreen();
}

void Fl_Ansi_Window::drawImage(Image* image, int x, int y, int sx, int sy, 
                               int width, int height) {
    begin_offscreen();
    image->copy(x, y, width, height, sx, sy);
    end_offscreen();
}

void Fl_Ansi_Window::setPixel(int x, int y, int c) {
#if defined(WIN32) 
    begin_offscreen();
    if (c < 0) {
        ::SetPixel(fl_bitmap_dc, x,y, -c);
    } else {
        Color fltkColor = ansiToFltk(c);
        int r = (fltkColor>>24) & 0xFF;
        int g = (fltkColor>>16) & 0xFF;
        int b = (fltkColor>>8) & 0xFF;
        ::SetPixel(fl_bitmap_dc, x,y, RGB(r,g,b));
    }
    end_offscreen();
#else
    // TODO: fix linux set/get pixel
    // XDrawPoint(x_disp, x_win, x_gc, x, y);
#endif
}

int Fl_Ansi_Window::getPixel(int x, int y) {
#if defined(WIN32) 
    begin_offscreen();
    COLORREF c = ::GetPixel(fl_bitmap_dc, x, y);
    end_offscreen();
    return c;
#else
    return 0;
#endif
}

int Fl_Ansi_Window::textWidth(const char* s) {
    begin_offscreen();
    int w = (int)getwidth(s);
    end_offscreen();
    return w;
}

int Fl_Ansi_Window::textHeight(void) {
    begin_offscreen();
    int h = (int)(getascent()+getdescent());
    end_offscreen();
    return h;
}

// callback for fl_scroll
void eraseBottomLine(void* data, int x, int y, int w, int h) {
    Fl_Ansi_Window* out = (Fl_Ansi_Window*)data;
    setcolor(out->color());
    fillrect(x, y, w, h);
}

void Fl_Ansi_Window::newLine() {
    int height = h();
    int fontHeight = (int)(getascent()+getdescent());

    curX = INITXY;
    if (curY+(fontHeight*2) >= height) {
        scrollrect(0, 0, w(), height, 0, -fontHeight, eraseBottomLine, this);
        // TODO: patched is_visible() in fl_scroll_area.cxx 
        // commented out OffsetRgn()
    } else {
        curY += fontHeight;
    }
}

int Fl_Ansi_Window::calcTab(int x) const {
    int c = 1;
    while (x > tabSize) {
        x -= tabSize;
        c++;
    }
    return c * tabSize;
}

Color Fl_Ansi_Window::ansiToFltk(long c) const {
    if (c < 0) {
        // windows style RGB packing
        int r = (c) & 0xFF;
        int g = (c>>8) & 0xFF;
        int b = (c>>16) & 0xFF;
        return fltk::color(r, g, b);
    }

    // see: http://www.uv.tietgen.dk/staff/mlha/PC/Soft/Prog/BAS/VB/Function.html
    switch (c) {
    case 0:  return fltk::BLACK;             // 0 black
    case 1:  return fltk::color(0,0,128);    // 1 blue
    case 2:  return fltk::color(0,128,0);    // 2 green
    case 3:  return fltk::color(0,128,128);  // 3 cyan
    case 4:  return fltk::color(128,0,0);    // 4 red
    case 5:  return fltk::color(128,0,128);  // 5 magenta
    case 6:  return fltk::color(128,128,0);  // 6 yellow
    case 7:  return fltk::color(192,192,192);// 7 white
    case 8:  return fltk::color(128,128,128);// 8 gray
    case 9:  return fltk::color(0,0,255);    // 9 light blue
    case 10: return fltk::color(0,255,0);    // 10 light green
    case 11: return fltk::color(0,255,255);  // 11 light cyan
    case 12: return fltk::color(255,0,0);    // 12 light red
    case 13: return fltk::color(255,0,255);  // 13 light magenta
    case 14: return fltk::color(255,255,0);  // 14 light yellow
    default: return fltk::WHITE;             // 15 bright white
    }
}

bool Fl_Ansi_Window::setGraphicsRendition(char c, int escValue) {
    switch (c) {
    case 'K': // \e[K - clear to eol
        setcolor(color());
        fillrect(curX, curY, w()-curX, (int)(getascent()+getdescent()));
        break;
    case 'G': // move to column
        curX = escValue;
        break;
    case 'T': // non-standard: move to n/80th of screen width
        curX = escValue*w()/80;
        break;
    case 's': // save cursor position
        curYSaved = curX;
        curXSaved = curY;
        break;
    case 'u': // restore cursor position
        curX = curYSaved;
        curY = curXSaved;
        break;
    case ';': // fallthru
    case 'm': // \e[...m  - ANSI terminal
        switch (escValue) {
        case 0:  // reset
            reset();
            break;
        case 1: // set bold on
            bold = true;
            return true;
        case 2: // set faint on
            break;
        case 3: // set italic on
            italic = true;
            return true;
        case 4: // set underline on
            underline = true;
            break;
        case 5: // set blink on
            break;
        case 6: // rapid blink on
            break;
        case 7: // reverse video on
            invert = true;
            break;
        case 8: // conceal on
            break;
        case 21: // set bold off
            bold = false;
            return true;
        case 23:
            italic = false;
            return true;
        case 24: // set underline off
            underline = false;
            break;
        case 27: // reverse video off
            invert = false;
            break;
            // colors - 30..37 foreground, 40..47 background
        case 30: // set black fg
            labelcolor(ansiToFltk(0));
            return true;
        case 31: // set red fg
            labelcolor(ansiToFltk(4));
            return true;
        case 32: // set green fg
            labelcolor(ansiToFltk(2));
            return true;
        case 33: // set yellow fg
            labelcolor(ansiToFltk(6));
            return true;
        case 34: // set blue fg
            labelcolor(ansiToFltk(1));
            return true;
        case 35: // set magenta fg
            labelcolor(ansiToFltk(5));
            return true;
        case 36: // set cyan fg
            labelcolor(ansiToFltk(3));
            return true;
        case 37: // set white fg
            labelcolor(ansiToFltk(7));
            return true;
        case 40: // set black bg
            color(ansiToFltk(0));
            return true;
        case 41: // set red bg
            color(ansiToFltk(4));
            return true;
        case 42: // set green bg
            color(ansiToFltk(2));
            return true;
        case 43: // set yellow bg
            color(ansiToFltk(6));
            return true;
        case 44: // set blue bg
            color(ansiToFltk(1));
            return true;
        case 45: // set magenta bg
            color(ansiToFltk(5));
            return true;
        case 46: // set cyan bg
            color(ansiToFltk(3));
            return true;
        case 47: // set white bg
            color(ansiToFltk(15));
            return true;
        case 48: // subscript on
            break;
        case 49: // superscript
            break;
        };                        
    }
    return false;
}

bool Fl_Ansi_Window::doEscape(unsigned char* &p) {
    int escValue = 0;
    while (isdigit(*p)) {
        escValue = (escValue * 10) + (*p - '0');
        p++;
    }

    if (setGraphicsRendition(*p, escValue)) {
        Font* font = labelfont();
        if (bold) {
            font = font->bold();
        }
        if (italic) {
            font = font->italic();
        }
        setfont(font, labelsize());
    }
    
    if (*p == ';') {
        p++; // next rendition
        return true;
    }
    return false;
}

/**
 *   Supported control codes:
 *   \t      tab (20 px)
 *   \a      beep
 *   \r      return
 *   \n      next line
 *   \xC     clear screen
 *   \e[K    clear to end of line
 *   \e[0m   reset all attributes to their defaults
 *   \e[1m   set bold on
 *   \e[4m   set underline on
 *   \e[7m   reverse video
 *   \e[21m  set bold off
 *   \e[24m  set underline off
 *   \e[27m  set reverse off
 */
void Fl_Ansi_Window::print(const char *str) {
    int len = strlen(str);
    if (len <= 0) {
        return;
    }

    begin_offscreen();
    int ascent = (int)getascent();
    int fontHeight = (int)(ascent+getdescent());
    unsigned char *p = (unsigned char*)str;

    while (*p) {
        switch (*p) {
        case '\a':   // beep
            //fl_beep();
            break;
        case '\t':
            curX = calcTab(curX+1);
            break;
        case '\xC':
            init();
            setcolor(color());
            fillrect(0, 0, w(), h());
            break;
        case '\033':  // ESC ctrl chars
            if (*(p+1) == '[' ) {
                p += 2;
                while(true) {
                    if (!doEscape(p)) {
                        break;
                    }
                }
            }
            break;
        case '\n': // new line
            newLine();
            break;
        case '\r': // return
            curX = INITXY;
            setcolor(color());
            fillrect(0, curY, w(), fontHeight);
            break;
        default:
            int numChars = 1; // print minimum of one character
            int cx = (int)getwidth((const char*)p, 1);
            int width = w()-1;

            if (curX + cx >= width) {
                newLine();
            }

            // print further non-control, non-null characters 
            // up to the width of the line
            while (p[numChars] > 31) {
                cx += (int)getwidth((const char*)p+numChars, 1);
                if (curX + cx < width) {
                    numChars++;
                } else {
                    break;
                }
            }
            
            if (invert) {
                setcolor(labelcolor());
                fillrect(curX, curY, cx, fontHeight);
                setcolor(color());
                drawtext((const char*)p, numChars, curX, curY+ascent);
            } else {
                setcolor(color());
                fillrect(curX, curY, cx, fontHeight);
                setcolor(labelcolor());
                drawtext((const char*)p, numChars, curX, curY+ascent);
            }

            if (underline) {
                drawline(curX, curY+ascent+1, curX+cx, curY+ascent+1);
            }
            
            // advance
            p += numChars-1; // allow for p++ 
            curX += cx;
        };
        
        if (*p == '\0') {
            break;
        }
        p++;
    }

    end_offscreen();
}

int Fl_Ansi_Window::handle(int e) {
    if (e == FOCUS) {
        return 2;
    }
    return Widget::handle(e);
}

#ifdef UNIT_TEST
#include <fltk/run.h>
int main(int argc, char **argv) {
    int w = 210; // must be > 104
    int h = 200;
    Window window(w, h, "SmallBASIC");
    window.begin();
    Fl_Ansi_Window out(0, 0, w, h);
    window.resizable(&out);
    window.end();
    window.show(argc,argv);
    check();
    out.print("1\n2\n3\n4\n5\n6\n7\n8\n");
    out.print("1\n2\n3\n4\n5\n6 six\n7 sevent\neight");
    out.print("what the!\rhuh\033[Kclear");
    for (int i=0; i<100; i++) {
        out.print("\033[3mitalic\033[23moff\033[4munderline\033[24moff");
        out.print("\033[7minverse\033[27moff");
        out.print("\033[1mbold\033[21moff");
    }
    return run();
}
#endif

