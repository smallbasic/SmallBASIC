/**
 * @file blib.h
 *
 * SmallBASIC: The Run-Time-Library
 *
 * 2000-05-27, Nicholas Christopoulos
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */

/**
 * @defgroup date Date and Time
 */

#if !defined(_blib_h)
#define _blib_h

#include "sys.h"
#include "var.h"

#if defined(__cplusplus)
extern "C" {
#endif

void dump_stack(void) SEC(TRASH);

// first class
void cmd_udp(int) SEC(BLIB);
void cmd_call_unit_udp(int cmd, int udp_tid, addr_t goto_addr, addr_t rvid);
void cmd_udpret(void) SEC(BLIB);
void cmd_crvar(void) SEC(BLIB);
void cmd_param(void) SEC(BLIB);
int cmd_exit(void) SEC(BLIB);
void cmd_RTE(void) SEC(BLIB);

void cmd_let(int);
void cmd_dim(int);
void cmd_redim(void) SEC(BLIB);
void cmd_ladd(void) SEC(BLIB);
void cmd_lins(void) SEC(BLIB);
void cmd_ldel(void) SEC(BLIB);
void cmd_erase(void) SEC(BLIB);
void cmd_print(int output) SEC(BLIB);
void logprint_var(var_t * var) SEC(TRASH);
void cmd_logprint(void) SEC(TRASH);
void cmd_input(int input) SEC(BLIB);
void cmd_if(void) SEC(BLIB);
void cmd_else(void) SEC(BLIB);
void cmd_elif(void) SEC(BLIB);
void cmd_for(void) SEC(BLIB);
void cmd_next(void) SEC(BLIB);
void cmd_while(void) SEC(BLIB);
void cmd_wend(void) SEC(BLIB);
void cmd_until(void) SEC(BLIB);
void cmd_select(void) SEC(BLIB);
void cmd_case(void) SEC(BLIB);
void cmd_case_else(void) SEC(BLIB);
void cmd_end_select(void) SEC(BLIB);
void cmd_gosub(void) SEC(BLIB);
void cmd_return(void) SEC(BLIB);
void cmd_on_go(void) SEC(BLIB);
void cmd_read(void) SEC(BLIB);
void cmd_data(void) SEC(BLIB);
void cmd_restore(void) SEC(TRASH);
void cmd_sort(void) SEC(BLIB);
void cmd_search(void) SEC(BLIB);
void cmd_swap(void) SEC(BLIB);

void cmd_chain(void) SEC(BLIB);
void cmd_run(int) SEC(BLIB);
void cmd_poke(void) SEC(BLIB);
void cmd_poke16(void) SEC(BLIB);
void cmd_poke32(void) SEC(BLIB);
void cmd_bcopy(void) SEC(BLIB);
void cmd_calladr(void) SEC(BLIB);

var_num_t cmd_math1(long funcCode, var_t *arg) SEC(BMATH);
var_int_t cmd_imath1(long funcCode, var_t *arg) SEC(BMATH);
void cmd_str1(long funcCode, var_t *arg, var_t *r) SEC(BMATH);
void cmd_ns1(long funcCode, var_t *arg, var_t *r) SEC(BMATH);
void cmd_str0(long funcCode, var_t *r) SEC(BMATH);
void cmd_split(void) SEC(BLIB);
void cmd_wsplit(void) SEC(BLIB);
void cmd_wjoin(void) SEC(BLIB);
void cmd_environ(void) SEC(BIO2);
void cmd_datedmy(void) SEC(BMATH);
void cmd_timehms(void) SEC(BMATH);
void cmd_deriv(void) SEC(BMATH);
void cmd_diffeq(void) SEC(BMATH);

// not basic, but speed is needed
void graph_reset(void) SEC(BLIB); // graphics module - reset
void cmd_pset(void) SEC(BIO2);
void cmd_line(void) SEC(BIO2);
void cmd_rect(void) SEC(BIO2);
void cmd_circle(void) SEC(BIO2);
void cmd_arc(void) SEC(BIO2);
char *draw_getval(const char *src, int *c) SEC(BIO2);
void cmd_draw(void) SEC(BIO2);
void cmd_paint(void) SEC(BIO2);
void cmd_pen(void) SEC(BIO2);
void cmd_view(void) SEC(BIO2);
void cmd_window(void) SEC(BIO2);
void chart_draw(int x1, int y1, int x2, int y2, double *vals, int count,
                double *xvals, int xcount, int chart, int marks) SEC(BIO2);
void cmd_chart_fstr(double v, char *buf) SEC(BIO2);
void cmd_chart(void) SEC(BIO2);
void cmd_drawpoly(void) SEC(BIO2);
var_t *par_getm3(void) SEC(BIO2);
void m3ident(double[3][3]) SEC(BIO2);
void m3combine(var_t *, double[3][3]) SEC(BIO2);
void cmd_m3ident(void) SEC(BIO2);
void cmd_m3rotate(void) SEC(BIO2);
void cmd_m3scale(void) SEC(BIO2);
void cmd_m3translate(void) SEC(BIO2);
void cmd_m3apply(void) SEC(BIO2);
void cmd_intersect(void) SEC(BIO2);
void cmd_polyext(void) SEC(BIO2);
void cmd_exprseq(void) SEC(BIO2);
void cmd_plot(void) SEC(BIO2);
void cmd_plot2(void) SEC(BIO2);

// third class
void cmd_strN(long, var_t *) SEC(BMATH);  // its ok, i want all functions
// to BMATH
void cmd_intN(long, var_t *) SEC(BMATH);
void cmd_numN(long, var_t *) SEC(BMATH);
void cmd_genfunc(long, var_t *) SEC(BMATH);
var_num_t line_length(var_num_t Ax, var_num_t Ay, var_num_t Bx, var_num_t By) SEC(BMATH);
double line_segangle(int type, double Adx, double Ady, double Bdx,
                     double Bdy) SEC(BMATH);
void cmd_integral(void) SEC(BMATH);
void cmd_root() SEC(BMATH);

void cmd_randomize(void) SEC(BIO2);
void cmd_at(void) SEC(BIO2);
void cmd_locate(void) SEC(BIO2);
void cmd_color(void) SEC(BIO2);

void cmd_beep(void) SEC(BIO2);
void cmd_sound(void) SEC(BIO2);
void cmd_nosound(void) SEC(BIO2);
void cmd_play_reset(void) SEC(BIO2);
void cmd_play(void) SEC(BIO2);
void cmd_pause(void) SEC(BIO2);
void cmd_delay(void);

// FILE I/O
void cmd_fopen(void) SEC(BIO2);
void cmd_fclose(void) SEC(BIO2);
void cmd_fprint(void) SEC(BIO2);
void cmd_finput(void) SEC(BIO2);
void cmd_fwrite(void) SEC(BIO2);
void cmd_fread(void) SEC(BIO2);
void cmd_flineinput(void) SEC(BIO2);
void cmd_fkill(void) SEC(BIO2);
void cmd_fseek(void) SEC(BIO2);
void cmd_filecp(int mv) SEC(BIO2);
void cmd_chdir(void) SEC(BIO2);
void cmd_mkdir(void) SEC(BIO2);
void cmd_rmdir(void) SEC(BIO2);
void cmd_floadln(void) SEC(BIO2);
void cmd_fsaveln(void) SEC(BIO2);
void cmd_flock(void) SEC(BIO2);
void cmd_chmod(void) SEC(BIO2);
void cmd_dirwalk(void) SEC(BIO2);
void cmd_bputc(void) SEC(BIO2);
void cmd_bload(void) SEC(BIO2);
void cmd_bsave(void) SEC(BIO2);

void cmd_html(void) SEC(BMATH);
void cmd_image(void) SEC(BMATH);

// blib_func

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
long date_julian(long d, long m, long y) SEC(BMATH2);

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
void date_fmt(char *fmt, char *buf, long d, long m, long y) SEC(BMATH2);

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
void date_str2dmy(char *str, long *d, long *m, long *y) SEC(BMATH2);

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
void date_str2hms(char *str, long *h, long *m, long *s) SEC(BMATH2);

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
void date_jul2dmy(long j, long *d, long *m, long *y) SEC(BMATH2);

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
void date_tim2hms(long t, long *h, long *m, long *s) SEC(BMATH2);

#if defined(__cplusplus)
}
#endif
#endif
