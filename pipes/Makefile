CC=gcc

CFLAGS=-Wall -std=c11 -g

.PHONY: all

all: ex1 ex2

ex1: integral.c
	$(CC) $(CFLAGS) -o ex1 integral.c

ex2: integral_main.c integral_calculator.c
	$(CC) $(CFLAGS) -o ex2_1 integral_main.c
	$(CC) $(CFLAGS) -o ex2_2 integral_calculator.c


clean:
	rm -f ex1 ex2_1 ex2_2 report.txt