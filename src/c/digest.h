//===-- digest.h - digest functions -------------------------------*- C -*-===//

#ifndef DIGEST_H
#define DIGEST_H

#include "config.h" // autoconf


#include <stddef.h>
#include <limits.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif


#include "platform.h" // platform


#include "bigint.h"

#include "crc32.h"
#include "fnv32a.h"
#include "fnv64a.h"

#include "farm.h"
#include "spooky.h"

static size_t num_digs = 0;

enum digest {

  dig_crc32  = 'C',

  dig_fnv32a = 'N',
  dig_fnv64a = 'V',

  dig_frm64  = 'F',
  dig_frm128 = 'M',

  dig_sky64  = 'S',
  dig_sky128 = 'P',
};

struct digest_t {

  uint32_t crc32;

  uint32_t fnv32a;

  uint64_t fnv64a;

  uint64_t frm64;

  uint64_t sky64;

  uint128_t frm128;

  uint128_t sky128;
};

static inline void _digest_init() {

  enum digest d = dig_crc32;

  size_t digests = 0;

  switch(d) { 
    // emit compiler warning
    
    case dig_crc32:   digests++;
    case dig_fnv32a:  digests++;
    case dig_fnv64a:  digests++;
    case dig_frm64:   digests++;
    case dig_frm128:  digests++;
    case dig_sky64:   digests++;
    case dig_sky128:  digests++;
  };

  num_digs = digests;
}


static inline const char* to_string_digest(const enum digest d) {

  switch(d) {

    case dig_crc32:
      return "crc32";

    case dig_fnv32a:
      return "fnv32a";

    case dig_fnv64a:
      return "fnv64a";

    case dig_frm64:
      return "frm64";

    case dig_frm128:
      return "frm128";

    case dig_sky64:
      return "sky64";

    case dig_sky128:
      return "sky128";
  }

  return NULL;
}

static inline int digest_compare(const struct digest_t* const a, const struct digest_t* const b, const enum digest* const ts) {

  int cmp = 0;
  enum digest t = ts[0];

  for (size_t i = 0; !cmp && t; i++) {

    t = ts[i];

    switch (t) {

      case dig_crc32:
        cmp = (a->crc32 > b->crc32) - (b->crc32 < a->crc32);
        break;

      case dig_fnv32a:
        cmp = (a->fnv32a > b->fnv32a) - (b->fnv32a < a->fnv32a);
        break;

      case dig_fnv64a:
        cmp = (a->fnv64a > b->fnv64a) - (b->fnv64a < a->fnv64a);
        break;

      case dig_frm64:
        cmp = (a->frm64 > b->frm64) - (b->frm64 < a->frm64);
        break;

      case dig_sky64:
        cmp = (a->sky64 > b->sky64) - (b->sky64 < a->sky64);
        break;

      case dig_frm128:
        cmp = uint128_gt(&a->frm128, &b->frm128) - uint128_lt(&b->frm128, &a->frm128);
        break;

      case dig_sky128:
        cmp = uint128_gt(&a->sky128, &b->sky128) - uint128_lt(&b->sky128, &a->sky128);
        break;
    }
  }

  return cmp;
}

static inline void digest_init(struct digest_t* dig, const enum digest* const ts) {

  enum digest t = ts[0];

  for (size_t i = 0; t; i++) {

    t = ts[i];

    switch (t) {

      case dig_crc32:
        crc32_init(&dig->crc32);
        break;

      case dig_fnv32a:
        fnv32a_init(&dig->fnv32a);
        break;

      case dig_fnv64a:
        fnv64a_init(&dig->fnv64a);
        break;

      case dig_frm64:
        frm64_init(&dig->frm64);
        break;

      case dig_frm128:
        frm128_init(&dig->frm128);
        break;

      case dig_sky64:
        sky64_init(&dig->sky64);
        break;

      case dig_sky128:
        sky128_init(&dig->sky128);
        break;
    }
  }
}

static inline void digest_finalize(struct digest_t* dig, const enum digest* const ts) {

  enum digest t = ts[0];

  for (size_t i = 0; t; i++) {

    t = ts[i];

    switch (t) {

      case dig_crc32:
        crc32_finalize(&dig->crc32);
        break;

      case dig_fnv32a:
        fnv32a_finalize(&dig->fnv32a);
        break;

      case dig_fnv64a:
        fnv64a_finalize(&dig->fnv64a);
        break;

      case dig_frm64:
        frm64_finalize(&dig->frm64);
        break;

      case dig_frm128:
        frm128_finalize(&dig->frm128);
        break;

      case dig_sky64:
        sky64_finalize(&dig->sky64);
        break;

      case dig_sky128:
        sky128_finalize(&dig->sky128);
        break;
    }
  }
}

static inline void digest(const size_t len, const uint8_t data[len], struct digest_t* dig, const enum digest* const ts) {

  enum digest t = ts[0];

  for (size_t i = 0; t; i++) {

    t = ts[i];

    switch (t) {

      case dig_crc32:
        crc32(len, data, &dig->crc32);
        break;

      case dig_fnv32a:
        fnv32a(len, data, &dig->fnv32a);
        break;

      case dig_fnv64a:
        fnv64a(len, data, &dig->fnv64a);
        break;

      case dig_frm64:
        frm64(len, data, &dig->frm64);
        break;

      case dig_frm128:
        frm128(len, data, &dig->frm128);
        break;

      case dig_sky64:
        sky64(len, data, &dig->sky64);
        break;

      case dig_sky128:
        sky128(len, data, &dig->sky128);
        break;
    }
  }
}

#endif // DIGEST_H
// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
