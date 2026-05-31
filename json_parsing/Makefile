CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean: 
	rm main

.PHONY: clean all
