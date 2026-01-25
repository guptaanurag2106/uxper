CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2) 
LIBS = $(shell pkg-config --libs sdl2) -lm

main: boid.c main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

