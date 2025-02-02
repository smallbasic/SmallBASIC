#include "lib/str.h"
#include "config.h"

#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *dst, const char *src, size_t siz) {
  char *d = dst;
  const char *s = src;
  size_t n = siz;

  // Copy as many bytes as will fit
  if (n != 0) {
    while (--n != 0) {
      if ((*d++ = *s++) == '\0') {
        break;
      }
    }
  }

  // Not enough room in dst, add NUL and traverse rest of src
  if (n == 0) {
    if (siz != 0) {
      // NUL-terminate dst
      *d = '\0';
    }
    while (*s++)
      ;
  }

  // count does not include NUL
  return (s - src - 1);
}
#endif

#if !defined(HAVE_STRLCAT)
size_t strlcat(char *dst, const char *src, size_t siz) {
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  size_t dlen;

  // Find the end of dst and adjust bytes left but don't go past end
  while (n-- != 0 && *d != '\0') {
    d++;
  }
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0) {
    return(dlen + strlen(s));
  }
  while (*s != '\0') {
    if (n != 1) {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  // count does not include NUL
  return (dlen + (s - src));
}
#endif

#if defined(_MCU)
#include <ctype.h>
char* strcasestr(const char *haystack, const char *needle) {
  if (!*needle) {
    // If needle is empty, return the haystack
    return (char *)haystack;
  }

  for (; *haystack; haystack++) {
    const char *h = haystack;
    const char *n = needle;

    // Compare characters case-insensitively
    while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
      h++;
      n++;
    }

    // If we have matched the entire needle
    if (!*n) {
      return (char *)haystack;
    }
  }
   // Not found
  return NULL;
}

#endif
