#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


void
on_open1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
/*     GtkWidget      *file_chooser = create_filechooserdialog(); */
/*     // gtk_widget_show(file_chooser); */
/*     // gtk_grab_add(file_chooser); */
/*     // gtk_window_set_modal */
/*     // k_window_set_transient_for (GTK_WINDOW(chooser), */
/*     // GTK_WINDOW(window)); */
/*     gint            response = gtk_dialog_run(GTK_DIALOG(file_chooser)); */
/*     g_print("response = %d\n", response); */
/*     if (response == GTK_RESPONSE_OK) { */
/* 	g_print("selected file = ?"); */
/*     } */
/*     gtk_widget_destroy(file_chooser); */
}


void
on_quit1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    gtk_main_quit();
}


void
on_cut1_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_copy1_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_paste1_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_delete1_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_about1_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}

gboolean
on_drawingarea1_expose_event(GtkWidget * widget,
			     GdkEventExpose * event, gpointer user_data)
{

    return FALSE;
}


gboolean
on_drawingarea1_configure_event(GtkWidget * widget,
				GdkEventConfigure * event,
				gpointer user_data)
{

    return FALSE;
}
