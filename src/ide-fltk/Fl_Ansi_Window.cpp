// -*- c-file-style: "java" -*-
// $Id: Fl_Ansi_Window.cpp,v 1.2 2004-10-28 23:34:20 zeeb90au Exp $
// This file is part of EBjLib
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Return_Button.H>
#include <FL/x.H>
#include <ctype.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Fl_Ansi_Window.h"

// uncomment for unit testing and then run:
// make Fl_Ansi_Window.exe
#define UNIT_TEST 1

#define begin_offscreen()                       \
    initOffscreen();                            \
    fl_begin_offscreen(img);

#define end_offscreen()                         \
    fl_end_offscreen();                         \
    redraw();

Fl_Ansi_Window::Fl_Ansi_Window(int x, int y, int w, int h) : 
    Fl_Widget(x, y, w, h, 0) {
    init();
}

Fl_Ansi_Window::~Fl_Ansi_Window() {
    if (img) {
        fl_delete_offscreen(img);
    }
}

void Fl_Ansi_Window::init() {
    if (img) {
        fl_delete_offscreen(img);
    }
    img = 0;
    curY = 0;
    curX = 0;
    tabSize = 40; // tab size in pixels (160/32 = 5)
    reset();
}

void Fl_Ansi_Window::initOffscreen() {
    // can only be called following Fl::check() or Fl::run()
    if (img == 0) {
        img = fl_create_offscreen(w(), h());
        fl_begin_offscreen(img);
        fl_color(color());
        fl_rectf(0, 0, w(), h());
        fl_color(labelcolor());
        fl_font(labelfont(), labelsize());
        curY = fl_height();
        fl_end_offscreen();
    }
}

void Fl_Ansi_Window::reset() {
    curYSaved = 0;
    curXSaved = 0;
    invert = false;
    underline = false;
    color(FL_WHITE); // bg
    labelcolor(FL_BLACK); // fg
    labelfont(FL_COURIER);
    labelsize(11);
}

void Fl_Ansi_Window::resize(int x, int y, int width, int height) {
    Fl_Widget::resize(x, y, width, height);
    if (img) {
        // preserve old image
        Fl_Offscreen old = img;
        img = 0;
        begin_offscreen();
        fl_copy_offscreen(0, 0, w(), h(), old, 0, 0);
        end_offscreen();
        fl_delete_offscreen(old);
    }
}

void Fl_Ansi_Window::draw() {
    if (img) {
        fl_copy_offscreen(0, 0, w(), h(), img, 0, 0);
    }
}

void Fl_Ansi_Window::drawLine(int x1, int y1, int x2, int y2) {
    begin_offscreen();
    fl_line(x1, y1, x2, y2);
    end_offscreen();
}

void Fl_Ansi_Window::drawFGRectFilled(int x, int y, int width, int height) {
    begin_offscreen();
    fl_rectf(x, y, width, height, labelcolor());
    end_offscreen();
}

void Fl_Ansi_Window::drawBGRectFilled(int x, int y, int width, int height) {
    begin_offscreen();
    fl_rectf(x, y, width, height, color());
    end_offscreen();
}

void Fl_Ansi_Window::drawFGRect(int x, int y, int width, int height) {
    begin_offscreen();
    fl_rect(x, y, width, height, labelcolor());
    end_offscreen();
}

void Fl_Ansi_Window::drawBGRect(int x, int y, int width, int height) {
    begin_offscreen();
    fl_rect(x, y, width, height, color());
    end_offscreen();
}

void Fl_Ansi_Window::clearScreen() {
    init();
    begin_offscreen();
    fl_rectf(0, 0, w(), h(), color());
    end_offscreen();
}

void Fl_Ansi_Window::saveScreen() {
}

void Fl_Ansi_Window::restoreScreen() {
    redraw();
}

// callback for fl_scroll
void eraseBottomLine(void* data, int x, int y, int w, int h) {
    Fl_Ansi_Window* out = (Fl_Ansi_Window*)data;
    fl_rectf(x, y, w, h, out->color());
}

