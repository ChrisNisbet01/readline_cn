AUTOMAKE_OPTIONS = foreign
lib_LTLIBRARIES = libeditline.la
libeditline_la_SOURCES = editline.c \
						word_completion.c \
						editline_context.c \
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
						history_entries.c
						
libeditline_la_CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wno-unused-result

