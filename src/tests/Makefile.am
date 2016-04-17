AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4

test_cxxflags = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..

AM_CXXFLAGS := $(AM_CXXFLAGS) $(test_cxxflags)
AM_LDFLAGS = -lCppUTest -lCppUTestExt

check_PROGRAMS = \
			test_history_entries \
			test_history \
			test_common_prefix_length \
			test_strdup_partial \
			test_split_path \
			test_tokenise

test_history_entries_SOURCES = AllTests.cpp test_history_entries.cpp ../history_entries.c

test_history_SOURCES = AllTests.cpp test_history.cpp ../history_entries.c ../history.c

test_common_prefix_length_SOURCES = AllTests.cpp test_common_prefix_length.cpp ../common_prefix_length.c

test_strdup_partial_SOURCES = AllTests.cpp test_strdup_partial.cpp ../strdup_partial.c

test_split_path_SOURCES = AllTests.cpp test_split_path.cpp ../split_path.c ../strdup_partial.c

test_tokenise_SOURCES = AllTests.cpp test_tokenise.cpp ../tokenise.c ../strdup_partial.c

TESTS = $(check_PROGRAMS)

