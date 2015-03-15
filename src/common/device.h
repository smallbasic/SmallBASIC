// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup dev High-level drivers
 */
/**
 * @defgroup dev_g High-level graphics driver
 */
/**
 * @defgroup dev_s High-level sound driver
 */
/**
 * @defgroup dev_i High-level input driver
 */
/**
 * @defgroup dev_f High-level file-system driver
 */

#if !defined(_device_h)
#define _device_h

#include "common/sys.h"
#if !defined(SCAN_MODULE)
#include "common/var.h"
#endif

#if USE_TERM_IO
#include <termios.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup dev_g
 * @struct pt_t
 * point
 */
typedef struct {
  double x, y;
} pt_t;

/**
 * @ingroup dev_g
 * @struct ipt_t
 * point of integers
 */
typedef struct {
  int32 x, y;
} ipt_t;

/**
 * @ingroup dev_g
 *
 * Bresenham's algorithm. For graphics drivers that has no lines, like term and SDL.
 *
 * @param x1 line coordinates
 * @param y1 line coordinates
 * @param x2 line coordinates
 * @param y2 line coordinates
 * @param dotproc setpixel() function
 */
void g_line(int x1, int y1, int x2, int y2, void (*dotproc) (int, int));

/*
 *
 * colors - VGA16 compatible
 *
 * There are 16 colors. The second set of 8 colors its the high-light of the first set.
 * Example:
 *   dark gray its CLR_BLACK+8
 *   light green its CLR_GREEN+8
 *   yellow its CLR_BROWN+8
 */
#define CLR_BLACK 0
#define CLR_BLUE  1
#define CLR_GREEN 2
#define CLR_CYAN  3
#define CLR_RED   4
#define CLR_MAGENTA 5
#define CLR_BROWN 6
#define CLR_GRAY  7
#define CLR_WHITE 15

/**
 * @ingroup dev
 *
 * @page glob_dev_var System driver global variables
 *
 * Globals are needed for speed and for memory-usage optimization
 *
 * @code
 * byte  os_charset;   // System's charset (see os_charset_codes)
 * dword os_color_depth; // The number of bits of the supported colors
 *             // (i.e.: 8 for 256 colors, 15 or 16 for 64K, 24 or 32 for 1.6M)
 * byte  os_graphics;  // Non-zero if the driver supports graphics
 * int   os_graf_mx;   // Graphic mode: screen width
 * int   os_graf_my;   // Graphic mode: screen height
 * int32 dev_Vx1, dev_Vy1, dev_Vx2, dev_Vy2;
 * int32 dev_Vdx, dev_Vdy; // Graphics - viewport
 * int32 dev_Wx1, dev_Wy1, dev_Wx2, dev_Wy2;
 * int32 dev_Wdx, dev_Wdy; // Graphics - window world coordinates
 * long  dev_fgcolor, dev_bgcolor; // Graphics - current colors
 * @endcode
 */

/**
 * @ingroup dev
 * @enum os_charset_codes Charsets
 */
enum os_charset_codes {
  enc_utf8,       /**< 8bit - All European languages - default */
  enc_sjis,       /**< Japanese characters support */
  enc_big5,       /**< Chinese characters support */
  enc_gmb,        /**< Generic multibyte */
  enc_unicode       /**< Unicode */
};

#if !defined(DEVICE_MODULE)
extern byte os_charset;

extern byte os_color;         // true if the output has real colors (256+ colors)
extern dword os_color_depth;  // the number of bits of the supported colors
                              // (ex: 8 for 256 colors, 15 or 16 for 64K, 24 or 32 for 1.6M)
extern byte os_graphics;      // non-zero if the driver supports graphics
extern int os_graf_mx;        // graphic mode: maximum x
extern int os_graf_my;        // graphic mode: maximum y

// graphics - viewport
extern int32 dev_Vx1;
extern int32 dev_Vy1;
extern int32 dev_Vx2;
extern int32 dev_Vy2;

