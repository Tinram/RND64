
# makefile for RND64

CC = gcc

CFLAGS = -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -Wformat=2 -Wno-unused-parameter -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -std=gnu99 -s

NAME = rnd64

BINDIR = bin/


$(NAME): $(NAME).o
	$(CC) $(CFLAGS) $(NAME).o -lpthread -o $(BINDIR)$(NAME)

install:
	sudo cp $(BINDIR)$(NAME) /usr/local/bin/$(NAME)
	@echo "Attempted to copy $(NAME) to /usr/local/bin"

clean:
	rm -f $(NAME).o
