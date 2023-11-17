CC=gcc
CFLAGS=-std=c99 -w

all : process_memory zombie parallel_sum utils.c

zombie : zombie.c
	$(CC) -o $@ zombie.c $(CFLAGS)

process_memory : process_memory.c
	$(CC) -o $@ process_memory.c $(CFLAGS)

parallel_sum : parallel_sum.c utils.c
	$(CC) -o $@ parallel_sum.c utils.c $(CFLAGS) -pthread

clean:
	rm process_memory zombie parallel_sum