extern int32 dev_Vdx;
extern int32 dev_Vdy;

// graphics - window world coordinates
extern int32 dev_Wx1;
extern int32 dev_Wy1;
extern int32 dev_Wx2;
extern int32 dev_Wy2;

extern int32 dev_Wdx;
extern int32 dev_Wdy;

// graphics - current colors
extern long dev_fgcolor;
extern long dev_bgcolor;

#endif

/*
 *
 * Driver basics
 *
 */

/**
 * @ingroup dev
 *
 * initialize OS drivers
 *
 * @param mode non-zero for graphics
 * @param flags zero for normal (internal flags for reinitialize, used by win32 driver)
 * @return non-zero on success
 */
int dev_init(int mode, int flags);

/**
 * @ingroup dev
 *
 * close the driver and restore computer mode
 *
 * @return non-zero on success
 */
int dev_restore(void);

/*
 *
 * System basics
 *
 */

/**
 * @ingroup dev_i
 *
 * Internal keyboard buffer: store a key into the buffer
 *
 * @param ch the key-code
 */
void dev_pushkey(word ch);

/**
 * @ingroup dev_i
 *
 * Internal keyboard buffer: returns true if there is a key
 *
 * @return non-zero if there is a key on keyboard-buffer
 */
int dev_kbhit(void);

/**
 * @ingroup dev_i
 *
 * Internal keyboard buffer: clears keyboard buffer
 */
void dev_clrkb(void);

/**
 * @ingroup dev
 *
 * run process
 *
 * @param prog is the command-line
 * @param retflg non-zero for return (system()); otherwise dev_run() never returns (exec())
 * @return non-zero on success
 */
int dev_run(const char *prog, int retflg);

#if defined(_Win32)
char *pw_shell(const char *cmd);
#endif

/**
 * @ingroup dev
 *
 * Check's the system events queue
 *
 * a) stores keys into the keyboard buffer,
 * b) store pen/mouse events,
 * c) any other supported device.
 *
 * @param dead_loop
 *   if dead_loop is zero, the dev_events() will checks the events and then will return immediatly.
 *   if dead_loop is true, the dev_events() will wait for a new event.
 *
 * @return
 *   0 if there is not new events in the queue.
 *   >0 the number of the new events.
 *   -1 that means BREAK (brun_break() it was called).
 *   -2 that means BREAK (executor displays "BREAK" message).
 */
int dev_events(int dead_loop);

/**
 * @ingroup dev
 *
 * delay for 'ms' milliseconds
 *
 * @param ms the milliseconds to pause
 */
void dev_delay(dword ms);

/**
 * @ingroup dev_i
 *
 * Mouse & lightpen!
 *
 * Since 1988 the mouse was an new device for PC's, there is no mouse support on QB.
 * We shall use the PEN(x) to support the mouse, and we must maintain the Palm compatibility.
 *
 * <pre>
 PalmOS PEN, Lightpen & Mouse API
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
int dev_getpen(int code);

/**
 * @ingroup dev_i
 *
 * enable/disable pen/mouse driver.
 *
 * @note That is neccessary on some systems like PalmOS because the pointing device it may be used by the OS.
 *
 * @note That is the PEN ON/OFF command
 *
 * @param enable non-zero to enable pen/mouse driver.
 */
void dev_setpenmode(int enable);

/*
 *
 * terminal input/output
 *
 */

/**
 * @ingroup dev_g
 *
 * prints a string to the SB's console
 *
 * @param str the string
 */
void dev_print(const char *str);

/**
 * @ingroup dev_g
 *
 * prints a string by using printf-style to the SB's console
 *
 * @param fmt the format
 * @param ... the format's parameters
 */
void dev_printf(const char *fmt, ...);

/**
 * @ingroup dev_g
 *
 * prints a string by using printf-style to the SB's log file/console
 *
 * @param fmt the format
 * @param ... the format's parameters
 */
void log_printf(const char *fmt, ...);

/**
 * @ingroup dev_g
 *
 * clear screen
 */
