/* -*- c-file-style: "java" -*-
 * $Id: output.c,v 1.1 2006-02-07 02:02:09 zeeb90au Exp $
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
#include "output_model.h"

extern OutputModel output;

/* Create a new backing pixmap of the appropriate size */
static gboolean configure_event(GtkWidget         *widget,
                                GdkEventConfigure *event ) {
    if (output.pixmap) {
        g_object_unref(output.pixmap);
    }
    
    output.pixmap = gdk_pixmap_new(widget->window,
                                   widget->allocation.width,
                                   widget->allocation.height,
                                   -1);
    gdk_draw_rectangle(output.pixmap,
                       widget->style->white_gc,
                       TRUE,
                       0, 0,
                       widget->allocation.width,
                       widget->allocation.height);
    return TRUE;
}

/* Redraw the screen from the backing pixmap */
static gboolean expose_event(GtkWidget      *widget,
                             GdkEventExpose *event ) {
    gdk_draw_drawable(widget->window,
                      widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                      output.pixmap,
                      event->area.x, event->area.y,
                      event->area.x, event->area.y,
                      event->area.width, event->area.height);
    return FALSE;
}

static gboolean init_drawing_area(GtkWidget *window) {

    GtkWidget *drawing_area = 
        g_object_get_data(G_OBJECT(window), "drawing_area");

    /* Signals used to handle backing pixmap */
    g_signal_connect (G_OBJECT (drawing_area), "expose_event",
                      G_CALLBACK (expose_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area),"configure_event",
                      G_CALLBACK (configure_event), NULL);
    
    /* Event signals */
#if 0
    g_signal_connect(G_OBJECT (drawing_area), "motion_notify_event",
                     G_CALLBACK (motion_notify_event), NULL);
    g_signal_connect(G_OBJECT (drawing_area), "button_press_event",
                     G_CALLBACK (button_press_event), NULL);
#endif

    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK |
                          GDK_LEAVE_NOTIFY_MASK  |
                          GDK_BUTTON_PRESS_MASK  |
                          GDK_POINTER_MOTION_MASK|
                          GDK_POINTER_MOTION_HINT_MASK);

    /* The following call enables tracking and processing of extension
       events for the drawing area */
    gtk_widget_set_extension_events(drawing_area, GDK_EXTENSION_EVENTS_CURSOR);
    output_model_init();
}

void osd_setcolor(long color) {
}

void osd_settextcolor(long fg, long bg) {
}

void osd_refresh() {
}

int osd_devrestore() {
    return 1;
}

/**
 *   system event-loop
 *   return value:
 *     0 continue 
 *    -1 close sbpad application
 *    -2 stop running basic application
 */
int osd_events(int wait_flag) {
    return 1;
}

void osd_setpenmode(int enable) {
}

int osd_getpen(int code) {
    return 1;
}

int osd_getx() {
    return 1;
}

int osd_gety() {
    return 1;
}

void osd_setxy(int x, int y) {

}

void osd_cls() {

}

int osd_textwidth(const char *str) {

}

int osd_textheight(const char *str) {

}

void osd_setpixel(int x, int y) {

}

long osd_getpixel(int x, int y) {

}

void osd_line(int x1, int y1, int x2, int y2) {

}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {

}

int drvsound_init(void) {
}

/*
*   restores device state
*/
void drvsound_close(void) {
}

/*
*   plays a tone 
*       frq = frequency
*       ms  = duration in milliseconds
*       vol = volume (0..100)
*       bgplay = true for play in background
*/
void drvsound_sound(int frq, int ms, int vol, int bgplay) {
}

/*
*   beep!
*/
void drvsound_beep(void) {
}

/*
*	clear background queue
*/
void drvsound_clear_queue(void) {
}

/*
*   For OSes that does not supports threads, this enables background plays
*   (Its called every ~50ms)
*/
void drvsound_event(void) {
}


/* End of "$Id: output.c,v 1.1 2006-02-07 02:02:09 zeeb90au Exp $". */

