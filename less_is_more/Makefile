CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -I./libdeflate/
LDFLAGS = -L./libdeflate/build/ -ldeflate
SRC = main.c
INCLUDES = utils.h
TARGET = main


$(TARGET): $(SRC) $(INCLUDES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

