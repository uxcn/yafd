//===-- city.c - cityhash implementations -------------------------*- C -*-===// 

#include <assert.h>
#include <string.h>

#include "city.h"

#include "config.h" // autoconf

#include "platform.h" // platform


#include "math.h"

#define likely(x) (__builtin_expect(!!(x), 1))

#ifdef LITTLE_ENDIAN
#define uint32_t_in_expected_order(x) (x)
#define uint64_t_in_expected_order(x) (x)
#else
#define uint32_t_in_expected_order(x) (bswap32(x))
#define uint64_t_in_expected_order(x) (bswap64(x))
#endif

#define PERMUTE3_64(a, b, c)                                                   \
  do {                                                                         \
    swap64(a, b);                                                              \
    swap64(a, c);                                                              \
  } while (0)

// some primes between 2^63 and 2^64 for various uses
const uint64_t k0 = 0xc3a5c85c97cb3127;
const uint64_t k1 = 0xb492b66fbe98f273;
const uint64_t k2 = 0x9ae16a3b2f90404f;

static inline uint64_t uload64(const uint8_t* p) {
  uint64_t result;

  memcpy(&result, p, sizeof(result));

  return result;
}

static inline uint32_t uload32(const uint8_t* p) {
  uint32_t result;

  memcpy(&result, p, sizeof(result));

  return result;
}

static inline uint64_t fetch64(const uint8_t* p) {
  return uint64_t_in_expected_order(uload64(p));
}

static inline uint32_t fetch32(const uint8_t* p) {
  return uint32_t_in_expected_order(uload32(p));
}

#ifndef LITTLE_ENDIAN
static inline uint32_t bswap32(const uint32_t x) {

  uint32_t y = x;

  for (size_t i = 0; i<sizeof(uint32_t)>> 1; i++) {

    size_t d = sizeof(uint32_t) - i - 1;

    uint32_t mh = ((uint32_t)0xff) << (d << 3);
    uint32_t ml = ((uint32_t)0xff) << (i << 3);

    uint32_t h = x & mh;
    uint32_t l = x & ml;

    uint32_t t = (l << ((d - i) << 3)) | (h >> ((d - i) << 3));

    y = t | (y & ~(mh | ml));
  }

  return y;
}
#endif

static inline uint64_t bswap64(const uint64_t x) {

  uint64_t y = x;

  for (size_t i = 0; i<sizeof(uint64_t)>> 1; i++) {

    size_t d = sizeof(uint64_t) - i - 1;

    uint64_t mh = ((uint64_t)0xff) << (d << 3);
    uint64_t ml = ((uint64_t)0xff) << (i << 3);

    uint64_t h = x & mh;
    uint64_t l = x & ml;

    uint64_t t = (l << ((d - i) << 3)) | (h >> ((d - i) << 3));

    y = t | (y & ~(mh | ml));
  }

  return y;
}

static inline void swap64(uint64_t* a, uint64_t* b) {
  uint64_t t;

  t = *a;
  *a = *b;
  *b = t;
}

static uint64_t smix(uint64_t val) { return val ^ (val >> 47); }

static inline uint64_t hash_128_to_64(const uint128_t x) {

  // murmur-inspired hashing.
  const uint64_t kmul = 0x9ddfea08eb382d69; // 11376068507788127593

  uint64_t a, b;

  a = (x.a ^ x.b) * kmul;
  a ^= (a >> 47);

  b = (x.b ^ a) * kmul;
  b ^= (b >> 47);

  b *= kmul;

  return b;
}

static inline uint64_t hash_16(uint64_t u, uint64_t v) {

  uint128_t result = {u, v};
  return hash_128_to_64(result);
}

static inline uint64_t hash_mur_16(uint64_t u, uint64_t v, uint64_t mul) {
  // murmur-inspired hashing

  uint64_t a = (u ^ v) * mul;
  a ^= (a >> 47);
  uint64_t b = (v ^ a) * mul;
  b ^= (b >> 47);
  b *= mul;

  return b;
}

