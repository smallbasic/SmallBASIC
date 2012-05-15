// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef DEV_GTK_OUTPUT_H
#define DEV_GTK_OUTPUT_H

gboolean drawing_area_init(GtkWidget *window);
void invalidate_rect(int x1, int y1, int x2, int y2);

#endif

