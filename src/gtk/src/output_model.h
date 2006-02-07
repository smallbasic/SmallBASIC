/* -*- c-file-style: "java" -*-
 * $Id: output_model.h,v 1.1 2006-02-07 02:02:09 zeeb90au Exp $
 * This file is part of SmallBASIC
 *
 * Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */ 

#ifndef DEV_GTK_OUTPUT_MODEL_H
#define DEV_GTK_OUTPUT_MODEL_H

typedef struct OutputModel {
    GdkPixmap *pixmap; /* Backing pixmap for drawing area */
    int underline;
    int invert;
    int bold;
    int italic;
    int resized;
    int curY;
    int curX;
    int curYSaved;
    int curXSaved;
    int tabSize;
    int penMode;
} OutputModel;

#endif

/* End of "$Id: output_model.h,v 1.1 2006-02-07 02:02:09 zeeb90au Exp $". */
