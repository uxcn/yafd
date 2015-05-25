//===-- thread.c - thread functions -------------------------------*- C -*-===//

#include "thread.h"

#include <pthread.h>

#include <sched.h>

#include <sys/resource.h>

#include "options.h"

#include "options.h"

#include "math.h"

#include "error.h"
#include "memory.h"


size_t pz; // page size

static pthread_key_t key;

static pthread_once_t once = PTHREAD_ONCE_INIT;

void _thread_init() {

  struct rlimit r;

  if (getrlimit(RLIMIT_STACK, &r))
    print_error("unable to determine stack limit");

  pz = r.rlim_cur >> 4;

  pz = min(pz, (size_t) opts.pagesize);
}

static void thread_key_destroy(void* a) {

  free(a);
}

static void thread_key_init() {

  if (pthread_key_create(&key, thread_key_destroy))
    on_fatal("unable to create thread local key");
}

int num_cpus() {

  pthread_t this = pthread_self();

  cpu_set_t cs;
  CPU_ZERO(&cs);

  int err = pthread_getaffinity_np(this, sizeof(cs), &cs);

  if (err)
    return err;

  return CPU_COUNT(&cs);
}

uint8_t* thread_local_buffer(const size_t n) {

  if (pthread_once(&once, thread_key_init))
    on_fatal("unable to initialize thread local key");

  uint8_t* b = pthread_getspecific(key);

  uint8_t* r = frealloc(b, n);

  if (r != b) {

    if (pthread_setspecific(key, r))
      on_fatal("unable to set thread local");
  }

  return r;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
