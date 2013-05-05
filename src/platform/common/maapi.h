/*
  Copyright (C) 2009 Mobile Sorcery AB

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License, version 2, as published by
  the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to the Free
  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.
*/

#if !defined(MAAPI_H)
#define MAAPI_H

#include <stdint.h>

#define EXTENT_Y(e) ((short)(e))
#define EXTENT_X(e) ((short)((e) >> 16))
#define TRANS_NONE 0
#define FONT_TYPE_SERIF 0
#define FONT_TYPE_SANS_SERIF 1
#define FONT_TYPE_MONOSPACE 2
#define FONT_STYLE_NORMAL 0
#define FONT_STYLE_BOLD 1
#define FONT_STYLE_ITALIC 2
#define HANDLE_LOCAL 0
#define RES_OUT_OF_MEMORY -1
#define RES_BAD_INPUT -2
#define RES_OK 1
#define HANDLE_SCREEN 0
#define RES_FONT_OK 1
#define MAK_SPACE 32
#define MAK_Z 122
#define MAK_CLEAR 12
#define EVENT_TYPE_POINTER_PRESSED 8
#define EVENT_TYPE_POINTER_RELEASED 9
#define EVENT_TYPE_POINTER_DRAGGED 10

#ifndef _WCHAR_DEFINED
#define _WCHAR_DEFINED
typedef wchar_t wchar;
#endif  //_WCHAR_DEFINED

#ifndef _SYSV_TYPES_DEFINED
#define _SYSV_TYPES_DEFINED
typedef unsigned short ushort;
typedef unsigned int uint;
#endif  //_SYSV_TYPES_DEFINED

typedef int MAExtent;
typedef void* MAAddress;
typedef intptr_t MAHandle;

typedef struct MARect {
  int left;
  int top;
  int width;
  int height;
} MARect;

typedef struct MAPoint2d {
  int x;
  int y;
} MAPoint2d;

/**
 * \brief An event; a message indicating that something has happened, e.g. that a key has been pressed.
 */
typedef struct MAEvent {
  /**
   * One of the \link #EVENT_TYPE_CLOSE EVENT_TYPE \endlink constants.
   */
  int type;
  union {
    struct {
      /**
       * In POINTER events, this will be the location of the pointer.
       */
      MAPoint2d point;
    };
  };
} MAEvent;

/**
 * Deletes a loaded font
 * \param 'font' A font handle
 * \return RES_FONT_OK, RES_FONT_INVALID_HANDLE, or RES_FONT_DELETE_DENIED.
 */
int maFontDelete(MAHandle font);

/**
 * Sets the color used by drawing functions. Returns previous color. Initial color is 0 (black).
 * \param rgb A color in RGB8 format (0xRRGGBB). The top byte is ignored.
 */
int maSetColor(int rgb);

/**
 * Sets the clipping rectangle for the current draw target.
 * The screen and every drawable image each maintains a clipping rectangle.
 * Drawing operations have no effect outside the clipping rectangle.
 * The default clipping rectangle covers the entire draw target, so that
 * clipping occurs at the draw target's edges.
 */
void maSetClipRect(int left, int top, int width, int height);

/**
 * Draws a single pixel using the current color.
 * \see maSetColor()
 */
void maPlot(int posX, int posY);

/**
 * Draws a line using the current color.
 * \see maSetColor()
 */
void maLine(int startX, int startY, int endX, int endY);

/**
 * Draws a filled rectangle using the current color.
 * Width and height must be greater than zero.
 * \see maSetColor()
 */
void maFillRect(int left, int top, int width, int height);

/**
 * Draws Latin-1 text using the current color.
 * The coordinates are the top-left corner of the text's bounding box.
 * \see maSetColor()
 */
void maDrawText(int left, int top, const char* str);

/**
 * Copies the back buffer to the physical screen.
 */
void maUpdateScreen(void);

