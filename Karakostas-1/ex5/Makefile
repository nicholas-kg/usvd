CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -pthread 
LDLIBS = -lm

all: pthread_barrier cond_barrier barrier_sense 

pthread_barrier: barrier.c 
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

cond_barrier: barrier_cond.c 
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

barrier_sense: sense_barrier.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f pthread_barrier cond_barrier barrier_sense

.PHONY: all clean