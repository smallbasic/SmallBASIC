/* -*- c-file-style: "java" -*-
 * $Id: output_model.c,v 1.8 2006-02-09 12:29:47 zeeb90au Exp $
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

#define COLOR(r,g,b) {0, (r*65535/255), (g*65535/255), (b*65535/255)}
#define WHITE COLOR(255,255,255)

static GdkColor colors[] = {
    COLOR(0,0,0),      // 0 black
    COLOR(0,0,128),    // 1 blue
    COLOR(0,128,0),    // 2 green
    COLOR(0,128,128),  // 3 cyan
    COLOR(128,0,0),    // 4 red
    COLOR(128,0,128),  // 5 magenta
    COLOR(128,128,0),  // 6 yellow
    COLOR(192,192,192),// 7 white
    COLOR(128,128,128),// 8 gray
    COLOR(0,0,255),    // 9 light blue
    COLOR(0,255,0),    // 10 light green
    COLOR(0,255,255),  // 11 light cyan
    COLOR(255,0,0),    // 12 light red
    COLOR(255,0,255),  // 13 light magenta
    COLOR(255,255,0),  // 14 light yellow
    WHITE              // 15 bright white
};

void om_init(GtkWidget *widget) {
    output.widget = widget;
    output.pixmap = 0;
    output.layout = 0;
    output.gc = 0; 
    output.breakExec = 0;
    output.font_desc = pango_font_description_new(); /* pango_font_description_from_string*/
    pango_font_description_set_size(output.font_desc, 9*PANGO_SCALE);
    pango_font_description_set_family(output.font_desc, "monospace");
}

void om_cleanup() {
    g_object_unref(output.layout);
    g_object_unref(output.gc);
    g_object_unref(output.pixmap);
    pango_font_description_free(output.font_desc);
}

void om_reset(int reset_cursor) {
    if (reset_cursor) {
        output.curY = INITXY;
        output.curX = INITXY;
    }

    output.underline = 0;
    output.invert = 0;
    output.resized = 0;
    output.curYSaved = 0;
    output.curXSaved = 0;
    output.tabSize = 40; /* tab size in pixels (160/32 = 5) */
    output.penMode = 0;
    output.penState = 0;
    output.penDownX = 0;
    output.penDownY = 0;

    om_set_fg_color(C_BLUE);     /* blue foreground */
    om_set_bg_color(C_BRIGHT_WH); /* white background */

    pango_font_description_set_weight(output.font_desc, PANGO_WEIGHT_NORMAL);
    pango_font_description_set_style(output.font_desc, PANGO_STYLE_NORMAL);
    om_font_init();
}

void om_font_init() {
    if (output.layout) {
        //g_object_unref(output.layout);
    }
    output.layout = gtk_widget_create_pango_layout(output.widget, "zz");
    pango_layout_set_width(output.layout, -1);
    pango_layout_set_font_description(output.layout, output.font_desc); 

    gtk_widget_modify_font(output.widget, output.font_desc);
    PangoContext* context = gtk_widget_get_pango_context(output.widget);
    PangoFontMetrics* metrics = 
        pango_context_get_metrics(context, output.font_desc,
                                  pango_context_get_language(context));
    output.ascent = PANGO_PIXELS(pango_font_metrics_get_ascent(metrics));
    output.descent = PANGO_PIXELS(pango_font_metrics_get_descent(metrics));
    output.font_width = 
        PANGO_PIXELS(pango_font_metrics_get_approximate_digit_width(metrics));
    pango_font_metrics_unref(metrics);
    g_object_unref(context);
}

GdkColor om_get_sb_color(long c) {
    if (c < 0) {
        // assume color is windows style RGB packing
        // RGB(r,g,b) ((COLORREF)((BYTE)(r)|((BYTE)(g) << 8)|((BYTE)(b) << 16)))
        c = -c;
        int b = (c>>16) & 0xFF;
        int g = (c>>8) & 0xFF;
        int r = (c) & 0xFF;
        GdkColor color = COLOR(r,g,b);
        return color;
    }
    if (c > 16) {
        GdkColor color = WHITE;
        return color;
    }
    return colors[c];
}

void om_set_fg_color(int color) {
    output.fg = om_get_sb_color(color);
}

void om_set_bg_color(int color) {
    output.bg = om_get_sb_color(color);
}

/* End of "$Id: output_model.c,v 1.8 2006-02-09 12:29:47 zeeb90au Exp $". */

