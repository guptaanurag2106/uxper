CC=gcc
#CFLAGS=-Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS=-O3 -march=native -funroll-loops -flto -ffunction-sections \
                 -fdata-sections -ffast-math

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean: 
	rm main

.PHONY: clean all
