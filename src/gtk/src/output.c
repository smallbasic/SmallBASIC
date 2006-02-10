/* -*- c-file-style: "java" -*-
 * $Id: output.c,v 1.9 2006-02-10 02:40:27 zeeb90au Exp $
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

#include <device.h>
#include <gtk/gtk.h>
#include "interface.h"
#include "support.h"
#include "output.h"
#include "output_model.h"

extern OutputModel output;

#define PEN_ON  2
#define PEN_OFF 0

int osd_devinit() {
    os_graphics = 1;
    os_graf_mx = output.widget->allocation.width;
    os_graf_my = output.widget->allocation.height;
    os_ver = 1;
    os_color = 1;
    os_color_depth = 16;
    setsysvar_str(SYSVAR_OSNAME, "GTK");
    dev_clrkb();
}

void osd_setcolor(long color) {
    om_set_fg_color(color);
}

void osd_settextcolor(long fg, long bg) {
    om_set_fg_color(fg);
    om_set_bg_color(bg);
}

void osd_refresh() {
    invalidate_rect(output.widget->allocation.x,
                    output.widget->allocation.y,
                    output.widget->allocation.width,
                    output.widget->allocation.height);
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
    if (wait_flag ||
        (output.penState == 0 &&
         output.penMode == PEN_ON &&
         gtk_events_pending() == FALSE)) {
        // in a "while 1" loop checking for a pen/mouse
        // event with pen(0) or executing input statement.
        gtk_main_iteration_do(TRUE);
    }

    if (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    if (output.breakExec == 1) {
        output.breakExec = 0;
        return -2;
    }
    return 0;
}

void osd_setpenmode(int enable) {
    output.penMode = enable;
}

void get_mouse_xy() {
}

int osd_getpen(int code) {
    if (output.penMode == PEN_OFF) {
        gtk_main_iteration_do(TRUE);
    }

    switch (code) {
    case 0: // return true if there is a waiting pen event (up/down)
        if (output.penState != 0) {
            output.penState = 0;
            get_mouse_xy();
            //fltk::Rectangle* rc = wnd->out;
            //if (rc->contains(wnd->penDownX, wnd->penDownY)) {
            //return 1;
            //}
        }
        gtk_main_iteration_do(TRUE); // UNTIL PEN(0)
        // fallthru to re-test 

    case 3: // returns true if the pen is down (and save curpos)
        return 0;

    case 1:  // last pen-down x
        return output.penDownX;

    case 2:  // last pen-down y
        return output.penDownY;

    case 4:  // cur pen-down x
    case 10:
        get_mouse_xy();
        return output.penDownX;

    case 5:  // cur pen-down y
    case 11:
        get_mouse_xy();
        return output.penDownY;

    case 12: // true if left button pressed
        //return get_key_state(LeftButton);

    case 13: // true if right button pressed
        //return get_key_state(RightButton);

    case 14: // true if middle button pressed
        //return get_key_state(MiddleButton);
        break;
    }
    return 0;

}

int osd_getx() {
    return output.curX;
}

int osd_gety() {
    return output.curY;
}

void osd_setxy(int x, int y) {
    output.curX = x;
    output.curY = y;
}

void osd_cls() {
    gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
    gdk_draw_rectangle(output.pixmap, output.gc, TRUE, 0, 0,
                       output.widget->allocation.width,
                       output.widget->allocation.height);
    osd_refresh();
}

int osd_textwidth(const char *text) {
    return (text && text[0] ? strlen(text)*output.font_width : 0);
}

int osd_textheight(const char *text) {
    return output.ascent+output.descent;
}

void osd_setpixel(int x, int y) {
    GdkImage* image = gdk_drawable_get_image(output.pixmap, x, y, 1, 1);
    gdk_image_put_pixel(image, 0, 0, dev_fgcolor);
    gdk_draw_image(output.pixmap,
                   output.gc,
                   image, 0, 0, x, y, 1, 1);
    g_object_unref(image);
    invalidate_rect(x, y, 1, 1);
}

long osd_getpixel(int x, int y) {
    GdkImage* image = gdk_drawable_get_image(output.pixmap, x, y, 1, 1);
    guint32 px = gdk_image_get_pixel(image, 0, 0);
    g_object_unref(image);
    return px;
}

void osd_line(int x1, int y1, int x2, int y2) {
    gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
    gdk_draw_line(output.pixmap, output.gc, x1, y1, x2, y2);
    invalidate_rect(x1, y1, x2-x1, y2-y1);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
    gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
    gdk_draw_rectangle(output.pixmap, output.gc, bFill, x1, y1, x2-x1, y2-y1);
    invalidate_rect(x1, y1, x2-x1, y2-y1);
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

void invalidate_rect(int x, int y, int w, int h) {
    GdkRectangle rc = {x, y, w, h};
    gdk_window_invalidate_rect(output.widget->window, &rc, TRUE);
}

/* Create a new backing pixmap of the appropriate size */
gboolean configure_event(GtkWidget* widget, GdkEventConfigure *event) {
    if (output.gc == 0) {
        /* deferred init to here since we don't run gtk_main() */
        output.gc = gdk_gc_new(widget->window);
        om_reset(TRUE); 
    }

    if (output.pixmap) {
        /* copy old image onto new/resized image */
        int old_w, old_h;
        gdk_drawable_get_size(output.pixmap, &old_w, &old_h);
        int w = MAX(widget->allocation.width, old_w);
        int h = MAX(widget->allocation.height, old_h);

        GdkPixmap* pixmap = gdk_pixmap_new(widget->window, w, h, -1);
        gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
        gdk_draw_rectangle(pixmap, output.gc, TRUE, 0, 0, w, h);
        gdk_draw_drawable(pixmap,
                          output.gc,
                          output.pixmap,
                          0,0,0,0, /* src/dest x/y */
                          old_w, old_h);
        g_object_unref(output.pixmap);
        output.pixmap = pixmap;
    } else {
        /* create a new pixmap */
        output.pixmap = gdk_pixmap_new(widget->window,
                                       widget->allocation.width,
                                       widget->allocation.height, -1);
        osd_cls();
    }
    return TRUE;
}

