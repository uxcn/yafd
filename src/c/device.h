//===-- device.h - device structure -------------------------------*- C -*-===//

#ifndef DEVICE_H
#define DEVICE_H

#include "config.h" // autoconf


#include <stddef.h>

#include <sys/types.h>


#include "platform.h" // platform


#include "memory.h"

#include "vector.h"

#include "entry.h"

static const size_t default_init_entries = 4;

struct device {

  dev_t device;

  struct vector entries;
};

static inline int device_compare(const void* const k, const void* const v) {
  const dev_t* const dk = (const dev_t* const) k;
  const struct device* const dv = (const struct device* const) v;

  return (*dk > dv->device) - (*dk < dv->device);
}

static inline void device_init(struct device* const v) {

  vector_init(&v->entries, default_init_entries);
}

static inline struct device* device_create(const dev_t n) {

  struct device* const v = fmalloc(sizeof(struct device));

  device_init(v);

  v->device = n;

  return v;
}

static inline void device_destroy(struct device* const v) {
  
  struct entry* e;

  vector_for_each(e, &v->entries)
    entry_destroy(e);

  vector_destroy(&v->entries);

  free(v);
}

#endif // DEVICE_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
