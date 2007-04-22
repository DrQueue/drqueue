# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([drqueue],[0.65.x])
AC_CONFIG_HEADERS([lconfig.h])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME,[0.65.x])
AC_GNU_SOURCE

gtk_version=gtk+-2.0
AC_CANONICAL_HOST()
case "$host" in
	*linux*) my_sys=-D__LINUX ;;
	*darwin*) my_sys=-D__OSX ;;
	*irix*) my_sys=-D__IRIX ;;
	*freebsd*) my_sys=-D__FREEBSD ;;
	*win*) my_sys=-D__CYGWIN
           platform_win32=yes
           gtk_version=gtk+-win32-2.0
           ;;
	*) AC_MSG_ERROR(No known build system $build) ;;
esac
AC_SUBST([my_sys])

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

# Checks for programs.
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([wrap], [host_access])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h limits.h malloc.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h pwd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm dup2 getcwd memset select strchr strdup strerror strrchr strstr])

PKG_CHECK_MODULES([GTK2],[$gtk_version >= 2.8],[have_gtk2=yes],[have_gtk2=no])
AC_SUBST(GTK2_CFLAGS)
AC_SUBST(GTK2_LIBS)

AC_CONFIG_FILES([Makefile libdrqueue/Makefile drqman/Makefile])
AC_OUTPUT