void dev_cls(void);

/**
 * @ingroup dev_g
 *
 * clear from cursor to end-of-line
 */
void dev_clreol(void);

/**
 * @ingroup dev_g
 *
 * sets the current x, y for texts or graphics
 *
 * @note AT command
 *
 * @param x the x in pixels
 * @param y the y in pixels
 */
void dev_setxy(int x, int y, int transform);

/**
 * @ingroup dev_g
 *
 * returns the current x position
 *
 * @return the current x position
 */
int dev_getx(void);

/**
 * @ingroup dev_g
 *
 * returns the current y position
 *
 * @return the current y position
 */
int dev_gety(void);

/**
 * @ingroup dev_g
 *
 * sets the foreground and the background color
 *
 * @param fg the standard VGA foreground color
 * @param bg the standard VGA foreground color
 */
void dev_settextcolor(long fg, long bg);

/**
 * @ingroup dev_i
 *
 * waits until a key is pressed and returns its code
 *
 * @return the key-code
 */
long int dev_getch(void);

/**
 * @ingroup dev_i
 *
 * reads a string from the console
 *
 * @note part of INPUT's code
 *
 * @param buf the buffer to store the string
 * @param size the size of the buffer
 * @return the buf or NULL on ESCAPE
 */
char *dev_gets(char *buf, int size);

/*
 *
 * Graphics
 *
 */

/**
 * @ingroup dev_g
 *
 * sets the viewport.
 * the viewport is a rectangle of the screen on that all output commands are write (todays we use the word 'window' for that).
 * the window-command will maps its coordinates on viewport.
 *
 * a value of 0, 0, 0, 0 will reset (the viewport and the window) to defaults (the whole screen).
 *
 * @param x1 left
 * @param y1 up
 * @param x2 right
 * @param y2 bottom
 */
void dev_viewport(int x1, int y1, int x2, int y2);

/**
 * @ingroup dev_g
 *
 * sets the window.
 * the window are the world-coordinates system that maps to viewport.
 *
 * a value of 0, 0, 0, 0 will reset to defaults (same as viewport's).
 *
 * @param x1 left
 * @param y1 up
 * @param x2 right
 * @param y2 bottom
 */
void dev_window(int x1, int y1, int x2, int y2);

/**
 * @ingroup dev_g
 *
 * returns the width of the text in pixels
 *
 * @param str the text
 * @return the width of the text in pixels
 */
int dev_textwidth(const char *str);

/**
 * @ingroup dev_g
 *
 * returns the height of the text in pixels
 *
 * @param str the text
 * @return the height of the text in pixels
 */
int dev_textheight(const char *str);

/**
 * @ingroup dev_g
 *
 * sets the foreground color. if the color is >= 0 the driver uses the standard 16 VGA colors,
 * if the color is negative, the driver it must use RGB (-color) color value.
 *
 * @note RGB colors are new in SB, so avoid to use because it is not supported by all the drivers yet.
 *
 * @param color the foreground color
 */
void dev_setcolor(long color);

/**
 * @ingroup dev_g
 *
 * sets the pixel's value
 *
 * @param x the x position
 * @param y the y position
 */
void dev_setpixel(int x, int y);

/**
 * @ingroup dev_g
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
long dev_getpixel(int x, int y);

/**
 * @ingroup dev_g
 *
 * Cohen-Sutherland clipping algorithm.
 *
 * @param x1 line coordinates
 * @param y1 line coordinates
 * @param x2 line coordinates
 * @param y2 line coordinates
 * @param visible non-zero if the line or part of the line is visible
 */
void dev_clipline(int *x1, int *y1, int *x2, int *y2, int *visible);

/**
 * @ingroup dev_g
 *
 * draw a line
 *
 * @param x1 line coordinates
 * @param y1 line coordinates
 * @param x2 line coordinates
 * @param y2 line coordinates
 */
void dev_line(int x1, int y1, int x2, int y2);

