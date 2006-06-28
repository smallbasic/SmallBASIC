/*
 * $Id: callbacks.c,v 1.13 2006-06-28 13:06:46 zeeb90au Exp $
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
#include <gdk/gdkkeysyms.h>

#ifdef USE_HILDON
#include <libosso.h>
#endif

#include "callbacks.h"
#include "interface.h"
#include "output_model.h"

extern OutputModel output;
extern int keymap[];

void on_break(GtkMenuItem * menuitem, gpointer user_data) {
    output.break_exec = 1;
}

void on_about_activate(GtkMenuItem * menuitem, gpointer user_data) {
    GtkWidget *about = create_aboutdialog();
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}

void on_reset_keys(GtkMenuItem* menuitem, gpointer user_data) {
    int i;
    for (i=KEYMAP_FIRST; i<=KEYMAP_LAST; i++) {
        keymap[i]=0;
    }
}

void on_quit_activate(GtkMenuItem * menuitem, gpointer user_data) {
    exit(1);
}

/*
 * End of "$Id: callbacks.c,v 1.13 2006-06-28 13:06:46 zeeb90au Exp $". 
 */
