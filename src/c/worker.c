//===-- worker.c - worker functions -------------------------------*- C -*-===//

#include "worker.h"

#include <assert.h>

#include "config.h" // autoconf

#include <stdlib.h>

#include <string.h>

#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

#include <sys/types.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif


#include "platform.h" // platform


#include "options.h"

#include "thread.h"

#include "queue.h"

#include "path.h"
#include "duplicate.h"
#include "device.h"
#include "entry.h"

#include "util.h"

#include "digest.h"

#ifdef HAVE_WINDOWS
#include "mingw.h"
#endif

#if defined(HAVE_DARWIN)
#include "pthread-dw.h"
#endif

struct queue paths;

struct queue duplicates;

static struct hash stod;

static struct queue digests;

static struct queue compares;

#ifdef HAVE_PTHREAD_H
static pthread_once_t worker_once = PTHREAD_ONCE_INIT;
#endif

#ifdef HAVE_PTHREAD_H
static pthread_barrier_t worker_barrier;
#endif


void _worker_init(const int threads) {

  assert(threads >= 0);

  hash_init(&stod, 1024);

  queue_init(&paths);
  queue_init(&digests);
  queue_init(&compares);
  queue_init(&duplicates);

  if (threads) {

#ifdef HAVE_PTHREAD_H
    if (pthread_barrier_init(&worker_barrier, NULL, (unsigned) threads))
      on_fatal("unable to initialize barrier");
#endif
  }

}

#ifdef HAVE_PTHREAD_H
int worker_start(const int threads, pthread_t tids[threads]) {

  for (int i = 0; i < threads; i++) {

    int err = pthread_create(&tids[i], NULL, worker, NULL);

    if (err)
      return err;
  }

  return 0;
}
#endif

#ifdef HAVE_PTHREAD_H
int worker_end(const int threads, pthread_t tids[threads]) {

  for (int i = 0; i < threads; i++) {

    int err = pthread_join(tids[i], NULL);

    if (err)
      return err;
  }

  return 0;
}
#endif

static int on_file(const char* const file, const ino_t i, const struct stat* const s, struct duplicate** dp) {

  bool z = opts.zero;

  struct duplicate* d = *dp;
  struct hash_entry* he = &d->hash;

  size_t ds;
  bool n;

  if (!s->st_size && !z)
    return 0;

  char* a = memcpy(fmalloc(strlen(file) + 1), file, strlen(file) + 1);

  he->value = (size_t) s->st_size;
  he = hash_get_or_put_atomic(&stod, he);

  d = container_of(he, struct duplicate, hash);

  assert(d->hash.value == (size_t) s->st_size);

  duplicate_entry_lazy_get_atomic(d, s->st_dev, i, s, a, &ds, &n);

  if (ds == 2 && n)
      queue_add(&digests, &d->queue);

  if (d == *dp)
    *dp = duplicate_create();

  return 0;
}

static int on_dir(const char* const dir, struct duplicate** dp) {
  bool r = opts.recurse;

  DIR* d = opendir(dir); 

  if (!d)
    return - 1;

  struct dirent* de;

  while ((de = readdir(d))) {

    ino_t i;
    struct stat s;

    size_t bs = strlen(dir) + strlen(de->d_name) + 2;
    char b[bs];

    const char* const a = strjoin(b, dir, de->d_name, '/');

    if (lstat(a, &s)) {

      print_error("unable to stat %s", d);
      continue;
    }

#ifndef HAVE_WINDOWS
    i = s.st_ino;
#endif

#ifdef HAVE_WINDOWS
    if (inode_w(a, &i)) {

      print_error("unable to inode %s", d);
      continue;
    }
#endif

    if (S_ISREG(s.st_mode))
      if (on_file(a, i, &s, dp))
        print_error("unable to process file %s/%s", dir, de->d_name);
  }

  if (!r) {
    
    if (closedir(d))
      on_fatal("unable to close directory %s", dir);

    return 0;
  }

  rewinddir(d);

  while ((de = readdir(d))) {

    struct stat ds;

    size_t bs = strlen(dir) + strlen(de->d_name) + 2;
    char b[bs];

    const char* const d = strjoin(b, dir, de->d_name, '/');

    if (lstat(d, &ds)) {

      print_error("unable to stat %s", d);
      continue;
    }

    if (S_ISDIR(ds.st_mode)) {

      const char* const dot = ".";
      const char* const dotdot = "..";

      if (!strcmp(dot, de->d_name) || !strcmp(dotdot, de->d_name))
        continue;

      struct path* p = path_create(strcpy(fmalloc(bs), d));

      queue_add(&paths, &p->queue);
    }
  }

  if (closedir(d))
    print_error("unable to close directory %s", dir);

  return 0;
}

