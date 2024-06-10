CC=gcc

CFLAGS=-Wall -std=c11 -g

.PHONY: all

all: compile run clean

compile: semaphores.c
	$(CC) $(CFLAGS) -o semaphores semaphores.c

run: semaphores
	./semaphores


clean:
	rm -f semaphores