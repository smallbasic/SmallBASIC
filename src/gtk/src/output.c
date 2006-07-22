/* -*- c-file-style: "java" -*-
 * $Id: output.c,v 1.36 2006-07-22 13:16:08 zeeb90au Exp $
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

#include "device.h"
#include "osd.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifdef USE_HILDON
#include <libosso.h>
#include <hildon-widgets/hildon-program.h>
#endif

#include "interface.h"
#include "output.h"
#include "output_model.h"

#ifndef USE_HILDON
#define gtk_im_context_show(imctx)
#endif

extern OutputModel output;

#define PEN_ON  2
#define PEN_OFF 0

GtkWidget* entry = 0;
GtkWidget* html_widget = 0;
int keymap[KEYMAP_LAST+1];
typedef struct {
    GtkWidget* dialog;
    GtkWidget* label;
    GtkIMContext* imctx;
    int index;
} keymap_data;

gboolean key_press_event(GtkWidget* widget, 
                         GdkEventKey* event, 
                         keymap_data* data);

int osd_devinit() {
    os_graphics = 1;
    os_ver = 1;
    os_color = 1;
    os_color_depth = 16;
    os_graf_mx = output.width;
    os_graf_my = output.height;
    setsysvar_str(SYSVAR_OSNAME, "GTK");
    dev_clrkb();
    osd_cls();
    output.break_exec = 0;
}

void osd_setcolor(long color) {
    om_set_fg_color(color);
}

void osd_settextcolor(long fg, long bg) {
    om_set_fg_color(fg);
    om_set_bg_color(bg);
}

void osd_refresh() {
    GdkRectangle rc = {0, 0, os_graf_mx, os_graf_my};
    gdk_window_invalidate_rect(output.widget->window, &rc, TRUE);
}

void close_html() {
    if (html_widget) {
        gtk_container_remove(GTK_CONTAINER(output.widget), html_widget);
        html_widget = 0;
    }
}

int osd_devrestore() {
    ui_reset();
    close_html();
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
        return 0;

    case 3: // returns true if the pen is down(and save curpos)
        if (output.pen_down != 0) {
            gdk_window_get_pointer(output.widget->window,
                                   &output.pen_down_x,
                                   &output.pen_down_y,
                                   &mask);
            return 1;
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
    gint width,height;
    gdk_drawable_get_size(output.pixmap, &width, &height);
    gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
    gdk_draw_rectangle(output.pixmap, output.gc, TRUE, 0, 0, width, height);
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
    gdk_gc_set_rgb_fg_color(output.gc, &output.fg);
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

/*
 *   beep!
 */
void drvsound_beep(void) {
    gdk_beep();
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


void dev_html(const char* html, const char* t, int x, int y, int w, int h) {
    close_html();
    if (html == 0 || html[0] == 0) {
        return;
    }
    // fit within the output box
    if (w < 1 || x+w > output.width) {
        w = output.width-x;
    }
    if (h < 1 || y+h > output.height) {
        h = output.height-y;
    }

    html_widget = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(html_widget);
    gtk_layout_put(GTK_LAYOUT(output.widget), html_widget, x, y);
    gtk_widget_set_size_request(html_widget, w, h);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(html_widget), 
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    GtkWidget* viewport = gtk_viewport_new(NULL, NULL);
    gtk_widget_show(viewport);
    gtk_container_add(GTK_CONTAINER(html_widget), viewport);
        
    GtkWidget* label = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(viewport), label);
    gtk_misc_set_alignment(GTK_MISC(label), 0,0);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_label_set_text(GTK_LABEL(label), html);
    gtk_widget_show(label);
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
        // output screen area image to jpeg
        GdkPixbuf*  pixbuf = 
            gdk_pixbuf_get_from_drawable(NULL, output.pixmap, NULL,
                                         x, y, 0, 0, sx, sy);
        if (pixbuf) {
            GError *error = 0;
            gdk_pixbuf_save(pixbuf, filep->name, "jpeg", &error, 
                            "quality", "100", NULL);
            g_object_unref(pixbuf);
            g_clear_error(&error);
        }
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

gboolean dialog_focus(GtkWidget *dialog,
                      GdkEventFocus *event,
                      GtkIMContext *imctx) {
    if (event->in) {
        gtk_im_context_focus_in(imctx);
        gtk_im_context_show(imctx);
    } else {
        gtk_im_context_focus_out(imctx);
    }
    return FALSE;
}

void press_key(int key) {
    if (entry) {
        // input s$
        char t[] = {key, 0};
        int pos = gtk_editable_get_position(GTK_EDITABLE(entry));
        gtk_editable_insert_text(GTK_EDITABLE(entry), t, 1, &pos);
        gtk_editable_set_position(GTK_EDITABLE(entry), pos);
    } else {
        // k = inkey
        dev_pushkey(key);
    }
}

gboolean im_context_commit(GtkIMContext *ctx,
                           const gchar *str,
                           keymap_data* data) {
    if (str && str[0]) {
        keymap[data->index] = str[0];
        press_key(keymap[data->index]);
        gtk_dialog_response(GTK_DIALOG(data->dialog), GTK_RESPONSE_ACCEPT);
    }
    return TRUE;
}

void handle_key(int index, int def_key, int keyval, keymap_data* data) {
    if (keymap[index]) {
        // press the key
        press_key(keymap[index]);
    } else if (data) {
        // assign default 
        keymap[index] = def_key;
        press_key(keymap[index]);
        if (data->dialog) {
            gtk_dialog_response(GTK_DIALOG(data->dialog), GTK_RESPONSE_ACCEPT);
        }
    } else {
        // ask for key assignment
        keymap_data* data = g_new0(keymap_data, 1);
        data->index = index;
        data->dialog =
            gtk_dialog_new_with_buttons("Map Key",
                                        GTK_WINDOW(output.main_view->parent),
                                        GTK_DIALOG_MODAL |
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_STOCK_CANCEL, 
                                        GTK_RESPONSE_CANCEL,
                                        NULL);
        data->label = gtk_label_new("Press a key (or cancel)");
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(data->dialog)->vbox), 
                                    data->label);
        g_signal_connect(G_OBJECT(data->dialog), "key_press_event",
                         G_CALLBACK(key_press_event), data);
      
        gtk_widget_show_all(data->dialog);

        data->imctx = gtk_im_multicontext_new();
        g_signal_connect(G_OBJECT(data->imctx), "commit",
                         G_CALLBACK(im_context_commit), data);
        gtk_im_context_set_client_window(data->imctx, GTK_WIDGET(data->dialog)->window);
        g_signal_connect(G_OBJECT(data->dialog), "focus-in-event",
                         G_CALLBACK(dialog_focus), data->imctx);
        g_signal_connect(G_OBJECT(data->dialog), "focus-out-event",
                         G_CALLBACK(dialog_focus), data->imctx);

        gtk_dialog_run(GTK_DIALOG(data->dialog));

        // cleanup
        gtk_widget_hide(data->dialog);
        gtk_widget_destroy(data->dialog);
        gtk_im_context_focus_out(data->imctx);
        g_free(data);
    }
}

