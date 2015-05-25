//===-- action.c - actions ----------------------------------------*- C -*-===//

#include "action.h"

#include "config.h" // autoconf

#include <stdio.h>
#include <stdbool.h>

#include <ctype.h>
#include <unistd.h>

#include "options.h"

#include "entry.h"
#include "duplicate.h"

static bool ask_user(const struct entry* const e, const enum action a, const size_t i) {

  const char* const n = vector_get(&e->aliases, i);
  const bool s = e->aliases.size == 1;

  char* u = NULL;
  size_t un = 0;

  do {

    if (s)
      printf("%s ...%s (Y/n)? ", n, to_string_action(a));
    else if (i)
      printf(" * %s ...%s (Y/n)? ", n, to_string_action(a));
    else
      printf("%s\n * %s ...%s (Y/n)? ", n, n, to_string_action(a));

    ssize_t r = getline(&u, &un, stdin);

    if (r == -1)
      on_fatal("unable to read input");

    if (r == 1 || (r == 2 && (u[0] == 'y' || u[0] == 'n')))
      break;

  } while (true); 

  bool b = (u[0] == '\n' || u[0] == 'y');

  free(u);

  return b;
}

static enum action ask_user_input(const struct entry* const e, const size_t i) {

  const char* const n = vector_get(&e->aliases, i);
  const bool s = e->aliases.size == 1;
  enum action a = act_none;

  char* u = NULL;
  size_t un = 0;

  do {

    if (s)
      printf("%s ...%s/%s/%s? ", n, to_prompt_string_action(act_link), to_prompt_string_action(act_delete), to_prompt_string_action(act_none));
    else if (i)
      printf(" * %s ...%s/%s/%s? ", n, to_prompt_string_action(act_link), to_prompt_string_action(act_delete), to_prompt_string_action(act_none));
    else
      printf("%s\n * %s ...%s/%s/%s? ", n, n, to_prompt_string_action(act_link), to_prompt_string_action(act_delete), to_prompt_string_action(act_none));

    ssize_t r = getline(&u, &un, stdin);

    if (r == -1)
      on_fatal("unable to read input");

    if (r != 2)
      continue;

    a = (enum action) u[0];
  } while (a != act_link && a != act_delete && a != act_none);

  free(u);

  return a;
}

static void action_output(const struct entry* const e, const enum action a, const size_t i) {
  const char* const n = vector_get(&e->aliases, i);
  const bool s = e->aliases.size == 1;

  if (opts.quiet)
    return;

  if (s)
    printf("%s ...%s\n", n, to_string_action(a));
  else if (i)
    printf(" * %s ...%s\n", n, to_string_action(a));
  else
    printf("%s\n * %s ...%s\n", n, n, to_string_action(a));
}

int do_link(const struct duplicate* d) {

  bool f = opts.force;

  const struct entry* ce = NULL;
  const char* c = NULL;

  const struct device* v;

  vector_for_each(v, &d->devices) {

    struct entry* e;

    vector_for_each(e, &v->entries) {

      const char* const n = entry_alias(e);

      if (entry_handle_modify(e))
        continue;

      if (!c) {

        if (f) {

          ce = e;
          c = n;

          const char* a;

          vector_for_each(a, &ce->aliases)
            action_output(ce, act_link, i);

        } else {

          if (ask_user(e, act_link, 0)) {

            ce = e;
            c = n;

            for (size_t k = 1; k < e->aliases.size; k++)
              action_output(ce, act_link, k);

          } else {

            for (size_t k = 1; k < e->aliases.size; k++)
              action_output(e, act_none, k);
          }
        }

        continue;
      }

      if (f) {

        const char* a;

        if (!is_same_perm(ce, e)) {

          vector_for_each(a, &e->aliases) {

            action_output(e, act_link, i);
            print_error("unable to link %s -> %s", c, a);
          }

          continue;
        }

        vector_for_each(a, &e->aliases) {

          action_output(e, act_link, i);

          if (unlink(a) || link(c, a))
            print_error("unable to link %s -> %s", c, a);
        }

      } else {

        const char* a;

        vector_for_each(a, &e->aliases)
          if (ask_user(e, act_link, i) && (unlink(a) || link(c, a)))
            print_error("unable to link %s -> %s", c, a);
      }
    }
  }

  return 0;
}

int do_print(const struct duplicate* d) {

  const bool n = opts.null;
  const char de = n ? '\0' : '\n';

  if (opts.quiet)
    return 0;

  const struct device* v;

  vector_for_each(v, &d->devices) {

    struct entry* e;

    vector_for_each(e, &v->entries) {

      const char* const a = entry_alias(e);

      if (entry_handle_modify(e))
          continue;

      printf("%s%c", a, de);

      if (e->aliases.size == 1)
        continue;

      for (size_t k = 1; k < e->aliases.size; k++) {

        const char* const a = vector_get(&e->aliases, k);

        printf("  * %s%c", a, de);
      }
    }
  }

  return 0;
}

int do_delete(const struct duplicate* d) {

  bool f = opts.force;

  const struct device* v;

  vector_for_each(v, &d->devices) {

    struct entry* e;

    vector_for_each(e, &v->entries) {

      const char* a;

      if (entry_handle_modify(e))
          continue;

      if (f) {

        if (i > 0) {

          vector_for_each(a, &e->aliases) {

            action_output(e, act_delete, i);

            if (unlink(a))
              print_error("unable to delete %s", a);

          }
        } else {

          action_output(e, act_none, i);

          for (size_t k = 1; k < e->aliases.size; k++) {

            a = vector_get(&e->aliases, k);

            action_output(e, act_delete, k);

            if (unlink(a))
              print_error("unable to delete %s", a);
          }
        }

      } else {

        vector_for_each(a, &e->aliases) {

          if (ask_user(e, act_delete, i) && unlink(a))
            print_error("unable to delete %s", a);
        }
      }
    }
  }

  return 0;
}

int do_interactive(const struct duplicate* d) {

  const struct entry* ce = NULL;
  const char* c = NULL;

  const struct device* v;

  vector_for_each(v, &d->devices) {

    struct entry* e;

    vector_for_each(e, &v->entries) {

      const char* n;

      if (entry_handle_modify(e))
        continue;

      vector_for_each(n, &e->aliases) {

        switch (ask_user_input(e, i)) {

          case act_link:

            if (!ce) {
              ce = e;
              c = n;

              continue;
            }

            if (e == ce)
              continue;

            if (!is_same_perm(ce, e) || unlink(n) || link(c, n))
              print_error("unable to link %s -> %s", c, n);
            break;

          case act_delete:

            if (unlink(n))
              print_error("unable to delete %s", n);
            break;

          case act_none:
            break;

          default:
            continue;
        }
      }
    }
  }

  return 0;
}

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
