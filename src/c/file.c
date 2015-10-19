//===-- file.c - file utility functions ---------------------------*- C -*-===//

#include "file.h"


#include "config.h" // autoconf


#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include <strings.h>

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif


#include "platform.h" // platform


#include "math.h"

int read_n(const size_t n, const int fs[n], const size_t c, uint8_t ds[n][c]) {

  size_t cs[n];
  size_t rs = c * n;

  memset(cs, 0, n * sizeof(size_t));

  do {

    for (size_t i = 0; i < n; i++) {

      ssize_t r = read(fs[i], &ds[i][cs[i]], (unsigned) (c - cs[i]));

      if (r < 0)
        return -1;

      cs[i] += (size_t) r;
      rs -= (size_t) r;
    }
  } while (rs);

  return 0;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