/* Redraw the screen from the backing pixmap */
gboolean expose_event(GtkWidget* widget, GdkEventExpose* event) {
    gdk_draw_drawable(widget->window, output.gc, output.pixmap,
                      event->area.x, event->area.y, /* src */
                      event->area.x, event->area.y, /* dest */
                      event->area.width, event->area.height);
    return FALSE;
}

gboolean button_press_event(GtkWidget *widget, GdkEventButton *event) {
    if (event->button == 1) {
        gdouble pressure;
        gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure);
        //draw_brush (widget, event->device->source, event->x, event->y, pressure);
    }
    return TRUE;
}

gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event) {
    gdouble x, y;
    gdouble pressure;
    GdkModifierType state;
    
    if (event->is_hint) {
        gdk_device_get_state (event->device, event->window, NULL, &state);
        gdk_event_get_axis((GdkEvent *)event, GDK_AXIS_X, &x);
        gdk_event_get_axis((GdkEvent *)event, GDK_AXIS_Y, &y);
        gdk_event_get_axis((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure);
    } else {
        x = event->x;
        y = event->y;
        gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure);
        state = event->state;
    }
    
    if (state & GDK_BUTTON1_MASK && output.pixmap != NULL) {
        //draw_brush (widget, event->device->source, x, y, pressure);
    }
 
    return TRUE;
}

gboolean drawing_area_init(GtkWidget *main_window) {
    GtkWidget *drawing_area = 
        g_object_get_data(G_OBJECT(main_window), "drawing_area");

    /* Signals used to handle backing pixmap */
    g_signal_connect (G_OBJECT (drawing_area), "expose_event",
                      G_CALLBACK (expose_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area),"configure_event",
                      G_CALLBACK (configure_event), NULL);
    /* Event signals */
    g_signal_connect(G_OBJECT (drawing_area), "motion_notify_event",
                     G_CALLBACK (motion_notify_event), NULL);
    g_signal_connect(G_OBJECT (drawing_area), "button_press_event",
                     G_CALLBACK (button_press_event), NULL);

    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK |
                          GDK_LEAVE_NOTIFY_MASK  |
                          GDK_BUTTON_PRESS_MASK  |
                          GDK_POINTER_MOTION_MASK|
                          GDK_POINTER_MOTION_HINT_MASK);

    /* The following call enables tracking and processing of extension
       events for the drawing area */
    gtk_widget_set_extension_events(drawing_area, GDK_EXTENSION_EVENTS_CURSOR);

    om_init(drawing_area);
}

/* End of "$Id: output.c,v 1.9 2006-02-10 02:40:27 zeeb90au Exp $". */

