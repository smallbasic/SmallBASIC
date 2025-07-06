// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

enum KeyCode : char {
  K_CUT      = (char)-1,
  K_COPY     = (char)-2,
  K_PASTE    = (char)-3,
  K_SAVE     = (char)-4,
  K_HELP     = (char)-5,
  K_SEARCH   = (char)-6,
  K_RUN      = (char)-7,
  K_TOGGLE   = (char)-8,
  K_FIND     = (char)-9,
  K_LINE_UP  = (char)-10,
  K_PAGE_UP  = (char)-11,
  K_LINE_DOWN= (char)-12,
  K_PAGE_DOWN= (char)-13,
  K_TAG      = (char)-14,
  K_EXT1     = (char)-15,
  K_EXT2     = (char)-16,
  K_EXT3     = (char)-17,
  K_EXT4     = (char)-18,
  K_EXT5     = (char)-19,
  K_NULL     = 0,
  K_BACKSPACE = 8,
  K_TAB      = 9,
  K_NEWLINE  = 10,
  K_ENTER    = 13,
  K_ESCAPE   = 27,
  K_SPACE    = 32,
  K_EXCLAIM  = 33,  // !
  K_QUOTE    = 34,  // "
  K_HASH     = 35,  // #
  K_DOLLAR   = 36,  // $
  K_PERCENT  = 37,  // %
  K_AMPERSAND = 38, // &
  K_APOSTROPHE = 39, // '
  K_LPAREN   = 40,  // (
  K_RPAREN   = 41,  // )
  K_ASTERISK = 42,  // *
  K_PLUS     = 43,  // +
  K_COMMA    = 44,  // ,
  K_MINUS    = 45,  // -
  K_PERIOD   = 46,  // .
  K_SLASH    = 47,  // /
  K_0 = 48, K_1 = 49, K_2 = 50, K_3 = 51, K_4 = 52,
  K_5 = 53, K_6 = 54, K_7 = 55, K_8 = 56, K_9 = 57,
  K_COLON    = 58,  // :
  K_SEMICOLON = 59, // ;
  K_LESS     = 60,  // <
  K_EQUALS   = 61,  // =
  K_GREATER  = 62,  // >
  K_QUESTION = 63,  // ?
  K_AT       = 64,  // @
  K_A = 65, K_B = 66, K_C = 67, K_D = 68, K_E = 69,
  K_F = 70, K_G = 71, K_H = 72, K_I = 73, K_J = 74,
  K_K = 75, K_L = 76, K_M = 77, K_N = 78, K_O = 79,
  K_P = 80, K_Q = 81, K_R = 82, K_S = 83, K_T = 84,
  K_U = 85, K_V = 86, K_W = 87, K_X = 88, K_Y = 89,
  K_Z = 90,
  K_LBRACKET = 91,  // [
  K_BACKSLASH = 92, //
  K_RBRACKET = 93,  // ]
  K_CARET    = 94,  // ^
  K_UNDERSCORE = 95, // _
  K_BACKTICK = 96,  // `
  K_a = 97,  K_b = 98,  K_c = 99,  K_d = 100, K_e = 101,
  K_f = 102, K_g = 103, K_h = 104, K_i = 105, K_j = 106,
  K_k = 107, K_l = 108, K_m = 109, K_n = 110, K_o = 111,
  K_p = 112, K_q = 113, K_r = 114, K_s = 115, K_t = 116,
  K_u = 117, K_v = 118, K_w = 119, K_x = 120, K_y = 121,
  K_z = 122,
  K_LBRACE   = 123, // {
  K_PIPE     = 124, // |
  K_RBRACE   = 125, // }
  K_TILDE    = 126, // ~
  K_DEL      = 127  // DEL
};

inline bool isPrintable(KeyCode key) {
  return key >= K_SPACE && key <= K_TILDE;
}

inline bool isNumber(KeyCode key) {
  return key >= K_0 && key <= K_9;
}

inline bool isExtended(KeyCode key) {
  return key >= K_EXT5 && key <= K_EXT1;
}

inline bool isArrow(KeyCode key) {
  return key == K_LINE_UP || key == K_PAGE_UP || key == K_LINE_DOWN || key == K_PAGE_DOWN;
}

inline bool isRightMargin(KeyCode key) {
  return key == K_ENTER || key == K_HELP || key == K_BACKSPACE;
}
