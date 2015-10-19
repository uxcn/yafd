//===-- file.h - file utility functions ---------------------------*- C -*-===//

#ifndef FILE_H
#define FILE_H


#include "config.h" // autoconf

#include <stddef.h>
#include <sys/types.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif


#include "platform.h" // platform

int read_n(const size_t n, const int fs[n], const size_t c, uint8_t ds[n][c]);

#endif // FILE_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
