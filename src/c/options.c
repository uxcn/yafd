//===-- options.c - program options ---------------------------*- C -*-===//

#include "options.h"


#include "config.h" // autoconf


#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include <stdio.h>

#include <stdint.h>

#include <string.h>
#include <strings.h>

#include <unistd.h>

#include <errno.h>
#include <getopt.h>


#include "platform.h" // platform


#include "memory.h"
#include "error.h"
#include "thread.h"
#include "action.h"

struct opts opts;

enum opt {
  opt_recurse     = 'r',
  opt_quiet       = 'q',
  opt_null        = '0',
  opt_force       = 'f',
#ifdef HAVE_MMAP
  opt_mmap        = 'm',
#endif
  opt_link        = 'l',
  opt_print       = 'p',
  opt_delete      = 'd',
  opt_interactive = 'i',
  opt_crc32       = 'C',
  opt_fnv32a      = 'N',
  opt_fnv64a      = 'V',
  opt_frm64       = 'F',
  opt_frm128      = 'M',
  opt_sky64       = 'S',
  opt_sky128      = 'P',
  opt_zero        = 'z',
#ifdef HAVE_PTHREAD_H
  opt_threads     = 't',
#endif
  opt_blocksize   = 'k',
  opt_pagesize    = 'g',
  opt_bytes       = 'b',
  opt_offset      = 'o',
  opt_version     = 'v',
  opt_help        = 'h'
};

struct opt_desc {

  const char short_name;

  const char* long_name;

  const char* description;

  const int argument;
};

static const struct opt_desc opt_descs[] = {
  { opt_help,         "help",         "print help",                        no_argument },
  { opt_version,      "version",      "print version",                     no_argument },
  { opt_recurse,      "recurse",      "recurse into sub-directories",      no_argument },
  { opt_quiet,        "quiet",        "suppress normal output",            no_argument },
  { opt_null,         "null",         "delimit files using ASCII NUL",     no_argument },
  { opt_force,        "force",        "don't ask before doing action",     no_argument },
#ifdef HAVE_MMAP
  { opt_mmap,         "mmap",         "use memory mapped I/O",             no_argument },
#endif
  { opt_link,         "link",         "hard link all duplicate files",     no_argument },
  { opt_print,        "print",        "print duplicate files",             no_argument },
  { opt_delete,       "delete",       "delete duplicate files",            no_argument },
  { opt_zero,         "zero",         "interpret zero sized files " 
                                      "as duplicate",                      no_argument },
  { opt_interactive,  "interactive",  "interactively choose action for " 
                                      "each duplicate",                    no_argument },
  { opt_crc32,        "crc32",        "use CRC (32)",                      no_argument },
  { opt_fnv32a,       "fnv32a",       "use FNV-1a (32)",                   no_argument },
  { opt_fnv64a,       "fnv64a",       "use FNV-1a (64)",                   no_argument },
  { opt_frm64,        "frm64",        "use FarmHash (64)",                 no_argument },
  { opt_frm128,       "frm128",       "use FarmHash (128)",                no_argument },
  { opt_sky64,        "sky64",        "use SpookyHash (64)",               no_argument },
  { opt_sky128,       "sky128",       "use SpookyHash (128)",              no_argument },
#ifdef HAVE_PTHREAD_H
  { opt_threads,      "threads",      "the number of concurrent threads",  required_argument },
#endif
  { opt_blocksize,    "blocksize",    "the default filesystem block size", required_argument },
  { opt_pagesize,     "pagesize",     "the default memory page size",      required_argument },
  { opt_bytes,        "bytes",        "the number of bytes to use in " 
                                      "digest to identify potential " 
                                      "duplicates",                       required_argument },
  { opt_offset,       "offset",       "a byte offset into files to "
                                      "use for digest, negative values " 
                                      "are interpreted as an offset " 
                                      "from the end of the file",         required_argument },  
                                      
  { 0, 0, 0, 0 },
};

static const char* short_options;

static const struct option* long_options;

static const char* to_short(const struct opt_desc opt_descs[]) {

  const struct opt_desc* p;

  size_t n = 0;

  for (p  = &opt_descs[0]; p->short_name; p++)
   n += (size_t) p->argument + 1; 

  char* const s = fmalloc(n + 1);

  int si = 0;

  if (!s)
    return NULL;

  for (p  = &opt_descs[0]; p->short_name; p++) {
    s[si++] = p->short_name;

    if (p->argument)
      s[si++] = ':';
  }

  return s;
}

