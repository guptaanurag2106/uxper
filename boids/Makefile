CC = gcc
# CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2) -O3 -march=native -flto -pipe -s
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2)
LIBS = $(shell pkg-config --libs sdl2) -lm

main: main.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

