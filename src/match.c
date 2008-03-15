/*
        The regular expressions routines is based on match.c by J. Kercheval:

        File: match.c
        Author: J. Kercheval
        Created: Sat, 01/05/1991  22:21:49

        J. Kercheval  Wed, 02/20/1991  22:29:01  Released to Public Domain
        J. Kercheval  Fri, 02/22/1991  15:29:01  fix '\' bugs (two :( of them)
        J. Kercheval  Sun, 03/10/1991  19:31:29  add error return to RegMatche()
        J. Kercheval  Sun, 03/10/1991  20:11:11  add IsValidRegPattern code
        J. Kercheval  Sun, 03/10/1991  20:37:11  beef up main()
        J. Kercheval  Tue, 03/12/1991  22:25:10  Released as V1.1 to Public Domain

        The file match.c coexists in the same directory with the string class.
*/

/**
*  In the pattern string:
*       `*' RegMatches any sequence of characters (zero or more)
*       `?' RegMatches any character
*       [SET] RegMatches any character in the specified set,
*       [!SET] or [^SET] RegMatches any character not in the specified set.
*
*  A set is composed of characters or ranges; a range looks like
*  character hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the
*  minimal set of characters allowed in the [..] pattern construct.
*  Other characters are allowed (ie. 8 bit characters) if your system
*  will support them.
*
*
*  To suppress the special syntactic significance of any of `[]*?!^-\',
*  and RegMatch the character exactly, precede it with a `\'.
*/

#include "match.h"
#include "smbas.h"
#include "sberr.h"

#ifdef USE_PCRE
#include <pcre.h>
#define OVECCOUNT 30            /* should be a multiple of 3 */
#endif

int reg_match_after_star(const char *p, char *t) SEC(BIO);
int reg_match_jk(const char *p, char *t) SEC(BIO);

int reg_match_jk(const char *p, char *t)
{
  char range_start, range_end;  /* start and end in range */
  int invert;                   /* is this [..] or [!..] */
  int member_match;             /* have I matched the [..] construct? */
  int loop;                     /* should I terminate? */

  for (; *p; p++, t++) {
    /*
     * if this is the end of the text then this is the end of the reg_match 
     */
    if (*t == '\0')
      return (*p == '*' && *++p == '\0') ? reg_match_valid : reg_match_abort;

    /*
     * determine and react to pattern type 
     */
    switch (*p) {
    case '?':                  /* single any character RegMatch */
      break;
    case '*':                  /* multiple any character RegMatch */
      return reg_match_after_star(p, t);
    case '[':                  /* [..] construct, single member/exclusion *
                                 * character RegMatch */
      {
        /*
         * move to beginning of range 
         */
        p++;

        /*
         * check if this is a member reg_match or exclusion reg_match 
         */
        invert = 0;             // false
        if (*p == '!' || *p == '^') {
          invert = -1;          // true
          p++;
        }

        /*
         * if closing bracket here or at range start then we have a malformed
         * pattern 
         */
        if (*p == ']')
          return reg_match_bad_pattern;

        member_match = 0;       // false
        loop = -1;              // true

        while (loop) {          /* if end of construct then loop is done */
          if (*p == ']') {
            loop = 0;           // false
            continue;
          }

          /*
           * RegMatching a '!', '^', '-', '\' or a ']' 
           */
          if (*p == '\\')
            range_start = range_end = *++p;
          else
            range_start = range_end = *p;

          /*
           * if end of pattern then bad pattern (Missing ']') 
           */
          if (*p == '\0')
            return reg_match_bad_pattern;

          /*
           * check for range bar 
           */
          if (*++p == '-') {
            /*
             * get the range end 
             */
            range_end = *++p;

            /*
             * if end of pattern or construct then bad pattern 
             */
            if (range_end == '\0' || range_end == ']')
              return reg_match_bad_pattern;

            /*
             * special character range end 
             */
            if (range_end == '\\') {
              range_end = *++p;

              /*
               * if end of text then we have a bad pattern 
               */
              if (!range_end)
                return reg_match_bad_pattern;
            }

            /*
             * move just beyond this range 
             */
            p++;
          }

          /*
           * if the text character is in range then RegMatch found. make sure
           * the range letters have the proper relationship to one another
           * before comparison 
           */

          if (range_start < range_end) {
            if (*t >= range_start && *t <= range_end) {
              member_match = -1;  // true
              loop = 0;         // false
            }
          }
          else {
            if (*t >= range_end && *t <= range_start) {
              member_match = -1;  // true
              loop = 0;         // false
            }
          }
        }                       // while ?

        /*
         * if there was a match in an exclusion set then no match 
         */
        /*
         * if there was no match in a member set then no match 
         */

        if ((invert && member_match) || !(invert || member_match))
          return reg_match_range_failure;

        /*
         * if this is not an exclusion then skip the rest of the [...]
         * construct that already RegMatched. 
         */

        if (member_match) {
          while (*p != ']') {
            /*
             * bad pattern (Missing ']') 
             */
            if (*p == '\0')
              return reg_match_bad_pattern;

            /*
             * skip exact RegMatch 
             */
            if (*p == '\\') {
              p++;

              /*
               * if end of text then we have a bad pattern 
               */
              if (*p == '\0')
                return reg_match_bad_pattern;
            }

            /*
             * move to next pattern char 
             */
            p++;
          }                     // while
        }
        break;
      }
    case '\\':                 /* next character is quoted and must match *
                                 * exactly */
      /*
       * move pattern pointer to quoted char and fall through 
       */
      p++;

      /*
       * if end of text then we have a bad pattern 
       */
      if (*p == '\0')
        return reg_match_bad_pattern;

      /*
       * must match this character exactly 
       */
    default:
      if (*p != *t)
        return reg_match_literal_failure;
    }                           // switch!
  }                             // first for

  /*
   * if end of text not reached then the pattern fails 
   */
  if (*t)
    return reg_match_premature_end;
  return reg_match_valid;
}

