//===-- path.h - path utilities -------------------------*- C -*-===//

#ifndef PATH_H
#define PATH_H

#include "memory.h"

#include "queue.h"

struct path {

  const char* name;

  struct queue_entry queue;
};

static inline struct path* path_create(const char* const n) {

  struct path* d = fmalloc(sizeof(struct path));

  d->name = n;

  return d;
}

static inline void path_destroy(struct path* const p) {

  free((char*) p->name);
  free(p);
}

static inline bool path_take(struct queue* const q, struct path** d) {

  struct queue_entry* e = NULL;

  if (!queue_take(q, &e))
    return false;

  *d = container_of(e, struct path, queue);

  return true;
}

#endif // PATH_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
