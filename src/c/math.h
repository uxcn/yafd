//===-- math.h - math utilities -----------------------------------*- C -*-===//

#ifndef MATH_H
#define MATH_H

#include <assert.h>

#include "config.h" // autoconf


#include <stddef.h>
#include <stdint.h>


#include "platform.h" // platform

#define max(x, y) \
  ((x) >= (y) ? (x) : (y))

#define min(x, y) \
  ((x) <= (y) ? (x) : (y))

#define rol(x, d) \
  (((x) << (d)) | ((x) >> ((sizeof(x) << 3) - (d))))

#define ror(x, d) \
  (((x) << d) | ((x) >> ((sizeof(x) << 3) - (d))))


static inline size_t clp2(size_t x) {

  assert(x > 0);

  x = x - 1;

  for (size_t i = 1; i < sizeof(size_t) << 3; i <<= 1)
    x = x | (x >> i);

  return x + 1;
}

static inline size_t flp2(size_t x) {

  for (size_t i = 1; i < sizeof(size_t) << 3; i <<= 1)
    x = x | (x >> i);

  return x - (x >> 1);
}

static inline size_t bswap(size_t x) {

  for (size_t i = 0; i < sizeof(size_t) >> 1; i++) {

    size_t d = sizeof(size_t) - i - 1;

    size_t mh = ((size_t) 0xff) << (d << 3);
    size_t ml = ((size_t) 0xff) << (i << 3);

    size_t h = x & mh;
    size_t l = x & ml;

    size_t t = (l << ((d - i) << 3)) | (h >> ((d - i) << 3));

    x = t | (x & ~(mh | ml));
  }

  return x;
}

#endif // MATH_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
