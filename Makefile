CC=gcc
SOURCES = chedit.c
BIN = chedit
CFLAGS=-std=gnu99
LDFLAGS=-lncurses

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(BIN) $(LDFLAGS)

clean:
	rm -f chedit