static int on_path(const char* const path, struct duplicate** dp) {

  ino_t i;
  struct stat s;

  if (lstat(path, &s)) 
    return -1;

#ifndef HAVE_WINDOWS
  i = s.st_ino;
#endif

#ifdef HAVE_WINDOWS
    if (inode_w(path, &i))
      return -1;
#endif

  if (S_ISDIR(s.st_mode))
    return on_dir(path, dp);
  else if (S_ISREG(s.st_mode))
    return on_file(path, i, &s, dp);

  return 0;
}

static void do_path(struct path* const p, struct duplicate** dp) {

    if (on_path(p->name, dp))
      print_error("unable to process path %s", p->name);

    path_destroy(p);
}

static void do_digest(struct duplicate* const d) {

  const struct device* const v = vector_head(&d->devices);
  const struct entry* const e = vector_head(&v->entries);

  const size_t bs = min(opts.bytes, (size_t) e->size);
  enum digest* gs = opts.digs;
  const off_t o = opts.offset;

#ifdef HAVE_MMAP
  const bool m = opts.mmap;
#endif

  const bool uo = opts.user_offset;

  struct vector ds;

  off_t k = o;

  if (!bs) {

    queue_add(&compares, &d->queue);
    return;
  }

  if (!uo) {

    char u[sizeof(off_t)];

    for (size_t i = 0; i < sizeof(off_t) / sizeof(int); i++) {
      int r = rand();

      memcpy(u + (i * sizeof(int)), &r, sizeof(int));
    }

    memcpy(&k, u, sizeof(off_t));
  }

  k = k < 0 ?  e->size + (k % e->size) : k;
  k = (e->size > (off_t) bs) ? k % (e->size - (off_t) bs) : 0;

  size_t cs[d->count];
  struct digest_t hs[d->count];
  struct entry* es[d->count];

  size_t n = 0;

  while (d->devices.size) {

    struct device* const v = vector_pop(&d->devices); 

    while (v->entries.size) {

      const struct entry* const e = vector_pop(&v->entries);
      const char* const a = entry_alias(e);
      d->count--;

      es[n] = (struct entry*) e;

      assert(e->size == es[0]->size);

      if (entry_handle_size_modify(es[n]))
        continue;

#ifdef HAVE_WINDOWS
      es[n]->fd = open(a, O_RDONLY | O_BINARY);
#else
      es[n]->fd = open(a, O_RDONLY);
#endif

      digest_init(&hs[n], gs);

#ifdef HAVE_MMAP
      if (m) {

        es[n]->mm = mmap(0, (size_t) es[n]->size, PROT_READ, MAP_SHARED, es[n]->fd, 0);

#if defined(HAVE_POSIX_MADVISE) && defined(HAVE_STAT_BLKSIZE)
        if (posix_madvise((uint8_t*) es[n]->mm + (k - (k % e->bs)), bs + (size_t) (k % e->bs), POSIX_MADV_WILLNEED))
          print_error("unable to memory advise %s", entry_alias(es[n]));
#endif
        cs[n++] = bs;
        continue;
      }
#endif

#ifdef HAVE_POSIX_FADVISE
      if (posix_fadvise(es[n]->fd, k, (off_t) bs, POSIX_FADV_SEQUENTIAL))
        print_error("unable to file advise %s", entry_alias(es[n]));
#endif

      if (lseek(es[n]->fd, k, SEEK_SET) < 0) {

        print_error("unable to seek %s", entry_alias(es[n]));

        if (close(es[n]->fd))
          print_error("unable to close %s", entry_alias(es[n]));

        entry_destroy(es[n]);
        continue;
      }

      cs[n++] = bs;
    }

    device_destroy(v);
  }

  duplicate_destroy(d); // husk

  vector_init(&ds, 2);

  uint8_t db[bs > pz ? 1 : bs]; // zero length undefined
#ifdef HAVE_PTHREAD_H
  uint8_t* dg = bs > pz ? thread_local_buffer(bs) : db;
#else
  uint8_t* dg = bs > pz ? frealloc(bf, bs) : db;
#endif

  size_t rs = bs * n;

  memset(dg, 0, bs);

  do {

    for (size_t i = 0; i < n; i++) {

      if (!cs[i])
        continue;

#ifdef HAVE_MMAP
      uint8_t* dp = m ? (uint8_t*) es[i]->mm + k : dg;
      ssize_t r = m ? (ssize_t) cs[i] : read(es[i]->fd, dp, cs[i]);
#else
      uint8_t* dp = dg;
      ssize_t r = read(es[i]->fd, dp, (unsigned) cs[i]);
#endif
      size_t c = (size_t) r;

      if (r < 0) {

        print_error("unable to read %s", entry_alias(es[i]));

        rs -= cs[i];
        cs[i] = 0;

        if (close(es[i]->fd)) {

          print_error("unable to close %s", entry_alias(es[i]));
        }

        entry_destroy(es[i]);
        continue;
      }

      cs[i] -= c;
      rs -= c;

      if (cs[i])
        continue;

      digest(bs, dp, &hs[i], gs);
      digest_finalize(&hs[i], gs);

#ifdef HAVE_MMAP
      if (m && munmap(es[i]->mm, bs)) {

        print_error("unable to unmap %s", entry_alias(es[i]));

        entry_destroy(es[i]);
        continue;
      }
#endif

      if (close(es[i]->fd)) {

        print_error("unable to close %s", entry_alias(es[i]));

        entry_destroy(es[i]);
        continue;
      }

      size_t di;

      if (vector_bsearch(&ds, &hs[i], duplicate_digest_compare, &di)) {

        struct duplicate* const d = vector_get(&ds, di);

        duplicate_entry_add(d, es[i]);

        if (d->count == 2)
          queue_add(&compares, &d->queue);

        continue;
      }

      struct duplicate* const d = duplicate_create();

      d->digest = hs[i];

      duplicate_entry_add(d, es[i]);

      vector_insert(&ds, di, d);
    }
  } while (rs);

  struct duplicate* dc;

  vector_for_each(dc, &ds) {

    if (dc->count < 2)
      duplicate_destroy(dc);
  }

  vector_destroy(&ds);
}