static uint64_t hash_0_to_16(const size_t len, const uint8_t data[len]) {

  if (len >= 8) {

    uint64_t mul = k2 + len * 2;
    uint64_t a = fetch64(&data[0]) + k2;
    uint64_t b = fetch64(&data[len - 8]);
    uint64_t c = ror(b, 37) * mul + a;
    uint64_t d = (ror(a, 25) + b) * mul;

    return hash_mur_16(c, d, mul);
  }

  if (len >= 4) {

    uint64_t mul = k2 + len * 2;
    uint64_t a = fetch32(&data[0]);

    return hash_mur_16(len + (a << 3), fetch32(&data[len - 4]), mul);
  }

  if (len > 0) {

    uint8_t a = data[0];
    uint8_t b = data[len >> 1];
    uint8_t c = data[len - 1];
    uint32_t y = ((uint32_t)a) + (((uint32_t)b) << 8);
    uint32_t z = ((uint32_t)len) + (((uint32_t)c) << 2);

    return smix(y * k2 ^ z * k0) * k2;
  }

  return k2;
}

// this probably works well for 16-byte strings as well, but it may be overkill
// in that case
static uint64_t hash_17_to_32(const size_t len, const uint8_t data[len]) {

  uint64_t mul = k2 + len * 2;
  uint64_t a = fetch64(&data[0]) * k1;
  uint64_t b = fetch64(&data[8]);
  uint64_t c = fetch64(&data[len - 8]) * mul;
  uint64_t d = fetch64(&data[len - 16]) * k2;

  return hash_mur_16(ror(a + b, 43) + ror(c, 30) + d,
                     a + ror(b + k2, 18) + c, mul);
}

// return a 16-byte hash for 48 bytes, quick and dirty
// callers do best to use "random-looking" values for a and b
static uint128_t weak_hash_32_with_seeds(uint64_t w, uint64_t x, uint64_t y,
                                         uint64_t z, uint64_t a, uint64_t b) {

  a += w;
  b = ror(b + a + z, 21);
  uint64_t c = a;
  a += x;
  a += y;
  b += ror(a, 44);

  uint128_t result = {a + z, b + c};

  return result;
}

// return a 16-byte hash for s[0] ... s[31], a, and b, quick and dirty
static uint128_t weak_hash_32_with_seeds_raw(const uint8_t data[32], uint64_t a,
                                             uint64_t b) {

  return weak_hash_32_with_seeds(fetch64(&data[0]), fetch64(&data[8]), fetch64(&data[16]),
                                 fetch64(&data[24]), a, b);
}

// return an 8-byte hash for 33 to 64 bytes
static uint64_t hash_33_to_64(const size_t len, const uint8_t data[len]) {

  uint64_t mul = k2 + len * 2;
  uint64_t a = fetch64(&data[0]) * k2;
  uint64_t b = fetch64(&data[8]);
  uint64_t c = fetch64(&data[len - 24]);
  uint64_t d = fetch64(&data[len - 32]);
  uint64_t e = fetch64(&data[16]) * k2;
  uint64_t f = fetch64(&data[24]) * 9;
  uint64_t g = fetch64(&data[len - 8]);
  uint64_t h = fetch64(&data[len - 16]) * mul;
  uint64_t u = ror(a + g, 43) + (ror(b, 30) + c) * 9;
  uint64_t v = ((a + g) ^ d) + f + 1;
  uint64_t w = bswap64((u + v) * mul) + h;
  uint64_t x = ror(e + f, 42) + c;
  uint64_t y = (bswap64((v + w) * mul) + g) * mul;
  uint64_t z = e + f + c;

  a = bswap64((x + z) * mul + y) + b;
  b = smix((z + a) * mul + d + h) * mul;

  return b + x;
}

