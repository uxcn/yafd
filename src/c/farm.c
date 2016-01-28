//===-- farm.c - farmhash implementations -------------------------*- C -*-===//

#include "farm.h"

#include <string.h>

#if defined(__SSSE3__) || defined(__SSE4_1__)
#include <immintrin.h>
#endif

#if defined(__SSE4_2__)
#include <nmmintrin.h>
#endif

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

static inline uint64_t fetch64(const uint8_t* p) {
  uint64_t result;
  memcpy(&result, p, sizeof(result));

  return uint64_t_in_expected_order(result);
}

static inline uint32_t fetch32(const uint8_t* p) {
  uint32_t result;
  memcpy(&result, p, sizeof(result));

  return uint32_t_in_expected_order(result);
}

#if defined(__SSE4_1__) || defined(__SSE4_2__)

static inline __m128i fetch128(const uint8_t* s) {
  return _mm_loadu_si128((const __m128i*) s);
}

#endif

static inline void swap64(uint64_t* a, uint64_t* b) {
  uint64_t t;

  t = *a;
  *a = *b;
  *b = t;
}

#if defined(__SSE4_1__) || defined(__SSE4_2__)

static inline void swap128(__m128i* a, __m128i* b) {
  __m128i t;

  t = *a;
  *a = *b;
  *b = t;
}

#endif

// Helpers for data-parallel operations (1x 128 bits or 2x64 or 4x32 or 8x16).

#if defined(__SSE4_1__) || defined(__SSE4_2__)

static inline __m128i add64x2(__m128i x, __m128i y) { return _mm_add_epi64(x, y); }

static inline __m128i xor128(__m128i x, __m128i y) { return _mm_xor_si128(x, y); }

#endif

#if defined(__x86_64) && defined(__SSSE3__) && defined(__SSE4_1__)

static inline __m128i shuf8x16(__m128i x, __m128i y) { return _mm_shuffle_epi8(y, x); }

static inline __m128i mul32x4(__m128i x, __m128i y) { return _mm_mullo_epi32(x, y); }

#endif

// Building blocks for hash functions

// Some primes between 2^63 and 2^64 for various uses.
static const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
static const uint64_t k1 = 0xb492b66fbe98f273ULL;
static const uint64_t k2 = 0x9ae16a3b2f90404fULL;

static inline uint64_t smix(uint64_t val) {
  return val ^ (val >> 47);
}

static inline uint64_t frm128_to_64(uint128_t x) {
  // Murmur-inspired hashing.
  const uint64_t k_mul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (x.a ^ x.b) * k_mul;
  a ^= (a >> 47);
  uint64_t b = (x.b ^ a) * k_mul;
  b ^= (b >> 47);
  b *= k_mul;
  return b;
}

static inline uint64_t debug_tweak64(uint64_t x) {
#ifndef NDEBUG
    x = ~bswap64(x * k1);
#endif

  return x;
}

uint128_t debug_tweak128(uint128_t x) {
#ifndef NDEBUG
  uint64_t y = debug_tweak64(x.a);
  uint64_t z = debug_tweak64(x.b);
  y += z;
  z += y;
  x = (uint128_t) {y, z * k1};
#endif

  return x;
}

static inline uint64_t frm_len_16(uint64_t u, uint64_t v) {
  return frm128_to_64((uint128_t) {u, v});
}

static inline uint64_t frm_len_16_mul(uint64_t u, uint64_t v, uint64_t mul) {
  // Murmur-inspired hashing.
  uint64_t a = (u ^ v) * mul;
  a ^= (a >> 47);
  uint64_t b = (v ^ a) * mul;
  b ^= (b >> 47);
  b *= mul;
  return b;
}

// farmhash na

