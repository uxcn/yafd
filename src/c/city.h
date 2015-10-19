//===-- city.h - cityhash implementations -------------------------*- C -*-===// 

#ifndef CITY_H
#define CITY_H

#include "config.h" // autoconf


#include <stdlib.h>
#include <stdint.h>


#include "platform.h" // platform


#include "bigint.h"

static inline void cty64_init(uint64_t* cty) { *cty = 0; }

static inline void cty64_finalize(uint64_t* cty) { *cty = *cty; }

void cty64(size_t len, const uint8_t data[len], uint64_t* cty);


static inline void cty128_init(uint128_t* cty) { cty->a = 0, cty->b = 0; }

static inline void cty128_finalize(uint128_t* cty) { *cty = *cty; }

void cty128(size_t len, const uint8_t data[len], uint128_t* cty);

#if defined(__SSE4_2__) && defined(__x86_64)

void cty128_crc(size_t len, const uint8_t data[len], uint128_t* cty);

void cty256_crc(size_t len, const uint8_t data[len], uint256_t* cty);

#endif

#endif // CITY_H
// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
