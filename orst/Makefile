CFLAGS=-Wall -Wextra -Wpedantic -O2 -ggdb -std=gnu17 -fopenmp -L../utils -lm
all: sort

sort: sort.c
	$(CC) $(CFLAGS) -o $@ $<

debug: sort
	gf2 sort

clean:
	rm sort

.PHONY: clean sort
