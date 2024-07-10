CC=gcc

CFLAGS=-Wall -std=c11 -g

.PHONY: all

all: compile

compile: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c


clean:
	rm -f server client