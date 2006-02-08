/* -*- c-file-style: "java" -*-
 * $Id: output_model.c,v 1.5 2006-02-08 12:01:13 zeeb90au Exp $
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

void om_reset(int reset_cursor) {
    if (reset_cursor) {
        curY = INITXY;
        curX = INITXY;
    }

    output.underline = 0;
    output.invert = 0;
    output.resized = 0;
    output.curY = 0;
    output.curX = 0;
    output.curYSaved = 0;
    output.curXSaved = 0;
    output.tabSize = 40; /* tab size in pixels (160/32 = 5) */
    output.penMode = 0;
    output.penState = 0;
    output.penDownX = 0;
    output.penDownY = 0;

    gdk_gc_set_rgb_bg_color(output.gc, get_sb_color(15)); /* white background */
    gdk_gc_set_rgb_fg_color(output.gc, get_sb_color(0)); /* black foreground */
    pango_font_description_set_weight(output.font, PANGO_WEIGHT_NORMAL);
    pango_font_description_set_style(output.font, PANGO_STYLE_NORMAL);
}

void om_init(GtkWidget *widget) {
    output.pixmap = 0;
    output.widget = widget;
    output.gc = gdk_gc_new(widget->window);
    output.font = pango_font_description_new(); /* pango_font_description_from_string*/
    pango_font_description_set_size(output.font, 10);
    pango_font_description_set_family(output.font, "monospace");
    om_reset(TRUE);
}

void om_cleanup() {
    pango_font_description_free(output.font);
    g_object_unref(output.gc);
    g_object_unref(output.pixmap);
}


/* End of "$Id: output_model.c,v 1.5 2006-02-08 12:01:13 zeeb90au Exp $". */

