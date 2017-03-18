
# makefile for rnd64_st

CC = gcc
CFLAGS = -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s
NAME = rnd64_st


$(NAME): $(NAME).o
	$(CC) $(CFLAGS) $(NAME).o -o $(NAME)

clean:
	rm -f $(NAME).o
