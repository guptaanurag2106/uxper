CC = gcc
# CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2) -O3 -march=native -flto -pipe -s -fopenmp
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2) -fopenmp
LIBS = $(shell pkg-config --libs sdl2) -lm

main: boid.c main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

