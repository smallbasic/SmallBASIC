/**
 * @file panic.h
 * System error manager
 *
 * Nicolas Christopoulos
 */

#if !defined(_panic_h)
#define _panic_h

#include "sys.h"

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
void panic(const char *fmt, ...) SEC(IDE);

/**
 * @ingroup sys
 *
 * print a message
 *
 * @param fmt the printf's style format
 * @param ... the format's parameters
 */
void warning(const char *fmt, ...) SEC(IDE);

/**
 * @ingroup sys
 *
 * print a message only in debug mode
 *
 * @param fmt the printf's style format
 * @param ... the format's parameters
 */
void debug(const char *fmt, ...) SEC(IDE);

/**
 * @ingroup sys
 *
 * dumps a memory block in hex
 *
 * @param block is the block
 * @param size is the size of the block
 */
void hex_dump(const unsigned char *block, int size) SEC(IDE);

#if defined(__cplusplus)
}
#endif
#endif
