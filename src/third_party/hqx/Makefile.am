ACLOCAL_AMFLAGS = -I autoconf/m4

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = hqx.pc

EXTRA_DIST = README.md

bin_PROGRAMS = hqx
lib_LTLIBRARIES = libhqx.la

libhqx_la_SOURCES = \
  src/init.c \
  src/hq2x.c \
  src/hq3x.c \
  src/hq4x.c \
  src/common.h
libhqx_la_LDFLAGS = -no-undefined -version-info @HQX_LTVERSION@

include_HEADERS = src/hqx.h

hqx_SOURCES = src/hqx.c
hqx_CFLAGS = $(IL_CFLAGS) -I.
hqx_LDADD = libhqx.la $(IL_LIBS)
