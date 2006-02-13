/* -*- c-file-style: "java" -*-
 * $Id: output.c,v 1.16 2006-02-13 12:25:25 zeeb90au Exp $
 * This file is part of SmallBASIC
 *
 * Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License(GPL) from www.gnu.org
 */ 

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <device.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "interface.h"
#include "support.h"
#include "output.h"
#include "output_model.h"

#ifdef USE_HILDON
#include "hildon-lgpl/hildon-widgets/hildon-app.h"
#endif

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
        0, 0,
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
        brun_break();
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
        brun_break();
        return 1; // break from WHILE PEN(0)=0
    }

    GdkModifierType mask;
    int x,y;

    switch(code) {
    case 0: // return true if there is a waiting pen event(up/down)
        if (output.pen_down != 0) {
            gtk_main_iteration_do(FALSE);
            gdk_window_get_pointer(output.widget->window,
                                   &output.pen_down_x,
                                   &output.pen_down_y,
                                   &mask);
            return 1;
        }
        gtk_main_iteration_do(TRUE); // UNTIL PEN(0)
        // fallthru to re-test 

    case 3: // returns true if the pen is down(and save curpos)
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
        return(output.pen_down==1);

    case 13: // true if right button pressed
        return(output.pen_down==3);

    case 14: // true if middle button pressed
        return(output.pen_down==2);
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
    om_reset(TRUE);
    osd_refresh();
}

int osd_textwidth(const char *text) {
    return(text && text[0] ? strlen(text)*output.font_width : 0);
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
    invalidate_rect(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
    gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
    gdk_draw_rectangle(output.pixmap, output.gc, bFill, x1, y1, x2-x1, y2-y1);
    invalidate_rect(x1, y1, x2, y2);
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
 *       vol = volume(0..100)
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
 *  (Its called every ~50ms)
 */
void drvsound_event(void) {
}

gint timeout_callback(gpointer data) {
    output.modal_flag = FALSE;
}

void dev_delay(dword ms) {
    gint timer_id = g_timeout_add(ms, timeout_callback, 0);
    output.modal_flag = TRUE;
    while (output.modal_flag && output.break_exec == 0) {
        gtk_main_iteration_do(TRUE);
    }
    g_source_remove(timer_id);
}

/*
 * Image commmands ...
 */
GdkPixbuf* get_image(dev_file_t* filep, int index) {
    GtkWidget* image = gtk_image_new_from_file(filep->name);
    if (gtk_image_get_storage_type(GTK_IMAGE(image)) != GTK_IMAGE_PIXBUF) {
        return 0;
    }
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    return pixbuf;
}

void dev_image(int handle, int index, int x, int y, 
               int sx, int sy, int w, int h) {
    int imgw = -1;
    int imgh = -1;
    dev_file_t* filep = dev_getfileptr(handle);
    if (filep == 0) {
        return;
    }

    if (filep->open_flags == DEV_FILE_INPUT) {
        GdkPixbuf* pixbuf = get_image(filep, index);
        if (pixbuf) {
            gdk_draw_pixbuf(output.pixmap, 
                            output.gc,
                            pixbuf,
                            sx,sy,x,y,w,h,
                            GDK_RGB_DITHER_NORMAL, 0,0);
            GdkRectangle rc = {x-1, y-1, w+2, h+2};
            gdk_window_invalidate_rect(output.widget->window, &rc, TRUE);
        }
    } else {
        /* output screen area image to jpeg */
    }
}

int dev_image_width(int handle, int index) {
    dev_file_t* filep = dev_getfileptr(handle);
    if (filep == 0 || filep->open_flags != DEV_FILE_INPUT) {
        return 0;
    }
    GdkPixbuf* pixbuf = get_image(filep, index);
    return pixbuf != 0 ? gdk_pixbuf_get_width(pixbuf) : -1;
}

int dev_image_height(int handle, int index) {
    int imgw = -1;
    int imgh = -1;
    dev_file_t* filep = dev_getfileptr(handle);
    if (filep == 0 || filep->open_flags != DEV_FILE_INPUT) {
        return 0;
    }
    GdkPixbuf* pixbuf = get_image(filep, index);
    return pixbuf != 0 ? gdk_pixbuf_get_height(pixbuf) : -1;
}

gboolean in_key_press(GtkWidget* entry, GdkEventKey* event, gpointer data) {
    switch (event->keyval) {
    case GDK_Return:
        output.modal_flag = FALSE;
        return TRUE;
    case GDK_C:
    case GDK_c:
        if (event->state & GDK_CONTROL_MASK) {
            output.break_exec = TRUE;
            return TRUE;
        }
    default:
        break;
    }
    return FALSE;
}

gboolean in_key_release(GtkWidget* entry, GdkEventKey* event, gpointer data) {
    const gchar* value = gtk_entry_get_text(GTK_ENTRY(entry));    
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 1+strlen(value));
    return FALSE;
}

/*
 * Keyboard input command
 */
char* dev_gets(char *dest, int size) {
    GtkWidget* entry = gtk_entry_new();

    gtk_fixed_put(GTK_FIXED(output.widget->parent), entry, 
                  output.cur_x, output.cur_y-2);
    gtk_entry_set_has_frame(GTK_ENTRY(entry), FALSE);
    gtk_entry_set_max_length(GTK_ENTRY(entry), size);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 1);
    gtk_widget_modify_font(entry, output.font_desc);
    gtk_widget_grab_focus(entry);
    gtk_widget_show(entry);

    g_signal_connect(G_OBJECT(entry), "key_press_event", 
                     G_CALLBACK(in_key_press), NULL);
    g_signal_connect(G_OBJECT(entry), "key_release_event", 
                     G_CALLBACK(in_key_release), NULL);

    output.modal_flag = TRUE;
    while (output.modal_flag && output.break_exec == 0) {
        gtk_main_iteration_do(TRUE);
    }

    const gchar* value = gtk_entry_get_text(GTK_ENTRY(entry));
    strcpy(dest, value);
    osd_write(dest);

    /* remove and destroy the entry widget */
    gtk_container_remove(GTK_CONTAINER(output.widget->parent), entry);

    if (output.break_exec == 1) {
        brun_break();
    }

    return dest;
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
        w = 1;
    }
    if (y1<y2) {
        y = y1;
        h = y2-y1;
    } else if (y1>y2) {
        y = y2;
        h = y1-y2;
    } else {
        y = y1;
        h = 1;
    }
    GdkRectangle rc = {x-1, y-1, w+2, h+2};
    gdk_window_invalidate_rect(output.widget->window, &rc, TRUE);
}

