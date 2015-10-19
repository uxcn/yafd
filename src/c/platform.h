//===-- platform.h - platform identification ----------------------*- C -*-===//

#ifndef PLATFORM_H
#define PLATFORM_H

#include "config.h" // autoconf

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)  && !defined(BIG_ENDIAN)
#define BIG_ENDIAN
#endif

#if defined(__linux__)
#define HAVE_LINUX
#elif defined(__FreeBSD__)
#define HAVE_FREEBSD
#elif defined(__APPLE__)
#define HAVE_DARWIN
#elif defined(__WIN32__) || defined(__WIN64__)
#define HAVE_WINDOWS
#endif

#endif // PLATFORM_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
