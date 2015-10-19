//===-- bigint.h - big integer types ------------------------------*- C -*-===//

#ifndef BIGINT_H
#define BIGINT_H

#include "config.h" // autoconf


#include <stdint.h>
#include <stdbool.h>


#include "platform.h" // platform

struct uint128_t {
  uint64_t a;
  uint64_t b;
};

typedef struct uint128_t uint128_t;

struct uint256_t {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  uint64_t d;
};

typedef struct uint256_t uint256_t;

static inline bool uint128_gt(const uint128_t* const a, const uint128_t* const b) {


  return (a->b == b->b) ? (a->a > b->a) : (a->b > b->b);
}

static inline bool uint128_lt(const uint128_t* const a, const uint128_t* const b) {


  return (a->b == b->b) ? (a->a < b->a) : (a->b < b->b);
}


#endif // BIGINT_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
