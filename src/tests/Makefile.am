AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4

test_ldflags = -lCppUTest -lCppUTestExt
test_cxxflags = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..
test_cflags = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result -I..

check_PROGRAMS = test_history_entries test_history test_common_prefix_length test_strdup_partial test_split_path

test_history_entries_SOURCES = AllTests.cpp test_history_entries.cpp ../history_entries.c
test_history_entries_LDFLAGS = $(test_ldflags)
test_history_entries_CXXFLAGS = $(test_cxxflags)
test_history_entries_CFLAGS = $(test_cflags)

test_history_SOURCES = AllTests.cpp test_history.cpp ../history_entries.c ../history.c
test_history_LDFLAGS = $(test_ldflags)
test_history_CXXFLAGS = $(test_cxxflags)
test_history_CFLAGS = $(test_cflags)

test_common_prefix_length_SOURCES = AllTests.cpp test_common_prefix_length.cpp ../common_prefix_length.c
test_common_prefix_length_LDFLAGS = $(test_ldflags)
test_common_prefix_length_CXXFLAGS = $(test_cxxflags)
test_common_prefix_length_CFLAGS = $(test_cflags)

test_strdup_partial_SOURCES = AllTests.cpp test_strdup_partial.cpp ../strdup_partial.c
test_strdup_partial_LDFLAGS = $(test_ldflags)
test_strdup_partial_CXXFLAGS = $(test_cxxflags)
test_strdup_partial_CFLAGS = $(test_cflags)

test_split_path_SOURCES = AllTests.cpp test_split_path.cpp ../split_path.c ../strdup_partial.c
test_split_path_LDFLAGS = $(test_ldflags)
test_split_path_CXXFLAGS = $(test_cxxflags)
test_split_path_CFLAGS = $(test_cflags)

TESTS = $(check_PROGRAMS)

