//===-- thread.c - thread functions -------------------------------*- C -*-===//

#include "thread.h"


#include "config.h" // autoconf


#include <limits.h>

#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif

#include <unistd.h>


#include "platform.h" // platform

#include "options.h"

#include "math.h"

#include "error.h"
#include "memory.h"

#if defined(HAVE_GETRLIMIT)
#include <sys/resource.h>
#endif

#if defined(HAVE_PTHREAD_H)

#if defined(HAVE_FREEBSD)

#include <pthread_np.h>

#elif defined(HAVE_DARWIN)

#include <sys/types.h>
#include <sys/sysctl.h>

#elif defined(HAVE_WINDOWS)

#include <windows.h>

#endif

#endif

size_t pz; // page size

void* bf; // process buffer

#if defined(HAVE_PTHREAD_H)
static pthread_key_t key;

static pthread_once_t once = PTHREAD_ONCE_INIT;

#if defined(HAVE_FREEBSD)

typedef cpuset_t cpu_set_t;

#endif

#endif

void _thread_init() {

#ifdef HAVE_GETPAGESIZE
  pz = (size_t) getpagesize();
#else
  pz = (size_t) 4096;
#endif
}

#if defined(HAVE_PTHREAD_H)
static void thread_key_destroy(void* a) {

  free(a);
}
#endif

#if defined(HAVE_PTHREAD_H)
static void thread_key_init() {

  if (pthread_key_create(&key, thread_key_destroy))
    on_fatal("unable to create thread local key");
}
#endif

#if defined(HAVE_PTHREAD_H)
int num_cpus() {

  const long default_cpus = 2;

  long cs = 0;

#if defined(HAVE_LINUX) || defined(HAVE_FREEBSD)

  pthread_t this = pthread_self();

  cpu_set_t cpus;

  CPU_ZERO(&cpus);

  int err = pthread_getaffinity_np(this, sizeof(cpus), &cpus);

  if (err)
    return err;

  cs = (long) CPU_COUNT(&cpus);

#elif defined(HAVE_DARWIN)

  int mib[2] = {CTL_HW, HW_NCPU};
  size_t cslen = sizeof(cs);

  int err = sysctl(mib, 2, &cs, &cslen, NULL, 0);

  if (err)
    return err;

#elif defined(_SC_NPROCESSORS_ONLN)

  cs = sysconf(_SC_NPROCESSORS_ONLN);

#elif defined(HAVE_WINDOWS)

  SYSTEM_INFO si;
  GetSystemInfo(&si);

  cs = (long) si.dwNumberOfProcessors;

#endif

  if (cs <= 0) {

    print_error("unable to determine number of cpus... using default (%li)", default_cpus);

    return (int) default_cpus;

  } else if (cs > INT_MAX) {
  
    return INT_MAX; // wow

  } else {

    return (int) cs;
  }
}
#endif

#ifdef HAVE_PTHREAD_H
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
#endif

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
