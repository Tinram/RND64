
# makefile for rnd64

CC = gcc
CFLAGS = -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s
NAME = rnd64


$(NAME): $(NAME).o
	$(CC) $(CFLAGS) $(NAME).o -lpthread -o $(NAME)

install:
	sudo cp $(NAME) /usr/local/bin/$(NAME)
	@echo "Attempted to copy $(NAME) to /usr/local/bin"

clean:
	rm -f $(NAME).o
