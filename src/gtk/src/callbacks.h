/*
 * $Id: callbacks.h,v 1.3 2006-02-10 02:40:27 zeeb90au Exp $
 * This file is part of SmallBASIC
 *
 * Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */

void            on_stop_activate(GtkMenuItem * menuitem,
				 gpointer user_data);
void            on_about_activate(GtkMenuItem * menuitem,
				  gpointer user_data);
void            on_help_activate(GtkMenuItem * menuitem,
				 gpointer user_data);
void            on_quit_activate(GtkMenuItem * menuitem,
				 gpointer user_data);

/*
 * End of "$Id: callbacks.h,v 1.3 2006-02-10 02:40:27 zeeb90au Exp $". 
 */
