/* -*- c-file-style: "java" -*-
 * $Id: form_ui.c,v 1.27 2006-07-26 11:29:48 zeeb90au Exp $
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

#include "sys.h"
#include "var.h"
#include "kw.h"
#include "pproc.h"
#include "device.h"
#include "smbas.h"
#include "blib_ui.h"

#include <gtk/gtk.h>

#ifdef USE_HILDON
#include <libosso.h>
#endif

#include "output_model.h"

GtkWidget* form = 0;
int modeless = FALSE;
int rows = 1;
int cols = 1;
char buff[40];
extern OutputModel output;

#ifndef USE_HILDON
#define gtk_im_context_show(imctx)
#endif

#define TABLE_GAP 2

typedef enum ControlType {
    ctrl_button,
    ctrl_radio,
    ctrl_check,
    ctrl_text,
    ctrl_label,
    ctrl_list,
    ctrl_grid,
    ctrl_calendar,
    ctrl_file_button,
    ctrl_font_button,
    ctrl_color_button
} ControlType;

typedef struct WidgetInfo {
    ControlType type;
    var_t* var;
} WidgetInfo;

WidgetInfo* get_widget_info(GtkWidget* w) {
    return (WidgetInfo*)g_object_get_data(G_OBJECT(w), "widget_info");
}

void set_widget_info(GtkWidget* w, WidgetInfo* inf) {
    g_object_set_data(G_OBJECT(w), "widget_info", inf);
}

// create the form
void ui_begin() {
    if (form == 0) {
        form = gtk_table_new(rows, cols, FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(form), TABLE_GAP);
        gtk_table_set_row_spacings(GTK_TABLE(form), TABLE_GAP);
        gtk_container_add(GTK_CONTAINER(output.widget), form);
        gtk_widget_show(form);
    }
}

// destroy the form
void ui_reset() {
    if (form != 0) {
        GList* list = gtk_container_get_children(GTK_CONTAINER(form));
        int n = g_list_length(list);
        int i;
        for (i=0; i<n; i++) {
            GtkWidget* w = (GtkWidget*)g_list_nth_data(list, i);
            WidgetInfo* inf = get_widget_info(w);
            g_free(inf);
            gtk_container_remove(GTK_CONTAINER(form), w);
        }
        gtk_container_remove(GTK_CONTAINER(output.widget), form);
        g_list_free(list);
        form = 0;
    }
    rows = 1;
    cols = 1;
    modeless = FALSE;
}

// copy widget data into its matching basic variable
void update_vars(GtkWidget* widget) {
    WidgetInfo* inf = get_widget_info(widget);
    gchar* text = 0;
    guint year, month, day;
    char buf[11]; // DD/MM/YYYY
    GdkColor color;

    switch (inf->type) {
    case ctrl_label:
        if (inf->var->type == V_STR && inf->var->v.p.ptr &&
            g_ascii_strcasecmp(inf->var->v.p.ptr,
                               gtk_label_get_text(GTK_LABEL(widget))) != 0) {
            gtk_label_set_text(GTK_LABEL(widget), inf->var->v.p.ptr);
        }
        break;
    case ctrl_check:
    case ctrl_radio:
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
            v_setstr(inf->var, gtk_button_get_label(GTK_BUTTON(widget)));
        }
        break;
    case ctrl_text:
        text = (gchar*)gtk_entry_get_text(GTK_ENTRY(widget));
        if (text && text[0]) {
            v_setstr(inf->var, text);
        }
        // internal text not freed
        break;
    case ctrl_list:
        text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
        if (text && text[0]) {
            v_setstr(inf->var, text);
        }
        g_free(text);
        break;
    case ctrl_calendar:
        gtk_calendar_get_date(GTK_CALENDAR(widget), &year, &month, &day);
        sprintf(buf, "%02d/%02d/%d", day, month+1, year);
        v_setstr(inf->var, buf);
        break;
    case ctrl_file_button:
        text = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
        if (text && text[0]) {
            v_setstr(inf->var, text);
        }
        g_free(text);
        break;
    case ctrl_font_button:
        text = (char*)gtk_font_button_get_font_name(GTK_FONT_BUTTON(widget));
        if (text && text[0]) {
            v_setstr(inf->var, text);
        }
        break;
    case ctrl_color_button:
        gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
        text = gtk_color_selection_palette_to_string(&color, 1);
        if (text && text[0]) {
            v_setstr(inf->var, text);
        }
        g_free(text);
        break;
    default:
        break;
    }
}

// copy form data into basic variables
void ui_transfer_data() {
    if (form) {
        GList* list = gtk_container_get_children(GTK_CONTAINER(form));
        int n = g_list_length(list);
        int i;
        for (i=0; i<n; i++) {
            update_vars((GtkWidget*)g_list_nth_data(list, i));
        }
        g_list_free(list);
    }
}

// button callback
void button_clicked(GtkWidget* button, gpointer user_data) {
    WidgetInfo* inf = get_widget_info(button);
    v_setstr(inf->var, gtk_button_get_label(GTK_BUTTON(button)));

    if (user_data) {
        // close the form
        output.modal_flag = FALSE;
        if (modeless) {
            ui_reset();
        }
    }
}

// set the radio into a group that shares a common basic variable
void set_radio_group(var_t* v, GtkWidget* radio_widget) {
    GSList *radio_group = NULL;
    if (v == 0 || v->type != V_STR) {
        return;
    }

    GList* list = gtk_container_get_children(GTK_CONTAINER(form));
    int n = g_list_length(list);
    int i;
    for (i=0; i<n; i++) {
        GtkWidget* widget = (GtkWidget*)g_list_nth_data(list, i);
        WidgetInfo* inf = get_widget_info(widget);
        if (inf->type == ctrl_radio &&
            inf->var->type == V_STR &&
            (inf->var == v || inf->var->v.p.ptr == v->v.p.ptr)) {
            radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
            gtk_radio_button_set_group(GTK_RADIO_BUTTON(radio_widget), radio_group);
            break;
        }
    }
    g_list_free(list);
}

// refer to rev 1.10 or less for the obsolete layout version
void add_form_child(GtkWidget* widget, int expand, int x1, int x2, int y1, int y2) {
    int resized = FALSE;
    if (x2 > cols) {
        cols = x2;
        resized = TRUE;
    }
    if (y2 > rows) {
        rows = y2;
        resized = TRUE;
    }
    if (resized) {
        gtk_table_resize(GTK_TABLE(form), rows, cols);
    }

    if (expand) {
        gtk_table_attach(GTK_TABLE(form), widget, x1, x2, y1, y2,
                         (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                         (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
    } else {
        gtk_table_attach(GTK_TABLE(form), widget, x1, x2, y1, y2,
                         (GtkAttachOptions)(GTK_FILL),
                         (GtkAttachOptions)(0), 0, 0);
    }
    if (modeless) {
        gtk_widget_show(widget);
    }
}

// create a row in the grid from the given basic variable
void create_grid_row(var_t* row_p, GtkTreeStore* model, GtkTreeIter* parent_row) {
    GtkTreeIter row_iter;
    int col = 0;
    int i;

    if (row_p->type != V_ARRAY &&
        row_p->type != V_STR &&
        row_p->type != V_INT) {
        return;
    }

    gtk_tree_store_append(model, &row_iter, parent_row);
    if (row_p->type == V_STR) {
        // basic variable is a 1D array, eg: f = files("*.bas")
        gtk_tree_store_set(model, &row_iter, 0, row_p->v.p.ptr, -1);
        return;
    }
    if (row_p->type == V_INT) {
        sprintf(buff, "%d", row_p->v.i);
        gtk_tree_store_set(model, &row_iter, 0, buff, -1);
        return;
    }

    for (col=0, i=0; i<row_p->v.a.size; i++) {
        var_t* col_p = (var_t*)(row_p->v.a.ptr + sizeof(var_t)*col);
        if (col_p->type == V_STR) {
            gtk_tree_store_set(model, &row_iter, col++, col_p->v.p.ptr, -1);
        } else if (col_p->type == V_ARRAY) {
            create_grid_row(col_p, model, &row_iter);
        } else if (row_p->type == V_INT) {
            sprintf(buff, "%d", row_p->v.i);
            gtk_tree_store_set(model, &row_iter, 0, buff, -1);
        }
    }
}

// called when a row is selected
void on_grid_selection(GtkTreeSelection* selection, var_t* v) {
    GtkTreeModel* model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GValue value = {0};
        gtk_tree_model_get_value(model, &iter, 0, &value);
        v_setstr(v, g_value_get_string(&value));
        g_value_unset(&value);
    }
}

// create a grid control type
GtkWidget* create_grid(const char* caption, var_t* v) {
    GtkWidget* view = gtk_tree_view_new();
    GtkCellRenderer* renderer = 0;
    int i;
    int len = caption ? strlen(caption) : 0;
    int n_columns = 0;

    // create the title area from the caption
    for (i=0; i<len; i++) {
        const char* c = strchr(caption+i, '|');
        int end_index = c ? c-caption : len;
        GString* s = g_string_new_len(caption+i, end_index-i);

        GtkTreeViewColumn* col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, s->str);
        gtk_tree_view_column_set_sort_column_id(col, n_columns);
        gtk_tree_view_column_set_sort_indicator(col, TRUE);

        // pack tree view column into tree view
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

        // pack cell renderer into tree view column
        if (renderer == 0) {
            renderer = gtk_cell_renderer_text_new();
        }
        gtk_tree_view_column_pack_start(col, renderer, TRUE);

        // set the 'text' property of the cell renderer to be 
        // populated from the nth column of the tree-view-column
        gtk_tree_view_column_add_attribute(col, renderer, "text", n_columns++);

        i = end_index;
        g_string_free(s, TRUE);
    }

    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(view), TRUE);

    GType* types = (GType*)g_malloc(sizeof(GType)*n_columns);
    for (i=0; i<n_columns; i++) {
        types[i] = G_TYPE_STRING;
    }

    GtkTreeStore* model = gtk_tree_store_newv(n_columns, types);
    g_free(types);
    gtk_tree_view_set_model(GTK_TREE_VIEW(view), 
                            gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(model)));
    g_object_unref(model); // destroy model automatically with view

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

    // if the first row contains a string then 
    // use it as the row selection container
    var_t* sel_row = (var_t*)v->v.a.ptr;
    if (sel_row->type == V_STR) {
        g_signal_connect(G_OBJECT(selection), "changed",
                         G_CALLBACK(on_grid_selection), sel_row);
        i = 1; // populate from second element onwards
    } else {
        i = 0;
    }

    // populate the grid
    for (; i<v->v.a.size; i++) {
        var_t* row_p = (var_t*)(v->v.a.ptr + sizeof(var_t)*i);
        create_grid_row(row_p, model, NULL);
    }

    GtkWidget* scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER(scrolledwindow), view);

    return scrolledwindow;
}

// BUTTON x1, x2, y1, y2, variable, caption [,type]
//
void cmd_button() {
    int x1, x2, y1, y2;
    var_t* v = 0;
    char* caption = 0;
    char* type = 0;

    if (-1 != par_massget("IIIIPSs", &x1, &x2, &y1, &y2, &v, &caption, &type)) {
        GtkWidget* widget = 0;
        WidgetInfo* inf = (WidgetInfo*)g_malloc(sizeof(WidgetInfo));
        inf->var = v;

        ui_begin();
        if (type) {
            if (g_ascii_strcasecmp("radio", type) == 0) {
                inf->type = ctrl_radio;
                widget = gtk_radio_button_new_with_label(NULL, caption);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
                set_radio_group(v, widget);
                g_signal_connect((gpointer)widget, "clicked",
                                 G_CALLBACK(button_clicked),NULL);
            } else if (g_ascii_strcasecmp("checkbox", type) == 0) {
                inf->type = ctrl_check;
                widget = gtk_check_button_new_with_label(caption);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
                g_signal_connect((gpointer)widget, "clicked",
                                 G_CALLBACK(button_clicked), NULL);
            } else if (g_ascii_strcasecmp("submit", type) == 0) {
                inf->type = ctrl_button;
                widget = gtk_button_new_with_mnemonic(caption);
                g_signal_connect((gpointer)widget, "clicked",
                                 G_CALLBACK(button_clicked), (gpointer)TRUE);
            } else if (g_ascii_strcasecmp("label", type) == 0) {
                inf->type = ctrl_label;
                widget = gtk_label_new(caption);
                gtk_misc_set_alignment(GTK_MISC(widget), 0,0);
            } else if (g_ascii_strcasecmp("calendar", type) == 0) {
                inf->type = ctrl_calendar;
                widget = gtk_calendar_new();
                gtk_calendar_display_options(GTK_CALENDAR(widget),
                                             GTK_CALENDAR_SHOW_HEADING |
                                             GTK_CALENDAR_SHOW_DAY_NAMES);
            } else if (g_ascii_strcasecmp("file", type) == 0) {
                inf->type = ctrl_file_button;
                widget = gtk_file_chooser_button_new(caption,
                                                     GTK_FILE_CHOOSER_ACTION_OPEN);
                if (v->type == V_STR && v->v.p.ptr) {
                    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget), v->v.p.ptr);
                }
            } else if (g_ascii_strcasecmp("font", type) == 0) {
                inf->type = ctrl_font_button;
                widget = gtk_font_button_new();
                gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget), caption);
                if (v->type == V_STR && v->v.p.ptr) {
                    gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget), v->v.p.ptr);
                }
            } else if (g_ascii_strcasecmp("color", type) == 0) {
                inf->type = ctrl_color_button;
                widget= gtk_color_button_new();
                if (v->type == V_STR && v->v.p.ptr) {
                    gint n_colors = 1;
                    GdkColor* colors = 0;
                    gtk_color_selection_palette_from_string(v->v.p.ptr, &colors, &n_colors);
                    if (n_colors) {
                        gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), colors);
                    }
                    g_free(colors);
                }
            } else if (g_ascii_strcasecmp("choice", type) == 0) {
                inf->type = ctrl_list;
                widget = gtk_combo_box_new_text();
                // "Easy|Medium|Hard"
                int item_index = 0;
                int len = caption ? strlen(caption) : 0;
                int i;
                for (i=0; i<len; i++) {
                    const char* c = strchr(caption+i, '|');
                    int end_index = c ? c-caption : len;
                    GString* s = g_string_new_len(caption+i, end_index-i);
                    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), s->str);
                    i = end_index;
                    if (v->type == V_STR && v->v.p.ptr &&
                        strcmp((const char*)v->v.p.ptr, s->str) == 0) {
                        // item text same as variable - set selected
                        gtk_combo_box_set_active(GTK_COMBO_BOX(widget), item_index);
                    }
                    item_index++;
                    g_string_free(s, TRUE);
                }
            } else if (g_ascii_strcasecmp("grid", type) == 0) {
                if (v->type != V_ARRAY || caption == 0 || caption[0] == 0) {
                    rt_raise("INVALID GRID BUTTON ARRAY");
                } else {
                    inf->type = ctrl_grid;
                    widget = create_grid(caption, v);
                }
            } else {
                rt_raise("UNKNOWN BUTTON TYPE: %s", type);
            }
            
            if (prog_error) {
                ui_reset();
                pfree2(caption, type);
                g_free(inf);
                return;
            }
        }

        if (widget == 0) {
            // becomes a submit button when not modeless
            inf->type = ctrl_button;
            widget = gtk_button_new_with_mnemonic(caption);
            g_signal_connect((gpointer)widget, "clicked",
                             G_CALLBACK(button_clicked),
                             (gpointer)(modeless ? FALSE : TRUE));
        }

        set_widget_info(widget, inf);
        add_form_child(widget, (inf->type == ctrl_grid), x1, x2, y1, y2);

        // prime input field from variable
        if (v->type == V_STR && v->v.p.ptr &&
            strcmp((const char*)v->v.p.ptr, caption) == 0) {
            if (inf->type == ctrl_check ||
                inf->type == ctrl_radio) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
            }
        }
    }
    pfree2(caption, type);
}

// TEXT x1, x2, y1, y2, variable
// When DOFORM returns the variable contains the user entered value
//
void cmd_text() {
    int x1, x2, y1, y2;
    var_t arg;
    var_t* v = 0;

    v_init(&arg);
    eval(&arg);

    if (arg.type == V_STR) {
        GtkWidget* dialog =
            gtk_message_dialog_new(GTK_WINDOW(output.main_view->parent),
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_OK,
                                   "%s", arg.v.p.ptr);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        v_free(&arg);
        return;
    } else {
        x1 = v_igetval(&arg);
        v_free(&arg);
    }

    if (-1 != par_massget("IIIP", &x2, &y1, &y2, &v)) {
        ui_begin();
        GtkWidget* widget = gtk_entry_new();

        // prime field from var_t
        if (v->type == V_STR && v->v.p.ptr) {
            gtk_entry_set_text(GTK_ENTRY(widget), (const char*)v->v.p.ptr);
        }

        add_form_child(widget, FALSE, x1, x2, y1, y2);
        gtk_entry_set_has_frame(GTK_ENTRY(widget), TRUE);
        gtk_entry_set_max_length(GTK_ENTRY(widget), 100);

        GtkIMContext* imctx = gtk_im_multicontext_new();
        gtk_im_context_set_client_window(imctx, output.widget->window);
        gtk_im_context_focus_in(imctx);
        gtk_im_context_show(imctx);
        gtk_widget_grab_focus(widget);

        WidgetInfo* inf = (WidgetInfo*)g_malloc(sizeof(WidgetInfo));
        inf->var = v;
        inf->type = ctrl_text;
        set_widget_info(widget, inf);
    }
}

// DOFORM [x,y,w,h]
//
// Modal syntax:
//   BUTTON ...
//   DOFORM ...
//
// Modeless syntax:
//   DOFORM 'begin modeless form
//   BUTTON ....
//   DOFORM x,y,w,h
//   DOFORM 'continue modeless form
//
void cmd_doform() {
    int x, y, w, h;
    int num_args;

    x = y = w = h = 0;
    num_args = par_massget("iiii", &x, &y, &w, &h);

    if (form == 0) {
        // begin modeless state
        modeless = TRUE;
        return;
    }

    if (modeless) {
        if (form == 0) {
            rt_raise("UI: FORM HAS CLOSED");
            return;
        }

        // continue modeless state
        gtk_main_iteration_do(TRUE);
        ui_transfer_data();
        if (form == 0 || num_args == 0) {
            // default button click or no dimension args
            return;
        }
    }

    switch (num_args) {
    case 0:
        x = y = w = h = 0;
        break;
    case 2:
        w = h = 0; // updated below
        break;
    case 4:
        break;
    default:
        ui_reset();
        rt_raise("UI: INVALID FORM ARGUMENTS: %d", num_args);
        return;
    }

    if (w < 1 || x+w > output.width) {
        w = output.width-x;
    }
    if (h < 1 || y+h > output.height) {
        h = output.height-y;
    }

    gtk_layout_move(GTK_LAYOUT(output.widget), form, x, y);
    gtk_widget_set_size_request(GTK_WIDGET(form), w, h);
    gtk_widget_grab_focus(form);
    gtk_widget_show_all(form);

    if (modeless == FALSE) {
        output.modal_flag = TRUE;
        while (output.modal_flag && output.break_exec == 0) {
            gtk_main_iteration_do(TRUE);
        }

        ui_transfer_data();
        ui_reset();
    }
}

/* End of "$Id: form_ui.c,v 1.27 2006-07-26 11:29:48 zeeb90au Exp $". */