static const struct option* to_long(const struct opt_desc opt_descs[]) {

  const struct opt_desc* p;

  size_t n = 0;

  for (p = &opt_descs[0]; p->short_name; p++)
   n++; 

  struct option* const os = fcalloc(n + 1, sizeof(struct option));

  int oi = 0;

  if (!os)
    return NULL;

  for (p = &opt_descs[0]; p->short_name; p++) {
    struct option* const o = &os[oi++];

    o->val      = p->short_name;
    o->name     = p->long_name;
    o->has_arg  = p->argument;
  }

  return os;
}

void _options_init() {

  _digest_init();

  short_options = to_short(opt_descs);
  long_options = to_long(opt_descs);
}

void _options_destroy() {

  free((void*) short_options);
  free((void*) long_options);
}

void options_parse(const int argc, const char* const argv[]) {

#ifndef OFF_T_MIN
  assert(sizeof(off_t) >= sizeof(int8_t) && sizeof(off_t) <= sizeof(intmax_t));

  const off_t OFF_T_MIN = sizeof(off_t) == sizeof(int8_t)   ? INT8_MIN    :
                          sizeof(off_t) == sizeof(int16_t)  ? INT16_MIN   :
                          sizeof(off_t) == sizeof(int32_t)  ? INT32_MIN   :
                          sizeof(off_t) == sizeof(int64_t)  ? INT64_MIN   :
                          sizeof(off_t) == sizeof(intmax_t) ? INTMAX_MIN  : 0;
#endif

#ifndef OFF_T_MAX
  assert(sizeof(off_t) >= sizeof(int8_t) && sizeof(off_t) <= sizeof(intmax_t));

  const off_t OFF_T_MAX = sizeof(off_t) == sizeof(int8_t)   ? INT8_MAX    :
                          sizeof(off_t) == sizeof(int16_t)  ? INT16_MAX   :
                          sizeof(off_t) == sizeof(int32_t)  ? INT32_MAX   :
                          sizeof(off_t) == sizeof(int64_t)  ? INT64_MAX   :
                          sizeof(off_t) == sizeof(intmax_t) ? INTMAX_MAX  : 0;
#endif

#ifndef SIZE_T_MAX
  const size_t SIZE_T_MAX = (size_t) -1;
#endif

#ifdef HAVE_PTHREAD_H
  unsigned long threads;
#endif
  unsigned long blocksize;
  unsigned long pagesize;
  unsigned long long bytes;
  long long offset;
  int i;

  memset(&opts, 0, sizeof(struct opts));

  opts.digs = fcalloc(num_digs + 1, sizeof(enum digest));

#ifdef HAVE_PTHREAD_H
  opts.threads = num_cpus() * 2;
#endif

  opts.bytes = OPT_BYTES;

  while ((i = getopt_long(argc, (char**) argv, short_options, long_options, NULL)) != -1) {

    enum opt o = (enum opt) i;

    switch (o) {

      case opt_recurse:
        opts.recurse = true;
        break;

      case opt_quiet:
        if (opts.act == act_interactive)
          on_fatal("quiet can't be specified with interactive");

        opts.quiet = true;
        break;

      case opt_null:
        if (opts.act == act_interactive ||
            opts.act == act_delete ||
            opts.act == act_link)
          on_fatal("null can only be used for printing");
        if (opts.quiet)
          on_fatal("null can't be specified with quiet");

        opts.null = true;
        break;

      case opt_force:
        if (opts.act == act_interactive)
          on_fatal("force can't be specified with interactive");

        opts.force = true;
        break;

#ifdef HAVE_MMAP
      case opt_mmap:
        opts.mmap = true;
        break;
#endif

      case opt_interactive:
        if (opts.force)
          on_fatal("force can't be specified with interactive");
        if (opts.quiet)
          on_fatal("quiet can't be specified with interactive");
        if (opts.null)
          on_fatal("null can't be specified with interactive");

        opts.act = act_interactive;
        break;

      case opt_delete:
        if (opts.act)
          on_fatal("multiple actions specified (%s, %s)", to_string_action(opts.act), to_string_action(act_delete));
        if (opts.null)
          on_fatal("null can't be specified with delete");

        opts.act = act_delete;
        break;

      case opt_link:
        if (opts.act)
          on_fatal("multiple actions specified (%s, %s)", to_string_action(opts.act), to_string_action(act_link));
        if (opts.null)
          on_fatal("null can't be specified with link");

        opts.act = act_link;
        break;

      case opt_print:
        if (opts.act)
          on_fatal("multiple actions specified (%s, %s)", to_string_action(opts.act), to_string_action(act_print));

        opts.act = act_print;
        break;

      case opt_zero:
        opts.zero = true;
        break;

#ifdef HAVE_PTHREAD_H
      case opt_threads:
        threads = strtoul(optarg, NULL, 10);

        if (errno == ERANGE || threads > (unsigned long) INT_MAX)
          on_fatal("invalid number of threads (%s)", optarg);

        opts.threads = (int) threads;
        break;
#endif

      case opt_blocksize:
        blocksize = strtoul(optarg, NULL, 10);

        if (errno == ERANGE || blocksize > (unsigned long) INT_MAX)
          on_fatal("invalid blocksize (%s)", optarg);

        opts.blocksize = (int) blocksize;
        break;

      case opt_pagesize:
        pagesize = strtoul(optarg, NULL, 10);

        if (errno == ERANGE || pagesize > (unsigned long) INT_MAX)
          on_fatal("invalid pagesize (%s)", optarg);

        opts.pagesize = (int) pagesize;
        break;

      case opt_bytes:
        bytes = strtoull(optarg, NULL, 10);

        if (errno == ERANGE || bytes > SIZE_T_MAX || strtoll(optarg, NULL, 10) < 0)
          on_fatal("invalid number of digest bytes (%s)", optarg);

        opts.bytes = (size_t) bytes;
        break;

      case opt_offset:
        offset = strtoll(optarg, NULL, 10);

        if (errno == ERANGE || offset > OFF_T_MAX || offset < OFF_T_MIN)
          on_fatal("invalid digest byte offset (%s)", optarg);

        opts.user_offset = true;
        opts.offset = (off_t) offset;
        break;

      case opt_crc32:

        opts.digs[opts.num_digs] = dig_crc32;
        opts.num_digs++;
        break;

      case opt_fnv32a:

        opts.digs[opts.num_digs] = dig_fnv32a;
        opts.num_digs++;
        break;

      case opt_fnv64a:

        opts.digs[opts.num_digs] = dig_fnv64a;
        opts.num_digs++;
        break;

      case opt_frm64:

        opts.digs[opts.num_digs] = dig_frm64;
        opts.num_digs++;
        break;

      case opt_frm128:

        opts.digs[opts.num_digs] = dig_frm128;
        opts.num_digs++;
        break;

      case opt_sky64:

        opts.digs[opts.num_digs] = dig_sky64;
        opts.num_digs++;
        break;

      case opt_sky128:

        opts.digs[opts.num_digs] = dig_sky128;
        opts.num_digs++;
        break;

      case opt_version:
        print_version();

      case opt_help:
        print_help();
    }
  }

  if (!opts.blocksize)
    opts.blocksize = OPT_BLKSIZE;

  if (!opts.pagesize)
    opts.pagesize = OPT_PAGSIZE;

  if (!opts.act)
    opts.act = OPT_ACTION;

  if (!opts.num_digs) {

    opts.digs[opts.num_digs] = OPT_DIGEST;
    opts.num_digs++;
  }

  opts.num_paths = argc - optind;
  opts.paths = &argv[optind];

  if (!opts.num_paths) {

    if (isatty(STDIN_FILENO)) {

      print_usage();
      exit(EXIT_SUCCESS);
    }
  }
}
  
