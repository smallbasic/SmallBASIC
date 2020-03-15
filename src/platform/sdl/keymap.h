// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef KEYMAP
#define KEYMAP

const int keymap[][2] = {
  {SDLK_RETURN, SB_KEY_ENTER},
  {SDLK_ESCAPE, SB_KEY_ESCAPE},
  {SDLK_KP_ENTER, SB_KEY_ENTER},
  {SDLK_TAB, SB_KEY_TAB},
  {SDLK_BACKSPACE, SB_KEY_BACKSPACE},
  {SDLK_DELETE, SB_KEY_DELETE},
  {SDLK_UP, SB_KEY_UP},
  {SDLK_DOWN, SB_KEY_DN},
  {SDLK_LEFT, SB_KEY_LEFT},
  {SDLK_RIGHT, SB_KEY_RIGHT},
  {SDLK_PAGEUP, SB_KEY_PGUP},
  {SDLK_PAGEDOWN, SB_KEY_PGDN},
  {SDLK_HOME, SB_KEY_HOME},
  {SDLK_END, SB_KEY_END},
  {SDLK_INSERT, SB_KEY_INSERT},
  {SDLK_F1, SB_KEY_F(1)},
  {SDLK_F2, SB_KEY_F(2)},
  {SDLK_F3, SB_KEY_F(3)},
  {SDLK_F4, SB_KEY_F(4)},
  {SDLK_F5, SB_KEY_F(5)},
  {SDLK_F6, SB_KEY_F(6)},
  {SDLK_F7, SB_KEY_F(7)},
  {SDLK_F8, SB_KEY_F(8)},
  {SDLK_F9, SB_KEY_F(9)},
  {SDLK_F10, SB_KEY_F(10)},
  {SDLK_F11, SB_KEY_F(11)},
  {SDLK_F12, SB_KEY_F(12)}
};

#endif

