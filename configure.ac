#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(zx, 1.0, mustiigezen at gmail dot com)
AC_CONFIG_SRCDIR([src/zx.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([xcb], [xcb])
PKG_CHECK_MODULES([i3ipc], [i3ipc-glib-1.0])
PKG_CHECK_MODULES([pango], [pango])
PKG_CHECK_MODULES([pangocairo], [pangocairo])
PKG_CHECK_MODULES([cairo], [cairo])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h glib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup strtoul])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PATH_GLIB_2_0
AM_SILENT_RULES([yes])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
