//===-- farm.h - farmhash implementations -------------------------*- C -*-===//

#ifndef FARMHASH_H
#define FARMHASH_H

#include <stdlib.h>
#include <stdint.h>

#include "platform.h" // platform

#include "bigint.h"


static inline void frm64_init(uint64_t* frm) { *frm = 0; }

static inline void frm64_finalize(uint64_t* frm) { *frm = *frm; }

void frm64(size_t len, const uint8_t data[len], uint64_t* frm);


static inline void frm128_init(uint128_t* frm) { frm->a = 0, frm->b = 0; }

static inline void frm128_finalize(uint128_t* frm) { *frm = *frm; }

void frm128(size_t len, const uint8_t data[len], uint128_t* frm);

#endif  // FARMHASH_H
