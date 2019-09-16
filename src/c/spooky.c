//===-- spooky.c - sky implementations ---------------------*- C -*-===//

#include <memory.h>

#include "spooky.h"


#include "config.h" // autoconf

#include "platform.h" // platform


#include "math.h"

#define ALLOW_UNALIGNED_READS 1

static const size_t sc_num_vars = 12;

static const size_t sc_block_size = 96; // sc_num_vars * 8

static const size_t sc_buf_size = 192; // sc_block_size * 2

static const uint64_t sc_const = 0xdeadbeefdeadbeef;

static inline void sky_mix(const uint64_t* data, uint64_t* s0,
                                  uint64_t* s1, uint64_t* s2, uint64_t* s3,
                                  uint64_t* s4, uint64_t* s5, uint64_t* s6,
                                  uint64_t* s7, uint64_t* s8, uint64_t* s9,
                                  uint64_t* s10, uint64_t* s11) {
  *s0 += data[0];
  *s2 ^= *s10;
  *s11 ^= *s0;
  *s0 = rol(*s0, 11);
  *s11 += *s1;
  *s1 += data[1];
  *s3 ^= *s11;
  *s0 ^= *s1;
  *s1 = rol(*s1, 32);
  *s0 += *s2;
  *s2 += data[2];
  *s4 ^= *s0;
  *s1 ^= *s2;
  *s2 = rol(*s2, 43);
  *s1 += *s3;
  *s3 += data[3];
  *s5 ^= *s1;
  *s2 ^= *s3;
  *s3 = rol(*s3, 31);
  *s2 += *s4;
  *s4 += data[4];
  *s6 ^= *s2;
  *s3 ^= *s4;
  *s4 = rol(*s4, 17);
  *s3 += *s5;
  *s5 += data[5];
  *s7 ^= *s3;
  *s4 ^= *s5;
  *s5 = rol(*s5, 28);
  *s4 += *s6;
  *s6 += data[6];
  *s8 ^= *s4;
  *s5 ^= *s6;
  *s6 = rol(*s6, 39);
  *s5 += *s7;
  *s7 += data[7];
  *s9 ^= *s5;
  *s6 ^= *s7;
  *s7 = rol(*s7, 57);
  *s6 += *s8;
  *s8 += data[8];
  *s10 ^= *s6;
  *s7 ^= *s8;
  *s8 = rol(*s8, 55);
  *s7 += *s9;
  *s9 += data[9];
  *s11 ^= *s7;
  *s8 ^= *s9;
  *s9 = rol(*s9, 54);
  *s8 += *s10;
  *s10 += data[10];
  *s0 ^= *s8;
  *s9 ^= *s10;
  *s10 = rol(*s10, 22);
  *s9 += *s11;
  *s11 += data[11];
  *s1 ^= *s9;
  *s10 ^= *s11;
  *s11 = rol(*s11, 46);
  *s10 += *s0;
}

static inline void sky_end_partial(uint64_t* h0, uint64_t* h1,
                                          uint64_t* h2, uint64_t* h3,
                                          uint64_t* h4, uint64_t* h5,
                                          uint64_t* h6, uint64_t* h7,
                                          uint64_t* h8, uint64_t* h9,
                                          uint64_t* h10, uint64_t* h11) {
  *h11 += *h1;
  *h2 ^= *h11;
  *h1 = rol(*h1, 44);
  *h0 += *h2;
  *h3 ^= *h0;
  *h2 = rol(*h2, 15);
  *h1 += *h3;
  *h4 ^= *h1;
  *h3 = rol(*h3, 34);
  *h2 += *h4;
  *h5 ^= *h2;
  *h4 = rol(*h4, 21);
  *h3 += *h5;
  *h6 ^= *h3;
  *h5 = rol(*h5, 38);
  *h4 += *h6;
  *h7 ^= *h4;
  *h6 = rol(*h6, 33);
  *h5 += *h7;
  *h8 ^= *h5;
  *h7 = rol(*h7, 10);
  *h6 += *h8;
  *h9 ^= *h6;
  *h8 = rol(*h8, 13);
  *h7 += *h9;
  *h10 ^= *h7;
  *h9 = rol(*h9, 38);
  *h8 += *h10;
  *h11 ^= *h8;
  *h10 = rol(*h10, 53);
  *h9 += *h11;
  *h0 ^= *h9;
  *h11 = rol(*h11, 42);
  *h10 += *h0;
  *h1 ^= *h10;
  *h0 = rol(*h0, 54);
}

