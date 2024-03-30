CC=gcc

CFLAGS=-Wall -std=c11 -g

.PHONY: all


all: ex1 ex2 run clean


DEF_N=10
DEF_DIR=.

ex1: ex1.c
	$(CC) $(CFLAGS) -o ex1 ex1.c
run_ex1: ex1
	./ex1 $(DEF_N)

ex2:
	$(CC) $(CFLAGS) -o ex2 ex2.c
run_ex2: ex2
	./ex2 $(DEF_DIR)


run: run_ex1 run_ex2

clean:
	rm -f ex1 ex2