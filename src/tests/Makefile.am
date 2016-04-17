AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4

check_PROGRAMS = tests
tests_SOURCES = AllTests.cpp test_history_entries.cpp ../history_entries.c
tests_LDFLAGS = -lCppUTest -lCppUTestExt
tests_CXXFLAGS = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..
tests_CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..
TESTS = $(check_PROGRAMS)

