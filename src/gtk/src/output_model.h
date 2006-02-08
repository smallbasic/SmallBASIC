/* -*- c-file-style: "java" -*-
 * $Id: output_model.h,v 1.4 2006-02-08 05:56:04 zeeb90au Exp $
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

typedef struct OutputModel {
    GdkPixmap* pixmap; /* Backing pixmap for drawing area */
    GtkWidget* widget; /* the drawing_area widget */
    GdkGC* gc;         /* current drawing colors */
    PangoFontDescription* font; /* bold, italic */
    int underline;
    int invert;
    int resized;
    int curY;
    int curX;
    int curYSaved;
    int curXSaved;
    int tabSize;
    int penMode;
    int penState;
    int penDownX;
    int penDownY;
} OutputModel;

void om_init(GtkWidget *widget);
void om_cleanup();
void om_set_bg_color(int ansi_color);
gint om_font_height();
gint om_getascent();

#endif

/* End of "$Id: output_model.h,v 1.4 2006-02-08 05:56:04 zeeb90au Exp $". */
