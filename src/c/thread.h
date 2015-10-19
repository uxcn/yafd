//===-- thread.h - thread functions -------------------------------*- C -*-===//

#ifndef THREAD_H
#define THREAD_H

#include "config.h" // autoconf


#include <stddef.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "platform.h" // platform

extern size_t pz; // page size

#ifndef HAVE_PTHREAD_H
extern void* bf; // process buffer
#endif

void _thread_init();

#ifdef HAVE_PTHREAD_H
int num_cpus();
#endif

#ifdef HAVE_PTHREAD_H
uint8_t* thread_local_buffer(const size_t n);
#endif


#endif // THREAD_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
