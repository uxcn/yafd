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
#include "file.h"
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
  const size_t bs = (size_t) opts.blocksize;
#endif

  const int fds[2] = {ea->fd, eb->fd};

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


    typedef uint8_t buffer_t[2][c];

    uint8_t db[2][c > pz ? 1 : c]; // zero length undefined
#ifdef HAVE_PTHREAD_H
    uint8_t* dg = c > pz ? thread_local_buffer(c << 1) : (uint8_t*) db;
#else
    uint8_t* dg = c > pz ? frealloc(bf, c << 1) : (uint8_t*) db;
#endif

    buffer_t* da = (buffer_t*) dg; // magic

    if (read_n(2, fds, c, *da))
      return -2;

    const int cmp = memcmp(&(*da)[0], &(*da)[1], c);

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
  const size_t bs = (size_t) opts.blocksize;
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
