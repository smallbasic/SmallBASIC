/**
 * -*- c-file-style: "java" -*-
 * AnsiWindow version 1.0 Chris Warren-Smith 12/12/2001
 * Derived from SmallBASIC - http://smallbasic.sourceforge.net
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

#include <gui.h>
#include <stdlib.h>
#include <ctype.h>
#include "AnsiWindow.h"

extern "C" {
#include <lcd.h>
}

AnsiWindow::AnsiWindow(U16 id) : 
    CWindow(id, -2, -2, 
            LCD_QueryWidth() + 4, 
            LCD_QueryHeight() + 4, 
            0, 0, 0, 0) {
    init();
    lcdWidth = LCD_QueryWidth();
    lcdHeight = LCD_QueryHeight();
    GUI_CancelUpdate(); 
}

AnsiWindow::AnsiWindow(U16 id, int width, int height) : 
    CWindow(id, -2, -2, width, height, 0, 0, 0, 0) {
    init();
    lcdWidth = width;
    lcdHeight = height;
    GUI_CancelUpdate(); 
}

AnsiWindow::~AnsiWindow() {
    if (lcd) {
        imgFree(lcd);
    }
}

int AnsiWindow::getStyle() const {
    if (italic && bold)
        return CTRL_BOLDITALIC;
    if (italic)
        return CTRL_ITALIC;
    if (bold) 
        return CTRL_BOLD;
    return CTRL_NORMAL;
}

void AnsiWindow::clearScreen() {
    init();
    drawBGRectFilled(0, 0, lcdWidth, lcdHeight);
}

void AnsiWindow::saveScreen() {
    if (lcd == 0) {
        lcd = imgAlloc(lcdWidth, lcdHeight, COLOR_MODE_GRAY16);
    }
    imgRectCombine(lcd, 0,0, imgGetBase(), 0,0, 
                   lcdWidth, lcdHeight, IMG_CMB_SRC);
}

void AnsiWindow::restoreScreen() {
    if (lcd) {
        imgRectCombine(imgGetBase(), 0,0, lcd, 0, 0, 
                       lcdWidth, lcdHeight, IMG_CMB_SRC);
        imgFree(lcd);
        imgUpdate();
        GUIFLAGS *guiFlags = GUI_Flags_ptr();
        *guiFlags &= ~GUIFLAG_NEED_FLUSH;
        lcd = 0;
    }
}

void AnsiWindow::newLine() {
    curX = 0;
    curY += fontHeight;

    if (curY + fontHeight >= lcdHeight) {
        // scroll
        int height = lcdHeight-fontHeight;

        imgRectCombine(imgGetBase(), 0, 0, 
                       imgGetBase(), 0, fontHeight, 
                       lcdWidth, height, IMG_CMB_SRC);
        curY -= fontHeight;
        // erase bottom line 
        drawBGRectFilled(0, curY, lcdWidth, fontHeight+1); 
        imgUpdate();
        GUIFLAGS *guiFlags = GUI_Flags_ptr();
        *guiFlags &= ~GUIFLAG_NEED_FLUSH;
    }

    // reset height based on current font size since it may 
    // have been set to a previous maximum
    fontHeight = GUI_FontHeight(font);
}

int AnsiWindow::calcTab(int x) const {
    int c = 1;
    while (x > tabSize) {
        x -= tabSize;
        c++;
    }
    return c * tabSize;
}

COLOR AnsiWindow::qbToEbmColor(long color) const {
    switch(color) {
    case 0: return COLOR_BLACK;
    case 1: return COLOR_GRAY7;
    case 2: return COLOR_GRAY13;
    case 3: return COLOR_GRAY20;
    case 4: return COLOR_GRAY27;
    case 5: return COLOR_GRAY33;
    case 6: return COLOR_GRAY40;
    case 7: return COLOR_GRAY47;
    case 8: return COLOR_GRAY53;
    case 9: return COLOR_GRAY60;
    case 10: return COLOR_GRAY67;
    case 11: return COLOR_GRAY73;
    case 12: return COLOR_GRAY80;
    case 13: return COLOR_GRAY87;
    case 14: return COLOR_GRAY93;
    default: return COLOR_WHITE;
    }
}

void AnsiWindow::setGraphicsRendition(char c, int escValue) {
    switch (c) {
    case 'K': // \e[K - clear to eol
        drawBGRectFilled(curX, curY, lcdWidth-curX, fontHeight);
        break;
    case 'G': // move to column
        curX = escValue;
        break;
    case 'T': // non-standard: move to n/80th of screen width
        curX = escValue*lcdWidth/80;
        break;
    case 'F': { // non-standard: flush the screen image
        imgUpdate();
        GUIFLAGS *guiFlags = GUI_Flags_ptr();
        *guiFlags &= ~GUIFLAG_NEED_FLUSH;
        }
        break;
    case 's': // save cursor position
        curYSaved = curX;
        curXSaved = curY;
        break;
    case 'u': // restore cursor position
        curX = curYSaved;
        curY = curXSaved;
        break;
    case ';': // fallthru
    case 'm': // \e[...m  - ANSI terminal
        switch (escValue) {
        case 0:  // reset
            reset();
            break;
        case 1: // set bold on
            bold = true;
            break;
        case 2: // set faint on
            break;
        case 3:
            italic = true;
            break;
        case 4: // set underline on
            underline = true;
            break;
        case 7: // reverse video on
            invert = true;
            break;
        case 21: // set bold off
            bold = false;
            break;
        case 23:
            italic = false;
            break;
        case 24: // set underline off
            underline = false;
            break;
        case 27: // reverse video off
            invert = false;
            break;
            // colors - 30..37 foreground, 40..47 background
        case 30: // set black fg
            fgColor = qbToEbmColor(0);
            break;
        case 31: // set red fg
            fgColor = qbToEbmColor(4);
            break;
        case 32: // set green fg
            fgColor = qbToEbmColor(2);
            break;
        case 33: // set brown fg
            fgColor = qbToEbmColor(6);
            break;
        case 34: // set blue fg
            fgColor = qbToEbmColor(1);
            break;
        case 35: // set magenta fg
            fgColor = qbToEbmColor(5);
            break;
        case 36: // set cyan fg
            fgColor = qbToEbmColor(3);
            break;
        case 37: // set white fg
            fgColor = qbToEbmColor(7);
            break;
        case 40: // set black bg
            bgColor = qbToEbmColor(0);
            break;
        case 41: // set red bg
            bgColor = qbToEbmColor(4);
            break;
        case 42: // set green bg
            bgColor = qbToEbmColor(2);
            break;
        case 43: // set brown bg
            bgColor = qbToEbmColor(6);
            break;
        case 44: // set blue bg
            bgColor = qbToEbmColor(1);
            break;
        case 45: // set magenta bg
            bgColor = qbToEbmColor(5);
            break;
        case 46: // set cyan bg
            bgColor = qbToEbmColor(3);
            break;
        case 47: // set white bg
            bgColor = qbToEbmColor(7);
            break;
        case 50:
            size=9;
            break;
        case 51:
            size=12;
            break;
        case 52:
            size=16;
            break;
        };                        
    }
}

bool AnsiWindow::doEscape(U8* &p) {
    int escValue = 0;
    while (isdigit(*p)) {
        escValue = (escValue * 10) + (*p - '0');
        p++;
    }

    int oldSize = size;
    int oldStyle = getStyle();
    setGraphicsRendition(*p, escValue);
    
    int style = getStyle();
    if (style != oldStyle || size != oldSize) {
        font = GUI_GetFont(size, style);
    }
    
    if (size != oldSize) {
        int height = GUI_FontHeight(font);
        if (height > fontHeight) {
            fontHeight = height;
        }
    }
    if (*p == ';') {
        p++; // next rendition
        return true;
    }
    return false;
}

/**
 *   Supported control codes:
 *   \t      tab (20 px)
 *   \a      beep
 *   \r      return
 *   \n      next line
 *   \xC     clear screen
 *   \e[K    clear to end of line
 *   \e[0m   reset all attributes to their defaults
 *   \e[1m   set bold on
 *   \e[4m   set underline on
 *   \e[7m   reverse video
 *   \e[21m  set bold off
 *   \e[24m  set underline off
 *   \e[27m  set reverse off
 */
