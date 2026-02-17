CC = gcc
CFLAGS = -ggdb -Wall -Wextra

BIN = benchmark

all: $(BIN)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BIN)

.PHONY: all clean
