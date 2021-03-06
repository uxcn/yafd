//===-- hash.h - hash table ---------------------------------------*- C -*-===//

#ifndef HASH_H
#define HASH_H

#include <assert.h>

#include "config.h" // autoconf

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif


#include "platform.h" // platform


#include "math.h"
#include "memory.h"

#define hash_for_each(entry, hash)                                             \
  for (size_t i = 0; i < (hash)->limit; i++)                                   \
    if (((entry) = (hash)->table[i]))                                          \

struct hash_entry {

  size_t value;
};

struct hash {

#ifdef HAVE_PTHREAD_H
  pthread_mutex_t mutex;
#endif

  size_t size;
  size_t limit;

  struct hash_entry** table;
};

static inline size_t hash_farm(const size_t v) {
  
  // farm hash
  
  const size_t k = 0x9ae16a3b2f90404f;  
  const size_t mul = k + 16;

  size_t a = ror(v, 37) * mul + (v + k);
  size_t b = (ror(v + k, 25) + v) * mul;

  a = (a ^ b) * mul;
  a ^= (a >> 47);

  b = (b ^ a) * mul;;
  b ^= (b >> 47);

  return b * mul;
}

static inline void hash_reset(struct hash* const h, size_t s) {
    
  s = max(s, 16);

  h->size = 0;
  h->limit = clp2(s); // 2^ceil(log2(x))
  h->table = fcalloc(h->limit, sizeof(struct hash_entry*));
}

static inline void hash_init(struct hash* const h, size_t s) {

  assert(h != NULL);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_init(&h->mutex, NULL);
#endif

  hash_reset(h, s);
}

static inline void hash_release(struct hash* const h) {

  if (!h->limit)
    return;

  h->size = 0;
  h->limit = 0;
  free(h->table);
}

static inline void hash_destroy(struct hash* const h) {

  assert(h != NULL);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_destroy(&h->mutex);
#endif

  hash_release(h);
}

static inline void hash_resize(struct hash* const h, const size_t s) {

  struct hash o = *h;
  struct hash* a = &o;

  assert(h->size < s);

  hash_reset(h, s);

  struct hash_entry* e = NULL;

  hash_for_each(e, a) {

    size_t p = hash_farm(e->value);
    struct hash_entry* he = NULL;

    do {

      he = h->table[p++ & (h->limit - 1)]; 

    } while (he);

    h->table[(p - 1) & (h->limit - 1)] = e;

  }

  h->size = a->size;
  
  hash_release(a);
}

static inline struct hash_entry* hash_get(const struct hash* const h,
                                          const size_t v) {
  
  size_t p = hash_farm(v);

  struct hash_entry* he = NULL;

  do {

    he = h->table[p++ & (h->limit - 1)];

    if (!he || he->value == v)
      return he;

  } while (he);

  return NULL;
}

static inline bool hash_put(struct hash* const h, struct hash_entry* const e) {

  if (h->size == h->limit >> 1)
    hash_resize(h, h->limit << 1);

  size_t p = hash_farm(e->value);
  struct hash_entry* he = NULL;

  do {

    he = h->table[p++ & (h->limit - 1)]; 

  } while (he);

  h->table[(p - 1) & (h->limit - 1)] = e;

  h->size++;

  return true;
}

static inline struct hash_entry* hash_get_or_put_atomic(struct hash* const h,
                                                        struct hash_entry* const e) {

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock(&h->mutex);
#endif

  if (h->size == h->limit >> 1)
    hash_resize(h, h->limit << 1);

  size_t p = hash_farm(e->value);
  struct hash_entry* he = NULL;

  do {

    he = h->table[p++ & (h->limit - 1)]; 

    if (he && he->value == e->value) {

#ifdef HAVE_PTHREAD_H
      pthread_mutex_unlock(&h->mutex);
#endif
      return he;
    }

  } while (he);

  h->table[(p - 1) & (h->limit - 1)] = e;

  h->size++;

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock(&h->mutex);
#endif
  
  return e;
}

static inline void hash_remove(struct hash* const h,
                               struct hash_entry* const e) {

  size_t p = hash_farm(e->value);
  struct hash_entry* he = NULL; 

  assert(h->size > 0);

  do {

    he = h->table[p++ & (h->limit - 1)]; 

    if (he->value == e->value) {
      
      do {

        he = h->table[(p - 1) & (h->limit - 1)] = h->table[p & (h->limit - 1)];
        p++;

      } while (he);

      return;
    }

  } while (he);

  h->size--;
}

#endif // HASH_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
