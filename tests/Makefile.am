SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir)/src -Wall -Werror -Wextra -Wshadow -Wduplicated-cond -Wunused-parameter -Wsign-promo -Wconversion -Wsign-conversion

TESTS = testarithmetique
check_PROGRAMS = $(TESTS)
testarithmetique_SOURCES = testarithmetique.cc \
	addition_test.h addition_test.cc \
	soustraction_test.h soustraction_test.cc \
	multiplication_test.h multiplication_test.cc \
	division_test.h division_test.cc
testarithmetique_CPPFLAGS = $(LIBCPPUNIT_CFLAGS)
testarithmetique_LDFLAGS = $(LIBCPPUNIT_LIBS) -no-install
testarithmetique_LDADD = ../src/libarithmetique.la
