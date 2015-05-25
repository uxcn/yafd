//===-- thread.h - thread functions -------------------------------*- C -*-===//

#ifndef THREAD_H
#define THREAD_H

#include <stddef.h>

#include "config.h" // autoconf

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

extern size_t pz; // page size

int num_cpus();

uint8_t* thread_local_buffer(const size_t n);


#endif // THREAD_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
