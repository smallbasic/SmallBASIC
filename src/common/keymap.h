// This file is part of SmallBASIC
//
// keyboard map
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/var.h"

#ifndef KEYMAP_H
#define KEYMAP_H

#if defined(__cplusplus)
extern "C" {
#endif

#define PCKBSIZE 256

// Keyboard codes
#define SB_KEY_BACKSPACE  8
#define SB_KEY_DELETE     127 
#define SB_KEY_BREAK      3
#define SB_KEY_TAB        9
#define SB_KEY_ENTER      13
#define SB_KEY_SPACE      32

// first 16 - common with handhelds any extra key will be there
#define SB_KEY_PGUP     0xFF01
#define SB_KEY_PRIOR    SB_KEY_PGUP
#define SB_KEY_PGDN     0xFF02
#define SB_KEY_NEXT     SB_KEY_PGDN
#define SB_KEY_LEFT     0xFF04
#define SB_KEY_RIGHT    0xFF05
#define SB_KEY_UP       0xFF09
#define SB_KEY_DN       0xFF0A
#define SB_KEY_DOWN     SB_KEY_DN

// second 16 - common on PCs
#define SB_KEY_INSERT   0xFF10
#define SB_KEY_HOME     0xFF11
#define SB_KEY_END      0xFF12

// other application keys
#define SB_KEY_MENU     0xFF1F

// function keys (16 codes)
#define SB_KEY_F(x)     (0xFFF0+(x))
#define SB_KEY_SF(x)    (0xFFE0+(x))

// Control & Alt keys (parameter = capital character)
#define SB_KEY_CTRL(c)     (0xF1000000 + (c))
#define SB_KEY_ALT(c)      (0xF2000000 + (c))
#define SB_KEY_CTRL_ALT(c) (0xF4000000 + (c))
#define SB_KEY_SHIFT(c)    (0xF8000000 + (c))

// keypad
#define SB_KEY_KP_DIV     0xFFDA
#define SB_KEY_KP_MUL     0xFFDB
#define SB_KEY_KP_MINUS   0xFFDC
#define SB_KEY_KP_PLUS    0xFFDD
#define SB_KEY_KP_ENTER   0xFFDE
#define SB_KEY_KP_HOME    0xFFD7
#define SB_KEY_KP_UP      0xFFD8
#define SB_KEY_KP_PGUP    0xFFD9
#define SB_KEY_KP_LEFT    0xFFD4
#define SB_KEY_KP_CENTER  0xFFD5
#define SB_KEY_KP_RIGHT   0xFFD6
#define SB_KEY_KP_END     0xFFD1
#define SB_KEY_KP_DOWN    0xFFD2
#define SB_KEY_KP_PGDN    0xFFD3
#define SB_KEY_KP_INS     0xFFD0
#define SB_KEY_KP_DEL     0xFFDF
#define SB_KEY_MK_PUSH    0xFFC0
#define SB_KEY_MK_DRAG    0xFFC1
#define SB_KEY_MK_MOVE    0xFFC2
#define SB_KEY_MK_RELEASE 0xFFC3
#define SB_KEY_MK_WHEEL   0xFFC4
#define SB_PKEY_SIZE_CHG  0xFFC5
#define SB_KEY_MK_FIRST   SB_KEY_MK_PUSH
#define SB_KEY_MK_LAST    SB_PKEY_SIZE_CHG

void keymap_init();
void keymap_free();
void keymap_add(int key, bcip_t ip);
int keymap_invoke(word key);
int keymap_kbhit();
int keymap_kbpeek();

void timer_free(timer_s *timer);
void timer_add(var_num_t timer, bcip_t ip);
void timer_run(dword now);

#if defined(__cplusplus)
}
#endif

#endif
