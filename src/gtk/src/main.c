/*
 * $Id: main.c,v 1.2 2006-02-07 03:54:40 zeeb90au Exp $
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

#include "interface.h"
#include "support.h"
#include "output.h"

#ifdef G_OS_WIN32
gchar *package_prefix = PACKAGE_PREFIX;
gchar *package_data_dir = PACKAGE_DATA_DIR;
gchar *package_locale_dir = PACKAGE_LOCALE_DIR;
#endif

int main(int argc, char *argv[]) {
    GtkWidget *main_window;
    gchar *pixmap_dir;

#ifdef G_OS_WIN32
    package_prefix = g_win32_get_package_installation_directory(NULL, NULL);
    package_data_dir = g_build_filename(package_prefix, "share", NULL);
    package_locale_dir = g_build_filename(package_prefix, "share", "locale", NULL);
#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, package_locale_dir);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif
#else
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif
#endif

    gtk_set_locale();
    gtk_init(&argc, &argv);

#ifdef G_OS_WIN32
    pixmap_dir = g_build_filename(package_data_dir, PACKAGE, "pixmaps", NULL);
    add_pixmap_directory(pixmap_dir);
    g_free(pixmap_dir);
#else
    add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");
#endif

    main_window = create_smallbasic();
    gtk_widget_show(main_window);
    g_signal_connect((gpointer) main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* dialog =
        gtk_file_chooser_dialog_new("Open File",
                                    GTK_WINDOW(main_window),
                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                    NULL);
    while (1) {
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
            gtk_widget_hide(dialog);
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            sbasic_main(filename);
            g_free(filename);
        } else {
            break;
        }
    }
    gtk_widget_destroy(dialog);

#ifdef G_OS_WIN32
    g_free(package_prefix);
    g_free(package_data_dir);
    g_free(package_locale_dir);
#endif

    return 0;
}
#ifdef _MSC_VER
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    return main(__argc, __argv);
}
#endif

/* End of "$Id: main.c,v 1.2 2006-02-07 03:54:40 zeeb90au Exp $". */