/*
*/
#ifdef USE_PCRE
int reg_match_pcre(const char *p, char *t)
{
  pcre *re;
  const char *error;
  int errofs;

  re =
    pcre_compile(p, (opt_usepcre == 2) ? PCRE_CASELESS : 0, &error, &errofs, NULL);
  if (!re) {
    rt_raise("REGULAR EXPRESSION SYNTAX ERROR (offset %d) -> %s", error, errofs);
    return reg_match_bad_pattern;
  }
  else {
    int rc;
    int erroffset;
    int ovector[OVECCOUNT];

    rc = pcre_exec(re, NULL, t, strlen(t), 0, 0, ovector, OVECCOUNT);
    if (rc >= 0)
      return reg_match_valid;
  }

  return reg_match_literal_failure;
}
#endif

/*
*/
int reg_match(const char *p, char *t)
{
#ifdef USE_PCRE
  if (opt_usepcre)
    return reg_match_pcre(p, t);
#endif
  return reg_match_jk(p, t);
}

/*----------------------------------------------------------------------------
*
* recursively call RegMatche() with final segment of PATTERN and of TEXT.
*
----------------------------------------------------------------------------*/
int reg_match_after_star(const char *p, char *t)
{
  int RegMatch = 1;             // unused code
  int nextp;

  /*
   * pass over existing ? and * in pattern 
   */
  while (*p == '?' || *p == '*') {
    /*
     * take one char for each ? and + 
     */
    if (*p == '?') {
      /*
       * if end of text then no RegMatch 
       */
      if (!*t++)
        return reg_match_abort;
    }

    /*
     * move to next char in pattern 
     */
    p++;
  }

  /*
   * if end of pattern we have RegMatched regardless of text left 
   */
  if (!*p)
    return reg_match_valid;

  /*
   * get the next character to RegMatch which must be a literal or '[' 
   */
  nextp = *p;
  if (nextp == '\\') {
    nextp = p[1];

    /*
     * if end of text then we have a bad pattern 
     */
    if (!nextp)
      return reg_match_bad_pattern;
  }

  /*
   * Continue until we run out of text or definite result seen 
   */
  do {
    /*
     * a precondition for RegMatching is that the next character in the pattern 
     * RegMatch the next character in the text or that the next pattern char is 
     * the beginning of a range.  Increment text pointer as we go here 
     */

    if (nextp == *t || nextp == '[')
      RegMatch = reg_match(p, t);

    /*
     * if the end of text is reached then no RegMatch 
     */

    if (!*t++)
      RegMatch = reg_match_abort;

  }
  while (RegMatch != reg_match_valid && RegMatch != reg_match_abort &&
         RegMatch != reg_match_bad_pattern);

  /*
   * return result 
   */
  return RegMatch;
}
