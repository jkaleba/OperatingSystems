CC=gcc

CFLAGS=-Wall -std=c11 -g

.PHONY: all flip

SOURCE_DIR=source_files
DEST_DIR=destination_files

all: flip run


flip: flipper.c
	$(CC) $(CFLAGS) -o flipper flipper.c

run:
	./flipper $(SOURCE_DIR) $(DEST_DIR)

clean:
	rm -f flipper
	rm -rf $(DEST_DIR)/*
	rmdir $(DEST_DIR)

