CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g -I../utils/raylib-5.5_linux_amd64/include -I../utils/
LDFLAGS=-L../utils/raylib-5.5_linux_amd64/lib -l:libraylib.a -lm

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm main

.PHONY: all clean
