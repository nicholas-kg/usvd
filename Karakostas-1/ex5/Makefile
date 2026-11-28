CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread -lm

TARGETS = barrier barrier_cond barrier_sense barrier_tree

.PHONY: all run plot clean

all: $(TARGETS)

barrier: barrier.c
	$(CC) $(CFLAGS) barrier.c -o barrier $(LDFLAGS)

barrier_cond: barrier_cond.c
	$(CC) $(CFLAGS) barrier_cond.c -o barrier_cond $(LDFLAGS)

barrier_sense: barrier_sense.c
	$(CC) $(CFLAGS) barrier_sense.c -o barrier_sense $(LDFLAGS)

barrier_tree: barrier_tree.c
	$(CC) $(CFLAGS) barrier_tree.c -o barrier_tree $(LDFLAGS)

run: all
	chmod +x test.sh
	./test.sh

plot: results_barriers.csv
	python3 plot.py

clean:
	rm -f $(TARGETS) results_barriers.csv barriers_comparison.png