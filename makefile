
# makefile for rnd64

CC = gcc
CFLAGS = -O3 -Wall -Wextra -Wuninitialized -Wunused -Werror -std=gnu99 -s

rnd64: rnd64.o
	$(CC) $(CFLAGS) rnd64.o -lpthread -o rnd64

clean:
	rm -f rnd64.o
