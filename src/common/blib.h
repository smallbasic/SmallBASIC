// This file is part of SmallBASIC
//
// SmallBASIC RTL - STANDARD COMMANDS
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup date Date and Time
 */

#if !defined(_blib_h)
#define _blib_h

#include "common/sys.h"
#include "common/var.h"

#if defined(__cplusplus)
extern "C" {
#endif

void dump_stack(void);

// first class
void cmd_udp(int);
bcip_t cmd_push_args(int cmd, bcip_t goto_addr, bcip_t rvid);
void cmd_call_unit_udp(int cmd, int udp_tid, bcip_t goto_addr, bcip_t rvid);
void cmd_udpret(void);
void cmd_crvar(void);
void cmd_param(void);
int cmd_exit(void);
void cmd_let(int);
void cmd_let_opt();
void cmd_packed_let();
void cmd_dim(int);
void cmd_redim(void);
void cmd_append(void);
void cmd_append_opt(void);
void cmd_lins(void);
void cmd_ldel(void);
void cmd_erase(void);
void cmd_print(int output);
void logprint_var(var_t *var);
void cmd_input(int input);
void cmd_if(void);
void cmd_else(void);
void cmd_elif(void);
void cmd_endif(void);
void cmd_for(void);
void cmd_next(void);
void cmd_while(void);
void cmd_wend(void);
void cmd_until(void);
void cmd_repeat(void);
void cmd_select(void);
void cmd_case(void);
void cmd_case_else(void);
void cmd_end_select(void);
void cmd_gosub(void);
void cmd_return(void);
void cmd_on_go(void);
void cmd_read(void);
void cmd_data(void);
void cmd_restore(void);
void cmd_sort(void);
void cmd_search(void);
void cmd_swap(void);
void cmd_chain(void);
void cmd_run(int);
void cmd_try();
void cmd_catch();
void cmd_end_try();
void cmd_call_vfunc();
void cmd_timer();

var_num_t cmd_math0(long funcCode);
var_num_t cmd_math1(long funcCode, var_t *arg);
var_int_t cmd_imath0(long funcCode);
var_int_t cmd_imath1(long funcCode, var_t *arg);
void cmd_str1(long funcCode, var_t *arg, var_t *r);
void cmd_ns1(long funcCode, var_t *arg, var_t *r);
void cmd_str0(long funcCode, var_t *r);
void cmd_split(void);
void cmd_wsplit(void);
void cmd_wjoin(void);
void cmd_environ(void);
void cmd_datedmy(void);
void cmd_timehms(void);
void cmd_deriv(void);
void cmd_diffeq(void);

// not basic, but speed is needed
void graph_reset(void);
void cmd_pset(void);
void cmd_line(void);
void cmd_rect(void);
void cmd_circle(void);
void cmd_arc(void);
char *draw_getval(const char *src, int *c);
void cmd_draw(void);
void cmd_paint(void);
void cmd_pen(void);
void cmd_view(void);
void cmd_window(void);
void chart_draw(int x1, int y1, int x2, int y2, 
                var_num_t *vals, 
                int count, var_num_t *xvals, int xcount,
                int chart, int marks);
void cmd_chart_fstr(var_num_t v, char *buf);
void cmd_chart(void);
void cmd_drawpoly(void);
var_t *par_getm3(void);
void m3ident(var_num_t[3][3]);
void m3combine(var_t *, var_num_t[3][3]);
void cmd_m3ident(void);
void cmd_m3rotate(void);
void cmd_m3scale(void);
void cmd_m3translate(void);
void cmd_m3apply(void);
void cmd_intersect(void);
void cmd_polyext(void);
void cmd_exprseq(void);
void cmd_plot(void);

// third class
void cmd_strN(long, var_t*);  // its ok, i want all functions
// to BMATH
void cmd_intN(long, var_t*);
void cmd_numN(long, var_t*);
void cmd_genfunc(long, var_t*);
var_num_t line_length(var_num_t Ax, var_num_t Ay, var_num_t Bx, var_num_t By);
void cmd_integral(void);
void cmd_root();

void cmd_randomize(void);
void cmd_at(void);
void cmd_locate(void);
void cmd_color(void);

void cmd_beep(void);
void cmd_sound(void);
void cmd_nosound(void);
void cmd_play_reset(void);
void cmd_play(void);
void cmd_pause(void);
void cmd_delay(void);

// FILE I/O
void cmd_fopen(void);
void cmd_fclose(void);
void cmd_fwrite(void);
void cmd_fread(void);
void cmd_flineinput(void);
void cmd_fkill(void);
void cmd_fseek(void);
void cmd_filecp(int mv);
void cmd_chdir(void);
void cmd_mkdir(void);
void cmd_rmdir(void);
void cmd_floadln(void);
void cmd_fsaveln(void);
void cmd_flock(void);
void cmd_chmod(void);
void cmd_dirwalk(void);
void cmd_bputc(void);
void cmd_bload(void);
void cmd_bsave(void);
void cmd_definekey(void);

/**
 * @ingroup date
 *
 * returns the julian-date
 *
 * @param d is the day
 * @param m is the month
 * @param y is the year
 * @return the julian-date
 */
long date_julian(long d, long m, long y);

/**
 * @ingroup date
 *
 * formats a date string
 *
 * format
 @code
 d    = day
 dd   = day, zero-leaded if < 10
 ddd  = the weekday with 3 chars
 dddd = the weekday

 m    = month
 mm   = month, zero-leaded if < 10
 mmm  = the month-name with 3 chars
 mmmm = the month-name

 yy   = year in 2-digits
 yyyy = year in 4-digits
 @endcode
 *
 * @param fmt the format
 * @param buf the destination buffer
 * @param d is the day
 * @param m is the month
 * @param y is the year
 * @return the julian-date
 */
char *date_fmt(char *fmt, long d, long m, long y);

/**
 * @ingroup date
 *
 * returns the day,month,year from a string-date
 *
 * @param str is the string that contains the date
 * @param d is the day
 * @param m is the month
 * @param y is the year
 */
void date_str2dmy(char *str, long *d, long *m, long *y);

/**
 * @ingroup date
 *
 * returns the hour,minutes,seconds from a string-time
 *
 * @param str is the string that contains the date
 * @param h is the hours
 * @param m is the minutes
 * @param s is the seconds
 */
void date_str2hms(char *str, long *h, long *m, long *s);

/**
 * @ingroup date
 *
 * returns the day,month,year from a julian date
 *
 * @param j is the julian-date
 * @param d is the day
 * @param m is the month
 * @param y is the year
 */
void date_jul2dmy(long j, long *d, long *m, long *y);

/**
 * @ingroup date
 *
 * returns the hour,minutes,seconds from a timer (TIMER or time_t)
 *
 * @param t is the seconds from midnight
 * @param h is the hours
 * @param m is the minutes
 * @param s is the seconds
 */
void date_tim2hms(long t, long *h, long *m, long *s);

#if defined(__cplusplus)
}
#endif
#endif