static void do_compare(struct duplicate* const d) {

  const struct device* const v = vector_head(&d->devices);
  const struct entry* const e = vector_head(&v->entries);

#ifdef HAVE_MMAP
  const bool m = opts.mmap;
#endif

  struct vector ds;

  if (!e->size) {

    queue_add(&duplicates, &d->queue);
    return;
  }

  vector_init(&ds, 2);

  while (d->devices.size) {

    struct device* const v = vector_pop(&d->devices);

    while (v->entries.size) {

      struct entry* const e = vector_pop(&v->entries);
      d->count--;

      size_t i;

      if (entry_handle_modify(e))
        continue;

#ifdef HAVE_WINDOWS
      e->fd = open(entry_alias(e), O_RDONLY | O_BINARY);
#else
      e->fd = open(entry_alias(e), O_RDONLY);
#endif

#ifdef HAVE_MMAP
      if (m)
        e->mm = mmap(0, (size_t) e->size, PROT_READ, MAP_SHARED, e->fd, 0);
      
      int (* const f) (const void*, const void*) = m ? duplicate_content_compare_mmap : duplicate_content_compare;
#else
      int (* const f) (const void*, const void*) = duplicate_content_compare;
#endif

      if (vector_bsearch(&ds, e, f, &i)) {

        struct duplicate* const d = vector_get(&ds, i);

        duplicate_entry_add(d, e);

        if (d->count == 2)
          queue_add(&duplicates, &d->queue);

        continue;
      }

      struct duplicate* const nd = duplicate_create();

      nd->digest = d->digest;

      duplicate_entry_add(nd, e);

      vector_insert(&ds, i, nd);
    }

    device_destroy(v);
  }

  duplicate_destroy(d);
  
  struct duplicate* td;
  struct device* tv;
  struct entry* te;

  vector_for_each(td, &ds) {
    vector_for_each(tv, &td->devices) {
      vector_for_each(te, &tv->entries) {

#ifdef HAVE_MMAP
        if (m && munmap(te->mm, (size_t) te->size)) {
        
          print_error("unable to unmap %s", entry_alias(te));
          te->valid = false;
        }
#endif

        if (close(te->fd)) {
        
          print_error("unable to close %s", entry_alias(te));
          te->valid = false;
        }
      }
    }

    if (td->count < 2)
      duplicate_destroy(td);
  }

  vector_destroy(&ds);
}

static void stod_destroy() {

  struct hash_entry* he;

  hash_for_each(he, &stod) {

    struct duplicate* d = container_of(he, struct duplicate, hash);
    
    if (d->count < 2)
      duplicate_destroy(d);
  }

  hash_destroy(&stod);
}

#ifdef HAVE_PTHREAD_H
static int worker_wait() {

  int ts = opts.threads;

  if (ts)
    return pthread_barrier_wait(&worker_barrier);

  return 1;
}
#endif


void* worker(void* a) {

  if (a)
    return NULL; // compiler warning

  struct path* p;
  struct duplicate* d = duplicate_create();

  while (path_take(&paths, &p))
      do_path(p, &d);

  duplicate_destroy(d);

#ifdef HAVE_PTHREAD_H
  worker_wait();

  pthread_once(&worker_once, stod_destroy);
#endif

#ifndef HAVE_PTHREAD_H
  stod_destroy();
#endif

  while (duplicate_take(&digests, &d))
    do_digest(d);

#ifdef HAVE_PTHREAD_H
  worker_wait();
#endif

#ifndef HAVE_PTHREAD_H
  free(bf);
#endif

  while (duplicate_take(&compares, &d))
    do_compare(d);

  return NULL;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
