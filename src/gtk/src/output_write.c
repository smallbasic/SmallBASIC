/* -*- c-file-style: "java" -*-
 * $Id: output_write.c,v 1.15 2006-06-14 10:35:41 zeeb90au Exp $
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

void speak_string(const char* s, int len) {
#ifdef USE_HILDON
    int retval;
    int speed = 2;
    int pitch = 4;
    osso_rpc_run(output.osso, 
                 "com.nokia.flite",
                 "/com/nokia/flite",
                 "com.nokia.flite",
                 "flite_tts",
                 &retval,
                 DBUS_TYPE_UINT32, // id for the order
                 getpid(),
                 DBUS_TYPE_STRING, // string to be read
                 "Say something",
                 DBUS_TYPE_UINT32, // priority. From 0 to 20.
                 20,
                 DBUS_TYPE_DOUBLE, // speed of speeching. From 0.1 to 3
                 speed,
                 DBUS_TYPE_DOUBLE, // adjust pitch of voice. From -2 to 8
                 pitch,
                 DBUS_TYPE_STRING, // name of the voice (not implemented)
                 "cmu_us_kal",
                 DBUS_TYPE_INVALID);
#endif
}

void new_line() {
    gint font_height = output.ascent+output.descent;
    gint h = output.height;
    output.cur_x = INITXY;

    if (output.cur_y+font_height+font_height >= h) {
        // shift image up font_height pixels
        gint w = output.width;
        gdk_draw_drawable(output.pixmap,
                          output.gc,
                          output.pixmap,
                          0, font_height, // src x,y
                          0, 0, // dest x,y
                          w, h-font_height);
        // erase bottom line
        gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
        gdk_draw_rectangle(output.pixmap, output.gc, TRUE,
                           0, h-font_height, w, font_height);
        osd_refresh();
    } else {
        output.cur_y += font_height;
    }
}

int calc_tab(int x) {
    int c = 1;
    while (x > output.tab_size) {
        x -= output.tab_size;
        c++;
    }
    return c * output.tab_size;
}

int set_graphics_rendition(char c, int escValue) {
    gint font_height = output.ascent+output.descent;
    switch (c) {
    case 'K': // \e[K - clear to eol
        gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
        gdk_draw_rectangle(output.pixmap, output.gc, TRUE, output.cur_x, output.cur_y,
                           output.width-output.cur_x, font_height);
        break;
    case 'G': // move to column
        output.cur_x = escValue;
        break;
    case 'T': // non-standard: move to n/80th of screen width
        output.cur_x = escValue*output.width/80;
        break;
    case 's': // save cursor position
        output.cur_y_saved = output.cur_x;
        output.cur_x_saved = output.cur_y;
        break;
    case 'u': // restore cursor position
        output.cur_x = output.cur_y_saved;
        output.cur_y = output.cur_x_saved;
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
        case 100:
            output.flite_out = 1;
            break;
        case 101:
            output.flite_out = 0;
            break;
        };                        
    }
    return 0;
}

int do_escape(unsigned char** p) {
    int escValue = 0;
    while (isdigit(**p)) {
        escValue = (escValue * 10) + (**p - '0');
        (*p)++;
    }
    
    if (set_graphics_rendition(**p, escValue)) {
        om_calc_font_metrics();
    }
    if (**p == ';') {
        (*p)++; // next rendition
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
    int num_chars, cx, width;

    while (*p) {
        switch (*p) {
        case '\a':   // beep
            drvsound_beep();
            break;
        case '\t':
            output.cur_x = calc_tab(output.cur_x+1);
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
            output.cur_x = INITXY;
            gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
            gdk_draw_rectangle(output.pixmap, output.gc, TRUE, 0, output.cur_y,
                               output.width, output.ascent+output.descent);
            break;
        default:
            num_chars = 1; // print minimum of one character
            cx = output.font_width;
            width = output.width-1;

            if (output.cur_x + cx >= width) {
                new_line();
            }

            // print further non-control, non-null characters 
            // up to the width of the line
            while (p[num_chars] > 31) {
                cx += output.font_width;
                if (output.cur_x + cx < width) {
                    num_chars++;
                } else {
                    break;
                }
            }
            
            if (output.flite_out) {
                speak_string((const char*)p, num_chars);
            }

            if (output.invert) {
                GdkColor c;
                c.red =  (output.fg.red ^ 0xffff);
                c.blue = (output.fg.blue ^ 0xffff);
                c.green = (output.fg.green ^ 0xffff);
                gdk_gc_set_rgb_fg_color(output.gc, &c);
            } else {
                gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
            }
            gdk_draw_rectangle(output.pixmap, output.gc, TRUE,
                               output.cur_x, output.cur_y, cx, 
                               output.ascent+output.descent);

            pango_layout_set_text(output.layout, (const char*)p, num_chars);
            gdk_gc_set_rgb_fg_color(output.gc, &output.fg);
            gdk_draw_layout(output.pixmap, output.gc,
                            output.cur_x, output.cur_y, output.layout);

            if (output.underline) {
                gdk_draw_line(output.pixmap, output.gc,
                              output.cur_x, 
                              output.cur_y+output.ascent+1,
                              output.cur_x+cx, 
                              output.cur_y+output.ascent+1);
            }
            
            // advance
            p += num_chars-1; // allow for p++ 
            output.cur_x += cx;
        };
        
        if (*p == '\0') {
            break;
        }
        p++;
    }

    osd_refresh();
}

/* End of "$Id: output_write.c,v 1.15 2006-06-14 10:35:41 zeeb90au Exp $". */

