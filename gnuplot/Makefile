CFLAGS=-Wall -Wextra -Wpedantic -O2 -ggdb -std=gnu17 -fopenmp -L../utils -lm
all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm main

.PHONY: clean main
