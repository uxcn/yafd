//===-- spooky.h - spookyhash implementations ---------------------*- C -*-===//

#ifndef SPOOKY_H
#define SPOOKY_H

#include <stdlib.h>
#include <stdint.h>

#include "bigint.h"

static inline void sky64_init(uint64_t* sky) { *sky = 0; }

static inline void sky64_finalize(uint64_t* sky) { *sky = *sky; }

void sky64(const size_t len, const uint8_t data[len], uint64_t* sky);


static inline void sky128_init(uint128_t* sky) { sky->a = 0,  sky->b = 0; }

static inline void sky128_finalize(uint128_t* sky) { *sky = *sky; }

void sky128(const size_t len, const uint8_t data[len], uint128_t* sky);

#endif // SPOOKY_H
// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