static inline uint64_t frm_na_len_0_to_16(size_t len, const uint8_t data[len]) {
  if (len >= 8) {
    uint64_t mul = k2 + len * 2;
    uint64_t a = fetch64(&data[0]) + k2;
    uint64_t b = fetch64(&data[len - 8]);
    uint64_t c = ror(b, 37) * mul + a;
    uint64_t d = (ror(a, 25) + b) * mul;
    return frm_len_16_mul(c, d, mul);
  }
  if (len >= 4) {
    uint64_t mul = k2 + len * 2;
    uint64_t a = fetch32(&data[0]);
    return frm_len_16_mul(len + (a << 3), fetch32(data + len - 4), mul);
  }
  if (len > 0) {
    uint8_t a = data[0];
    uint8_t b = data[len >> 1];
    uint8_t c = data[len - 1];
    uint32_t y = (uint32_t) a + ((uint32_t) b << 8);
    uint32_t z = (uint32_t) len + ((uint32_t) c << 2);
    return smix(y * k2 ^ z * k0) * k2;
  }
  return k2;
}

// This probably works well for 16-byte strings as well, but it may be overkill
// in that case.
static inline uint64_t frm_na_len_17_to_32(size_t len, const uint8_t data[len]) {
  uint64_t mul = k2 + len * 2;
  uint64_t a = fetch64(&data[0]) * k1;
  uint64_t b = fetch64(&data[8]);
  uint64_t c = fetch64(&data[len - 8]) * mul;
  uint64_t d = fetch64(&data[len - 16]) * k2;
  return frm_len_16_mul(ror(a + b, 43) + ror(c, 30) + d,
                   a + ror(b + k2, 18) + c, mul);
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
static inline uint128_t weak_frm_na_len_32_with_seeds_vals(
    uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b) {
  a += w;
  b = ror(b + a + z, 21);
  uint64_t c = a;
  a += x;
  a += y;
  b += ror(a, 44);
  return (uint128_t) {a + z, b + c};
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
static inline uint128_t weak_frm_na_len_32_with_seeds(
    const uint8_t data[32], uint64_t a, uint64_t b) {
  return weak_frm_na_len_32_with_seeds_vals(fetch64(&data[0]),
                                fetch64(&data[8]),
                                fetch64(&data[16]),
                                fetch64(&data[24]),
                                a,
                                b);
}

// Return an 8-byte hash for 33 to 64 bytes.
static inline uint64_t frm_na_len_33_to_64(size_t len, const uint8_t data[len]) {
  uint64_t mul = k2 + len * 2;
  uint64_t a = fetch64(&data[0]) * k2;
  uint64_t b = fetch64(&data[8]);
  uint64_t c = fetch64(&data[len - 8]) * mul;
  uint64_t d = fetch64(&data[len - 16]) * k2;
  uint64_t y = ror(a + b, 43) + ror(c, 30) + d;
  uint64_t z = frm_len_16_mul(y, a + ror(b + k2, 18) + c, mul);
  uint64_t e = fetch64(&data[16]) * mul;
  uint64_t f = fetch64(&data[24]);
  uint64_t g = (y + fetch64(&data[len - 32])) * mul;
  uint64_t h = (z + fetch64(&data[len - 24])) * mul;
  return frm_len_16_mul(ror(e + f, 43) + ror(g, 30) + h,
                   e + ror(f + a, 18) + g, mul);
}

uint64_t frm64_na(size_t len, const uint8_t data[len]) {
  const uint64_t seed = 81;
  if (len <= 32) {
    if (len <= 16) {
      return frm_na_len_0_to_16(len, data);
    } else {
      return frm_na_len_17_to_32(len, data);
    }
  } else if (len <= 64) {
    return frm_na_len_33_to_64(len, data);
  }

  // For strings over 64 bytes we loop.  Internal state consists of
  // 56 bytes: v, w, x, y, and z.
  uint64_t x = seed;
  uint64_t y = seed * k1 + 113;
  uint64_t z = smix(y * k2 + 113) * k2;
  uint128_t v = (uint128_t) {0, 0};
  uint128_t w = (uint128_t) {0, 0};
  x = x * k2 + fetch64(&data[0]);

  // Count bytes so that after the loop we have 1 to 64 bytes left to process.
  size_t bs = 0;
  do {
    x = ror(x + y + v.a + fetch64(&data[bs  + 8]), 37) * k1;
    y = ror(y + v.b + fetch64(&data[bs + 48]), 42) * k1;
    x ^= w.b;
    y += v.a + fetch64(&data[bs + 40]);
    z = ror(z + w.a, 33) * k1;
    v = weak_frm_na_len_32_with_seeds(&data[bs], v.b * k1, x + w.a);
    w = weak_frm_na_len_32_with_seeds(&data[bs + 32], z + w.b, y + fetch64(&data[bs + 16]));
    swap64(&z, &x);
    bs += 64;
  } while (bs < ((len - 1) >> 6) << 6);
  uint64_t mul = k1 + ((z & 0xff) << 1);
  w.a += ((len - 1) & 63);
  v.a += w.a;
  w.a += v.a;
  x = ror(x + y + v.a + fetch64(&data[(len - 64) + 8]), 37) * mul;
  y = ror(y + v.b + fetch64(&data[(len - 64) + 48]), 42) * mul;
  x ^= w.b * 9;
  y += v.a * 9 + fetch64(&data[(len - 64) + 40]);
  z = ror(z + w.a, 33) * mul;
  v = weak_frm_na_len_32_with_seeds(&data[len - 64], v.b * mul, x + w.a);
  w = weak_frm_na_len_32_with_seeds(&data[(len - 64) + 32], z + w.b, y + fetch64(&data[(len - 64) + 16]));
  swap64(&z, &x);
  return frm_len_16_mul(frm_len_16_mul(v.a, w.a, mul) + smix(y) * k0 + z,
                   frm_len_16_mul(v.b, w.b, mul) + x,
                   mul);
}

uint64_t frm64_na_with_seeds(size_t len, const uint8_t data[len], uint64_t seed0, uint64_t seed1) {
  return frm_len_16(frm64_na(len, data) - seed0, seed1);
}

uint64_t frm64_na_with_seed(size_t len, const uint8_t data[len], uint64_t seed) {
  return frm64_na_with_seeds(len, data, k2, seed);
}

// farmhash uo

static inline uint64_t frm_uo_h(uint64_t x, uint64_t y, uint64_t mul, unsigned r) {
  uint64_t a = (x ^ y) * mul;
  a ^= (a >> 47);
  uint64_t b = (y ^ a) * mul;
  return ror(b, r) * mul;
}

uint64_t frm64_uo_with_seeds(size_t len, const uint8_t data[len],
                         uint64_t seed0, uint64_t seed1) {
  if (len <= 64) {
    return frm64_na_with_seeds(len, data, seed0, seed1);
  }

  // For strings over 64 bytes we loop.  Internal state consists of
  // 64 bytes: u, v, w, x, y, and z.
  uint64_t x = seed0;
  uint64_t y = seed1 * k2 + 113;
  uint64_t z = smix(y * k2) * k2;
  uint128_t v = (uint128_t) {seed0, seed1};
  uint128_t w = (uint128_t) {0, 0};
  uint64_t u = x - z;
  x *= k2;
  uint64_t mul = k2 + (u & 0x82);

  // Count bytes so that after the loop we have 1 to 64 bytes left to process.
  size_t bs = 0;
  do {
    uint64_t a0 = fetch64(&data[bs]);
    uint64_t a1 = fetch64(&data[bs + 8]);
    uint64_t a2 = fetch64(&data[bs + 16]);
    uint64_t a3 = fetch64(&data[bs + 24]);
    uint64_t a4 = fetch64(&data[bs + 32]);
    uint64_t a5 = fetch64(&data[bs + 40]);
    uint64_t a6 = fetch64(&data[bs + 48]);
    uint64_t a7 = fetch64(&data[bs + 56]);
    x += a0 + a1;
    y += a2;
    z += a3;
    v.a += a4;
    v.b += a5 + a1;
    w.a += a6;
    w.b += a7;

    x = ror(x, 26);
    x *= 9;
    y = ror(y, 29);
    z *= mul;
    v.a = ror(v.a, 33);
    v.b = ror(v.b, 30);
    w.a ^= x;
    w.a *= 9;
    z = ror(z, 32);
    z += w.b;
    w.b += z;
    z *= 9;
    swap64(&u, &y);

    z += a0 + a6;
    v.a += a2;
    v.b += a3;
    w.a += a4;
    w.b += a5 + a6;
    x += a1;
    y += a7;

    y += v.a;
    v.a += x - y;
    v.b += w.a;
    w.a += v.b;
    w.b += x - y;
    x += w.b;
    w.b = ror(w.b, 34);
    swap64(&u, &z);
    bs += 64;
  } while (bs < ((len - 1) >> 6) << 6);
  u *= 9;
  v.b = ror(v.b, 28);
  v.a = ror(v.a, 20);
  w.a += ((len - 1) & 63);
  u += y;
  y += u;
  x = ror(y - x + v.a + fetch64(&data[(len - 64) + 8]), 37) * mul;
  y = ror(y ^ v.b ^ fetch64(&data[(len - 64) + 48]), 42) * mul;
  x ^= w.b * 9;
  y += v.a + fetch64(&data[(len - 64) + 40]);
  z = ror(z + w.a, 33) * mul;
  v = weak_frm_na_len_32_with_seeds(&data[len - 64], v.b * mul, x + w.a);
  w = weak_frm_na_len_32_with_seeds(&data[(len - 64) + 32], z + w.b, y + fetch64(&data[(len - 64) + 16]));
  return frm_uo_h(frm_len_16_mul(v.a + x, w.a ^ y, mul) + z - u,
           frm_uo_h(v.b + y, w.b + z, k2, 30) ^ x,
           k2,
           31);
}

uint64_t frm64_uo_with_seed(size_t len, const uint8_t data[len], uint64_t seed) {
  return len <= 64 ? frm64_na_with_seed(len, data, seed) :
      frm64_uo_with_seeds(len, data, 0, seed);
}

uint64_t frm64_uo(size_t len, const uint8_t data[len]) {
  return len <= 64 ? frm64_na(len, data) :
      frm64_uo_with_seeds(len, data, 81, 0);
}

// farmhash xo

static inline uint64_t frm_xo_h32(size_t len, const uint8_t data[len], uint64_t mul,
                           uint64_t seed0, uint64_t seed1) {
  uint64_t a = fetch64(&data[0]) * k1;
  uint64_t b = fetch64(&data[8]);
  uint64_t c = fetch64(&data[len - 8]) * mul;
  uint64_t d = fetch64(&data[len - 16]) * k2;
  uint64_t u = ror(a + b, 43) + ror(c, 30) + d + seed0;
  uint64_t v = a + ror(b + k2, 18) + c + seed1;
  a = smix((u ^ v) * mul);
  b = smix((v ^ a) * mul);
  return b;
}

// Return an 8-byte hash for 33 to 64 bytes.
static inline uint64_t frm_xo_len_33_to_64(size_t len, const uint8_t data[len]) {
  uint64_t mul0 = k2 - 30;
  uint64_t mul1 = k2 - 30 + 2 * len;
  uint64_t h0 = frm_xo_h32(32, &data[0], mul0, 0, 0);
  uint64_t h1 = frm_xo_h32(32, &data[len - 32], mul1, 0, 0);
  return ((h1 * mul1) + h0) * mul1;
}

// Return an 8-byte hash for 65 to 96 bytes.
static inline uint64_t frm_xo_len_65_to_96(size_t len, const uint8_t data[len]) {
  uint64_t mul0 = k2 - 114;
  uint64_t mul1 = k2 - 114 + 2 * len;
  uint64_t h0 = frm_xo_h32(32, &data[0], mul0, 0, 0);
  uint64_t h1 = frm_xo_h32(32, &data[32], mul1, 0, 0);
  uint64_t h2 = frm_xo_h32(32, &data[len - 32], mul1, h0, h1);
  return (h2 * 9 + (h0 >> 17) + (h1 >> 21)) * mul1;
}

uint64_t frm64_xo(size_t len, const uint8_t data[len]) {
  if (len <= 32) {
    if (len <= 16) {
      return frm_na_len_0_to_16(len, data);
    } else {
      return frm_na_len_17_to_32(len, data);
    }
  } else if (len <= 64) {
    return frm_xo_len_33_to_64(len, data);
  } else if (len <= 96) {
    return frm_xo_len_65_to_96(len, data);
  } else if (len <= 256) {
    return frm64_na(len, data);
  } else {
    return frm64_uo(len, data);
  }
}

uint64_t frm64_xo_with_seeds(size_t len, const uint8_t data[len], uint64_t seed0, uint64_t seed1) {
  return frm64_uo_with_seeds(len, data, seed0, seed1);
}

uint64_t frm64_xo_with_seed(size_t len, const uint8_t data[len], uint64_t seed) {
  return frm64_uo_with_seed(len, data, seed);
}

// farmhash te

#if defined(__x86_64) && defined(__SSSE3__) && defined(__SSE4_1__)

// Requires n >= 256.  Requires SSE4.1. Should be slightly faster if the
// compiler uses AVX instructions (e.g., use the -mavx flag with GCC).
static inline uint64_t frm64_te_long(size_t n, const uint8_t data[n],
                                  uint64_t seed0, uint64_t seed1) {
  const __m128i k_shuf =
      _mm_set_epi8(4, 11, 10, 5, 8, 15, 6, 9, 12, 2, 14, 13, 0, 7, 3, 1);
  const __m128i k_mult =
      _mm_set_epi8('\xbd', '\xd6', '\x33', '\x39', '\x45', '\x54', '\xfa', '\x03',
                   '\x34', '\x3e', '\x33', '\xed', '\xcc', '\x9e', '\x2d', '\x51');
  uint64_t seed2 = (seed0 + 113) * (seed1 + 9);
  uint64_t seed3 = (ror(seed0, 23) + 27) * (ror(seed1, 30) + 111);
  __m128i d0 = _mm_cvtsi64_si128((long long) seed0);
  __m128i d1 = _mm_cvtsi64_si128((long long) seed1);
  __m128i d2 = shuf8x16(k_shuf, d0);
  __m128i d3 = shuf8x16(k_shuf, d1);
  __m128i d4 = xor128(d0, d1);
  __m128i d5 = xor128(d1, d2);
  __m128i d6 = xor128(d2, d4);
  __m128i d7 = _mm_set1_epi32((int) (seed2 >> 32));
  __m128i d8 = mul32x4(k_mult, d2);
  __m128i d9 = _mm_set1_epi32((int) (seed3 >> 32));
  __m128i d10 = _mm_set1_epi32((int) seed3);
  __m128i d11 = add64x2(d2, _mm_set1_epi32((int) seed2));
  size_t bs = 0;
  do {
    __m128i z;
    z = fetch128(&data[bs]);
    d0 = add64x2(d0, z);
    d1 = shuf8x16(k_shuf, d1);
    d2 = xor128(d2, d0);
    d4 = xor128(d4, z);
    d4 = xor128(d4, d1);
    swap128(&d0, &d6);
    z = fetch128(&data[bs + 16]);
    d5 = add64x2(d5, z);
    d6 = shuf8x16(k_shuf, d6);
    d8 = shuf8x16(k_shuf, d8);
    d7 = xor128(d7, d5);
    d0 = xor128(d0, z);
    d0 = xor128(d0, d6);
    swap128(&d5, &d11);
    z = fetch128(&data[bs + 32]);
    d1 = add64x2(d1, z);
    d2 = shuf8x16(k_shuf, d2);
    d4 = shuf8x16(k_shuf, d4);
    d5 = xor128(d5, z);
    d5 = xor128(d5, d2);
    swap128(&d10, &d4);
    z = fetch128(&data[bs + 48]);
    d6 = add64x2(d6, z);
    d7 = shuf8x16(k_shuf, d7);
    d0 = shuf8x16(k_shuf, d0);
    d8 = xor128(d8, d6);
    d1 = xor128(d1, z);
    d1 = add64x2(d1, d7);
    z = fetch128(&data[bs + 64]);
    d2 = add64x2(d2, z);
    d5 = shuf8x16(k_shuf, d5);
    d4 = add64x2(d4, d2);
    d6 = xor128(d6, z);
    d6 = xor128(d6, d11);
    swap128(&d8, &d2);
    z = fetch128(&data[bs + 80]);
    d7 = xor128(d7, z);
    d8 = shuf8x16(k_shuf, d8);
    d1 = shuf8x16(k_shuf, d1);
    d0 = add64x2(d0, d7);
    d2 = add64x2(d2, z);
    d2 = add64x2(d2, d8);
    swap128(&d1, &d7);
    z = fetch128(&data[bs + 96]);
    d4 = shuf8x16(k_shuf, d4);
    d6 = shuf8x16(k_shuf, d6);
    d8 = mul32x4(k_mult, d8);
    d5 = xor128(d5, d11);
    d7 = xor128(d7, z);
    d7 = add64x2(d7, d4);
    swap128(&d6, &d0);
    z = fetch128(&data[bs + 112]);
    d8 = add64x2(d8, z);
    d0 = shuf8x16(k_shuf, d0);
    d2 = shuf8x16(k_shuf, d2);
    d1 = xor128(d1, d8);
    d10 = xor128(d10, z);
    d10 = xor128(d10, d0);
    swap128(&d11, &d5);
    z = fetch128(&data[bs + 128]);
    d4 = add64x2(d4, z);
    d5 = shuf8x16(k_shuf, d5);
    d7 = shuf8x16(k_shuf, d7);
    d6 = add64x2(d6, d4);
    d8 = xor128(d8, z);
    d8 = xor128(d8, d5);
    swap128(&d4, &d10);
    z = fetch128(&data[bs + 144]);
    d0 = add64x2(d0, z);
    d1 = shuf8x16(k_shuf, d1);
    d2 = add64x2(d2, d0);
    d4 = xor128(d4, z);
    d4 = xor128(d4, d1);
    z = fetch128(&data[bs + 160]);
    d5 = add64x2(d5, z);
    d6 = shuf8x16(k_shuf, d6);
    d8 = shuf8x16(k_shuf, d8);
    d7 = xor128(d7, d5);
    d0 = xor128(d0, z);
    d0 = xor128(d0, d6);
    swap128(&d2, &d8);
    z = fetch128(&data[bs + 176]);
    d1 = add64x2(d1, z);
    d2 = shuf8x16(k_shuf, d2);
    d4 = shuf8x16(k_shuf, d4);
    d5 = mul32x4(k_mult, d5);
    d5 = xor128(d5, z);
    d5 = xor128(d5, d2);
    swap128(&d7, &d1);
    z = fetch128(&data[bs + 192]);
    d6 = add64x2(d6, z);
    d7 = shuf8x16(k_shuf, d7);
    d0 = shuf8x16(k_shuf, d0);
    d8 = add64x2(d8, d6);
    d1 = xor128(d1, z);
    d1 = xor128(d1, d7);
    swap128(&d0, &d6);
    z = fetch128(&data[bs + 208]);
    d2 = add64x2(d2, z);
    d5 = shuf8x16(k_shuf, d5);
    d4 = xor128(d4, d2);
    d6 = xor128(d6, z);
    d6 = xor128(d6, d9);
    swap128(&d5, &d11);
    z = fetch128(&data[bs + 224]);
    d7 = add64x2(d7, z);
    d8 = shuf8x16(k_shuf, d8);
    d1 = shuf8x16(k_shuf, d1);
    d0 = xor128(d0, d7);
    d2 = xor128(d2, z);
    d2 = xor128(d2, d8);
    swap128(&d10, &d4);
    z = fetch128(&data[bs + 240]);
    d3 = add64x2(d3, z);
    d4 = shuf8x16(k_shuf, d4);
    d6 = shuf8x16(k_shuf, d6);
    d7 = mul32x4(k_mult, d7);
    d5 = add64x2(d5, d3);
    d7 = xor128(d7, z);
    d7 = xor128(d7, d4);
    swap128(&d3, &d9);
    bs += 256;
  } while (bs < (n >> 8) << 8);
  d6 = add64x2(mul32x4(k_mult, d6), _mm_cvtsi64_si128((long long) n));
  if (n % 256 != 0) {
    d7 = add64x2(_mm_shuffle_epi32(d8, (0 << 6) + (3 << 4) + (2 << 2) + (1 << 0)), d7);
    d8 = add64x2(mul32x4(k_mult, d8), _mm_cvtsi64_si128((long long) frm64_xo(n % 256, data)));
  }
  __m128i t[8];
  d0 = mul32x4(k_mult, shuf8x16(k_shuf, mul32x4(k_mult, d0)));
  d3 = mul32x4(k_mult, shuf8x16(k_shuf, mul32x4(k_mult, d3)));
  d9 = mul32x4(k_mult, shuf8x16(k_shuf, mul32x4(k_mult, d9)));
  d1 = mul32x4(k_mult, shuf8x16(k_shuf, mul32x4(k_mult, d1)));
  d0 = add64x2(d11, d0);
  d3 = xor128(d7, d3);
  d9 = add64x2(d8, d9);
  d1 = add64x2(d10, d1);
  d4 = add64x2(d3, d4);
  d5 = add64x2(d9, d5);
  d6 = xor128(d1, d6);
  d2 = add64x2(d0, d2);
  t[0] = d0;
  t[1] = d3;
  t[2] = d9;
  t[3] = d1;
  t[4] = d4;
  t[5] = d5;
  t[6] = d6;
  t[7] = d2;
  return frm64_xo(sizeof(t), (uint8_t*) t);
}

uint64_t frm64_te(size_t len, const uint8_t data[len]) {
  // Empirically, farmhash xo seems faster until length 512.
  return len >= 512 ? frm64_te_long(len, data, k2, k1) : frm64_xo(len, data);
}

#endif

// farmhash cc

// This file provides a 32-bit hash equivalent to cityhash32 (v1.1.1)
// and a 128-bit hash equivalent to cityhash128 (v1.1.1).  It also provides
// a seeded 32-bit hash function similar to cityhash32.

static inline uint64_t frm_cc_len_0_to_16(size_t len, const uint8_t data[len]) {
  if (len >= 8) {
    uint64_t mul = k2 + len * 2;
    uint64_t a = fetch64(&data[0]) + k2;
    uint64_t b = fetch64(&data[len - 8]);
    uint64_t c = ror(b, 37) * mul + a;
    uint64_t d = (ror(a, 25) + b) * mul;
    return frm_len_16_mul(c, d, mul);
  }
  if (len >= 4) {
    uint64_t mul = k2 + len * 2;
    uint64_t a = fetch32(&data[0]);
    return frm_len_16_mul(len + (a << 3), fetch32(&data[len - 4]), mul);
  }
  if (len > 0) {
    uint8_t a = data[0];
    uint8_t b = data[len >> 1];
    uint8_t c = data[len - 1];
    uint32_t y = ((uint32_t) a) + (((uint32_t) b) << 8);
    uint32_t z = (uint32_t) len + (((uint32_t) c) << 2);
    return smix(y * k2 ^ z * k0) * k2;
  }
  return k2;
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
static inline uint128_t weak_frm_cc_len_32_with_seeds_vals(
    uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b) {
  a += w;
  b = ror(b + a + z, 21);
  uint64_t c = a;
  a += x;
  a += y;
  b += ror(a, 44);
  return (uint128_t) {a + z, b + c};
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
static inline uint128_t weak_frm_cc_len_32_with_seeds(
    const uint8_t data[32], uint64_t a, uint64_t b) {
  return weak_frm_cc_len_32_with_seeds_vals(fetch64(&data[0]),
                                fetch64(&data[8]),
                                fetch64(&data[16]),
                                fetch64(&data[24]),
                                a,
                                b);
}



// A subroutine for cityhash128().  Returns a decent 128-bit hash for strings
// of any length representable in signed long.  Based on City and Murmur.
static inline uint128_t frm_cc_city_murmur(size_t len, const uint8_t data[len], uint128_t seed) {
  uint64_t a = seed.a;
  uint64_t b = seed.b;
  uint64_t c = 0;
  uint64_t d = 0;
  if (len <= 16) {  // len <= 16
    a = smix(a * k1) * k1;
    c = b * k1 + frm_cc_len_0_to_16(len, data);
    d = smix(a + (len >= 8 ? fetch64(&data[0]) : c));
  } else {  // len > 16
    c = frm_len_16(fetch64(&data[len - 8]) + k1, a);
    d = frm_len_16(b + len, c + fetch64(&data[len - 16]));
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
  a = frm_len_16(a, c);
  b = frm_len_16(d, b);
  return (uint128_t) {a ^ b, frm_len_16(b, a)};
}

uint128_t frm128_cc_city_with_seed(size_t len, const uint8_t data[len], uint128_t seed) {
  if (len < 128) {
    return frm_cc_city_murmur(len, data, seed);
  }

  // We expect len >= 128 to be the common case.  Keep 56 bytes of state:
  // v, w, x, y, and z.
  uint128_t v, w;
  uint64_t x = seed.a;
  uint64_t y = seed.b;
  uint64_t z = len * k1;
  v.a = ror(y ^ k1, 49) * k1 + fetch64(&data[0]);
  v.b = ror(v.a, 42) * k1 + fetch64(&data[8]);
  w.a = ror(y + z, 35) * k1 + x;
  w.b = ror(x + fetch64(&data[88]), 53) * k1;

  // This is the same inner loop as cityhash64(), manually unrolled.
  size_t bs = 0;
  do {
    x = ror(x + y + v.a + fetch64(&data[bs + 8]), 37) * k1;
    y = ror(y + v.b + fetch64(&data[bs + 48]), 42) * k1;
    x ^= w.b;
    y += v.a + fetch64(&data[bs + 40]);
    z = ror(z + w.a, 33) * k1;
    v = weak_frm_cc_len_32_with_seeds(&data[bs], v.b * k1, x + w.a);
    w = weak_frm_cc_len_32_with_seeds(&data[bs + 32], z + w.b, y + fetch64(&data[16]));
    swap64(&z, &x);
    bs += 64;
    x = ror(x + y + v.a + fetch64(&data[bs + 8]), 37) * k1;
    y = ror(y + v.b + fetch64(&data[bs + 48]), 42) * k1;
    x ^= w.b;
    y += v.a + fetch64(&data[bs + 40]);
    z = ror(z + w.a, 33) * k1;
    v = weak_frm_cc_len_32_with_seeds(&data[bs], v.b * k1, x + w.a);
    w = weak_frm_cc_len_32_with_seeds(&data[bs + 32], z + w.b, y + fetch64(&data[16]));
    swap64(&z, &x);
    bs += 64;
    len -= 128;
  } while (likely(len >= 128));
  x += ror(v.a + z, 49) * k0;
  y = y * k0 + ror(w.b, 37);
  z = z * k0 + ror(w.a, 27);
  w.a *= 9;
  v.a *= k0;
  // If 0 < len < 128, hash up to 4 chunks of 32 bytes each from the end of s.
  for (size_t tail_done = 0; tail_done < len; ) {
    tail_done += 32;
    y = ror(x + y, 42) * k0 + v.b;
    w.a += fetch64(&data[bs + len - tail_done + 16]);
    x = x * k0 + w.a;
    z += w.b + fetch64(&data[bs + len - tail_done]);
    w.b += v.a;
    v = weak_frm_cc_len_32_with_seeds(&data[bs + len - tail_done], v.a + z, v.b);
    v.a *= k0;
  }
  // At this point our 56 bytes of state should contain more than
  // enough information for a strong 128-bit hash.  We use two
  // different 56-byte-to-8-byte hashes to get a 16-byte final result.
  x = frm_len_16(x, v.a);
  y = frm_len_16(y + z, w.a);
  return (uint128_t) {frm_len_16(x + v.b, w.b) + y,
                   frm_len_16(x + w.b, y + v.b)};
}

static inline uint128_t frm128_cc_city(size_t len, const uint8_t data[len]) {
  return len >= 16 ?
      frm128_cc_city_with_seed(len - 16, &data[16],
                          (uint128_t) {fetch64(&data[0]), fetch64(&data[8]) + k0}) :
      frm128_cc_city_with_seed(len, data, (uint128_t) {k0, k1});
}

void frm64(size_t len, const uint8_t data[len], uint64_t* frm) {
  *frm = debug_tweak64(
#if defined(__SSE4_2__) && defined(__x86_64)
      frm64_te(len, data)
#else
      frm64_xo(len, data)
#endif
  );
}

void frm128(size_t len, const uint8_t data[len], uint128_t* frm) {
  *frm = debug_tweak128(frm128_cc_city(len, data));
}
