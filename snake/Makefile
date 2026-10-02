CC=g++
CFLAGS=-Wall -Wextra -g $(shell pkg-config --cflags sdl3)  $(shell pkg-config --cflags sdl3-ttf)
LIBS=$(shell pkg-config --libs sdl3) $(shell pkg-config --libs sdl3-ttf)

main: main.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
