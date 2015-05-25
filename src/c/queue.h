//===-- queue.h - concurrent queue --------------------------------*- C -*-===//

#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#include <pthread.h>

#include "config.h" // autoconf

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "list.h"


#define queue_entry list_entry

struct queue {

  pthread_mutex_t mutex;

  struct list entries;
};

static inline void queue_init(struct queue* const q) {

  pthread_mutex_init(&q->mutex, NULL);

  list_init(&q->entries); 
}

static inline void queue_add(struct queue* const q, struct queue_entry* const e) {

  pthread_mutex_lock(&q->mutex);
    
  list_add_tail(&q->entries, e);

  pthread_mutex_unlock(&q->mutex);
}

static inline bool queue_take(struct queue* const q, struct queue_entry** e) {

  pthread_mutex_lock(&q->mutex);

  *e = list_remove_head(&q->entries);

  pthread_mutex_unlock(&q->mutex);

  return *e;
}

static inline bool is_empty_queue(struct queue* const q) {

  bool e;

  pthread_mutex_lock(&q->mutex);

  e = is_empty_list(&q->entries);

  pthread_mutex_unlock(&q->mutex);

  return e;
}

#endif // QUEUE_H
// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
