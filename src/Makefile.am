AUTOMAKE_OPTIONS = foreign
lib_LTLIBRARIES = libreadline_cn.la
libreadline_cn_la_SOURCES = readline.c \
						word_completion.c \
						help.c \
						readline_context.c \
						filename_completion.c \
						args.c \
						terminal.c \
						directory.c \
						split_path.c \
						strdup_partial.c \
						line_context.c \
						common_prefix_length.c \
						tokenise.c \
						print_words_in_columns.c \
						history.c \
						history_entries.c \
						handlers.c \
						read_char.c \
						terminal_cursor.c
EXTRA_DIST = \
						args.h \
						history.h \
						split_path.h \
						common_prefix_length.h \
						line_context.h \
						strdup_partial.h \
						directory.h \
						print_words_in_columns.h \
						terminal_cursor.h \
						directory_test.h \
						read_char.h \
						terminal.h \
						handlers.h \
						readline_context.h \
						tokenise.h \
						help.h \
						readline.h \
						utils.h \
						history_entries.h \
						readline_status.h \
						word_completion.h


libreadline_cn_la_CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wextra -Wunused-variable
SUBDIRS = tests

