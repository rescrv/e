#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([libe], [0.3.dev], [escriva@cs.cornell.edu])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
LT_PREREQ([2.2])
LT_INIT
AC_CONFIG_SRCDIR([e/buffer.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

ANAL_WARNINGS_CXX

# Checks for libraries.
GTEST_LIB_CHECK
PKG_CHECK_MODULES([PO6], [libpo6 >= 0.2])

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
AC_CHECK_FUNCS([clock_gettime memchr memmove strtoul strtoull mach_absolute_time])

AC_CONFIG_FILES([Makefile
                 libe.pc])
AC_OUTPUT
