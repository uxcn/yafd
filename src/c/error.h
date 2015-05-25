//===-- error.h - error functions ---------------------------------*- C -*-===//

#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <string.h>

#include <errno.h>

#include <pthread.h>

#include "config.h" // autoconf

static inline void print_error(const char* const s, ...) {

  va_list ps;
  va_start(ps, s);

  const size_t bs = 32;
  char b[bs];

  char* e = b;

  if (errno) {

#ifdef HAVE_STRERROR_R
    e = strerror_r(errno, e, bs);
#else
    e = "?";
#endif

      if (s) {

        fprintf(stderr, "error: ");

        vfprintf(stderr, s, ps);

        fprintf(stderr, " (%s)\n", e);
      } else {

        fprintf(stderr, "error: %s\n", e);
      }

    } else {

      if (s) {

        fprintf(stderr, "error: ");

        vfprintf(stderr, s, ps);

        fprintf(stderr, "\n");
      } else {

        fprintf(stderr, "error: %s\n", e);
      }
    }
}

static inline void on_error(const char* const s, ...) {

  va_list ps;
  va_start(ps, s);

  print_error(s, ps);
}

static inline void on_fatal(const char* const s, ...) {

  va_list ps;
  va_start(ps, s);

  print_error(s, ps);
  exit(EXIT_FAILURE);
}

#endif // ERROR_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
