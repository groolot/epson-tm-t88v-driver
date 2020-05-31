## EPSON TM-T88V Printer Driver for GNU/Linux
## Copyright (C) 2000-2008 AVASYS CORPORATION.
## Copyright (C) Seiko Epson Corporation 2000-2017.
## Copyright (C) 2020 Grégory DAVID
##  This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
AC_INIT([rastertotmt88v], [0.1.0], [dev@groolot.net])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/rastertotmt88v.cc])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_CONFIG_FILES([
  Makefile
  ppd/Makefile
  src/Makefile
])

# Checks for language version
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for arguments
AC_CHECK_PROG([have_cups_config], [cups-config], [yes], [no])

if test "xNONE" != "x${prefix}" ; then
   cups_default_prefix="${prefix}"
else
   cups_default_prefix="${ac_default_prefix}"
fi

AC_ARG_WITH([cupsfilterdir],
  [AS_HELP_STRING([--with-cupsfilterdir=DIR],
        [CUPS binary directory, where filters are stored.])],
  [],
  [with_cupsfilterdir=no])
if test "xno" = "x${with_cupsfilterdir}"; then
   if test "xyes" = "x$have_cups_config" ; then
      dnl `@<:@' , `@:>@' are replaced with  `[' , `]'
      CUPS_FILTER_DIR="${cups_default_prefix}`cups-config --serverbin | sed -e 's,^/@<:@^/@:>@@<:@^/@:>@*,,'`/filter"
   else
      CUPS_FILTER_DIR="${cups_default_prefix}/lib/cups/filter"
   fi
else
   CUPS_FILTER_DIR="${with_cupsfilterdir}"
fi

AC_ARG_WITH([cupsppddir],
  [AS_HELP_STRING([--with-cupsppddir=DIR],
        [CUPS ppd directory])],
  [],
  [with_cupsppddir=no])
if test "xno" = "x${with_cupsppddir}"; then
   if test -d "${cups_default_prefix}/share/ppd" ; then
      CUPS_PPD_DIR="${cups_default_prefix}/share/ppd"
   elif test "xyes" = "x$have_cups_config" ; then
      dnl `@<:@' , `@:>@' are replaced with  `[' , `]'
      CUPS_PPD_DIR="${cups_default_prefix}`cups-config --datadir | sed -e 's,^/@<:@^/@:>@@<:@^/@:>@*,,'`/model"
   else
      CUPS_PPD_DIR="${cups_default_prefix}/share/cups/model"
   fi
else
   CUPS_PPD_DIR="${with_cupsppddir}"
fi

AC_SUBST(CUPS_FILTER_DIR)
AC_SUBST(CUPS_PPD_DIR)


# Checks for libraries.
AC_CHECK_HEADERS([\
  cups/cups.h \
  cups/ppd.h \
  cups/raster.h \
])
AC_CHECK_HEADERS([\
  ctype.h \
  errno.h \
  fcntl.h \
  malloc.h \
  signal.h \
  stdarg.h \
  stddef.h \
  stdlib.h \
  string.h \
  sys/stat.h \
  sys/types.h \
  time.h \
  unistd.h \
])

AC_SEARCH_LIBS([ppdOpenFile], [cups])
AC_SEARCH_LIBS([cupsRasterOpen], [cupsimage])

# Display some information about this build
echo
echo About this package build:
echo
echo CC=\"$CC\"
echo CXX=\"$CXX\"
echo CPPFLAGS=\"$CPPFLAGS\"
echo CFLAGS=\"$CFLAGS\"
echo CXXFLAGS=\"$CXXFLAGS\"
echo LDFLAGS=\"$LDFLAGS\"
echo LIBS=\"$LIBS\"
echo cups_default_prefix=\"$cups_default_prefix\"
echo CUPS_FILTER_DIR=\"$CUPS_FILTER_DIR\"
echo CUPS_PPD_DIR=\"$CUPS_PPD_DIR\"
echo

AC_OUTPUT
