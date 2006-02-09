 /* -*- c-file-style: "java" -*-
 * $Id: output_write.c,v 1.6 2006-02-09 01:24:46 zeeb90au Exp $
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
#include "output.h"
#include "output_model.h"

extern OutputModel output;

void new_line() {
    gint font_height = output.ascent+output.descent;
    gint height = output.widget->allocation.height;
    output.curX = INITXY;

    if (output.curY+(font_height*2) >= height) {
        /* shift image up font_height pixels */
        gdk_draw_drawable(output.pixmap,
                          output.gc,
                          output.pixmap,
                          0, font_height, /* src x,y */
                          0, 0,          /* dest x,y */
                          output.widget->allocation.width,
                          output.widget->allocation.width-font_height);
        /* erase bottom line */
        gdk_draw_rectangle(output.pixmap, output.gc, TRUE,
                           output.widget->allocation.x,
                           output.widget->allocation.y,
                           output.widget->allocation.width,
                           output.widget->allocation.height);
        osd_refresh();
    } else {
        output.curY += font_height;
    }
}

int calc_tab(int x) {
    int c = 1;
    while (x > output.tabSize) {
        x -= output.tabSize;
        c++;
    }
    return c * output.tabSize;
}

int set_graphics_rendition(char c, int escValue) {
    gint font_height = output.ascent+output.descent;
    switch (c) {
    case 'K': // \e[K - clear to eol
        gdk_draw_rectangle(output.pixmap, output.gc, TRUE, output.curX, output.curY,
                           output.widget->allocation.width-output.curX, font_height);
        break;
    case 'G': // move to column
        output.curX = escValue;
        break;
    case 'T': // non-standard: move to n/80th of screen width
        output.curX = escValue*output.widget->allocation.width/80;
        break;
    case 's': // save cursor position
        output.curYSaved = output.curX;
        output.curXSaved = output.curY;
        break;
    case 'u': // restore cursor position
        output.curX = output.curYSaved;
        output.curY = output.curXSaved;
        break;
    case ';': // fallthru
    case 'm': // \e[...m  - ANSI terminal
        switch (escValue) {
        case 0:  // reset
            om_reset(FALSE);
            break;
        case 1: // set bold on
            pango_font_description_set_weight(output.font_desc, PANGO_WEIGHT_BOLD);
            return 1;
        case 2: // set faint on
            pango_font_description_set_weight(output.font_desc, PANGO_WEIGHT_ULTRALIGHT);
            break;
        case 3: // set italic on
            pango_font_description_set_style(output.font_desc, PANGO_STYLE_ITALIC);
            return 1;
        case 4: // set underline on
            output.underline = 1;
            break;
        case 5: // set blink on
            break;
        case 6: // rapid blink on
            break;
        case 7: // reverse video on
            output.invert = 1;
            break;
        case 8: // conceal on
            break;
        case 21: // set bold off
            pango_font_description_set_weight(output.font_desc, PANGO_WEIGHT_NORMAL);
            return 1;
        case 23: // italic off
            pango_font_description_set_style(output.font_desc, PANGO_STYLE_NORMAL);
            return 1;
        case 24: // set underline off
            output.underline = 0;
            break;
        case 27: // reverse video off
            output.invert = 0;
            break;
            // colors - 30..37 foreground, 40..47 background
        case 30: // set black fg
            om_set_fg_color(C_BLACK);
            break;
        case 31: // set red fg
            om_set_fg_color(C_RED);
            break;
        case 32: // set green fg
            om_set_fg_color(C_GREEN);
            break;
        case 33: // set yellow fg
            om_set_fg_color(C_YELLOW);
            break;
        case 34: // set blue fg
            om_set_fg_color(C_BLUE);
            break;
        case 35: // set magenta fg
            om_set_fg_color(C_MAGENTA);
            break;
        case 36: // set cyan fg
            om_set_fg_color(C_CYAN);
            break;
        case 37: // set white fg
            om_set_fg_color(C_WHITE);
            break;
        case 40: // set black bg
            om_set_bg_color(C_BLACK);
            break;
        case 41: // set red bg
            om_set_bg_color(C_RED);
            break;
        case 42: // set green bg
            om_set_bg_color(C_GREEN);
            break;
        case 43: // set yellow bg
            om_set_bg_color(C_YELLOW);
            break;
        case 44: // set blue bg
            om_set_bg_color(C_BLUE);
            break;
        case 45: // set magenta bg
            om_set_bg_color(C_MAGENTA);
            break;
        case 46: // set cyan bg
            om_set_bg_color(C_CYAN);
            break;
        case 47: // set white bg
            om_set_bg_color(C_WHITE);
            break;
        case 48: // subscript on
            break;
        case 49: // superscript
            break;
        };                        
    }
    return 0;
}

