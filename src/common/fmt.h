// This file is part of SmallBASIC
//
// formating numbers and strings
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sb_fmt_h)
#define _sb_fmt_h

#include "common/sys.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup str
 *
 * Part of floating point to string (by using integers) algorithm
 * where x any number 2^31 > x >= 0
 *
 * @param x is the number
 * @param dest is the string buffer
 */
void fptoa(var_num_t x, char *dest);

/**
 * @ingroup str
 *
 * best float to string
 *
 * @param x is the number
 * @param dest is the string buffer
 */
void bestfta(var_num_t x, char *dest);

/**
 * @ingroup str
 *
 *   float to string (user, E mode)
 *
 * @param x is the number
 * @param dest is the string buffer
 */
void expfta(var_num_t x, char *dest);

/**
 * @ingroup str
 *
 * format: format a number
 *
 * symbols:
 @code
 # = digit or space
 0 = digit or zero
 ^ = exponential digit/format
 . = decimal point
 , = thousands
 - = minus for negative
 + = sign of number
 @endcode
 *
 * @param dest is the string buffer
 * @param fmt_cnst is the format string
 * @param x is the number
 */
char *format_num(const char *fmt_cnst, var_num_t x);

/**
 * @ingroup str
 *
 * format: format a string
 *
 * symbols:
 @code
 & the whole string
 ! the first char
 \\  segment
 @endcode
 *
 * @param dest is the string buffer
 * @param fmt_cnst is the format string
 * @param str is the source string
 */
char *format_str(const char *fmt_cnst, const char *str);

/**
 * @ingroup str
 *
 * creates the internal-format queue
 *
 * @note part of USING
 *
 * @param fmt_cnst is the format
 */
void build_format(const char *fmt_cnst);

/**
 * @ingroup str
 *
 * clears the internal-format queue
 *
 * @note part of USING
 */
void free_format(void);

/**
 * @ingroup str
 *
 * prints a number using the queued format
 *
 * @note part of USING
 *
 * @param x is the number
 * @param output is the output-set-of-routines code (see PV_xxx macros)
 * @param handle is the output handle (depented on output-code)
 */
void fmt_printN(var_num_t x, int output, intptr_t handle);

/**
 * @ingroup str
 *
 * prints a string using the queued format
 *
 * @note part of USING
 *
 * @param str is the string
 * @param output is the output-set-of-routines code (see PV_xxx macros)
 * @param handle is the output handle (depented on output-code)
 */
void fmt_printS(const char *str, int output, intptr_t handle);

#if defined(__cplusplus)
}
#endif
#endif
