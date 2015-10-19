//===-- crc32.h - crc32 implementations ---------------------------*- C -*-===//

#ifndef CRC32_H
#define CRC32_H

#include "config.h" // autoconf


#include <stddef.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif


#include "platform.h" // platform



#include "crc32-algo.h"

static inline void crc32_init(uint32_t* crc) { *crc = 0xffffffff; }

static inline void crc32_finalize(uint32_t* crc) { *crc ^= 0xffffffff; }

static inline void crc32(const size_t len, const uint8_t data[len], uint32_t* crc) {

  uint64_t* dq = (uint64_t*)data;

  size_t qs = len >> 3;

  while (qs--)
    crc32q(dq++, crc);

  uint32_t* dd = (uint32_t*)dq;
  uint16_t* dw = (uint16_t*)dq;
  uint8_t* db = (uint8_t*)dq;

  // tail
  switch (len & 7) {

    case 7:
      crc32d(dd, crc);
      crc32w(dw + 2, crc);
      crc32b(db + 6, crc);
      break;

    case 6:
      crc32d(dd, crc);
      crc32w(dw + 2, crc);
      break;

    case 5:
      crc32d(dd, crc);
      crc32b(db + 4, crc);
      break;

    case 4:
      crc32d(dd, crc);
      break;

    case 3:
      crc32w(dw, crc);
      crc32b(db + 2, crc);
      break;

    case 2:
      crc32w(dw, crc);
      break;

    case 1:
      crc32b(db, crc);
  }
}

#endif // CRC32_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