static inline void sky_end(const uint64_t *data, uint64_t *h0,
                                  uint64_t *h1, uint64_t *h2, uint64_t *h3,
                                  uint64_t *h4, uint64_t *h5, uint64_t *h6,
                                  uint64_t *h7, uint64_t *h8, uint64_t *h9,
                                  uint64_t *h10, uint64_t *h11) {

  *h0 += data[0];   *h1 += data[1];   *h2 += data[2];   *h3 += data[3];
  *h4 += data[4];   *h5 += data[5];   *h6 += data[6];   *h7 += data[7];
  *h8 += data[8];   *h9 += data[9];   *h10 += data[10]; *h11 += data[11];
  sky_end_partial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
  sky_end_partial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
  sky_end_partial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
}

static inline void sky_smix(uint64_t* h0, uint64_t* h1, uint64_t* h2,
                                   uint64_t* h3) {

  *h2 = rol(*h2, 50);
  *h2 += *h3;
  *h0 ^= *h2;
  *h3 = rol(*h3, 52);
  *h3 += *h0;
  *h1 ^= *h3;
  *h0 = rol(*h0, 30);
  *h0 += *h1;
  *h2 ^= *h0;
  *h1 = rol(*h1, 41);
  *h1 += *h2;
  *h3 ^= *h1;
  *h2 = rol(*h2, 54);
  *h2 += *h3;
  *h0 ^= *h2;
  *h3 = rol(*h3, 48);
  *h3 += *h0;
  *h1 ^= *h3;
  *h0 = rol(*h0, 38);
  *h0 += *h1;
  *h2 ^= *h0;
  *h1 = rol(*h1, 37);
  *h1 += *h2;
  *h3 ^= *h1;
  *h2 = rol(*h2, 62);
  *h2 += *h3;
  *h0 ^= *h2;
  *h3 = rol(*h3, 34);
  *h3 += *h0;
  *h1 ^= *h3;
  *h0 = rol(*h0, 5);
  *h0 += *h1;
  *h2 ^= *h0;
  *h1 = rol(*h1, 36);
  *h1 += *h2;
  *h3 ^= *h1;
}

static inline void sky_short_end(uint64_t* h0, uint64_t* h1,
                                        uint64_t* h2, uint64_t* h3) {

  *h3 ^= *h2;
  *h2 = rol(*h2, 15);
  *h3 += *h2;
  *h0 ^= *h3;
  *h3 = rol(*h3, 52);
  *h0 += *h3;
  *h1 ^= *h0;
  *h0 = rol(*h0, 26);
  *h1 += *h0;
  *h2 ^= *h1;
  *h1 = rol(*h1, 51);
  *h2 += *h1;
  *h3 ^= *h2;
  *h2 = rol(*h2, 28);
  *h3 += *h2;
  *h0 ^= *h3;
  *h3 = rol(*h3, 9);
  *h0 += *h3;
  *h1 ^= *h0;
  *h0 = rol(*h0, 47);
  *h1 += *h0;
  *h2 ^= *h1;
  *h1 = rol(*h1, 54);
  *h2 += *h1;
  *h3 ^= *h2;
  *h2 = rol(*h2, 32);
  *h3 += *h2;
  *h0 ^= *h3;
  *h3 = rol(*h3, 25);
  *h0 += *h3;
  *h1 ^= *h0;
  *h0 = rol(*h0, 63);
  *h1 += *h0;
}

