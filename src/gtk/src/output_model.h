/* -*- c-file-style: "java" -*-
 * $Id: output_model.h,v 1.12 2006-03-09 20:28:25 zeeb90au Exp $
 * This file is part of SmallBASIC
 *
 * Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */ 

#ifndef DEV_GTK_OUTPUT_MODEL_H
#define DEV_GTK_OUTPUT_MODEL_H

#define C_BLACK     0
#define C_BLUE      1
#define C_GREEN     2
#define C_CYAN      3
#define C_RED       4
#define C_MAGENTA   5
#define C_YELLOW    6
#define C_WHITE     7
#define C_GRAY      8
#define C_LT_BLUE   9
#define C_LT_GREEN  10
#define C_LT_CYAN   11
#define C_LT_RED    12
#define C_LT_MAG    13
#define C_LT_YELLOW 14
#define C_BRIGHT_WH 15

typedef struct OutputModel {
    GdkPixmap* pixmap; /* Backing pixmap for drawing area */
    GtkWidget* widget; /* the drawing_area widget */
    GtkWidget *main_view;
    GdkGC* gc;         /* current drawing colors */
    PangoFontDescription* font_desc; /* bold, italic */
    GdkColor fg,bg;    /* couldn't find a gdk_gc_get_rgb_fg_color */
    PangoLayout* layout;
    int ascent;
    int descent;
    int font_width;
    int underline;
    int invert;
    int resized;
    int cur_x;
    int cur_y;
    int cur_y_saved;
    int cur_x_saved;
    int tab_size;
    int pen_mode;
    int pen_down;     /* index to current mouse button */
    int pen_down_x;
    int pen_down_y;
    int break_exec;
    int modal_flag;
    int full_screen;
    int width;
    int height;
} OutputModel;

void om_reset(int reset_cursor);
void om_init(GtkWidget *widget);
void om_cleanup();
GdkColor om_get_sb_color(long c);
void om_set_fg_color(int color);
void om_set_bg_color(int color);
void om_calc_font_metrics();

#define INITXY 4

// mapping for hardware keys
#define KEYMAP_UP     0
#define KEYMAP_DOWN   1
#define KEYMAP_LEFT   2
#define KEYMAP_RIGHT  3
#define KEYMAP_F7     4
#define KEYMAP_F8     5
#define KEYMAP_F6     6
#define KEYMAP_ENTER  7
#define KEYMAP_FIRST  0
#define KEYMAP_LAST   7

#endif

/* End of "$Id: output_model.h,v 1.12 2006-03-09 20:28:25 zeeb90au Exp $". */
