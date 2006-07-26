/*
 * $Id: callbacks.c,v 1.15 2006-07-26 11:29:48 zeeb90au Exp $
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

void on_save_screen(GtkMenuItem* menuitem, gpointer user_data) {
    GtkWidget* dialog = create_savedialog();

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        char* chooser_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char* filename = chooser_name;
        GString* filename_extn = 0;
        int len = strlen(filename);
        if (g_ascii_strcasecmp(filename+len-4, ".jpg") != 0 &&
            g_ascii_strcasecmp(filename+len-5, ".jpeg") != 0) {
            filename_extn = g_string_new(filename);
            g_string_append(filename_extn, ".jpeg");
            filename = filename_extn->str;
        }

        GdkPixbuf*  pixbuf = 
            gdk_pixbuf_get_from_drawable(NULL, output.pixmap, NULL,
                                         0, 0, 0, 0, 
                                         output.width, 
                                         output.height); 
        if (pixbuf) {
            GError *error = 0;
            gdk_pixbuf_save(pixbuf, filename, "jpeg", &error, 
                            "quality", "100", NULL);
            g_object_unref(pixbuf);
            g_clear_error(&error);
        }
        g_free(chooser_name);
        if (filename_extn) {
            g_string_free(filename_extn, TRUE);
        }
    }
    gtk_widget_destroy(dialog);
}

/*
 * End of "$Id: callbacks.c,v 1.15 2006-07-26 11:29:48 zeeb90au Exp $". 
 */