/**
 * Normally, a phone's backlight turns itself off after
 * a few seconds of the user not pressing any keys.
 * To avoid this behaviour, call this function periodically.
 * As the timeout period is different for every device, and sometimes even user-configurable,
 * it's recommended that you call this function at least once every 500 milliseconds
 * to ensure that the light stays on at all times.
 */
void maResetBacklight(void);

/**
 * Returns the size in pixels of Latin-1 text as it would appear on-screen.
 */
MAExtent maGetTextSize(const char* str);

/**
 * Returns the screen size.
 * Returns the screen size.
 */
MAExtent maGetScrSize(void);

/**
 * Returns a handle to one of the default fonts of the device, in the style and size you specify.
 * \param 'type' The type of the font, can be FONT_TYPE_[SANS_SERIF,SERIF,MONOSPACE].
 * \param 'style' The style of the font, can be FONT_STYLE_[NORMAL,BOLD,ITALIC].
 * \param 'size' The size of the font.
 * \return The handle to the font, RES_FONT_NO_TYPE_STYLE_COMBINATION, or RES_FONT_INVALID_SIZE.
 */
MAHandle maFontLoadDefault(int type, int style, int size);

/**
 * Sets the font to be used with maDrawText and maDrawTextW, and returns the handle
 * to the previous font.
 * \param 'font' an MAHandle for a font object.
 * \return The handle to the previous font, or RES_FONT_INVALID_HANDLE.
 */
MAHandle maFontSetCurrent(MAHandle font);

/**
 * Draws a portion of an image using a transformation.
 * \param image The source image.
 * \param srcRect The portion of the source image to be drawn.
 * Must not exceed the bounds of the source image.
 * \param dstPoint The top-left point on the draw target.
 * \param transformMode One of the \link #TRANS_NONE TRANS \endlink constants.
 * \see maDrawImage
 */
void maDrawImageRegion(MAHandle image, const MARect* srcRect, const MAPoint2d* dstPoint, int transformMode);

/**
 * Creates a drawable image of the specified size. A drawable image has no alpha channel,
 * which is to say, no transparency.
 * Its initial contents are undefined, so you should draw onto the entire surface to
 * be sure what will happen when you draw this image onto something else.
 * \param placeholder The resource handle of the new image.
 * \param width Width, in pixels, of the new image. Must be \> 0.
 * \param height Height, in pixels, of the new image. Must be \> 0.
 * \see maSetDrawTarget()
 * \returns #RES_OK if succeded and #RES_OUT_OF_MEMORY if failed.
 */
int maCreateDrawableImage(MAHandle placeholder, int width, int height);

/**
 *  Creates a new placeholder and returns the handle to it.
 */
MAHandle maCreatePlaceholder(void);

/**
 * Releases a handle returned by maCreatePlaceholder().
 * If the handle refers to an object, such as an image or a data object,
 * that object is destroyed, as if maDestroyObject() had been called.
 *
 * The released handle may be reused by the system
 * and returned by future calls to maCreatePlaceholder(),
 * or by other system functions that allocate resources dynamically.
 *
 * This function is preferred to maDestroyObject(), unless you need
 * to reuse the handle.
 *
 * Attempting to destroy a handle that has already been released,
 * or was not returned by maCreatePlaceholder(), will cause a MoSync Panic.
 *
 * @param handle The handle to be released.
 */
void maDestroyPlaceholder(MAHandle handle);

/**
 * Copies an image into an array of ints that represent pixels in ARGB format.
 * The destination rectangle is defined as { 0,0, \a srcRect.width, \a srcRect.height }.
 * Parts of the destination array that are outside the destination rectangle are not modified.
 * If \a srcRect is outside the bounds of the source image,
 * or if \a srcRect.width is greater than \a scanlength, a MoSync Panic is thrown.
 * \param image The handle to the source image.
 * \param dst The address of the destination array.
 * \param scanlength The width of the image, in pixels, represented by the destination array.
 * \param srcRect The portion of the source image to be copied.
 */
