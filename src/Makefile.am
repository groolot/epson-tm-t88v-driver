SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir)/src -Wall -Werror -Wextra -Wshadow -Wduplicated-cond -Wunused-parameter -Wsign-promo -Wconversion -Wsign-conversion -fstack-protector

lib_LTLIBRARIES = libarithmetique.la
libarithmetique_la_SOURCES = arithmetique.cc arithmetique.h
libarithmetique_la_LDFLAGS = -version-info @LIBARITHMETIQUE_SO_VERSION@

bin_PROGRAMS = projet-exemple
projet_exemple_SOURCES = main.cc
projet_exemple_LDFLAGS = -static-libtool-libs
projet_exemple_LDADD = libarithmetique.la
