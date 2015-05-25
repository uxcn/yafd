//===-- util.h - utility functions --------------------------------*- C -*-===// 
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <string.h>

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

#endif // UTIL_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
