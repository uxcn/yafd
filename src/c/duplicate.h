//===-- duplicate.h - duplicate structure and functions -----------*- C -*-===//

#ifndef DUPLICATE_H
#define DUPLICATE_H

#include <stddef.h>
#include <stdio.h>

#include <pthread.h>

#include "config.h" // autoconf

#include "options.h"

#include "memory.h"

#include "device.h"
#include "entry.h"

#include "hash.h"
#include "queue.h"
#include "vector.h"

#include "digest.h"


static const size_t default_init_devices = 8;

struct duplicate {

  pthread_mutex_t mutex;

  size_t count;

  struct digest_t digest;

  struct vector devices;

  struct hash_entry hash;

  struct queue_entry queue;
};

int duplicate_content_compare(const void* const a, const void* const b);

int duplicate_content_compare_mmap(const void* const a, const void* const b);

static inline int duplicate_digest_compare(const void* const k, const void* const v) {
  const struct digest_t* const dk = (const struct digest_t* const) k;
  const struct duplicate* const dv = (const struct duplicate* const) v;

  return digest_compare(dk, &dv->digest, opts.digs);
}

static inline void duplicate_init(struct duplicate* const d) {

  pthread_mutex_init(&d->mutex, NULL);

  d->count = 0;

  digest_init(&d->digest, opts.digs);

  vector_init(&d->devices, default_init_devices);
}

static inline struct duplicate* duplicate_create() {

  struct duplicate* d = fmalloc(sizeof(struct duplicate));

  duplicate_init(d);

  return d;
}

static inline void duplicate_destroy(struct duplicate* const d) {
  
  struct device* v;

  vector_for_each(v, &d->devices)
    device_destroy(v);

  vector_destroy(&d->devices);

  free(d);
}

static inline bool duplicate_take(struct queue* const q, struct duplicate** d) {

  struct queue_entry* e = NULL;

  if (!queue_take(q, &e))
    return false;

  *d = container_of(e, struct duplicate, queue);

  return true;
}

static inline struct device* duplicate_device_lazy_get(struct duplicate* const d, const dev_t n) {

  size_t p;

  if (vector_bsearch(&d->devices, &n, device_compare, &p))
    return vector_get(&d->devices, p);
  
  struct device* const v = device_create(n);

  vector_insert(&d->devices, p, v);

  return v;
}

static inline struct entry* duplicate_entry_lazy_get(struct duplicate* const d, const dev_t n, const struct stat* const s, const char* const a, size_t* c,  bool* i) {

  size_t p;

  struct device* const v = duplicate_device_lazy_get(d, n);

  if (vector_bsearch(&v->entries, &s->st_ino, entry_compare, &p)) {

    struct entry* const e = vector_get(&v->entries, p);

    entry_alias_add(e, a);

    *i = false;
    *c = d->count;

    return e;
  }
  
  struct entry* const e = entry_create(s, a);

  vector_insert(&v->entries, p, e);

  d->count++;
  
  *i = true;
  *c = d->count;

  return e;
}

static inline struct entry* duplicate_entry_lazy_get_atomic(struct duplicate* const d, const dev_t n, const struct stat* const s, const char* const a,  size_t* c, bool* i) {

  struct entry* e;

  pthread_mutex_lock(&d->mutex);

  e = duplicate_entry_lazy_get(d, n, s, a, c, i);

  pthread_mutex_unlock(&d->mutex);

  return e;
}


static inline void duplicate_entry_add(struct duplicate* const d, const struct entry* const e) {

  size_t p;

  struct device* v = duplicate_device_lazy_get(d, e->device);

  vector_bsearch(&v->entries, &e->inode, entry_compare, &p);

  vector_insert(&v->entries, p, e);

  d->count++;
}

static inline void duplicate_print(const struct duplicate* const d) {

  const struct device* v;
  const struct entry* e;

  vector_for_each(v, &d->devices) {

    printf("(%lu)\n", i);

    vector_for_each(e, &v->entries) {

      printf("[%lu] ", i);
      entry_print(NULL, e, NULL);
      printf("\n");
    }
  }
}

#endif // DUPLICATE_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
