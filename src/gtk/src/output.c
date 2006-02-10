/* -*- c-file-style: "java" -*-
 * $Id: output.c,v 1.10 2006-02-10 05:59:58 zeeb90au Exp $
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
    GdkRectangle rc = {
        output.widget->allocation.x,
        output.widget->allocation.y,
        output.widget->allocation.width,
        output.widget->allocation.height
    };
    gdk_window_invalidate_rect(output.widget->window, &rc, TRUE);
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
        (output.pen_down == 0 &&
         output.pen_mode == PEN_ON &&
         gtk_events_pending() == FALSE)) {
        // in a "while 1" loop checking for a pen/mouse
        // event with pen(0) or executing input statement.
        gtk_main_iteration_do(TRUE);
    }

    if (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    if (output.break_exec == 1) {
        output.break_exec = 0;
        return -2;
    }
    return 0;
}

void osd_setpenmode(int enable) {
    output.pen_mode = enable;
}

int osd_getpen(int code) {
    if (output.pen_mode == PEN_OFF) {
        gtk_main_iteration_do(TRUE);
    }

    if (output.break_exec == 1) {
        return 1; // break from WHILE PEN(0)=0
    }

    GdkModifierType mask;
    int x,y;

    switch (code) {
    case 0: // return true if there is a waiting pen event (up/down)
        if (output.pen_down != 0) {
            output.pen_down = 0;
            gdk_window_get_pointer(output.widget->window,
                                   &output.pen_down_x,
                                   &output.pen_down_y,
                                   &mask);
            return 1;
        }
        gtk_main_iteration_do(TRUE); // UNTIL PEN(0)
        // fallthru to re-test 

    case 3: // returns true if the pen is down (and save curpos)
        if (output.widget->window == 
            gdk_window_get_pointer(output.widget->window, &x, &y, &mask)) {
            if (mask &
                (GDK_BUTTON1_MASK |
                 GDK_BUTTON2_MASK |
                 GDK_BUTTON3_MASK)) {
                output.pen_down_x = x;
                output.pen_down_y = y;
                return 1;
            }
        }
        return 0;

    case 1:  // last pen-down x
        return output.pen_down_x;

    case 2:  // last pen-down y
        return output.pen_down_y;

    case 4:  // cur pen-down x
    case 10:
        gdk_window_get_pointer(output.widget->window, &x, &y, &mask);
        return x;

    case 5:  // cur pen-down y
    case 11:
        gdk_window_get_pointer(output.widget->window, &x, &y, &mask);
        return y;

    case 12: // true if left button pressed
        return (output.pen_down==1);

    case 13: // true if right button pressed
        return (output.pen_down==3);

    case 14: // true if middle button pressed
        return (output.pen_down==2);
        break;
    }
    return 0;

}

int osd_getx() {
    return output.cur_x;
}

int osd_gety() {
    return output.cur_y;
}

void osd_setxy(int x, int y) {
    output.cur_x = x;
    output.cur_y = y;
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
    invalidate_rect(x, y, x+1, y+1);
}

long osd_getpixel(int x, int y) {
    GdkImage* image = gdk_drawable_get_image(output.pixmap, x, y, 1, 1);
    guint32 px = gdk_image_get_pixel(image, 0, 0);
    g_object_unref(image);
    return px;
}

void osd_line(int x1, int y1, int x2, int y2) {
    gdk_gc_set_rgb_fg_color(output.gc, &output.fg);
    gdk_draw_line(output.pixmap, output.gc, x1, y1, x2, y2);
    osd_refresh();
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
    gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
    gdk_draw_rectangle(output.pixmap, output.gc, bFill, x1, y1, x2-x1, y2-y1);
    osd_refresh();
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

void invalidate_rect(int x1, int y1, int x2, int y2) {
    int x,y,w,h;
    if (x1<x2) {
        x = x1;
        w = x2-x1;
    } else if (x1>x2) {
        x = x2;
        w = x1-x2;
    } else {
        x = x1;
        w = 2;
    }
    if (y1<y2) {
        y = y1;
        h = y2-y1;
    } else if (y1>y2) {
        y = y2;
        h = y1-y2;
    } else {
        y = y1;
        h = 2;
    }
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
    if (output.layout == 0) {
        output.layout = gtk_widget_create_pango_layout(widget, 0);
        pango_layout_set_width(output.layout, -1);
        pango_layout_set_font_description(output.layout, output.font_desc);
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
    output.pen_down = event->button;
    return TRUE;
}

gboolean button_release_event(GtkWidget* w, GdkEventButton *e, gpointer data) {
    output.pen_down = 0;
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
    g_signal_connect(G_OBJECT (drawing_area), "button_press_event",
                     G_CALLBACK (button_press_event), NULL);
    g_signal_connect(G_OBJECT (drawing_area), "button_release_event",
                     G_CALLBACK (button_release_event), NULL);

    gtk_widget_set_events(drawing_area, 
                          GDK_POINTER_MOTION_HINT_MASK |
                          GDK_EXPOSURE_MASK |
                          GDK_BUTTON_PRESS_MASK   |
                          GDK_BUTTON_RELEASE_MASK);

    om_init(drawing_area);
}

/* End of "$Id: output.c,v 1.10 2006-02-10 05:59:58 zeeb90au Exp $". */

