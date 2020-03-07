# Libtool versioning: see https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
m4_define([version_major], 0)
m4_define([version_minor], 1)
m4_define([version_patch], 0)
m4_define([lt_current], [version_major])
m4_define([lt_revision], [version_minor])
m4_define([lt_age], 0)
m4_define([lt_version_info], [lt_current:lt_revision:lt_age])

AC_INIT([tests-unitaires-exemple], [0.1.0], [gregory.david@ac-nantes.fr])
AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
])

# Checks for language version
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_AR
AC_PROG_LIBTOOL

LIBARITHMETIQUE_SO_VERSION=lt_version_info
AC_SUBST(LIBARITHMETIQUE_SO_VERSION)
AC_DEFINE(LIBARITHMETIQUE_SO_VERSION, [{lt_current, lt_revision, lt_age}],
          [Libtool compatibility version])

# Checks for libraries.
# Check for cppunit facility
PKG_CHECK_MODULES([LIBCPPUNIT], [cppunit >= 1.13])

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
echo LIBCPPUNIT_LIBS=\"$LIBCPPUNIT_LIBS\"
echo

AC_OUTPUT