void print_version() {
  
  printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
  exit(EXIT_SUCCESS);
}

void print_usage() {

  printf("usage: %s [options] [path ...]\n", PACKAGE_NAME);
}

static void print_option(const struct opt_desc* const o) {

  const size_t ds = 25;
  const size_t de = 80;

  char s[de + 1];

  const size_t off = strlen(o->long_name) + 6;

  memset(s, ' ', de);
  s[de] = '\0';

  if (strlen(o->description) + ds <= de) {

    printf(" -%c --%s %s %s\n", o->short_name, o->long_name, s + (de - ds) + off + 2, o->description);
    return;
  }

  sprintf(s, " -%c --%s", o->short_name, o->long_name);
  s[off] = ' ';

  size_t start = 0;
  size_t end = 0;
  
  do {

    start = end;
    end = start + (de - ds);

    while (o->description[end] != ' ')
      end--;

    strncpy(s + ds, o->description + start, end++ - start);
    printf("%s\n", s);

    memset(s, ' ', de);
  } while (strlen(o->description) - end > (de - ds));

  strcpy(s + ds, o->description + end);
  printf("%s\n", s);
}

void print_help() {

  print_usage();

  printf("\noptions: \n");

  for (const struct opt_desc* p = &opt_descs[0]; p->short_name; p++)
    print_option(p);
  
  exit(EXIT_SUCCESS);
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
