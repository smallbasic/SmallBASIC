/*
 * $Id: callbacks.c,v 1.8 2006-03-04 00:11:07 zeeb90au Exp $
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

#include "callbacks.h"
#include "interface.h"
#include "output_model.h"

extern OutputModel output;

void
on_stop_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    brun_break();
    output.break_exec = 1;
}

void
on_about_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    GtkWidget *about = create_aboutdialog();
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}

void
on_help_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}

void
on_quit_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    output.break_exec = 1;
}

/*
 * End of "$Id: callbacks.c,v 1.8 2006-03-04 00:11:07 zeeb90au Exp $". 
 */
