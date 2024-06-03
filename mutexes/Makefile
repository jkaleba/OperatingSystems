CC = gcc
CFLAGS = -Wall

all: santa run clean

santa: santa.c
	$(CC) $(CFLAGS) santa.c -o santa -lncursesw

run: santa
	./santa

clean:
	rm -f santa