/**
 * @ingroup dev_g
 *
 * draw a rectangle
 *
 * @param x1 upper-left corner
 * @param y1 upper-left corner
 * @param x2 lower-right corner
 * @param y2 lower-right corner
 * @param fill non-zero to fill it with foreground color
 */
void dev_rect(int x1, int y1, int x2, int y2, int fill);

/**
 * @ingroup dev_g
 *
 * draw an ellipse
 *
 * @param xc x-center
 * @param yc y-center
 * @param xr x-radius
 * @param yr y-radius
 * @param aspect x/y (use 1)
 * @param fill non-zero to fill it with foreground color
 */
void dev_ellipse(int xc, int yc, int xr, int yr, double aspect, int fill);

/**
 * @ingroup dev_g
 *
 * draw an arc
 *
 * @param xc x-center
 * @param yc y-center
 * @param r  radius
 * @param as angle-start in rad
 * @param ae angle-end in rad
 * @param aspect x/y (use 1)
 */
void dev_arc(int xc, int yc, double r, double as, double ae,
             double aspect);

/**
 * @ingroup dev_g
 *
 * floodfill, fills an area with the fill-color.
 *
 * there are two ways for that
 *
 * a) the scan-until,
 * if you use the border-color (!=-1) the fill algoritm will fill all the area that included by
 * the specified border-color.
 *
 * b) the scan-while,
 * if you don't use the border-color (=-1) the fill algoritm will fill all the area that has
 * the same color as the getpixel(x0,y0)
 *
 * @param x0 the point to start
 * @param y0 the point to start
 * @param fill_color the color to use for fill
 * @param border_color the color of the border, use -1 for scan-while algorithm
 */
void dev_ffill(word x0, word y0, long fill_color, long border_color);

/**
 * @ingroup dev_g
 *
 * fill poly-line with the foreground color
 *
 * @param pts is a table of points
 * @param ptNum is the number of the points to use
 */
void dev_pfill(ipt_t *pts, int ptNum);

/**
 * @ingroup dev_g
 *
 * Refreshes the graphic output. When used, the drawing to the graphic
 * output window or screen is not visible until SHOWPAGE is performed
 */
void dev_show_page();

/*
 *
 * Sound
 *
 */

/**
 * @ingroup dev_s
 *
 * produce the standard system's beep :)
 */
void dev_beep(void); // just a BEEP! :)

/**
 * @ingroup dev_s
 *
 * produce a tone
 * duration in ms, volume in percent, bgplay = play on background
 *
 * @param freq is the frequency
 * @param dur_ms is the duration in milliseconds
 * @param vol_prc is the volume (0-99)
 * @param bgplay non-zero for play the tone in background
 */
void dev_sound(int freq, int dur_ms, int vol_prc, int bgplay); // note:

/**
 * @ingroup dev_f
 *
 * clear background sound queue
 */
void dev_clear_sound_queue();

/*
 *
 * FILE SYSTEM BASICS
 *
 */
/**
 * @ingroup dev_f
 *
 * matches a filename
 *
 * @param mask is the wild-cards
 * @param name is the filename
 * @return non-zero on success
 */
int wc_match(const char *mask, char *name);

/**
 * @ingroup dev_f
 *
 * initialize the file-system driver
 *
 * @return non-zero on success
 */
int dev_initfs(void);

/**
 * @ingroup dev_f
 *
 * close file-system (closes all files and all the drivers)
 */
void dev_closefs(void);

/**
 * @ingroup dev_f
 *
 * returns true if the file exists
 *
 * @param file is the filename
 * @return non-zero if file exists
 */
int dev_fexists(const char *file);

/**
 * @ingroup dev_f
 *
 * copies a file
 *
 * @param file is the source
 * @param file is the target
 * @return non-zero on success
 */
int dev_fcopy(const char *file, const char *newfile);

/**
 * @ingroup dev_f
 *
 * renames a file
 *
 * @param file is the source
 * @param file is the target
 * @return non-zero on success
 */
