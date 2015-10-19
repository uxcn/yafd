//===-- util.h - utility functions --------------------------------*- C -*-===// 
#ifndef UTIL_H
#define UTIL_H

#include "config.h" // autoconf


#include <stddef.h>
#include <string.h>


#include "platform.h" // platform

static inline char* strjoin(char* restrict const d, const char* restrict const a, const char* restrict const b, const char c) {
  size_t na = strlen(a);
  size_t nb = strlen(b);

  memcpy(d, a, na);
  d[na] = c;
  memcpy(d + na + 1, b, nb);

  d[na + nb + 1] = '\0';

  return d;
}

static inline char* strtrim(char* const s) {

  size_t ns = strlen(s) - 1;

  if (s[ns] == '\n')
    s[ns--] = '\0';

  if (s[ns] == '\r')
    s[ns--] = '\0';

  return s;
}

static inline char* strchmp(char* const s, const char c) {

  size_t ns = strlen(s) - 1;

  while (s[ns] == c)
    s[ns--] = '\0';

  return s;
}

static inline char* strclps(char* const s, const char c) {

  size_t t = 0;
  size_t f = 0;


  while (true) {
  while (true) {

    s[t++] = s[f];

    if (!s[f])
      goto end;

    if (s[f++] == c)
      break;
  }

    while (s[f] == c)
      f++;
  }

end:

  return s;
}

#endif // UTIL_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
