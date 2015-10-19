//===-- worker.h - worker functions -------------------------------*- C -*-===//

#ifndef WORKER_H
#define WORKER_H

#include "config.h" // autoconf


#include <pthread.h>


#include "platform.h" // platform


#include "queue.h"

extern struct queue paths;

extern struct queue duplicates;

void _worker_init(const int threads);

int worker_start(const int threads, pthread_t tids[threads]);

int worker_end(const int threads, pthread_t tids[threads]);

void* worker(void* a);

#endif // WORKER_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