/* handler for maemo hardware keys */
gboolean key_press_event(GtkWidget* widget, 
                         GdkEventKey* event, 
                         keymap_data* data) {
    switch(event->keyval) {
    case GDK_Up: // Navigation Key Up
        handle_key(KEYMAP_UP, SB_KEY_UP, GDK_Up, data);
        return TRUE;
        
    case GDK_Down: // Navigation Key Down
        handle_key(KEYMAP_DOWN, SB_KEY_DN, GDK_Down, data);
        return TRUE;
        
    case GDK_Left: // Navigation Key Left
        handle_key(KEYMAP_LEFT, SB_KEY_LEFT, GDK_Left, data);
        return TRUE;
        
    case GDK_Right: // Navigation Key Right
        handle_key(KEYMAP_RIGHT, SB_KEY_RIGHT, GDK_Right, data);
        return TRUE;

    case GDK_F7: // Increase(zoom in)
        handle_key(KEYMAP_F7, SB_KEY_PGUP, GDK_F7, data);
        return TRUE;
        
    case GDK_F8: // Decrease(zoom out)
        handle_key(KEYMAP_F8, SB_KEY_PGDN, GDK_F8, data);
        return TRUE;
        
    case GDK_Return: // Navigation Key select
        handle_key(KEYMAP_ENTER, '\n', GDK_Return, data);
        return TRUE;

    case GDK_F6: // Full screen
        handle_key(KEYMAP_F6, SB_KEY_HOME, GDK_F6, data);
        return TRUE;

    case GDK_KP_Enter:
        output.modal_flag = FALSE;
        return TRUE;
    
    case GDK_Escape: // Cancel/Close
        output.break_exec = 1;
        return TRUE;

    case GDK_C:
    case GDK_c:
        if (event->state & GDK_CONTROL_MASK) {
            output.break_exec = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

void input_changed(GtkEditable* editable, GtkWidget* entry) {
    // adjust the size of the input to fix the text
    const gchar* value = gtk_entry_get_text(GTK_ENTRY(entry));
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 1+strlen(value));
}

/*
 * Keyboard input command
 */
char* dev_gets(char *dest, int size) {
    entry = gtk_entry_new();

    gtk_layout_put(GTK_LAYOUT(output.widget), entry, 
                   output.cur_x-1, output.cur_y-1);
    gtk_entry_set_has_frame(GTK_ENTRY(entry), FALSE);
    gtk_entry_set_max_length(GTK_ENTRY(entry), size);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 1);
    gtk_entry_set_alignment(GTK_ENTRY(entry), 0);
    gtk_widget_modify_font(entry, output.font_desc);
    g_signal_connect(G_OBJECT(entry), "key_press_event",
                     G_CALLBACK(key_press_event), NULL);
    g_signal_connect(G_OBJECT(entry), "changed",
                     G_CALLBACK(input_changed), entry);
    gtk_widget_show(entry);

    GtkIMContext* imctx = gtk_im_multicontext_new();
    gtk_im_context_set_client_window(imctx, output.widget->window);
    gtk_im_context_focus_in(imctx);
    gtk_im_context_show(imctx);
    gtk_widget_grab_focus(entry);

    output.modal_flag = TRUE;
    while (output.modal_flag && output.break_exec == 0) {
        gtk_main_iteration_do(TRUE);
    }

    gtk_im_context_focus_out(imctx);

    const gchar* value = gtk_entry_get_text(GTK_ENTRY(entry));
    strcpy(dest, value);
    osd_write(dest);

    // remove and destroy the entry widget
    gtk_container_remove(GTK_CONTAINER(output.widget), entry);
    entry = 0;
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

/* Create a new backing pixmap of the appropriate size */
void on_realize(GtkWidget* widget, gpointer user_data) {
    if (output.gc == 0) {
        // deferred init to here since we don't run gtk_main()
        output.gc = gdk_gc_new(widget->window);
        om_reset(TRUE); 
    }

    if (output.layout == 0) {
        output.layout = gtk_widget_create_pango_layout(output.widget, 0);
        pango_layout_set_width(output.layout, -1);
        pango_layout_set_font_description(output.layout, output.font_desc);
    }

    if (output.pixmap) {
        // copy old image onto new/resized image
        int old_w, old_h;
        gdk_drawable_get_size(output.pixmap, &old_w, &old_h);
        int w = MAX(output.width, old_w);
        int h = MAX(output.height, old_h);

        GdkPixmap* pixmap = gdk_pixmap_new(widget->window, w, h, -1);
        gdk_gc_set_rgb_fg_color(output.gc, &output.bg);
        gdk_draw_rectangle(pixmap, output.gc, TRUE, 0, 0, w, h);
        gdk_draw_drawable(pixmap,
                          output.gc,
                          output.pixmap,
                          0,0,0,0, // src/dest x/y
                          old_w, old_h);
        g_object_unref(output.pixmap);
        output.pixmap = pixmap;
    } else {
        // create a new pixmap
        output.pixmap =
            gdk_pixmap_new(widget->window, output.width, output.height, -1);
        osd_cls();
    }
}

/* called when ever the client area size changes */
void on_size_allocate(GtkWidget* widget,
                      GdkRectangle* allocation, 
                      gpointer user_data) {
    if (output.width != allocation->width ||
        output.width != allocation->width) {
        output.width = allocation->width;
        output.height = allocation->height;
        if (widget->window) {
            on_realize(widget, user_data);
        }
    }
    return;
}

/* Redraw the screen from the backing pixmap */
gboolean expose_event(GtkWidget* widget, GdkEventExpose* event) {
    if (output.pixmap) {
        gdk_draw_drawable(GTK_LAYOUT(widget)->bin_window,
                          output.gc, 
                          output.pixmap,
                          event->area.x, event->area.y, // src 
                          event->area.x, event->area.y, // dest
                          event->area.width, event->area.height);
    }
    return FALSE;
}

gboolean button_press_event(GtkWidget *widget, GdkEventButton *event) {
    GdkModifierType mask;

    output.pen_down = event->button;
    if (output.pen_down) {
        gdk_window_get_pointer(output.widget->window,
                               &output.pen_down_x,
                               &output.pen_down_y,
                               &mask);
    }
    return TRUE;
}

gboolean button_release_event(GtkWidget* w, GdkEventButton *e, gpointer data) {
    output.pen_down = 0;
    return TRUE;
}

gboolean drawing_area_init(GtkWidget *main_window) {
    GtkWidget *drawing_area = 
        g_object_get_data(G_OBJECT(main_window), "drawing_area");

    output.main_view = main_window;

    // connect signals
    g_signal_connect(G_OBJECT(drawing_area), "size_allocate",
                     G_CALLBACK(on_size_allocate), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "realize",
                      G_CALLBACK(on_realize), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event",
                     G_CALLBACK(expose_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "button_press_event",
                     G_CALLBACK(button_press_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "button_release_event",
                     G_CALLBACK(button_release_event), NULL);
    g_signal_connect(G_OBJECT(main_window), "key_press_event",
                     G_CALLBACK(key_press_event), NULL);

    gtk_widget_set_events(drawing_area, 
                          GDK_KEY_PRESS_MASK |
                          GDK_EXPOSURE_MASK |
                          GDK_BUTTON_PRESS_MASK   |
                          GDK_BUTTON_RELEASE_MASK);

    om_init(drawing_area);
}

/* End of "$Id: output.c,v 1.36 2006-07-22 13:16:08 zeeb90au Exp $". */

