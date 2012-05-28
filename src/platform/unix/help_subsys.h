// This file is part of SmallBASIC
//
// SmallBASIC help subsystem (it is used from console, -h option)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sb_help_subsys)
#define _sb_help_subsys

/**
 * returns the index of the 'command' in the help data
 *
 * @param command is the command or function to get info for
 * @param start is from where to start (for multiple definitions)
 * @return the index of the command or -1
 */
int help_find(const char *command, int start);

/**
 * get description of 'command'
 *
 * @param command is the command or function to get info for
 * @param start is from where to start (for multiple definitions)
 * @return a constant string pointer to the information or null
 */
const char *help_getinfo(const char *command, int start);

/**
 * get syntax of 'command'
 *
 * @param command is the command or function to get info for
 * @param start is from where to start (for multiple definitions)
 * @return a constant string pointer to the information or null
 */
const char *help_getsyntax(const char *command, int start);

/*
 * prints to stdout the help text of a command
 *
 * @param command is the command or function to get info for
 */
void help_printinfo(const char *command);

#endif
