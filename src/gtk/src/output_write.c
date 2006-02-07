 /* -*- c-file-style: "java" -*-
 * $Id: output_write.c,v 1.1 2006-02-07 02:02:09 zeeb90au Exp $
 * This file is part of SmallBASIC
 *
 * Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */ 

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "output_model.h"

extern OutputModel output;

// callback for fl_scroll
void erase_bottom_line(void* data, const fltk::Rectangle& r) {
    AnsiWindow* out = (AnsiWindow*)data;
    setcolor(out->color());
    fillrect(r);
}

void new_line() {
    int height = h();
    int fontHeight = (int)(getascent()+getdescent());

    curX = INITXY;
    if (curY+(fontHeight*2) >= height) {
        scrollrect(Rectangle(w(), height), 0, -fontHeight, erase_bottom_line, this);
        // TODO: patched is_visible() in fl_scroll_area.cxx 
        // commented out OffsetRgn()
    } else {
        curY += fontHeight;
    }
}

int calc_tab(int x) const {
    int c = 1;
    while (x > tabSize) {
        x -= tabSize;
        c++;
    }
    return c * tabSize;
}

Color ansi_to_gtk(long c) {
    if (c < 0) {
        // assume color is windows style RGB packing
        // RGB(r,g,b) ((COLORREF)((BYTE)(r)|((BYTE)(g) << 8)|((BYTE)(b) << 16)))
        c = -c;
        int b = (c>>16) & 0xFF;
        int g = (c>>8) & 0xFF;
        int r = (c) & 0xFF;
        return fltk::color(r, g, b);
    }

    return (c > 16) ? WHITE : colors[c];
}

bool set_graphics_rendition(char c, int escValue) {
    switch (c) {
    case 'K': // \e[K - clear to eol
        setcolor(color());
        fillrect(Rectangle(curX, curY, w()-curX, (int)(getascent()+getdescent())));
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
            labelcolor(ansi_to_gtk(0));
            return true;
        case 31: // set red fg
            labelcolor(ansi_to_gtk(4));
            return true;
        case 32: // set green fg
            labelcolor(ansi_to_gtk(2));
            return true;
        case 33: // set yellow fg
            labelcolor(ansi_to_gtk(6));
            return true;
        case 34: // set blue fg
            labelcolor(ansi_to_gtk(1));
            return true;
        case 35: // set magenta fg
            labelcolor(ansi_to_gtk(5));
            return true;
        case 36: // set cyan fg
            labelcolor(ansi_to_gtk(3));
            return true;
        case 37: // set white fg
            labelcolor(ansi_to_gtk(7));
            return true;
        case 40: // set black bg
            color(ansi_to_gtk(0));
            return true;
        case 41: // set red bg
            color(ansi_to_gtk(4));
            return true;
        case 42: // set green bg
            color(ansi_to_gtk(2));
            return true;
        case 43: // set yellow bg
            color(ansi_to_gtk(6));
            return true;
        case 44: // set blue bg
            color(ansi_to_gtk(1));
            return true;
        case 45: // set magenta bg
            color(ansi_to_gtk(5));
            return true;
        case 46: // set cyan bg
            color(ansi_to_gtk(3));
            return true;
        case 47: // set white bg
            color(ansi_to_gtk(15));
            return true;
        case 48: // subscript on
            break;
        case 49: // superscript
            break;
        };                        
    }
    return false;
}

bool doEscape(unsigned char* &p) {
    int escValue = 0;
    while (isdigit(*p)) {
        escValue = (escValue * 10) + (*p - '0');
        p++;
    }

    if (set_graphics_rendition(*p, escValue)) {
        fltk::Font* font = labelfont();
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
void osd_write(const char *s) {
    int len = strlen(str);
    if (len <= 0) {
        return;
    }

    begin_offscreen();
    setfont(labelfont(), labelsize());
    int ascent = (int)getascent();
    int fontHeight = (int)(ascent+getdescent());
    unsigned char *p = (unsigned char*)str;

    while (*p) {
        switch (*p) {
        case '\a':   // beep
            beep();
            break;
        case '\t':
            curX = calc_tab(curX+1);
            break;
        case '\xC':
            init();
            setcolor(color());
            fillrect(Rectangle(w(), h()));
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
            new_line();
            break;
        case '\r': // return
            curX = INITXY;
            setcolor(color());
            fillrect(Rectangle(0, curY, w(), fontHeight));
            break;
        default:
            int numChars = 1; // print minimum of one character
            int cx = (int)getwidth((const char*)p, 1);
            int width = w()-1;

            if (curX + cx >= width) {
                new_line();
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
                fillrect(Rectangle(curX, curY, cx, fontHeight));
                setcolor(color());
                drawtext((const char*)p, numChars, curX, curY+ascent);
            } else {
                setcolor(color());
                fillrect(Rectangle(curX, curY, cx, fontHeight));
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

    redraw();
}

/* End of "$Id: output_write.c,v 1.1 2006-02-07 02:02:09 zeeb90au Exp $". */

