CC=gcc
CFLAGS=-ggdb -Wextra -Wall

all: xtea

xtea: xtea.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm xtea

.PHONY:
	clean all