int dev_frename(const char *file, const char *newname);

/**
 * @ingroup dev_f
 *
 * removes a file
 *
 * @param file is the filename
 * @return non-zero on success
 */
int dev_fremove(const char *file);

/**
 * @ingroup dev_f
 *
 * returns the access attributes of the file. for more info see the chmod()
 *
 * @param file is the filename
 * @return the access attributes of the file
 */
int dev_faccess(const char *file);

#define VFS_ATTR_FILE   1 /**< dev_fattr(), regular file      @ingroup dev_f */
#define VFS_ATTR_DIR    2 /**< dev_fattr(), directory       @ingroup dev_f */
#define VFS_ATTR_LINK   4 /**< dev_fattr(), symbolic-link     @ingroup dev_f */

/**
 * @ingroup dev_f
 *
 * returns some info about the file (symbolic-link, directory, regular file).
 *
 * see VFS_ATTR_FILE, VFS_ATTR_DIR, VFS_ATTR_LINK
 *
 * @param file is the filename
 * @return the access attributes of the file
 */
int dev_fattr(const char *file);

/*
 *
 * FILE I/O
 *
 */

/**
 * @ingroup dev_f
 *   @typedef dev_ftype_t
 * File-type (or better, what driver to use for that file)
 */
typedef enum {
  ft_stream,          /**< simple file */
  ft_random,
  ft_serial_port,     /**< COMx:speed, serial port */
  ft_printer_port,    /**< LPTx: parallel port */
  ft_socket_client,   /**< SCLT:address:port, socket client */
  ft_socket_server,   // SSVR:address:port
  ft_http_client,
  ft_ftp_client,      // FTP is a good example also
  ft_mail_client,     // MAIL (at least SMTP)
  ft_ztxt,
  ft_vfslib           /**< vfs-module */
} dev_ftype_t;

/**
 * @ingroup dev_f
 * @typedef file_t file structure
 */
typedef struct {
  char name[OS_PATHNAME_SIZE + 1];  /**< the file name */
  dev_ftype_t type;   /**< the driver to use */
  int port;           /**< the port (if device) */
  long devspeed;      /**< the speed (if device) */
  byte *drv_data;     /**< driver data used by low-level driver authors, (don't forget to set it to null on close()) */
  dword drv_dw[4];    /**< driver data used by low-level driver authors */

#if USE_TERM_IO
  struct termios oldtio, newtio;  /**< termios info */
#endif

  int handle;         /**< the file handle */
  int last_error;     /**< the last error-code */
  int vfslib;         /**< vfs-module, handle */
  int open_flags;     /**< the open()'s flags */
} dev_file_t;

// flags for dev_fopen()
#define DEV_FILE_INPUT    1 /**< dev_fopen() flags, open file for input (read-only)     @ingroup dev_f */
#define DEV_FILE_OUTPUT   2 /**< dev_fopen() flags, open file for output (create)     @ingroup dev_f */
#define DEV_FILE_APPEND   4 /**< dev_fopen() flags, open file for append (append or create) @ingroup dev_f */
#define DEV_FILE_EXCL   8 /**< dev_fopen() flags, open file exclusive           @ingroup dev_f */

/**
 * @ingroup dev_f
 *
 * returns a free file handle
 *
 * @return a free file handle
 */
int dev_freefilehandle(void);

/**
 * @ingroup dev_f
 *
 * returns the file info for a handle
 *
 * @return on success the file info; otherwise returns NULL
 */
dev_file_t *dev_getfileptr(int handle);

/**
 * @ingroup dev_f
 *
 * returns true if the file is opened
 *
 * @return true if the file is opened
 */
int dev_fstatus(int handle);

/**
 * @ingroup dev_f
 *
 * opens a file
 *
 * @param SBHandle is the RTL's file-handle
 * @param name is the filename
 * @param flags are the flags for open-mode (see DEV_FILE_xxx macros)
 * @returns non-zero on success
 */
int dev_fopen(int SBHandle, const char *name, int flags);

