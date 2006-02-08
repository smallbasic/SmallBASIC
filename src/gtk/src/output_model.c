/* -*- c-file-style: "java" -*-
 * $Id: output_model.c,v 1.4 2006-02-08 05:56:04 zeeb90au Exp $
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

#include <gtk/gtkpixmap.h>
#include "output_model.h"

struct OutputModel output;

void om_init(GtkWidget *widget) {
    output.pixmap = 0;
    output.widget = widget;
    output.font = pango_font_description_new();
    output.gc = gdk_gc_new(widget->window);
    output.underline = 0;
    output.invert = 0;
    output.resized = 0;
    output.curY = 0;
    output.curX = 0;
    output.curYSaved = 0;
    output.curXSaved = 0;
    output.tabSize = 0;
    output.penMode = 0;
    output.penState = 0;
    output.penDownX = 0;
    output.penDownY = 0;
}

void om_cleanup() {
    pango_font_description_free(output.font);
    g_object_unref(output.gc);
    g_object_unref(output.pixmap);
}

void om_set_bg_color(int ansi_color) {

}

gint om_font_height() {
    // TODO : use PangoFontDescription
    return 10;
}

gint om_getascent() {
    // TODO : use PangoFontDescription
    return gtk_style_get_font(output.widget->style)->ascent;
}

gint om_getdescent() {
    return gtk_style_get_font(output.widget->style)->descent;
}


/* End of "$Id: output_model.c,v 1.4 2006-02-08 05:56:04 zeeb90au Exp $". */