void Fl_Ansi_Window::newLine() {
    int height = h();
    int width = w();
    int fontHeight = fl_height();

    curX = 0;
    curY += fontHeight;

    if (curY >= height) {
        fl_scroll(0, 0, width, height, 0, -fontHeight, eraseBottomLine, this);
        curY -= fontHeight;
        fl_color(labelcolor());
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

Fl_Color Fl_Ansi_Window::ansiToFltk(long color) const {
    switch (color) {
    case 0: return FL_BLACK;
    case 1: return FL_RED;
    case 2: return FL_GREEN;
    case 3: return FL_YELLOW;
    case 4: return FL_BLUE;
    case 5: return FL_MAGENTA;
    case 6: return FL_CYAN;
    case 7: return FL_DARK_RED;
    case 8: return FL_DARK_GREEN;
    case 9: return FL_DARK_YELLOW;
    case 10: return FL_DARK_BLUE;
    case 11: return FL_DARK_MAGENTA;
    case 12: return FL_DARK_CYAN;
        // TODO: fix 13,14
    case 13: return FL_DARK_CYAN;
    case 14: return FL_DARK_CYAN;
    default : return FL_WHITE;
    }
}

bool Fl_Ansi_Window::setGraphicsRendition(char c, int escValue) {
    switch (c) {
    case 'K': // \e[K - clear to eol
        fl_rectf(curX, curY, w()-curX, fl_height(), color());
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
            labelfont(labelfont()+FL_BOLD);
            return true;
        case 2: // set faint on
            break;
        case 3: // set italic on
            labelfont(labelfont()+FL_ITALIC);
            return true;
        case 4: // set underline on
            underline = true;
            break;
        case 7: // reverse video on
            invert = true;
            break;
        case 21: // set bold off
            labelfont(labelfont()-FL_BOLD);
            return true;
        case 23:
            labelfont(labelfont()-FL_ITALIC);
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
        case 33: // set brown fg
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
        case 43: // set brown bg
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
            color(ansiToFltk(7));
            return true;
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
        fl_color(labelcolor());
        fl_font(labelfont(), labelsize());
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
    unsigned char *p = (unsigned char*)str;
    while (*p) {
        switch (*p) {
        case '\a':   // beep
            fl_beep();
            break;
        case '\t':
            curX = calcTab(curX+1);
            break;
        case '\xC':
            init();
            fl_rectf(0, 0, w(), h(), color());
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
            curX = 0;
            fl_rectf(0, curY, w(), fl_height(), color());
            break;
        default:
            int numChars = 1; // print minimum of one character
            int cx = (int)fl_width((const char*)p, 1);
            int width = w()-1;
            int fontHeight = fl_height();

            if (curX + cx >= width) {
                newLine();
            }

            // print further non-control, non-null characters 
            // up to the width of the line
            while (p[numChars] > 31) {
                cx += (int)fl_width((const char*)p+numChars, 1);
                if (curX + cx < width) {
                    numChars++;
                } else {
                    break;
                }
            }
            
            if (invert) {
                fl_rectf(curX, curY-fontHeight+fl_descent(), cx,
                         fontHeight, labelcolor());
                fl_color(color());
                fl_draw((const char*)p, numChars, curX, curY);
                fl_color(labelcolor());
            } else {
                fl_draw((const char*)p, numChars, curX, curY);
            }

            if (underline) {
                fl_line(curX, curY+fontHeight-1, curX+cx, curY+fontHeight-1);
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

#ifdef UNIT_TEST
int main(int argc, char **argv) {
    int w = 120;
    int h = 116;

    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);

    Fl_Window window(w, h, "SmallBASIC");
    Fl_Ansi_Window out(0, 0, w, h);
    window.resizable(&out);
    window.end();
    window.show(argc,argv);
    
    Fl::check();
    out.print("\033[1m");
    out.print("the quick brown fox jumps over the lazy dog from the future");
    out.print("\033[21m");
    out.print("the quick brown fox\033[3m jumps over\033[7m the lazye\033[0m dogs back");
    out.print("abcdefghiklmnop enouf text to force it over the line");

    return Fl::run();
}
#endif

