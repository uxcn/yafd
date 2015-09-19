//===-- process.c - file processing -------------------------------*- C -*-===//

#include "process.h"

#include <assert.h>

#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#include <sys/stat.h>

#include "config.h" // autoconf

#include "options.h"

#include "memory.h"

#include "hash.h"

#include "duplicate.h"
#include "device.h"
#include "entry.h"

#include "path.h"

#include "util.h"

#include "thread.h"
#include "worker.h"


static void process_init() {

  const int ts = opts.threads;

  time_t now = time(NULL);

  srand((unsigned int) now); // only seed entropy 

  _thread_init();
  _worker_init(ts);
}

static int on_duplicate(struct duplicate* const d) {

  const enum action a = opts.act;
  const bool q = opts.quiet;
  const bool n = opts.null;

  const char de = n ? '\0' : '\n';

  switch (a) {

    case act_link:
      if (do_link(d))
        print_error("unable to link duplicate");
      break;

    case act_print:
      if (do_print(d))
        print_error("unable to print duplicate");
      break;

    case act_delete:
      if (do_delete(d))
        print_error("unable to delete duplicate");
      break;

    case act_interactive:
      if (do_interactive(d))
        print_error("unable to interactive duplicate");
      break;

    case act_none:
      break; // okay
  }
  
  duplicate_destroy(d);

  if (!q && !is_empty_queue(&duplicates))
    printf("%c", de);

  return 0;
}

static int readpaths(FILE* f) {

  const bool n = opts.null;
  const char de = n ? '\0' : '\n';

  size_t z = 0;
  ssize_t cs = 0;
  char* l = NULL;

  while ((cs = getdelim(&l, &z, de, f)) != EOF) {

    l = strtrim(l);

    struct path* pt = path_create(strcpy(fmalloc(strlen(l) + 1), l));
    queue_add(&paths, &pt->queue);
  }


  if (l)
    free(l);

  if (!feof(f))
    return (int) cs;

  return 0;
}

static int scanpaths(const int n, const char* const ps[n]) {

  for (int i = 0; i < n; i++) {

    const char* const p = ps[i];
    struct path* pt = path_create(strcpy(fmalloc(strlen(p) + 1), p));

    queue_add(&paths, &pt->queue);
  }

  return 0;
}

void run_threads(int ts) {

  pthread_t tids[ts];

  if (worker_start(ts, tids))
    on_fatal("unable to spawn worker threads");

  if (worker_end(ts, tids))
    on_fatal("unable to join worker threads");

}

void run() {

  const int ts = opts.threads;
  const int nps = opts.num_paths;

  const char* const * ps = opts.paths;

  assert(ts >= 0);
  assert(nps >= 0);

  process_init();

  if (!nps) {

    if (readpaths(stdin))
      on_fatal("unable to read paths");

  } else {

    if (scanpaths(nps, ps)) 
      on_fatal("unable to scan paths");
  }

  if (ts)
    run_threads(ts);
  else
    worker(NULL);

  struct duplicate* d;

  while (duplicate_take(&duplicates, &d)) {
  
    if (on_duplicate(d))
      print_error("unable to process duplicate");
  }
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
