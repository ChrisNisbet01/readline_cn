AUTOMAKE_OPTIONS = foreign subdir-objects

test_cxxflags = -D_GNU_SOURCE -D_UNIT_TEST -Wall -Werror -Wno-unused-result -I..
test_cflags = -D_GNU_SOURCE -D_UNIT_TEST

AM_CXXFLAGS := $(AM_CXXFLAGS) $(test_cxxflags)
AM_CFLAGS := $(AM_CFLAGS) $(test_cflags)
AM_LDFLAGS = -lCppUTest -lCppUTestExt

check_PROGRAMS = \
			test_history_entries \
			test_history \
			test_common_prefix_length \
			test_strdup_partial \
			test_split_path \
			test_tokenise \
			test_directory \
			test_readline

test_history_entries_SOURCES = AllTests.cpp test_history_entries.cpp ../history_entries.c

test_history_SOURCES = AllTests.cpp test_history.cpp ../history_entries.c ../history.c

test_common_prefix_length_SOURCES = AllTests.cpp test_common_prefix_length.cpp ../common_prefix_length.c

test_strdup_partial_SOURCES = AllTests.cpp test_strdup_partial.cpp ../strdup_partial.c

test_split_path_SOURCES = AllTests.cpp test_split_path.cpp ../split_path.c ../strdup_partial.c

test_tokenise_SOURCES = AllTests.cpp test_tokenise.cpp ../tokenise.c ../strdup_partial.c

test_directory_SOURCES = AllTests.cpp test_directory.cpp ../directory.c

test_readline_SOURCES = AllTests.cpp test_readline.cpp \
						../readline.c \
						../word_completion.c \
						../help.c \
						../readline_context.c \
						../filename_completion.c \
						../args.c \
						../terminal.c \
						../directory.c \
						../split_path.c \
						../strdup_partial.c \
						../line_context.c \
						../common_prefix_length.c \
						../tokenise.c \
						../print_words_in_columns.c \
						../history.c \
						../history_entries.c \
						../handlers.c \
						../read_char.c \
						../terminal_cursor.c

#For some reason I need to specify these flags here to get the UNIT_TEST define to work.
test_directory_CXXFLAGS := $(AM_CXXFLAGS) $(test_cxxflags)
test_directory_CFLAGS := $(AM_CFLAGS) $(test_cflags)

TESTS = $(check_PROGRAMS)

