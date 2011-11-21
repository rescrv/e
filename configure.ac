#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.66])
AC_INIT([e], [0.1.dev], [escriva@cs.cornell.edu])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign dist-bzip2 dist-lzma])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([include/e/buffer.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
GTEST_LIB_CHECK

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

if test x"${target_cpu}" != "xx86_64"; then
    AC_MSG_ERROR(['this library requires a 64-bit x86 processor'])
fi

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([clock_gettime memchr memmove strtoul strtoull])

AC_CONFIG_FILES([Makefile
                 benchmarks/Makefile
                 include/Makefile
                 test/Makefile])
AC_OUTPUT
