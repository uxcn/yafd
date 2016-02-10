#!/bin/sh -e

# script - travis run script

if test -n "${enable_gcov}"; then ARGS="${ARGS} --enable-gcov"; fi
if test -n "${enable_asan}"; then ARGS="${ARGS} --enable-asan"; fi
if test -n "${enable_msan}"; then ARGS="${ARGS} --enable-msan"; fi
if test -n "${enable_tsan}"; then ARGS="${ARGS} --enable-tsan"; fi
if test -n "${enable_ubsan}"; then ARGS="${ARGS} --enable-ubsan"; fi

./autoconf.sh ${ARGS}

make
make test
