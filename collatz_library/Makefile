CC=gcc

CFLAGS=-Wall -std=c11 -g

.PHONY: all static shared dynamic

all: static shared dynamic

static: client.c l_collatz.a
	$(CC) $(CFLAGS) client.c -o static -L. l_collatz.a

l_collatz.a: collatz.o
	ar rcs l_collatz.a collatz.o

collatz.o: collatz.c collatz.h
	$(CC) $(CFLAGS) -c collatz.c

shared: client.c l_collatz.so
	$(CC) $(CFLAGS) client.c -o shared -L. l_collatz.so -Wl,-rpath=$PWD

l_collatz.so: collatz.o
	$(CC) -shared -o l_collatz.so collatz.o

dynamic: client.c l_collatz.so
	$(CC) $(CFLAGS) client.c -o dynamic -D DYNAMIC_DLOPEN -L. l_collatz.so

clean:
	rm -f static shared dynamic *.o *.a *.so