void AnsiWindow::write(const char *str) {
    int len = strlen(str);
    if (len <= 0) {
        return;
    }

    U8 *p = (U8*)str;
    while (*p) {
        switch (*p) {
        case '\a':   // beep
            GUI_Beep();
            break;
        case '\t':
            curX = calcTab(curX+1);
            break;
        case '\xC':
            clearScreen();
            break;
        case '\033':  // ESC ctrl chars
            if (*(p+1) == '[' ) {
                p += 2;
                while(true) {
                    if (!doEscape(p)) {
                        break;
                    }
                }
            }
            break;
        case '\n': // new line
            newLine();
            break;
        case '\r': // return
            curX = 0;
            drawBGRectFilled(0, curY, lcdWidth, fontHeight);
            break;
        default:
            int numChars = 1; // print minimum of one character
            int cx = GUI_TextWidth(font, (const char*)p, 1);
            if (curX + cx >= lcdWidth-4) {
                newLine();
            }

            // print further non-control, non-null characters 
            // up to the width of the line
            while (p[numChars] > 31 && curX + cx < lcdWidth-4) {
                cx = GUI_TextWidth(font, (const char*)p, ++numChars);
            }

            // create an image buffer and copy it to the base img
            GUI_BeginOffscreen(cx);
            for (int i=0; i<numChars; i++) {
                GUI_DrawCharOffscreen(font, *p);
                p++;
            }
            p--;
            imgSetColorFG(GUI_Gray16Intensity(fgColor));
            imgSetColorBG(GUI_Gray16Intensity(bgColor));
            IMAGE* img = GUI_EndOffscreen();
            imgRectCombine(imgGetBase(), curX, curY, img,
                           0, 0, img->img_width, img->img_height,
                           (invert ? IMG_CMB_SRC_INV : IMG_CMB_SRC));
            if (underline) {
                DrawLine(curX, curY+fontHeight-1, curX+cx, curY+fontHeight-1);
            }
            
            // advance
            curX += cx;
        };
        
        if (*p == '\0') {
            break;
        }
        p++;
    }

    // perform imgUpdate() in next event cycle
    GUIFLAGS *guiFlags = GUI_Flags_ptr();
    *guiFlags |= GUIFLAG_NEED_FLUSH;
}
