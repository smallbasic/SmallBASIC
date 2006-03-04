//
// $Id: main.c,v 1.7 2006-03-04 00:11:07 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sbapp.h>
#include <gtk/gtk.h>
#include "interface.h"
#include "output.h"

#ifdef USE_HILDON
#include <libosso.h>
#include "hildon-lgpl/hildon-widgets/hildon-app.h"
#include "hildon-lgpl/hildon-widgets/hildon-input-mode-hint.h"
HildonApp* app;
#else
#define app
#endif

void destroy_event(GtkObject *object, gpointer user_data) {
    exit(1);
}

int main(int argc, char *argv[]) {
    GtkWidget *main_window;
    char buf[1];

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    gtk_set_locale();
    gtk_init(&argc, &argv);

#ifdef USE_HILDON
    app = HILDON_APP(hildon_app_new());
    hildon_app_set_title(app, "SmallBASIC");
    osso_context_t* osso = osso_initialize(PACKAGE, VERSION, TRUE, NULL);
    main_window = create_main_window();
    hildon_app_set_appview(app, HILDON_APPVIEW(main_window));
#else
    main_window = create_main_window();
#endif

    drawing_area_init(main_window);
    g_signal_connect(G_OBJECT(main_window), "destroy", 
                     G_CALLBACK(destroy_event), NULL);

#ifdef USE_HILDON
    gtk_widget_show_all(GTK_WIDGET(app));
#else
    gtk_widget_show_all(main_window);
#endif

    // prepare runtime flags
    opt_graphics = 1;
    opt_quiet = 1;
    opt_interactive = 0;
    opt_nosave = 1;
    opt_ide = IDE_NONE; // for sberr.c
    opt_command[0] = 0;
    opt_pref_width = 0;
    opt_pref_height = 0;
    opt_pref_bpp = 0;

    while (1) {
        GtkWidget* dialog = create_opendialog(app);
        gtk_window_set_title(GTK_WINDOW(dialog), "Open BAS File");
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            gtk_widget_destroy(dialog);

            while (1) {
                osd_cls();
                sbasic_main(filename);
                osd_setxy(1,1);
                osd_write("\nRestart Y/N?");
                dev_gets(buf, 1);
                if (buf[0] != 'Y' && buf[0] != 'y') {
                    break;
                }
            }
            g_free(filename);

        } else {
            gtk_widget_destroy(dialog);
            break;
        }
    }
    om_cleanup();
    return 0;
}

/* End of "$Id: main.c,v 1.7 2006-03-04 00:11:07 zeeb90au Exp $". */
