AC_PREREQ(2.61)
AC_INIT([readline_cn], [1.0], [nisbet@ihug.co.nz])
AC_CONFIG_SRCDIR([src/readline.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile examples/Makefile])

# Prepare automake
AM_INIT_AUTOMAKE([no-dist-gzip dist-xz])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
LT_INIT

AM_CFLAGS="-std=gnu99 $inline_cflags -W -Wall -Wundef -Wunused -Wstrict-prototypes -Werror-implicit-function-declaration $nopointersign_cflags -Wshadow"
AC_SUBST(AM_CFLAGS)

AC_OUTPUT

