CC = gcc
CFLAGS = -Wall -Wextra -pedantic $(shell pkg-config --cflags sdl2) -O3
LIBS = $(shell pkg-config --libs sdl2) -lm

main: main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

