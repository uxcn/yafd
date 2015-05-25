//===-- fnv64a.h - fnv-1a (64) implementation ---------------------*- C -*-===//

#ifndef FNV64A_H
#define FNV64A_H

#include <stddef.h>

#include "config.h" // autoconf

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef FNVMUL

const uint64_t fnv64a_prime = 0x100000001b3; // 1099511628211

#endif

static inline void fnv64a_init(uint64_t* fnv) {

  *fnv = 0xcbf29ce484222325; // 14695981039346656037
}

static inline void fnv64a_finalize(uint64_t* fnv) { *fnv = *fnv; }

static inline void fnv64a(const size_t len, const uint8_t data[len], uint64_t* fnv) {

  for (size_t b = 0; b < len; b++)  {

    *fnv ^= data[b];

#ifdef FNVMUL

    *fnv *= fnv64a_prime;

#else

    *fnv += (*fnv << 40) + (*fnv << 8) + (*fnv << 7) + (*fnv << 5) + (*fnv << 4) + (*fnv << 1);

#endif

  }
}

#endif // FNV64A_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
