SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir)/src -Wall -Werror -Wextra -Wshadow -Wduplicated-cond -Wunused-parameter -Wsign-promo -Wconversion -Wsign-conversion

TESTS = test-addition test-soustraction test-multiplication test-division
check_PROGRAMS = $(TESTS)

test_addition_SOURCES = addition_test.h addition_test.cc
test_addition_CPPFLAGS = $(LIBCPPUNIT_CFLAGS)
test_addition_LDFLAGS = $(LIBCPPUNIT_LIBS) -no-install
test_addition_LDADD = ../src/libarithmetique.la

test_soustraction_SOURCES = soustraction_test.h soustraction_test.cc
test_soustraction_CPPFLAGS = $(LIBCPPUNIT_CFLAGS)
test_soustraction_LDFLAGS = $(LIBCPPUNIT_LIBS) -no-install
test_soustraction_LDADD = ../src/libarithmetique.la

test_multiplication_SOURCES = multiplication_test.h multiplication_test.cc
test_multiplication_CPPFLAGS = $(LIBCPPUNIT_CFLAGS)
test_multiplication_LDFLAGS = $(LIBCPPUNIT_LIBS) -no-install
test_multiplication_LDADD = ../src/libarithmetique.la

test_division_SOURCES = division_test.h division_test.cc
test_division_CPPFLAGS = $(LIBCPPUNIT_CFLAGS)
test_division_LDFLAGS = $(LIBCPPUNIT_LIBS) -no-install
test_division_LDADD = ../src/libarithmetique.la
