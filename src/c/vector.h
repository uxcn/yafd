//===-- vector.h - vector -----------------------------------------*- C -*-===//

#ifndef VECTOR_H
#define VECTOR_H

#include <assert.h>

#include <stdlib.h>
#include <stdbool.h>

#include <string.h>

#include "memory.h"

#include "math.h"

#define vector_for_each(entry, vector)                                         \
  for (size_t i = 0; (i < (vector)->size) && (entry = (vector)->store[i]); i++)\

struct vector {

  size_t size;
  size_t limit;

  void** store;
};

static inline void vector_init(struct vector* v, const size_t s) {

  v->size = 0;
  v->limit = max(s, 2);
  v->store = fmalloc(v->limit * sizeof(void*));
}

static inline void vector_destroy(struct vector* v) {

  free(v->store);
}

static inline void vector_resize(struct vector* const v, const size_t s) {

 v->limit = s;
 v->store = frealloc(v->store, s * sizeof(void*));
}

static inline void* vector_get(const struct vector* const v, const size_t i) {

  assert(i < v->size);

  return v->store[i];
}

static inline void* vector_set(const struct vector* const v, const size_t i, const void* const e) {

  assert(i < v->size);

  return v->store[i] = (void*) e;
}

static inline void* vector_head(const struct vector* const v) {

  assert(v->size > 0);

  return vector_get(v, 0);
}

static inline void* vector_tail(const struct vector* const v) {

  assert(v->size > 0);

  return vector_get(v, v->size - 1);
}

static inline void vector_insert(struct vector* const v, const size_t i, const void* const e) {

  assert(i < v->size + 1);

  if (++v->size == v->limit)
    vector_resize(v, v->limit << 1);

  memmove(&v->store[i + 1], &v->store[i], (v->size - i - 1) * sizeof(void*));

  v->store[i] = (void*) e;
}

static inline void* vector_remove(struct vector* const v, const size_t i) {

  assert(i < v->size);

  void* const e = v->store[i];

  memmove(&v->store[i], &v->store[i + 1], v->size-- - i - 1);

  return e;
}

static inline void vector_push(struct vector* const v, const void* const e) {

  vector_insert(v, v->size, e);
}

static inline void* vector_pop(struct vector* const v) {

  return vector_remove(v, v->size - 1);
}

static inline bool vector_bsearch(struct vector* const v, const void* const k, int (*cmp) (const void* a, const void* b), size_t* const i) {

  size_t b = 0;
  size_t e = v->size - 1;

  *i = 0;

  if (!v->size)
    return false;

  *i = (e - b) >> 1;
  int c = cmp(k, v->store[*i]);

  while (b < e) {

    switch (c) {

      // k < v 
      case -1:
        e = *i;
        break;

      // k == v
      case 0:
        return true;

      // k > v
      case 1:
        b = *i + 1; 
        break;

      default: on_fatal("unable to do compare");
    }

    *i = b + ((e - b) >> 1);
    c = cmp(k, v->store[*i]);
  }

  if (c > 0)
    *i += 1;

  return !c;
}

#endif // VECTOR_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
