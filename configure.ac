AC_PREREQ(2.61)
AC_INIT([editline], [1.0], [nisbet@ihug.co.nz])
AC_CONFIG_SRCDIR([src/editline.c])
AC_CONFIG_HEADER([config.h])

# Prepare automake
AM_INIT_AUTOMAKE([no-dist-gzip dist-xz])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
LT_INIT

AM_CFLAGS="-std=gnu99 $inline_cflags -W -Wall -Wundef -Wunused -Wstrict-prototypes -Werror-implicit-function-declaration $nopointersign_cflags -Wshadow"
AC_SUBST(AM_CFLAGS)

AC_OUTPUT(Makefile src/Makefile examples/Makefile)