int do_escape(unsigned char** p) {
    int escValue = 0;
    while (isdigit(**p)) {
        escValue = (escValue * 10) + (**p - '0');
        *p++;
    }

    if (set_graphics_rendition(**p, escValue)) {
        om_font_init();
    }
    if (**p == ';') {
        *p++; // next rendition
        return 1;
    }
    return 0;
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
void osd_write(const char *str) {
    int len = strlen(str);
    if (len <= 0) {
        return;
    }

    unsigned char *p = (unsigned char*)str;
    int numChars, cx, width;

    while (*p) {
        switch (*p) {
        case '\a':   // beep
            drvsound_beep();
            break;
        case '\t':
            output.curX = calc_tab(output.curX+1);
            break;
        case '\xC':
            om_reset(TRUE);
            osd_cls();
            break;
        case '\033':  // ESC ctrl chars
            if (*(p+1) == '[' ) {
                p += 2;
                while(1) {
                    if (!do_escape(&p)) {
                        break;
                    }
                }
            }
            break;
        case '\n': // new line
            new_line();
            break;
        case '\r': // return
            output.curX = INITXY;
            gdk_draw_rectangle(output.pixmap, output.gc, TRUE, 0, output.curY,
                               output.widget->allocation.width, 
                               output.ascent+output.descent);
            break;
        default:
            numChars = 1; // print minimum of one character
            cx = output.font_width;
            width = output.widget->allocation.width-1;

            if (output.curX + cx >= width) {
                new_line();
            }

            // print further non-control, non-null characters 
            // up to the width of the line
            while (p[numChars] > 31) {
                cx += output.font_width;
                if (output.curX + cx < width) {
                    numChars++;
                } else {
                    break;
                }
            }
            
            if (output.invert) {
                gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
                gdk_gc_set_rgb_bg_color(output.gc, &output.fg);
                gdk_draw_rectangle(output.pixmap, output.gc, TRUE,
                                   output.curX, output.curY, cx, 
                                   output.ascent+output.descent);
                gdk_gc_set_rgb_fg_color(output.gc, &output.fg);
                gdk_gc_set_rgb_bg_color(output.gc, &output.bg);
            } else {
                gdk_draw_rectangle(output.pixmap, output.gc, TRUE,
                                   output.curX, output.curY, cx,
                                   output.ascent+output.descent);
            }

            pango_layout_set_text(output.layout, (const char*)p, numChars);
            gdk_draw_layout(output.pixmap, output.gc,
                            output.curX, output.curY+output.ascent, output.layout);

            if (output.underline) {
                gdk_draw_line(output.pixmap, output.gc,
                              output.curX, 
                              output.curY+output.ascent+1,
                              output.curX+cx, 
                              output.curY+output.ascent+1);
            }
            
            // advance
            p += numChars-1; // allow for p++ 
            output.curX += cx;
        };
        
        if (*p == '\0') {
            break;
        }
        p++;
    }

    osd_refresh();
}

/* End of "$Id: output_write.c,v 1.6 2006-02-09 01:24:46 zeeb90au Exp $". */

