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
						read_char.c
						
libreadline_cn_la_CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result
SUBDIRS = tests

