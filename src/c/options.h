//===-- options.h - program options ---------------------------*- C -*-===//

#ifndef OPTIONS_H
#define OPTIONS_H

#include "config.h" // autoconf


#include <stddef.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>

#include "platform.h" // platform


#include "action.h"
#include "digest.h"

// option defaults

#ifndef OPT_BLKSIZE
#define OPT_BLKSIZE 4096
#endif

#ifndef OPT_PAGSIZE
#define OPT_PAGSIZE 4096
#endif

#ifndef OPT_BYTES
#define OPT_BYTES 4096
#endif

#ifndef OPT_ACTION
#define OPT_ACTION act_print
#endif

#ifndef OPT_DIGEST
#define OPT_DIGEST dig_frm128
#endif

struct opts {

  bool zero;

  bool recurse;

  bool quiet;

  bool null;

  bool force;

#ifdef HAVE_MMAP
  bool mmap;
#endif

  bool user_offset;

  int num_paths;

  int num_digs;

#ifdef HAVE_PTHREAD_H
  int threads;
#endif

  int blocksize;

  int pagesize;

  size_t bytes;

  off_t offset;

  enum action act;

  enum digest* digs;

  const char* const * paths;
};

extern struct opts opts;

void _options_init();

void _options_destroy();

void options_parse(const int argc, const char* const argv[]);

void print_help();

void print_usage();

void print_version();

#endif // OPTIONS_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
