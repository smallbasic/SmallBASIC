// This file is part of SmallBASIC
//
// SmallBASIC: low-level platform driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2001 Nicholas Christopoulos

/**
 * @defgroup lgraf Low-level graphics/sound driver
 */

#if !defined(_osd_h)
#define _osd_h

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup lgraf
 *
 * initialize driver
 *
 * @return non-zero on success
 */
int osd_devinit(void);

/**
 * @ingroup lgraf
 *
 * close driver
 *
 * @return non-zero on success
 */
int osd_devrestore(void);

/**
 * @ingroup lgraf
 *
 * the driver must check its events
 *
 * the keyboard events must be store in SB's keyboard buffer by using,
 * the dev_pushkey()
 *
 * @return
 *   0 if there is not new events in the queue.
 *   >0 the number of the new events.
 *   -1 that means BREAK (brun_break() it was called).
 *   -2 that means BREAK (executor displays "BREAK" message).
 */
int osd_events(int wait_flag);

/**
 * @ingroup lgraf
 *
 *   clear screen
 */
void osd_cls(void);

/**
 * @ingroup lgraf
 *
 *   set current position
 *
 * @param x the x position in pixels
 * @param y the y position in pixels
 */
void osd_setxy(int x, int y);

/**
 * @ingroup lgraf
 *
 *   return the current x position
 *
 *   @return the current x position
 */
int osd_getx(void);

/**
 * @ingroup lgraf
 *
 *   return the current y position
 *
 *   @return the current y position
 */
int osd_gety(void);

/**
 * @ingroup lgraf
 *
 * returns the width of the text in pixels
 *
 * @param str the text
 * @return the width of the text in pixels
 */
int osd_textwidth(const char *str);

/**
 * @ingroup lgraf
 *
 * returns the height of the text in pixels
 *
 * @param str the text
 * @return the height of the text in pixels
 */
int osd_textheight(const char *str);

/**
 * @ingroup lgraf
 *
 * enable/disable pen/mouse driver.
 *
 * @note PEN ON/OFF command
 *
 * @param enable non-zero to enable pen/mouse driver.
 */
void osd_setpenmode(int enable);

/**
 * @ingroup lgraf
 *
 * Mouse & lightpen!
 *
 * Since 1988 the mouse was an new device for PC's, there is no mouse support on QB.
 *
 * <pre>
 PEN, Lightpen & Mouse API
 ================================
 PEN(0) -> true (non zero) if there is a new pen or mouse event
 PEN(1) -> PEN: last pen-down x; MOUSE: last mouse button down x
 PEN(2) -> PEN: last pen-down y; MOUSE: last mouse button down y
 PEN(3) -> QB compatiblity, don't use it
 PEN(4) -> PEN: last/current x, MOUSE: the current x position only
 if the left mouse button is pressed (like PEN is down)
 PEN(5) -> PEN: last/current y, MOUSE: the current y position only
 if the left mouse button is pressed (like PEN is down)
 PEN(6) -> QB compatiblity, don't use it
 PEN(7) -> QB compatiblity, don't use it
 PEN(8) -> QB compatiblity, don't use it
 PEN(9) -> QB compatiblity, don't use it

 Mouse buttons:
 PEN(10) -> current mouse x position
 PEN(11) -> current mouse y position
 PEN(12) -> true if the left mouse button is pressed
 PEN(13) -> true if the right mouse button is pressed
 PEN(14) -> true if the middle mouse button is pressed
 * </pre>
 *
 * @note The PEN(x) function
 *
 * @param code is the information code
 * @return a value based on 'code'
 */
int osd_getpen(int mode);

/**
 * @ingroup lgraf
 *
 * writes a string to the SB's console
 *
 * this routine must supports the following control codes:
 *
 * <pre>
 \t    tab
 \a    beep
 \n    next line (cr/lf)
 \xC   clear screen
 \e[K  clear to end of line
 \e[0m reset all attributes to their defaults
 \e[1m set bold on
 \e[4m set underline on
 \e[7m reverse video
 \e[21m  set bold off
 \e[24m  set underline off
 \e[27m  set reverse off
 \e[nG move cursor to specified column
 \e[8xm  select system font (x is the font-number)
 if there are no fonts, do nothing
 \e[9xm  select buildin font (x is the font-number)
 if there are no fonts, do nothing
 * </pre>
 *
 * @param str the string
 */
void osd_write(const char *str);

/**
 * @ingroup lgraf
 *
 * sets the foreground color. if the color is >= 0 the driver uses the standard 16 VGA colors,
 * if the color is negative, the driver it must use RGB (-color) color value.
 *
 * @param color the foreground color
 */
void osd_setcolor(long color);

/**
 * @ingroup lgraf
 *
 * sets the foreground and the background color.
 * if the color is >= 0 the driver uses the standard 16 VGA colors,
 * if the color is negative, the driver it must use RGB (-color) color value.
 *
 * if the fg or the bg has -1 value, the settextcolor must ignore them.
 * (-1 means use the current color)
 *
 * @param fg the foreground color
 * @param bg the background color
 */
void osd_settextcolor(long fg, long bg);

/**
 * @ingroup lgraf
 *
 * sets the pixel's value
 *
 * @param x the x position
 * @param y the y position
 */
void osd_setpixel(int x, int y);

/**
 * @ingroup lgraf
 *
 * Returns the pixel's value.
 * Since in SB the basic colors are the 16 standard VGA colors,
 * the driver must returns 0-15 if the pixel match to VGA palette
 * or -RGB if it is not.
 *
 * @param x the x position
 * @param y the y position
 * @return the color
 */
long osd_getpixel(int x, int y);

/**
 * @ingroup lgraf
 *
 * draw a line using foreground color
 *
 * @param x1 line coordinates
 * @param y1 line coordinates
 * @param x2 line coordinates
 * @param y2 line coordinates
 */
void osd_line(int x1, int y1, int x2, int y2);

/**
 * @ingroup lgraf
 *
 *   draw parallelogram (filled or not)
 *
 * @param x1 upper-left corner
 * @param y1 upper-left corner
 * @param x2 lower-right corner
 * @param y2 lower-right corner
 * @param fill non-zero to fill it with foreground color
 */
void osd_rect(int x1, int y1, int x2, int y2, int fill);


/**
 * @ingroup lgraf
 *
 * plays an OGG or MP3 file
 *
 * @param path the path to the OGG or MP3 file
 */
void osd_audio(const char *path);

/**
 * @ingroup lgraf
 *
 * produce a tone. if the driver has no sound, it can use
 * an add-on driver like dev_oss, or do nothing.
 *
 * the background play means to store the tones on a queue
 * and play them in its time.
 *
 * osd_refresh() can help on that if the system does not
 * supports threads.
 *
 * @param freq is the frequency
 * @param dur_ms is the duration in milliseconds
 * @param vol_prc is the volume (0-99)
 * @param bgplay non-zero for play the tone in background
 */
void osd_sound(int frq, int dur, int vol, int bgplay);

/**
 * @ingroup lgraf
 *
 * clears background-sound queue
 */
void osd_clear_sound_queue();

/**
 * @ingroup lgraf
 *
 * produce the standard system's beep :)
 */
void osd_beep(void);

/**
 * @ingroup lgraf
 *
 * this routine it is called by SB every ~50ms.
 *   if framebuffer technique is used; this routine must write buffer to video-ram
 */
void osd_refresh(void);

#if defined(__cplusplus)
}
#endif
#endif
