#!/bin/sh -e

# after_success - travis after_success

if test -z "${enable_gcov}"; then
  exit
fi

case ${CC} in

  gcc*)
    GCOV="gcov${CC##gcc}";
    GCOV_OPTIONS="";;

  clang*)
    GCOV="llvm-cov${CC##clang}";
    GCOV_OPTIONS="gcov";;

  *)
    GCOV="gcov";
    GCOV_OPTIONS="";
    echo -e "unknown compiler (${CC})";;

esac

coveralls --gcov ${GCOV} --gcov-options ${GCOV_OPTIONS} --include src/c/
