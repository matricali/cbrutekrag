.DEFAULT_GOAL := all

CC=gcc
CFLAGS=-Wall -lssh

cbrutekrag:
	$(CC) $(CFLAGS) -o cbrutekrag cbrutekrag.c
all: clean cbrutekrag

clean:
	rm -f cbrutekrag
