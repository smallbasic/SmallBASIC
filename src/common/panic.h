// This file is part of SmallBASIC
//
// System error manager
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_panic_h)
#define _panic_h

#include "common/sys.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup sys
 *
 * print a message and quits
 *
 * @param fmt the printf's style format
 * @param ... the format's parameters
 */
void panic(const char *fmt, ...);

/**
 * @ingroup sys
 *
 * print a message
 *
 * @param fmt the printf's style format
 * @param ... the format's parameters
 */
void warning(const char *fmt, ...);

/**
 * @ingroup sys
 *
 * print a message only in debug mode
 *
 * @param fmt the printf's style format
 * @param ... the format's parameters
 */
void debug(const char *fmt, ...);

/**
 * @ingroup sys
 *
 * dumps a memory block in hex
 *
 * @param block is the block
 * @param size is the size of the block
 */
void hex_dump(const unsigned char *block, int size);

#if defined(__cplusplus)
  }
#endif
#endif