void cty64(size_t len, const uint8_t data[len], uint64_t* cty) {

  if (len <= 32) {

    if (len <= 16) {

      *cty = hash_0_to_16(len, data);
      return;
    } else {

      *cty = hash_17_to_32(len, data);
      return;
    }
  } else if (len <= 64) {

    *cty = hash_33_to_64(len, data);
    return;
  }

  // for strings over 64 bytes we hash the end first, and then as we
  // loop we keep 56 bytes of state: v, w, x, y, and z
  uint64_t x = fetch64(&data[len - 40]);
  uint64_t y = fetch64(&data[len - 16]) + fetch64(&data[len - 56]);
  uint64_t z = hash_16(fetch64(&data[len - 48]) + len, fetch64(&data[len - 24]));
  uint128_t v = weak_hash_32_with_seeds_raw(&data[len - 64], len, z);
  uint128_t w = weak_hash_32_with_seeds_raw(&data[len - 32], y + k1, x);

  x = x * k1 + fetch64(&data[0]);

  // decrease len to the nearest multiple of 64, and operate on 64-byte chunks
  len = (len - 1) & ~((size_t)63);
  size_t bs = 0;

  do {

    x = ror(x + y + v.a + fetch64(&data[bs + 8]), 37) * k1;
    y = ror(y + v.b + fetch64(&data[bs + 48]), 42) * k1;
    x ^= w.b;
    y += v.a + fetch64(&data[bs + 40]);
    z = ror(z + w.a, 33) * k1;
    v = weak_hash_32_with_seeds_raw(&data[bs], v.b * k1, x + w.a);
    w = weak_hash_32_with_seeds_raw(&data[bs + 32], z + w.b, y + fetch64(&data[bs + 16]));
    swap64(&z, &x);
    bs += 64;
    len -= 64;
  } while (len != 0);

  *cty = hash_16(hash_16(v.a, w.a) + smix(y) * k1 + z, hash_16(v.b, w.b) + x);
}

static inline uint64_t cty64_with_seeds(const size_t len, const uint8_t data[len], uint64_t seed0,
                               uint64_t seed1) {
  uint64_t cty;

  cty64(len, data, &cty);
  return hash_16(cty - seed0, seed1);
}

uint64_t cty64_with_seed(const size_t len, const uint8_t data[len], uint64_t seed) {
  return cty64_with_seeds(len, data, k2, seed);
}

// a subroutine for cty128(), returns a decent 128-bit hash for strings
// of any length representable in signed long, based on city and murmur
static uint128_t cty_murmur(size_t len, const uint8_t data[len], uint128_t seed) {

  uint64_t a = seed.a;
  uint64_t b = seed.b;
  uint64_t c = 0;
  uint64_t d = 0;

  if (len <= 16) { // len <= 16

    a = smix(a * k1) * k1;
    c = b * k1 + hash_0_to_16(len, data);
    d = smix(a + (len >= 8 ? fetch64(&data[0]) : c));
  } else { // len > 16

    c = hash_16(fetch64(&data[len - 8]) + k1, a);
    d = hash_16(b + len, c + fetch64(&data[len - 16]));
    a += d;

    size_t bs = 0;

    do {

      a ^= smix(fetch64(&data[bs]) * k1) * k1;
      a *= k1;
      b ^= a;
      c ^= smix(fetch64(&data[bs + 8]) * k1) * k1;
      c *= k1;
      d ^= c;
      bs += 16;
      len -= 16;
    } while (len > 16);
  }

  a = hash_16(a, c);
  b = hash_16(d, b);

  uint128_t result = {a ^ b, hash_16(b, a)};

  return result;
}

