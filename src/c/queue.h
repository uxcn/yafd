//===-- queue.h - concurrent queue --------------------------------*- C -*-===//

#ifndef QUEUE_H
#define QUEUE_H

#include <assert.h>

#include "config.h" // autoconf


#include <stdbool.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif


#include "platform.h" // platform


#include "list.h"

#define queue_entry list_entry

struct queue {

#ifdef HAVE_PTHREAD_H
  pthread_mutex_t mutex;
#endif

  struct list entries;
};

static inline void queue_init(struct queue* const q) {

  assert(q != NULL);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_init(&q->mutex, NULL);
#endif

  list_init(&q->entries); 
}

static inline void queue_add(struct queue* const q, struct queue_entry* const e) {

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock(&q->mutex);
#endif
    
  list_add_tail(&q->entries, e);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock(&q->mutex);
#endif
}

static inline bool queue_take(struct queue* const q, struct queue_entry** e) {

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock(&q->mutex);
#endif

  *e = list_remove_head(&q->entries);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock(&q->mutex);
#endif

  return *e;
}

static inline bool is_empty_queue(struct queue* const q) {

  bool e;

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock(&q->mutex);
#endif

  e = is_empty_list(&q->entries);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock(&q->mutex);
#endif

  return e;
}

#endif // QUEUE_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
