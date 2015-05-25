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
   [AC_MSG_WARN([pthread support is missing])]
)

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

CFLAGS="$CFLAGS $PTHREAD_CFLAGS -D_GNU_SOURCE -pipe -std=c99 -pedantic -Wall -Wextra -Wsign-conversion -Wconversion -Werror"
LDFLAGS="$LDFLAGS"

LIBS="$LIBS $PTHREAD_LIBS"

AC_ARG_ENABLE([sse4_2],
  AS_HELP_STRING([--enable-sse4_2], [Enable SSE4.2 optimizations]))

AS_IF([test "x$enable_sse4_2" = "xyes"], CFLAGS="-msse4.2 $CFLAGS"
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

AC_CHECK_FUNCS([bzero posix_madvise posix_fadvise])

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