uint128_t cty128_with_seed(size_t len, const uint8_t data[len], uint128_t seed) {

  if (len < 128) {
    return cty_murmur(len, data, seed);
  }

  // we expect len >= 128 to be the common case, keep 56 bytes of state:
  // v, w, x, y, and z
  uint128_t v, w;
  uint64_t x = seed.a;
  uint64_t y = seed.b;
  uint64_t z = len * k1;

  v.a = ror(y ^ k1, 49) * k1 + fetch64(&data[0]);
  v.b = ror(v.a, 42) * k1 + fetch64(&data[8]);
  w.a = ror(y + z, 35) * k1 + x;
  w.b = ror(x + fetch64(&data[88]), 53) * k1;

  size_t bs = 0;

  // this is the same inner loop as cty64(), manually unrolled
  do {

    x = ror(x + y + v.a + fetch64(&data[bs + 8]), 37) * k1;
    y = ror(y + v.b + fetch64(&data[bs + 48]), 42) * k1;
    x ^= w.b;
    y += v.a + fetch64(&data[bs + 40]);
    z = ror(z + w.a, 33) * k1;
    v = weak_hash_32_with_seeds_raw(&data[bs], v.b * k1, x + w.a);
    w = weak_hash_32_with_seeds_raw(&data[bs + 32], z + w.b, y + fetch64(&data[bs + 16]));
    swap64(&z, &x);
    bs += 64;
    x = ror(x + y + v.a + fetch64(&data[bs + 8]), 37) * k1;
    y = ror(y + v.b + fetch64(&data[bs + 48]), 42) * k1;
    x ^= w.b;
    y += v.a + fetch64(&data[bs + 40]);
    z = ror(z + w.a, 33) * k1;
    v = weak_hash_32_with_seeds_raw(&data[bs], v.b * k1, x + w.a);
    w = weak_hash_32_with_seeds_raw(&data[bs + 32], z + w.b, y + fetch64(&data[bs + 16]));
    swap64(&z, &x);
    bs += 64;
    len -= 128;
  } while (likely(len >= 128));

  x += ror(v.a + z, 49) * k0;
  y = y * k0 + ror(w.b, 37);
  z = z * k0 + ror(w.a, 27);
  w.a *= 9;
  v.a *= k0;

  // if 0 < len < 128, hash up to 4 chunks of 32 bytes each from the end of s
  for (size_t tail_done = 0; tail_done < len;) {

    tail_done += 32;
    y = ror(x + y, 42) * k0 + v.b;
    w.a += fetch64(&data[bs + len - tail_done + 16]);
    x = x * k0 + w.a;
    z += w.b + fetch64(&data[bs + len - tail_done]);
    w.b += v.a;
    v = weak_hash_32_with_seeds_raw(&data[bs + len - tail_done], v.a + z, v.b);
    v.a *= k0;
  }

  // at this point our 56 bytes of state should contain more than
  // enough information for a strong 128-bit hash, we use two
  // different 56-byte-to-8-byte hashes to get a 16-byte final result
  x = hash_16(x, v.a);
  y = hash_16(y + z, w.a);

  uint128_t result = {hash_16(x + v.b, w.b) + y, hash_16(x + w.b, y + v.b)};

  return result;
}

void cty128(const size_t len, const uint8_t data[len], uint128_t* cty) {

  if (len >= 16) {

    uint128_t seed = {fetch64(&data[0]), fetch64(&data[8]) + k0};
    *cty = cty128_with_seed(len - 16, &data[16], seed);
  } else {

    uint128_t seed = {k0, k1};
    *cty = cty128_with_seed(len, data, seed);
  }
}

// conditionally include declarations for versions of City that require SSE4.2
// instructions to be available
#if defined(__SSE4_2__) && defined(__x86_64)

#include <smmintrin.h>

