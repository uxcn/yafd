#!/bin/sh -e

# after_success - travis after_success

if test -z "${enable_gcov}"; then
  exit
fi

case ${CC} in

  gcc*)
    GCOV="gcov${CC##gcc}";;

  clang*)
    GCOV="gcov${CC##clang}";;

  *)
    GCOV="gcov";
    echo -e "unknown compiler (${CC})";;

esac

coveralls --gcov ${GCOV} --include src/c/
