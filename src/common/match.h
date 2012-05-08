// This file is part of SmallBASIC
//
// The regular expressions routines is based on match.c by J. Kercheval:
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @file match.h
   @author J. Kercheval
   @date Sat, 01/05/1991  22:21:49

 J. Kercheval  Wed, 02/20/1991  22:29:01  Released to Public Domain
 J. Kercheval  Fri, 02/22/1991  15:29:01  fix '\' bugs (two :( of them)
 J. Kercheval  Sun, 03/10/1991  19:31:29  add error return to RegMatche()
 J. Kercheval  Sun, 03/10/1991  20:11:11  add IsValidRegPattern code
 J. Kercheval  Sun, 03/10/1991  20:37:11  beef up main()
 J. Kercheval  Tue, 03/12/1991  22:25:10  Released as V1.1 to Public Domain

 The file match.c coexists in the same directory with the string class.
 */

#if !defined(_JKMATCH_H)
#define _JKMATCH_H

#include "common/sys.h"

/* return codes */
#define reg_match_literal_failure       -1  // reg_match failure on literal
// (not found)
#define reg_match_bad_pattern           -2  // bad pattern
#define reg_match_range_failure         -3  // reg_match failure on [..]
// construct
#define reg_match_abort                         -4  // premature end of
// text string
#define reg_match_premature_end         -5  // premature end of pattern
// string
#define reg_match_valid                         0 // valid reg_match
/**
 * @ingroup str
 *
 * The regular expressions routines, by J. Kercheval
 *
 @code
 In the pattern string:
 `*' RegMatches any sequence of characters (zero or more)
 `?' RegMatches any character
 [SET] RegMatches any character in the specified set,
 [!SET] or [^SET] RegMatches any character not in the specified set.

 A set is composed of characters or ranges; a range looks like
 character hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the
 minimal set of characters allowed in the [..] pattern construct.
 Other characters are allowed (ie. 8 bit characters) if your system
 will support them.

 To suppress the special syntactic significance of any of `[]*?!^-\',
 and RegMatch the character exactly, precede it with a `\'.
 @endcode
 *
 * @param p is the pattern
 * @param t is the text
 * @return 0 on success
 */

int reg_match(const char *p, char *t) SEC(BIO);

#endif