// requires len >= 240
static uint256_t cty256_crc_long(size_t len, const uint8_t data[len],
                                      uint32_t seed) {

  uint256_t result;

  uint64_t a = fetch64(&data[56]) + k0;
  uint64_t b = fetch64(&data[96]) + k0;
  uint64_t c = result.a = hash_16(b, len);
  uint64_t d = result.b = fetch64(&data[120]) * k0 + len;
  uint64_t e = fetch64(&data[184]) + seed;
  uint64_t f = 0;
  uint64_t g = 0;
  uint64_t h = c + d;
  uint64_t x = seed;
  uint64_t y = 0;
  uint64_t z = 0;

  // 240 bytes of input per iter
  size_t iters = len / 240;
  len -= iters * 240;

  size_t bs = 0;

  do {
#define CHUNK(r)                                                               \
  PERMUTE3_64(&x, &z, &y);                                                     \
  b += fetch64(&data[bs]);                                                     \
  c += fetch64(&data[bs + 8]);                                                 \
  d += fetch64(&data[bs + 16]);                                                \
  e += fetch64(&data[bs + 24]);                                                \
  f += fetch64(&data[bs + 32]);                                                \
  a += b;                                                                      \
  h += f;                                                                      \
  b += c;                                                                      \
  f += d;                                                                      \
  g += e;                                                                      \
  e += z;                                                                      \
  g += x;                                                                      \
  z = _mm_crc32_u64(z, b + g);                                                 \
  y = _mm_crc32_u64(y, e + h);                                                 \
  x = _mm_crc32_u64(x, f + a);                                                 \
  e = r == 0 ? e : ror(e, r);                                                  \
  c += e;                                                                      \
  bs += 40

    CHUNK(0);
    PERMUTE3_64(&a, &h, &c);
    CHUNK(33);
    PERMUTE3_64(&a, &h, &f);
    CHUNK(0);
    PERMUTE3_64(&b, &h, &f);
    CHUNK(42);
    PERMUTE3_64(&b, &h, &d);
    CHUNK(0);
    PERMUTE3_64(&b, &h, &e);
    CHUNK(33);
    PERMUTE3_64(&a, &h, &e);
  } while (--iters > 0);

  while (len >= 40) {

    CHUNK(29);
    e ^= ror(a, 20);
    h += ror(b, 30);
    g ^= ror(c, 40);
    f += ror(d, 34);
    PERMUTE3_64(&c, &h, &g);
    len -= 40;
  }

  if (len > 0) {
    bs = bs + len - 40;
    CHUNK(33);
    e ^= ror(a, 43);
    h += ror(b, 42);
    g ^= ror(c, 41);
    f += ror(d, 40);
  }

  result.a ^= h;
  result.b ^= g;
  g += h;
  a = hash_16(a, g + z);
  x += y << 32;
  b += x;
  c = hash_16(c, z) + h;
  d = hash_16(d, e + result.a);
  g += e;
  h += hash_16(x, f);
  e = hash_16(a, d) + g;
  z = hash_16(b, c) + a;
  y = hash_16(g, h) + c;
  result.a = e + z + y + x;
  a = smix((a + y) * k0) * k0 + b;
  result.b += a + result.a;
  a = smix(a * k0) * k0 + c;
  result.c = a + result.b;
  a = smix((a + e) * k0) * k0;
  result.d = a + result.c;

  return result;
}

// requires len < 240
static uint256_t cty256_crc_short(const size_t len, const uint8_t data[len]) {

  uint8_t buf[240];

  memcpy(buf, data, len);
  memset(buf + len, 0, 240 - len);

  return cty256_crc_long(240, buf, ~((uint32_t)len));
}

void cty256_crc(const size_t len, const uint8_t data[len], uint256_t* cty) {

  if (likely(len >= 240)) {
    *cty = cty256_crc_long(len, data, 0);
  } else {
    *cty = cty256_crc_short(len, data);
  }
}

uint128_t cty128_crc_with_seed(const size_t len, const uint8_t data[len],
                                    uint128_t seed) {

  if (len <= 900) {

    return cty128_with_seed(len, data, seed);

  } else {

    uint256_t hash;
    cty256_crc(len, data, &hash);

    uint64_t u = seed.b + hash.a;
    uint64_t v = seed.a + hash.b;

    uint128_t result = {hash_16(u, v + hash.c),
                        hash_16(ror(v, 32), u * k0 + hash.d)};
    return result;
  }
}

void cty128_crc(const size_t len, const uint8_t data[len], uint128_t* cty) {

  if (len <= 900) {

    cty128(len, data, cty);
  } else {

    uint256_t hash;
    cty256_crc(len, data, &hash);

    cty->a = hash.c;
    cty->b = hash.d;
  }
}

#endif
// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
