AC_INIT(
    [yafd],
    [0.1],
    [https://github.com/uxcn/yafd/issues],
    [yafd],
    [https://github.com/uxcn/yafd])

AC_PREREQ([2.65])

AC_PROG_CC

m4_include([m4/acx_pthread.m4])

ACX_PTHREAD(
  [AC_CHECK_HEADERS([pthread.h])],
  [AC_MSG_ERROR([pthread support is missing])]
)

AC_CHECK_DEFINE([__APPLE__],
  AC_DEFINE([HAVE_DARWIN], [1], [Define to 1 if you have a Darwin operating system])
  AC_DEFINE([_DARWIN_C_SOURCE], [900000L], [Enable Darwin extensions])
)

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

CFLAGS="$CFLAGS $PTHREAD_CFLAGS -D_GNU_SOURCE -pipe -std=c99 -pedantic -Wall -Wextra -Wsign-conversion -Wconversion -Werror"
LDFLAGS="$LDFLAGS"

LIBS="$LIBS $PTHREAD_LIBS"

AC_ARG_ENABLE([gcov],
  AS_HELP_STRING([--enable-gcov], [enable gcov instrumentation])
)

AS_IF([test "x$enable_gcov" = "xyes"], 
  CFLAGS="$CFLAGS -O0 -g -fprofile-arcs -ftest-coverage"
)

AC_ARG_ENABLE([asan],
  AS_HELP_STRING([--enable-asan], [enable address sanity checks])
)

AS_IF([test "x$enable_asan" = "xyes"], 
  [AS_IF([test "x$enable_msan" = "xyes" || test "x$enable_tsan" = "xyes" || test "x$enable_ubsan" = "xyes"],
    [AC_MSG_ERROR([multiple sanitizers can not be enabled simultaneously])], 
    CFLAGS="$CFLAGS -fsanitize=address"]
  )
)

AC_ARG_ENABLE([msan],
  AS_HELP_STRING([--enable-msan], [enable memory sanity checks]))

AS_IF([test "x$enable_msan" = "xyes"], 
  [AS_IF([test "x$enable_asan" = "xyes" || test "x$enable_tsan" = "xyes" || test "x$enable_ubsan" = "xyes"],
    [AC_MSG_ERROR([multiple sanitizers can not be enabled simultaneously])], 
    CFLAGS="$CFLAGS -fsanitize=memory"]
  )
)

AC_ARG_ENABLE([tsan],
  AS_HELP_STRING([--enable-tsan], [enable thread sanity checks])
)

AS_IF([test "x$enable_tsan" = "xyes"], 
  [AS_IF([test "x$enable_asan" = "xyes" || test "x$enable_msan" = "xyes" || test "x$enable_ubsan" = "xyes"],
    [AC_MSG_ERROR([multiple sanitizers can not be enabled simultaneously])], 
    CFLAGS="$CFLAGS -fsanitize=thread"]
  )
)

AC_ARG_ENABLE([ubsan],
  AS_HELP_STRING([--enable-ubsan], [enable undefined behavior sanity checks])
)

AS_IF([test "x$enable_ubsan" = "xyes"], 
  [AS_IF([test "x$enable_asan" = "xyes" || test "x$enable_msan" = "xyes" || test "x$enable_tsan" = "xyes"],
    [AC_MSG_ERROR([multiple sanitizers can not be enabled simultaneously])], 
    CFLAGS="$CFLAGS -fsanitize=undefined"]
  )
)

AC_ARG_ENABLE([sse4_2],
  AS_HELP_STRING([--enable-sse4_2], [enable SSE4.2 optimizations])
)

AS_IF([test "x$enable_sse4_2" = "xyes"],
  CFLAGS="$CFLAGS -msse4.2"
)


AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_TYPE_OFF_T
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_FUNC_STRERROR_R

AC_CHECK_FUNCS([posix_madvise posix_fadvise])

AC_CHECK_MEMBERS(
  [struct stat.st_blksize], 
  [AC_DEFINE([HAVE_STAT_BLKSIZE], 
  [], 
  [Define to 1 if you have stat.blksize])]
)

AC_SYS_LARGEFILE


AC_CONFIG_FILES([Makefile src/c/Makefile])
AC_CONFIG_HEADERS([src/c/config.h])

AC_OUTPUT
