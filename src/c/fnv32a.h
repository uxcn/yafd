//===-- fnv32a.h - fnv-1a (32) implementation ---------------------*- C -*-===//

#ifndef FNV32A_H
#define FNV32A_H

#include <stddef.h>

#include "config.h" // autoconf

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef FNVMUL 

const uint32_t fnv32a_prime = 0x01000193; // 16777619

#endif

static inline void fnv32a_init(uint32_t* fnv) {

  *fnv = 0x811c9dc5; // 2166136261
}

static inline void fnv32a_finalize(uint32_t* fnv) { *fnv = *fnv; }

static inline void fnv32a(const size_t len, const uint8_t data[len], uint32_t* fnv) {

  for (size_t b = 0; b < len; b++) {

    *fnv ^= data[b];
    
#ifdef FNVMUL

    *fnv *= fnv32a_prime;

#else

    *fnv += (*fnv<<1) + (*fnv<<4) + (*fnv<<7) + (*fnv<<8) + (*fnv<<24);

#endif

  }
}

#endif // FNV32A_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