void maGetImageData(MAHandle image, void* dst, const MARect* srcRect, int scanlength);

/**
 * Sets the current draw target.
 * The handle must be a drawable image or #HANDLE_SCREEN, which represents the back buffer.
 * The initial draw target is the back buffer.
 * If an image is set as draw target, its object handle goes into flux, which prevents
 * its destruction or use as a source in maDrawImage. When a different draw target is set,
 * the image's handle is restored. Returns the the previously set draw target.
 * \see maCreateDrawableImage()
 */
MAHandle maSetDrawTarget(MAHandle image);

/**
 * Returns the number of milliseconds that has passed since some unknown point in time.
 * Accuracy is platform-specific, but should be better than 20 ms.
 */
int maGetMilliSecondCount(void);

/**
 * Shows the virtual keyboard.
 */
int maShowVirtualKeyboard(void);

/**
 * Displays a special kind of dialog that has a list of possible choices.
 * The list of options is displayed as buttons on iOS, and as text views on Android.
 * By clicking any option the dialog gets dismissed and a #EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED event is sent back.
 *
 * \param title The dialog title.
 * \param destructiveButtonTitle The destructive button text. This is an iOS specific feature: it has different color than the other options,
 * and it indicates that it's action has destructive behaviour. On Android it is treated and it looks like a normal option.
 * \param cancelButtonTitle The dialog's Cancel button text. If left empty, the dialog is not cancelable.
 * \param otherButtonTitles The address to the buffer that stores the list of options.
 * \param otherButtonTitlesSize The size of the buffer, in bytes.
 */
void maOptionsBox(const wchar* title, const wchar* destructiveButtonTitle,
                  const wchar* cancelButtonTitle, const void* otherButtonTitles,
                  int otherButtonTitlesSize);

/**
 * There is a FIFO buffer that contains up to #EVENT_BUFFER_SIZE events.
 * Each event has a type. Some types have additional data.
 *
 * This function retrieves the next event, unless the queue is empty.
 * Use maWait() to wait until more events are available.
 * \param event Pointer to an MAEvent struct that will be filled with the next event.
 *
 * When the \link #EVENT_TYPE_CLOSE Close event \endlink is posted,
 * you must call maExit as soon as possible, or
 * your program will be forcibly terminated. The timeout is device-dependent, but
 * never longer than #EVENT_CLOSE_TIMEOUT milliseconds.
 *
 * After the Close event has been posted, most syscalls will stop working,
 * returning default values and doing nothing.
 * Only the following groups of functions are guaranteed to remain operational:
 * Memory management, math, Resource management, Store, time, logging, maExit() and maPanic().
 *
 * \note Not all platforms have the capability to generate a Close event.
 * You must always provide another way for the user to exit your application.
 *
 * \returns \> 0 on success, or zero if the buffer is empty.
 */
int maGetEvent(MAEvent* event);

/**
 * Suspends execution until there is an event in the buffer,
 * or \a timeout milliseconds have passed. A timeout <= 0 is considered infinite.
 * Timer accuracy is platform-specific, but should be better than 20 ms.
 *
 * Use this function rather than idle loops to save CPU/battery power.
 * \see maGetEvent()
 */
void maWait(int timeout);

/**
 * Displays a message to the user.
 * It contains optional title, message and 3 buttons for selection.
 * On Android the buttons point to a positive, negative or neutral action.
 * When one of the buttons are clicked the alert is automatically dismissed.
 * When a selection is made a #EVENT_TYPE_ALERT event is sent with the button index.
 * \param title The title of the message box shown.
 * \param message The message should be short, not more than 40 characters,
 * to ensure its visibility on all screen sizes.
 * \param button1 The text on the first button. On Android it indicates a positive selection.
 * \param button2 The text on the second button. On Android it indicates a neutral selection.
 * \param button3 The text on the third button. On Android it indicates that a negative selection.
 */
void maAlert(const char* title, const char* message, const char* button1,
             const char* button2, const char* button3);

#endif
