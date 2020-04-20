AC_INIT([hqx], [1.2], [https://github.com/grom358/hqx/issues],
  [hqx], [https://github.com/grom358/hqx])
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([autoconf/m4])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

AC_LANG([C])
AC_PROG_CPP
AC_PROG_CC
AM_PROG_AR
AC_C_INLINE
AC_C_BIGENDIAN

LT_INIT([win32-dll])
HQX_LTVERSION=1:0:0
AC_SUBST([HQX_LTVERSION])

PKG_CHECK_MODULES([IL], [IL], ,
  AC_MSG_ERROR([DevIL (http://openil.sf.net) is required.]))

AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug], [Compile a debug version of the library]))
AS_IF([test "x$enable_debug" = "xyes"],
  [CFLAGS="-Wall -g -O0"],
  [CFLAGS="-O3"])

AC_CONFIG_FILES([Makefile hqx.pc])
AC_OUTPUT
