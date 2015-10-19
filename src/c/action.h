//===-- action.h - actions ----------------------------------------*- C -*-===//

#ifndef ACTION_H
#define ACTION_H

#include "config.h" // autoconf


#include <stddef.h>


#include "platform.h" // platform

struct duplicate; // forward declare

enum action {
  act_link        = 'l',
  act_none        = 'n',
  act_print       = 'p',
  act_delete      = 'd',
  act_interactive = 'i'
};

static inline const char* to_string_action(const enum action act) {

  switch(act) {

    case act_link:
      return "link";

    case act_none:
      return "none";

    case act_print:
      return "print";

    case act_delete:
      return "delete";

    case act_interactive:
      return "interactive";
  }

  return NULL;
}

static inline const char* to_prompt_string_action(const enum action act) {

  switch(act) {

    case act_link:
      return "(l)ink";

    case act_delete:
      return "(d)elete";

    case act_none:
      return "(n)one";

    default:
      return "(?)???";
  }

  return NULL;
}

int do_link(const struct duplicate* d);

int do_print(const struct duplicate* d);

int do_delete(const struct duplicate* d);

int do_interactive(const struct duplicate* d);

#endif // ACTION_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
