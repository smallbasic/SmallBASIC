/*
 * $Id: callbacks.c,v 1.10 2006-03-09 12:07:19 zeeb90au Exp $
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

#include "callbacks.h"
#include "interface.h"
#include "output_model.h"

extern OutputModel output;
extern Keymap keymap[];

typedef struct {
    GtkWidget* dialog;
    GtkWidget* label;
    GtkIMContext* imctx;
    int index;
    int mode;
} keymap_dialog_data;

void on_break(GtkMenuItem * menuitem, gpointer user_data) {
    output.break_exec = 1;
}

void on_about_activate(GtkMenuItem * menuitem, gpointer user_data) {
    GtkWidget *about = create_aboutdialog();
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}

gboolean dialog_focus(GtkWidget *dialog,
                      GdkEventFocus *event,
                      GtkIMContext *imctx) {
    if (event->in) {
        gtk_im_context_focus_in(imctx);
        gtk_im_context_show(imctx);
    } else {
        gtk_im_context_focus_out(imctx);
    }
    return FALSE;
}

gboolean im_context_commit(GtkIMContext *ctx,
                           const gchar *str,
                           keymap_dialog_data* data) {
    if (data->mode == 1 && strlen(str) > 0) {
        keymap[data->index].c = str[0];
        gtk_dialog_response(GTK_DIALOG(data->dialog), GTK_RESPONSE_ACCEPT);
    }
    return TRUE;
}

gboolean map_key_press_event(GtkWidget* widget, 
                             GdkEventKey* event, 
                             keymap_dialog_data* data) {
    int index = -1;
    switch (event->keyval) {
    case GDK_Up: // Navigation Key Up
        index = KEYMAP_UP;
        break;
    case GDK_Down: // Navigation Key Down
        index = KEYMAP_DOWN;
        break;
    case GDK_Left: // Navigation Key Left
        index = KEYMAP_LEFT;
        break;
    case GDK_Right: // Navigation Key Right
        index = KEYMAP_RIGHT;
        break;
    case GDK_F7: // Increase(zoom in)
        index = KEYMAP_F7;
        break;
    case GDK_F8: // Decrease(zoom out)
        index = KEYMAP_F8;
        break;
    case GDK_F6: // Full screen
        index = KEYMAP_F6;
        break;
    case GDK_Return: // Navigation Key select
        index = KEYMAP_ENTER;
        break;
    }
    if (data->mode == 0 && index != -1) {
        data->mode = 1;
        data->index = index;
        data->imctx = gtk_im_multicontext_new();
        g_signal_connect(G_OBJECT(data->imctx), "commit",
                         G_CALLBACK(im_context_commit), data);
        gtk_im_context_set_client_window(data->imctx, GTK_WIDGET(data->dialog)->window);
        g_signal_connect(G_OBJECT(data->dialog), "focus-in-event",
                         G_CALLBACK(dialog_focus), data->imctx);
        g_signal_connect(G_OBJECT(data->dialog), "focus-out-event",
                         G_CALLBACK(dialog_focus), data->imctx);
        gtk_label_set_text(GTK_LABEL(data->label), "Press a keypad key (or cancel)");
        gtk_widget_grab_focus(data->dialog);
    }

    return FALSE;
}

void on_map_key(GtkMenuItem* menuitem, gpointer user_data) {

    keymap_dialog_data* data = g_new0(keymap_dialog_data, 1);
    data->mode = 0;
    data->dialog = 
        gtk_dialog_new_with_buttons("Map Key",
                                    GTK_WINDOW(output.main_view->parent),
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    NULL);
    data->label = gtk_label_new("Press a hardware key (or cancel)");
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(data->dialog)->vbox), 
                                data->label);
    g_signal_connect(G_OBJECT(data->dialog), "key_press_event",
                     G_CALLBACK(map_key_press_event), data);
    
    gtk_widget_show_all(data->dialog);
    gtk_dialog_run(GTK_DIALOG(data->dialog));
 
    gtk_widget_hide(data->dialog);
    gtk_widget_destroy(data->dialog);

    if (data->imctx) {
        gtk_im_context_focus_out(data->imctx);
        g_object_unref(G_OBJECT(data->imctx));
    }

    g_free(data);    

}

void on_quit_activate(GtkMenuItem * menuitem, gpointer user_data) {
    exit(1);
}

/*
 * End of "$Id: callbacks.c,v 1.10 2006-03-09 12:07:19 zeeb90au Exp $". 
 */
