AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4

test_ldflags = -lCppUTest -lCppUTestExt
test_cxxflags = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..
test_cflags = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..

check_PROGRAMS = test_history_entries test_history
test_history_entries_SOURCES = AllTests.cpp test_history_entries.cpp ../history_entries.c
test_history_entries_LDFLAGS = $(test_ldflags)
test_history_entries_CXXFLAGS = $(test_cxxflags)
test_history_entries_CFLAGS = $(test_cflags)

test_history_SOURCES = AllTests.cpp test_history.cpp ../history_entries.c ../history.c
test_history_LDFLAGS = $(test_ldflags)
test_history_CXXFLAGS = $(test_cxxflags)
test_history_CFLAGS = $(test_cflags)

TESTS = $(check_PROGRAMS)

