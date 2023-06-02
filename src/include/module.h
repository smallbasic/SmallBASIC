// This file is part of SmallBASIC
//
// SmallBASIC module header
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2018 Chris Warren-Smith

#include <stdint.h>

#if !defined(_INC_MODULE_H)
#define _INC_MODULE_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
  // the parameter
  var_t *var_p;

  // whether the parameter can be used by reference
  uint8_t byref;
} slib_par_t;

/**
 * @ingroup modstd
 *
 * Initialize the library. Called by module manager on loading.
 *
 * @return non-zero on success
 */
int sblib_init(const char *sourceFile);

/**
 * @ingroup modstd
 *
 * Closes the library. Called by module manager on unload.
 */
void sblib_close(void);

/**
 * @ingroup modstd
 *
 * plugin based event handling
 */
int sblib_events(int wait_flag, int *w, int *h);

/**
 * @ingroup modlib
 *
 * returns the number of procedures that are supported by the library
 *
 * @return the number of the procedures
 */
int sblib_proc_count(void);

/**
 * @ingroup modlib
 *
 * returns the name of the procedure 'index'
 *
 * @param index the procedure's index
 * @param proc_name the buffer to store the name
 * @return non-zero on success
 */
int sblib_proc_getname(int index, char *proc_name);

/**
 * @ingroup modlib
 *
 * executes a procedure
 *
 * the retval can be used to returns an error-message
 * in case of an error.
 *
 * @param index the procedure's index
 * @param param_count the number of the parameters
 * @param params the parameters table
 * @param retval a var_t object to set the return value
 * @return non-zero on success
 */
int sblib_proc_exec(int index, int param_count, slib_par_t *params, var_t *retval);

/**
 * @ingroup modlib
 *
 * returns the number of functions that are supported by the library
 *
 * @return the number of the functions
 */
int sblib_func_count(void);

/**
 * @ingroup modlib
 *
 * returns the name of the function 'index'
 *
 * @param index the function's index
 * @param func_name the buffer to store the name
 * @return non-zero on success
 */
int sblib_func_getname(int index, char *func_name);

/**
 * @ingroup modlib
 *
 * executes a function
 *
 * @param index the procedure's index
 * @param param_count the number of the parameters
 * @param params the parameters table
 * @param retval a var_t object to set the return value
 * @return non-zero on success
 */
int sblib_func_exec(int index, int param_count, slib_par_t *params, var_t *retval);

/**
 * @ingroup modlib
 *
 * executes a function
 *
 * @param cls_id the variable class identifier
 * @param id the variable instance identifier
 */
void sblib_free(int cls_id, int id);

/**
 * @ingroup modlib
 *
 * overrides for osd_xx functions
 */
int  sblib_getpen(int code);
int  sblib_getx();
int  sblib_gety();
int  sblib_textheight(const char *str);
int  sblib_textwidth(const char *str);
long sblib_getpixel(int x, int y);
void sblib_arc(int xc, int yc, double r, double as, double ae, double aspect);
void sblib_audio(const char *path);
void sblib_beep();
void sblib_clear_sound_queue();
void sblib_cls();
void sblib_devinit(const char *prog, int width, int height);
void sblib_ellipse(int xc, int yc, int xr, int yr, int fill);
void sblib_line(int x1, int y1, int x2, int y2);
void sblib_rect(int x1, int y1, int x2, int y2, int fill);
void sblib_refresh();
void sblib_setcolor(long color);
void sblib_setpenmode(int enable);
void sblib_setpixel(int x, int y);
void sblib_settextcolor(long fg, long bg);
void sblib_setxy(int x, int y);
void sblib_sound(int frq, int ms, int vol, int bgplay);
void sblib_write(const char *str);

#if defined(__cplusplus)
}
#endif

#endif
