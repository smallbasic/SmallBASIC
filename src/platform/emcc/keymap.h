// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

const int KEYMAP[][2] = {
  {DOM_VK_RETURN, SB_KEY_ENTER},
  {DOM_VK_ESCAPE, SB_KEY_ESCAPE},
  {DOM_VK_ENTER, SB_KEY_ENTER},
  {DOM_VK_TAB, SB_KEY_TAB},
  {DOM_VK_BACK_SPACE, SB_KEY_BACKSPACE},
  {DOM_VK_DELETE, SB_KEY_DELETE},
  {DOM_VK_UP, SB_KEY_UP},
  {DOM_VK_DOWN, SB_KEY_DN},
  {DOM_VK_LEFT, SB_KEY_LEFT},
  {DOM_VK_RIGHT, SB_KEY_RIGHT},
  {DOM_VK_PAGE_UP, SB_KEY_PGUP},
  {DOM_VK_PAGE_DOWN, SB_KEY_PGDN},
  {DOM_VK_HOME, SB_KEY_HOME},
  {DOM_VK_END, SB_KEY_END},
  {DOM_VK_INSERT, SB_KEY_INSERT},
  {DOM_VK_F1, SB_KEY_F(1)},
  {DOM_VK_F2, SB_KEY_F(2)},
  {DOM_VK_F3, SB_KEY_F(3)},
  {DOM_VK_F4, SB_KEY_F(4)},
  {DOM_VK_F5, SB_KEY_F(5)},
  {DOM_VK_F6, SB_KEY_F(6)},
  {DOM_VK_F7, SB_KEY_F(7)},
  {DOM_VK_F8, SB_KEY_F(8)},
  {DOM_VK_F9, SB_KEY_F(9)},
  {DOM_VK_F10, SB_KEY_F(10)},
  {DOM_VK_F11, SB_KEY_F(11)},
  {DOM_VK_F12, SB_KEY_F(12)},
  {DOM_VK_CIRCUMFLEX, '^'},
  {DOM_VK_EXCLAMATION, '!'},
  {DOM_VK_DOUBLE_QUOTE, '"'},
  {DOM_VK_HASH, '#'},
  {DOM_VK_DOLLAR, '$'},
  {DOM_VK_PERCENT, '%'},
  {DOM_VK_AMPERSAND, '&'},
  {DOM_VK_UNDERSCORE, '_'},
  {DOM_VK_OPEN_PAREN, '('},
  {DOM_VK_CLOSE_PAREN, ')'},
  {DOM_VK_ASTERISK, '*'},
  {DOM_VK_PLUS, '+'},
  {DOM_VK_PIPE, '|'},
  {DOM_VK_HYPHEN_MINUS, '-'},
  {DOM_VK_OPEN_CURLY_BRACKET, '{'},
  {DOM_VK_CLOSE_CURLY_BRACKET, '}'},
  {DOM_VK_TILDE, '~'},
  {DOM_VK_COMMA, ','},
  {DOM_VK_PERIOD, '.'},
  {DOM_VK_SLASH, '/'},
  {DOM_VK_BACK_QUOTE, '`'},
  {DOM_VK_OPEN_BRACKET, '['},
  {DOM_VK_BACK_SLASH, '\\'},
  {DOM_VK_CLOSE_BRACKET, ']'},
  {DOM_VK_QUOTE, '\''},
  {0xBD, '-'},
  {0xBB, '='},
  {0xBA, ';'},
};

const int SHIFT_KEYMAP[][2] = {
  {DOM_VK_0, ')'},
  {DOM_VK_1, '!'},
  {DOM_VK_2, '@'},
  {DOM_VK_3, '#'},
  {DOM_VK_4, '$'},
  {DOM_VK_5, '%'},
  {DOM_VK_6, '^'},
  {DOM_VK_7, '&'},
  {DOM_VK_8, '*'},
  {DOM_VK_9, '('},
  {DOM_VK_COLON, ':'},
  {DOM_VK_EQUALS, '+'},
  {'`', '~'},
  {'-', '_'},
  {'=', '+'},
  {'[', '{'},
  {']', '}'},
  {'\\', '|'},
  {'\'', '"'},
  {';', ':'},
  {',', '<'},
  {'.', '>'},
  {'/', '?'},
};

const int SHIFT_KEYMAP_LEN = sizeof(SHIFT_KEYMAP) / sizeof(SHIFT_KEYMAP[0]);
const int KEYMAP_LEN = sizeof(KEYMAP) / sizeof(KEYMAP[0]);

