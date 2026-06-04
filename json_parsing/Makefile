CC=gcc
#CFLAGS=-Wall -Wextra -g -I../utils -I../../raybun/thirdparty/cJSON-1.7.19 #-fsanitize=address,undefined -fno-omit-frame-pointer 
CFLAGS=-O3 -march=native -funroll-loops -flto -ffunction-sections \
                 -fdata-sections -ffast-math -I../utils/ -I../../raybun/thirdparty/cJSON-1.7.19
LDFLAGS=-L../../raybun/thirdparty/cJSON-1.7.19/build/ -l:libcjson.so -Wl,-rpath,../../raybun/thirdparty/cJSON-1.7.19/build/

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean: 
	rm main

.PHONY: clean all
