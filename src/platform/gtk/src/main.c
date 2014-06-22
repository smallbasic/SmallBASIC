// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sbapp.h>
#include <gtk/gtk.h>

#ifdef USE_HILDON
#include <libosso.h>
#include <hildon-widgets/hildon-program.h>
#endif

#include "interface.h"
#include "output.h"
#include "output_model.h"

extern OutputModel output;

void destroy_event(GtkObject *object, gpointer user_data) {
  exit(1);
}

int main(int argc, char *argv[]) {
  GtkWidget *main_window;

  gtk_set_locale();
  gtk_init(&argc, &argv);

  main_window = create_main_window();

#ifdef USE_HILDON
  HildonProgram *app = HILDON_PROGRAM(hildon_program_get_instance());
  g_set_application_name("SmallBASIC");
  output.osso = osso_initialize(PACKAGE, VERSION, TRUE, NULL);
  hildon_program_add_window(app, HILDON_WINDOW(main_window));
#endif

  drawing_area_init(main_window);
  gtk_widget_show_all(GTK_WIDGET(main_window));
  g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(destroy_event), NULL);

  // prepare runtime flags
  opt_graphics = 1;
  opt_quiet = 1;
  opt_interactive = 0;
  opt_nosave = 1;
  opt_ide = IDE_NONE;           // for sberr.c
  opt_command[0] = 0;
  opt_pref_width = 0;
  opt_pref_height = 0;
  opt_pref_bpp = 0;
  opt_file_permitted = 1;

  if (argc == 2 && access(argv[1], R_OK) == 0) {
    sbasic_main(argv[1]);
  }

  GtkWidget *dialog = create_opendialog();
  while (1) {
    gtk_widget_show(dialog);
    gtk_window_set_title(GTK_WINDOW(dialog), "Open BAS File");
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      const char *p = strrchr(filename, '/');
      gtk_window_set_title(GTK_WINDOW(main_window), p ? p + 1 : filename);
      gtk_widget_hide(dialog);
      sbasic_main(filename);
      g_free(filename);
    } else {
      break;
    }
  }
  gtk_widget_destroy(dialog);
  om_cleanup();
  return 0;
}

