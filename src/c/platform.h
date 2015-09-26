//===-- platform.h - platform identification ----------------------*- C -*-===//

#ifndef PLATFORM_H
#define PLATFORM_H

#include "platform.h" // platform

#if defined(__linux__)
#define HAVE_LINUX
#elif defined(__FreeBSD__)
#define HAVE_FREEBSD
#elif defined(__APPLE__)
#define HAVE_DARWIN
#endif

#endif // PLATFORM_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
