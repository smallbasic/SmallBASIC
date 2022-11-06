// This file is part of SmallBASIC
//
// Copyright(C) 2001-2020 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef THEME_H
#define THEME_H

#define THEME_COLOURS 17
#define NUM_THEMES 6

extern unsigned g_themeId;
extern int g_user_theme[];

const char *themeName();

struct EditTheme {
  EditTheme();
  EditTheme(int fg, int bg);
  void setId(const unsigned themeId);
  void selectTheme(const int theme[]);
  void contrast(EditTheme *other);

  int _color{};
  int _background{};
  int _selection_color{};
  int _selection_background{};
  int _number_color{};
  int _number_selection_color{};
  int _number_selection_background{};
  int _cursor_color{};
  int _cursor_background{};
  int _match_background{};
  int _row_cursor{};
  int _syntax_comments{};
  int _syntax_text{};
  int _syntax_command{};
  int _syntax_statement{};
  int _syntax_digit{};
  int _row_marker{};
};

#endif
