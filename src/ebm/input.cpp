/**
 * -*- c-file-style: "java" -*-
 * SmallBASIC for eBookMan
 * Copyright(C) 2001-2002 Chris Warren-Smith. Gawler, South Australia
 * cwarrens@twpo.com.au
 *
 *                  _.-_:\
 *                 /      \
 *                 \_.--*_/
 *                       v
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 * 
 */

#include "sys.h"
#include "device.h"
#include "smbas.h"

EXTERN_C_BEGIN

/**
 * gets a string (INPUT)
 */
char *dev_gets(char *dest, int size) {
    int code;
    word ch=0;
    word prev_x = dev_getx();
    word prev_y = dev_gety();
    word pos = 0;

    dev_clreol();
    dev_clrkb();

    *dest = '\0';

    do  {
        // wait for event
        while ((code = dev_events(1)) == 0);
        if (code < 0) { // BREAK event
            *dest = '\0';
            brun_break();
            return dest;
        }

        while (dev_kbhit()) {  // we have keys
            ch = dev_getch();
            switch (ch)   {
            case -1: 
            case -2:
                return dest;
            case 0: case 10: case 13:   // ignore
                break;
            case SB_KEY_BACKSPACE:      // backspace
                if  (pos) 
                    pos--;
                dest[pos] = '\0';
                
                // redraw
                dev_setxy(prev_x, prev_y);
                dev_clreol();
                dev_print(dest);
                break;

            case SB_KEY_LEFT:
            case SB_KEY_RIGHT:
                break;

            default:
                if ((ch & 0xFF00) != 0xFF00) { // Not an hardware key
                    // store it
                    dest[pos] = ch;
                    pos ++;
                    dest[pos] = '\0';
                }
                else
                    ch = 0;

                // check the size
                if  (pos >= (size-2))
                    break;

                // redraw
                if  (ch)  {
                    dev_setxy(prev_x, prev_y);
                    dev_clreol();
                    dev_print(dest);
                }
            }
        }   // dev_kbhit() loop
        
    } while (ch != '\n' && ch != '\r');

    dest[pos] = '\0';
    dev_setxy(prev_x, prev_y);
    dev_clreol();
    dev_print(dest);
    dev_print("\n");
    return dest;     
}

EXTERN_C_END