/* Callback for maemo hardware keys */
#ifdef USE_HILDON
gboolean key_press_event(GtkWidget* widget, GdkEventKey* event, HildonApp *app)
#else
gboolean key_press_event(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
#endif
{    
    switch(event->keyval) {
    case GDK_Up: // Navigation Key Up
        dev_pushkey(SB_KEY_UP);
        return TRUE;
        
    case GDK_Down: // Navigation Key Down
        dev_pushkey(SB_KEY_DN);
        return TRUE;
        
    case GDK_Left: // Navigation Key Left
        dev_pushkey(SB_KEY_LEFT);
        return TRUE;
        
    case GDK_Right: // Navigation Key Right
        dev_pushkey(SB_KEY_RIGHT);
        return TRUE;
        
    case GDK_Return: // Navigation Key select
        return FALSE;
        
    case GDK_F6: // Full screen
        return TRUE;
        
    case GDK_F7: // Increase(zoom in)
        dev_pushkey(SB_KEY_PGUP);
        return TRUE;
        
    case GDK_F8: // Decrease(zoom out)
        dev_pushkey(SB_KEY_PGDN);
        return TRUE;
        
    case GDK_Escape: // Cancel/Close
        output.break_exec = 1;
        return TRUE;
    default:
        dev_pushkey(event->keyval);
        return TRUE;
    }
    return FALSE;
}

/* Create a new backing pixmap of the appropriate size */
gboolean configure_event(GtkWidget* widget, 
                         GdkEventConfigure *event,
                         gpointer user_data) {
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

    /* connect signals */
    g_signal_connect(G_OBJECT(drawing_area), "expose_event",
                      G_CALLBACK(expose_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area),"configure_event",
                      G_CALLBACK(configure_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "button_press_event",
                     G_CALLBACK(button_press_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "button_release_event",
                     G_CALLBACK(button_release_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "key_press_event", 
                     G_CALLBACK(key_press_event), NULL);

    gtk_widget_set_events(drawing_area, 
                          GDK_KEY_PRESS_MASK |
                          GDK_EXPOSURE_MASK |
                          GDK_BUTTON_PRESS_MASK   |
                          GDK_BUTTON_RELEASE_MASK);

    om_init(drawing_area);
}

/* End of "$Id: output.c,v 1.16 2006-02-13 12:25:25 zeeb90au Exp $". */