static void sky_short(const size_t len, const uint8_t data[len], uint128_t* sky) {

  // short hash ... it could be used on any message,
  // but it's used by spooky just for short messages.

  uint64_t buf[2 * sc_num_vars];

  union {

    const uint8_t* p8;
    uint32_t* p32;
    uint64_t* p64;
    size_t i;
  } u;

  u.p8 = data;

  if (!ALLOW_UNALIGNED_READS && (u.i & 0x7)) {

    memcpy(buf, data, len);
    u.p64 = buf;
  }

  size_t remainder = len % 32;
  uint64_t a = sky->a;
  uint64_t b = sky->b;
  uint64_t c = sc_const;
  uint64_t d = sc_const;

  if (len > 15) {

    const uint64_t* end = u.p64 + (len / 32) * 4;

    // handle all complete sets of 32 bytes
    for (; u.p64 < end; u.p64 += 4) {

      c += u.p64[0];
      d += u.p64[1];
      sky_smix(&a, &b, &c, &d);
      a += u.p64[2];
      b += u.p64[3];
    }

    // Handle the case of 16+ remaining bytes.
    if (remainder >= 16) {

      c += u.p64[0];
      d += u.p64[1];
      sky_smix(&a, &b, &c, &d);
      u.p64 += 2;
      remainder -= 16;
    }
  }

  // handle the last 0..15 bytes, and its length
  d += ((uint64_t)len) << 56;

  switch (remainder) {

    case 15:
      d += ((uint64_t)u.p8[14]) << 48;
      // fall-through
    case 14:
      d += ((uint64_t)u.p8[13]) << 40;
      // fall-through
    case 13:
      d += ((uint64_t)u.p8[12]) << 32;
      // fall-through
    case 12:
      d += u.p32[2];
      c += u.p64[0];
      break;
    case 11:
      d += ((uint64_t)u.p8[10]) << 16;
      // fall-through
    case 10:
      d += ((uint64_t)u.p8[9]) << 8;
      // fall-through
    case 9:
      d += (uint64_t)u.p8[8];
      // fall-through
    case 8:
      c += u.p64[0];
      break;
    case 7:
      c += ((uint64_t)u.p8[6]) << 48;
      // fall-through
    case 6:
      c += ((uint64_t)u.p8[5]) << 40;
      // fall-through
    case 5:
      c += ((uint64_t)u.p8[4]) << 32;
      // fall-through
    case 4:
      c += u.p32[0];
      break;
    case 3:
      c += ((uint64_t)u.p8[2]) << 16;
      // fall-through
    case 2:
      c += ((uint64_t)u.p8[1]) << 8;
      // fall-through
    case 1:
      c += (uint64_t)u.p8[0];
      break;
    case 0:
      c += sc_const;
      d += sc_const;
  }

  sky_short_end(&a, &b, &c, &d);

  sky->a = a;
  sky->b = b;
}


void sky32(const size_t len, const uint8_t data[len], uint32_t* sky) {

  uint128_t x = { *sky, *sky };

  sky128(len, data, &x);

  *sky = (uint32_t) x.a;
}

void sky64(const size_t len, const uint8_t data[len], uint64_t* sky) {
                             
  uint128_t x = { *sky, *sky };

  sky128(len, data, &x);

  *sky = x.a;
}

// do the whole hash in one call
void sky128(size_t len, const uint8_t data[len], uint128_t* sky) {

  if (len < sc_buf_size) {

    sky_short(len, data, sky);
    return;
  }

  uint64_t h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
  uint64_t buf[sc_num_vars];
  size_t remainder;
  uint64_t* end;

  union {

    const uint8_t* p8;
    uint64_t* p64;
    size_t i;
  } u;

  h0 = h3 = h6 = h9 = sky->a;
  h1 = h4 = h7 = h10 = sky->b;
  h2 = h5 = h8 = h11 = sc_const;

  u.p8 = data;
  end = u.p64 + (len / sc_block_size) * sc_num_vars;

  // handle all whole sc_block_size blocks of bytes
  if (ALLOW_UNALIGNED_READS || ((u.i & 0x7) == 0)) {

    while (u.p64 < end) {

      sky_mix(u.p64, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9,
                     &h10, &h11);
      u.p64 += sc_num_vars;
    }
  } else {

    while (u.p64 < end) {

      memcpy(buf, u.p64, sc_block_size);
      sky_mix(buf, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9,
                     &h10, &h11);

      u.p64 += sc_num_vars;
    }
  }

  // handle the last partial block of sc_block_size bytes
  remainder = (len - (size_t)((const uint8_t*)end - (const uint8_t*)data));

  memcpy(buf, end, remainder);
  memset(((uint8_t*)buf) + remainder, 0, sc_block_size - remainder);

  ((uint8_t*)buf)[sc_block_size - 1] = (uint8_t) remainder;

  // do some final mixing
  sky_end(buf, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
  sky->a = h0;
  sky->b = h1;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
