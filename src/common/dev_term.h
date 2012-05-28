/*
 * SmallBASIC platform driver for Unix,
 * UNIX TERMINAL DRIVER
 *
 * 2001-12-12, Nicholas Christopoulos
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */

#if !defined(_sb_term_h)
#define _sb_term_h

#if defined(__cplusplus)
extern "C" {
#endif

int term_init();
int term_restore();
int term_events();
int term_israw();
void term_getsdraw(char *dest, int pos, int tm);
void term_print(const char *str);
int term_getx();
int term_gety();
void term_setxy(int x, int y);
void term_settextcolor(int fg, int bg);
void term_cls();
void term_settab(int tabsize);
void term_drawpoint(int x, int y);
int term_getpoint(int x, int y);
void term_drawline(int x1, int y1, int x2, int y2);
void term_drawrect(int x1, int y1, int x2, int y2, int fill);
void term_setcursor(int style);
int term_rows();
int term_cols();
void term_recalc_size();
int term_getch();             // dev_getch() !!!

#if defined(__cplusplus)
}
#endif
#endif
