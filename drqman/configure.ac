#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([PACKAGE],[PACKAGE_VERSION])
AC_CONFIG_SRCDIR([drqm_jobs_lightwave.h])
AC_CONFIG_HEADER([drqm_config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([wrap], [host_access])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([memset strchr strdup strerror strrchr])
AM_PATH_GTK_2_0(2.8.0,,AC_MSG_ERROR(drqman needs Gtk+ > 2.8.0 to work))

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