/**
 * @ingroup dev_f
 *
 * returns the size of the available data
 *
 * @param SBHandle is the RTL's file-handle
 * @return the size of the available data
 */
dword dev_flength(int SBHandle);

/**
 * @ingroup dev_f
 *
 * closes a file
 *
 * @param SBHandle is the RTL's file-handle
 * @returns non-zero on success
 */
int dev_fclose(int SBHandle);

/**
 * @ingroup dev_f
 *
 * moves the file-position-pointer to the specified offset
 *
 * @param SBHandle is the RTL's file-handle
 * @param offset the new position
 * @returns the new position
 */
dword dev_fseek(int SBHandle, dword offset);

/**
 * @ingroup dev_f
 *
 * returns the file-position-pointer
 *
 * @param SBHandle is the RTL's file-handle
 * @return the file-position-pointer
 */
dword dev_ftell(int SBHandle);

/**
 * @ingroup dev_f
 *
 * returns the end-of-file mark
 *
 * @param SBHandle is the RTL's file-handle
 * @return non-zero if eof
 */
int dev_feof(int SBHandle);

/**
 * @ingroup dev_f
 *
 * writes a buffer to the file
 *
 * @param SBHandle is the RTL's file-handle
 * @param buff is the data to write
 * @param size is the size of the data
 * @return non-zero on success
 */
int dev_fwrite(int SBHandle, byte *buff, dword size);

/**
 * @ingroup dev_f
 *
 * reads a size bytes from the file
 *
 * @param SBHandle is the RTL's file-handle
 * @param buff is a memory block to store the data
 * @param size is the number of bytes to read
 * @return non-zero on success
 */
int dev_fread(int SBHandle, byte *buff, dword size);

/**
 * @ingroup dev_f
 *
 * returns the entire file contents
 *
 * @param fileName the file to read
 * @return contents of the given file
 */
char *dev_read(const char *fileName);

/**
 * @ingroup dev_f
 *
 * creates a directory
 *
 * @param dir is the directory
 */
void dev_mkdir(const char *dir);

/**
 * @ingroup dev_f
 *
 * removes a directory
 *
 * @param dir is the directory
 */
void dev_rmdir(const char *dir);

/**
 * @ingroup dev_f
 *
 * changes the current directory
 *
 * @param dir is the directory
 */
void dev_chdir(const char *dir);

/**
 * @ingroup dev_f
 *
 * returns the current directory
 *
 * @return the current directory
 */
char *dev_getcwd(void);

/**
 * @ingroup dev
 *
 * returns the value of a specified system's environment variable
 *
 * @param var the name of the variable
 * @return on success the value; otherwise NULL
 */
char *dev_getenv(const char *var);

/**
 * @ingroup dev
 *
 * returns the number of the environment variables
 * @return the number of the environment variables
 */
int dev_env_count();

/**
 * @ingroup dev
 *
 * returns the value of the nth system's environment variable
 *
 * @param n the index of the variable
 * @return on success the value; otherwise NULL
 */
char *dev_getenv_n(int n);

/**
 * @ingroup dev
 *
 * sets a system environment variable
 *
 * @param setvar is a string in style "var=value"
 * @return non-zero on success
 */
int dev_putenv(const char *setvar);

/**
 * @ingroup dev_f
 *
 * creates a list of filenames using wild-cards
 *
 * @param wc is the wild-cards string
 * @param count is the number of the elements (returned)
 * @return a char** array
 */
char_p_t *dev_create_file_list(const char *wc, int *count);

/**
 * @ingroup dev_f
 *
 * destroys a file-list which was created with dev_create_file_list
 *
 * @param list is the char** array
 * @param count is the number of the elements
 */
void dev_destroy_file_list(char_p_t *list, int count);

/**
 * @ingroup dev_f
 *
 * Returns the number of milliseconds that has passed since
 * some unknown point in time.
 */
dword dev_get_millisecond_count();

#if defined(__cplusplus)
}
#endif
#endif
