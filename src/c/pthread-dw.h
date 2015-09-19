//===-- pthread-dw.h darwin pthreads ------------------------------*- C -*-===//

#ifndef PTHREAD_DW_H
#define PTHREAD_DW_H

#include <stddef.h>
#include <stdbool.h>

#include <pthread.h>

typedef struct pthread_barrier_t {

  size_t waiters;

  size_t threads;

  pthread_mutex_t mutex;

  pthread_cond_t cond;

} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t* restrict b, const void* restrict a, unsigned t) {

  if (a)
    return -1; // no attrib support

  b->waiters = 0;
  b->threads = t;

  pthread_mutex_init(&b->mutex, NULL);
  pthread_cond_init(&b->cond, NULL);

  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t* b) {

  b->waiters = 0;
  b->threads = 0;

  pthread_mutex_destroy(&b->mutex);
  pthread_cond_destroy(&b->cond);

  return 0;
}

int pthread_barrier_wait(pthread_barrier_t* b) {

  pthread_mutex_lock(&b->mutex);
  b->waiters++;

  
  if (b->waiters > b->threads) { 

    pthread_mutex_unlock(&b->mutex);
    return -1; // uninitialized
  }

  if (b->waiters == b->threads) {

    pthread_cond_broadcast(&b->cond);
    pthread_mutex_unlock(&b->mutex);

    return 1; // PTHREAD_BARRIER_SERIAL_THREAD
  }

retry:

  pthread_cond_wait(&b->cond, &b->mutex);

  if (b->waiters < b->threads)
    goto retry;

  pthread_mutex_unlock(&b->mutex);

  return 0;
}


#endif // PTHREAD_DW_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
