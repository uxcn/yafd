//===-- entry.h - entry structure and functions -------------------*- C -*-===//

#ifndef ENTRY_H
#define ENTRY_H

#include <assert.h>

#include "config.h" // autoconf


#include <stddef.h>
#include <stdio.h>

#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>


#include "platform.h" // platform


#include "memory.h"

#include "vector.h"

#ifdef HAVE_WINDOWS
#include "mingw.h"
#endif


static const size_t default_init_aliases = 2;

struct entry {

  dev_t device;
  ino_t inode;
  off_t size;

  uid_t uid;
  gid_t gid;
  mode_t mode;

#ifdef HAVE_STAT_BLKSIZE
  blksize_t bs;
#endif

  time_t mtime_sec;
  time_t mtime_nsec;

  bool valid;

  int fd;
#ifdef HAVE_MMAP
  void* mm;
#endif

  struct vector aliases;
};

static inline void to_entry(const ino_t i, const struct stat* const s,
                            struct entry* const e) {

  e->inode = (ino_t) i;

  e->device = (dev_t) s->st_dev;
  e->size = (off_t) s->st_size;
  e->uid = (uid_t) s->st_uid;
  e->gid = (gid_t) s->st_gid;
  e->mode = (mode_t) s->st_mode;

#ifdef HAVE_STAT_BLKSIZE
  e->bs = (blksize_t)  s->st_blksize;
#endif

#if defined(HAVE_DARWIN)
  e->mtime_sec = (time_t) s->st_mtimespec.tv_sec;
  e->mtime_nsec = (time_t) s->st_mtimespec.tv_nsec;
#elif defined(HAVE_STAT_MTIM)
  e->mtime_sec = (time_t) s->st_mtim.tv_sec;
  e->mtime_nsec = (time_t) s->st_mtim.tv_nsec;
#else
  e->mtime_sec = (time_t) s->st_mtime;
  e->mtime_nsec = (time_t) 0;
#endif

  e->valid = true;
}

static inline int entry_compare(const void* const k, const void* const v) {
  const ino_t* const ek = (const ino_t* const) k;
  const struct entry* const ev = (const struct entry* const) v;

  return (*ek > ev->inode) - (*ek < ev->inode);
}

static inline void entry_init(struct entry* const e) {

  assert(e != NULL);

  vector_init(&e->aliases, default_init_aliases);
}

static inline void entry_alias_add(struct entry* const e, const char* const f) {

  vector_push(&e->aliases, f);
}

static inline struct entry* entry_create(const ino_t i, const struct stat* const s, const char* const a) {

  struct entry* const e = fmalloc(sizeof(struct entry));

  entry_init(e);

  to_entry(i, s, e);

  entry_alias_add(e, a);

  return e;
}

static inline void entry_destroy(struct entry* const e) {

  char* a;

  assert(e != NULL);


  vector_for_each(a, &e->aliases)
    free(a);

  vector_destroy(&e->aliases);

  free(e);
}

static inline char* entry_alias(const struct entry* const e) {

  return vector_head(&e->aliases);
}

static inline int is_size_modified(const struct entry* const e, bool* b) {
      
  const char* const n = entry_alias(e);
  struct stat s;
      
  if (lstat(n, &s))
    return -1;

  *b = s.st_size != e->size;

  return 0;
}

static inline bool entry_handle_size_modify(struct entry* const e) {

    bool m;

    if (is_size_modified(e, &m)) {

      print_error("unable to determine if file size modified %s", entry_alias(e));

      entry_destroy(e);
      return true;
    }

    if (m) {

      print_error("file size modified %s", entry_alias(e));

      entry_destroy(e);
      return true;
    }

    return false;
}

static inline int is_modified(const struct entry* const e, bool* b) {
      
  const char* const n = entry_alias(e);
  struct stat s;
      
  if (lstat(n, &s))
    return -1;

#if defined(HAVE_DARWIN)
  time_t mtime_sec = (time_t) s.st_mtimespec.tv_sec;
  time_t mtime_nsec = (time_t) s.st_mtimespec.tv_nsec;
#elif defined(HAVE_STAT_MTIM)
  time_t mtime_sec = (time_t) s.st_mtim.tv_sec;
  time_t mtime_nsec = (time_t) s.st_mtim.tv_nsec;
#else
  time_t mtime_sec = (time_t) s.st_mtime;
  time_t mtime_nsec = (time_t) 0;
#endif

  *b = mtime_sec != e->mtime_sec || mtime_nsec != e->mtime_nsec;

  return 0;
}

static inline bool entry_handle_modify(struct entry* const e) {

    bool m;

    if (is_modified(e, &m)) {

      print_error("unable to determine modification %s", entry_alias(e));

      entry_destroy(e);
      return true;
    }

    if (m) {

      print_error("file was modified %s", entry_alias(e));

      entry_destroy(e);
      return true;
    }

    return false;
}

static inline bool is_same_perm(const struct entry* const a, const struct entry* const b) {

  return (a->uid == b->uid) && (a->gid == b->gid) && (a->mode == b->mode);
}

static inline void entry_print(const char* const p, const struct entry* const e, const char* const s) {

  if (p)
    printf("%s", p);

  printf("%s", entry_alias(e));

  if (e->aliases.size > 1)
    printf("...");

  if (s)
    printf(" %s", s);
}

#endif // ENTRY_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
