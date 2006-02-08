/* -*- c-file-style: "java" -*-
 * $Id: output_model.c,v 1.3 2006-02-08 03:29:50 zeeb90au Exp $
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

#include <device.h>
#include <gtk/gtkpixmap.h>
#include "output_model.h"

struct OutputModel output;

void output_model_init(GtkWidget *widget) {
    output.pixmap = 0;
    output.widget = widget;
    output.underline = 0;
    output.invert = 0;
    output.bold = 0;
    output.italic = 0;
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

int osd_devinit() {
    os_graphics = 1;
    os_graf_mx = output.widget->allocation.width;
    os_graf_my = output.widget->allocation.height;
    os_ver = 1;
    os_color = 1;
    os_color_depth = 16;
    setsysvar_str(SYSVAR_OSNAME, "GTK");
    dev_clrkb();
}


/* End of "$Id: output_model.c,v 1.3 2006-02-08 03:29:50 zeeb90au Exp $". */

