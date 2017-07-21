CC=gcc
SOURCES = string.c cursor.c document.c input.c chedit.c
BIN = chedit
CFLAGS=-std=gnu99
LDFLAGS=-lncurses

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(BIN) $(LDFLAGS)

clean:
	rm -f chedit
