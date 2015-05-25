//===-- file.c - file utility functions ---------------------------*- C -*-===//

#include "file.h"

#include <stddef.h>
#include <stdint.h>

#include <strings.h>

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h" // autoconf 

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "math.h"

int read_n(const size_t n, const int fs[n], const size_t c, uint8_t ds[n][c]) {

  size_t cs[n];
  size_t rs = c * n;

#ifdef HAVE_BZERO
  bzero(cs, n * sizeof(size_t));
#else
  memset(cs, 0, n * sizeof(size_t));
#endif

  do {

    for (size_t i = 0; i < n; i++) {

      ssize_t r = read(fs[i], &ds[i][cs[i]], c - cs[i]);

      if (r < 0)
        return -1;

      cs[i] += (size_t) r;
      rs -= (size_t) r;
    }
  } while (rs);

  return 0;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
