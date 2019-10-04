//===-- duplicate.c - duplicate functions -------------------------*- C -*-===//

#include "duplicate.h"

#include <assert.h>

#include "config.h" // autoconf


#include <stddef.h>
#include <fcntl.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif


#include "platform.h" // platform


#include "thread.h"

#include "device.h"
#include "entry.h"

#include "math.h"
#include "options.h"

int duplicate_content_compare(const void* const k, const void* const v) {

  const struct duplicate* const dv = (const struct duplicate*) v;
  const struct device* const dvv =  vector_head(&dv->devices);

  const struct entry* const ea = (const struct entry* const) k;
  const struct entry* const eb = (const struct entry* const) vector_head(&dvv->entries);

  assert(ea->size == eb->size);
  assert(ea->valid && eb->valid);

#ifdef HAVE_STAT_BLKSIZE
  const size_t bs = (size_t) min(ea->bs, eb->bs);
#else
  const size_t bs = (size_t) 4096;
#endif

  size_t rs = (size_t) ea->size;

#ifdef HAVE_POSIX_FADVISE
        if (posix_fadvise(ea->fd, 0, (off_t) rs, POSIX_FADV_SEQUENTIAL))
          print_error("unable to file advise compare %s", entry_alias(ea));

        if (posix_fadvise(eb->fd, 0, (off_t) rs, POSIX_FADV_SEQUENTIAL))
          print_error("unable to file advise compare %s", entry_alias(eb));
#endif

  if (lseek(ea->fd, 0, SEEK_SET) < 0) {

    print_error("unable to seek %s", entry_alias(ea));
    return -2;
  }

  if (lseek(eb->fd, 0, SEEK_SET) < 0) {

    print_error("unable to seek %s", entry_alias(eb));
    return -2;
  }

  do {

    const size_t c = min(rs, bs);

    uint8_t* dga[c];
    uint8_t* dgb[c];

    size_t abs = 0;
    size_t bbs = 0;

    ssize_t r = 0;
    size_t cs = c << 1;

    do {

      r = read(ea->fd, &dga[abs], (unsigned) (c - abs));

      if (r < 0)
        return -2;

      abs += (size_t) r;
      cs -= (size_t) r;

      r = read(eb->fd, &dgb[bbs], (unsigned) (c - bbs));

      if (r < 0)
        return -2;

      bbs += (size_t) r;
      cs -= (size_t) r;
    } while (cs);

    const int cmp = memcmp(dga, dgb, c);

    if (cmp)
      return cmp > 0 ? 1 : -1;

    rs -= c;
  } while (rs);

  return 0;
}

#ifdef HAVE_MMAP

int duplicate_content_compare_mmap(const void* const k, const void* const v) {

  const struct duplicate* const dv = (const struct duplicate*) v;
  const struct device* const dvv =  vector_head(&dv->devices);

  const struct entry* const ea = (const struct entry* const) k;
  const struct entry* const eb = (const struct entry* const) vector_head(&dvv->entries);

  assert(ea->size == eb->size);
  assert(ea->valid && eb->valid);

#ifdef HAVE_STAT_BLKSIZE
  const size_t bs = (size_t) min(ea->bs, eb->bs);
#else
  const size_t bs = (size_t) 4096;
#endif

#ifdef HAVE_POSIX_MADVISE
        if (posix_madvise(ea->mm, (size_t) ea->size, POSIX_MADV_WILLNEED))
          print_error("unable to memory advise compare %s", entry_alias(ea));

        if (posix_madvise(eb->mm, (size_t) eb->size, POSIX_MADV_WILLNEED))
          print_error("unable to memory advise compare %s", entry_alias(eb));
#endif

  size_t rs = (size_t) ea->size;

  const void* ap = ea->mm;
  const void* bp = eb->mm;

  do {

    const size_t c = min(rs, bs);

    const int cmp = memcmp(ap, bp, c);

    ap = (const void*) ((size_t) ap + c);
    bp = (const void*) ((size_t) bp + c);

    if (cmp)
      return cmp > 0 ? 1 : -1;

    rs -= c;
  } while (rs);

  return 0;
}

#endif // HAVE_MMAP

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
