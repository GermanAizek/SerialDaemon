#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([seriallogger], [0.2], [Heath Caldwell <hncaldwell@csupomona.edu>])
AC_CONFIG_SRCDIR([seriallogger.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h syslog.h termios.h unistd.h math.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_CHECK_FUNC([fork], , AC_MSG_ERROR(fork is required))
AC_CHECK_FUNC([memset], , AC_MSG_ERROR(memset is required))
AC_CHECK_FUNC([atexit], , AC_MSG_ERROR(atexit is required))
AC_CHECK_FUNC([getcwd], , AC_MSG_ERROR(getcwd is required))
AC_CHECK_FUNC([floor], , AC_MSG_ERROR(floor is required))
AC_CHECK_FUNC([log], , AC_MSG_ERROR(log is required))

AC_OUTPUT
