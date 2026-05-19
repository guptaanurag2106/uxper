CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -I../utils/libdeflate/ -O3
LDFLAGS = -L../utils/libdeflate/build/ -ldeflate  -Wl,-rpath=../utils/libdeflate/build
SRC = main.c
INCLUDES = utils.h
TARGET = main


$(TARGET): $(SRC) $(INCLUDES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

