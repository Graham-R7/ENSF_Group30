# Makefile for Lab03 - Group30

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Default target
all: shell.out

# Shell executable
shell.out: shell.c parser.c parser.h
	$(CC) $(CFLAGS) -o $@ shell.c parser.c

test.out: test.c parser.c parser.h
	$(CC) $(CFLAGS) -o $@ test.c parser.c

clean:
	rm -f shell.out test.out
