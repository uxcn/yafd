//===-- memory.h - memory functions -------------------------------*- C -*-===//

#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

#include "error.h"

static inline void* fmalloc(size_t n) {

  void* const m = malloc(n);

  if (!m)
    on_fatal("out of memory");

  return m;
}

static inline void* fcalloc(size_t es, size_t n) {

  void* const m = calloc(es, n);

  if (!m)
    on_fatal("out of memory");

  return m;
}

static inline void* frealloc(void* p, size_t n) {

  void* const m = realloc(p, n);

  if (!m)
    on_fatal("out of memory");

  return m;
}

#endif // MEMORY_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
