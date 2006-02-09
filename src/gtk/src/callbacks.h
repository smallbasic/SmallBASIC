#include <gtk/gtk.h>


void            on_open1_activate(GtkMenuItem * menuitem,
				  gpointer user_data);

void            on_quit1_activate(GtkMenuItem * menuitem,
				  gpointer user_data);

void            on_cut1_activate(GtkMenuItem * menuitem,
				 gpointer user_data);

void            on_copy1_activate(GtkMenuItem * menuitem,
				  gpointer user_data);

void            on_paste1_activate(GtkMenuItem * menuitem,
				   gpointer user_data);

void            on_delete1_activate(GtkMenuItem * menuitem,
				    gpointer user_data);

void            on_about1_activate(GtkMenuItem * menuitem,
				   gpointer user_data);

GtkWidget      *create_ansi_widget(gchar * widget_name,
				   gchar * string1,
				   gchar * string2, gint int1, gint int2);

GtkWidget      *ansi_widget_new(gchar * widget_name, gchar * string1,
				gchar * string2, gint int1, gint int2);

gboolean
 
 
 
 on_drawingarea1_expose_event(GtkWidget * widget,
			      GdkEventExpose * event, gpointer user_data);

gboolean
 
 
 
 on_drawingarea1_configure_event(GtkWidget * widget,
				 GdkEventConfigure * event,
				 gpointer user_data);

void
                on_stop_activate(GtkMenuItem * menuitem,
				 gpointer user_data);

void
                on_help1_activate(GtkMenuItem * menuitem,
				  gpointer user_data);

void
                on_about_activate(GtkMenuItem * menuitem,
				  gpointer user_data);
