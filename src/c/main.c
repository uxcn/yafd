//===-- yafd.c - yafd main ----------------------------------------*- C -*-===//

#include <errno.h>

#include "error.h"

#include "options.h"

#include "process.h"

int main(const int argc, const char* const argv[]) {

  _options_init();

  options_parse(argc, argv);

  _options_destroy();

  run();

  return 0;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
