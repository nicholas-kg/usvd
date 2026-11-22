CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread

TARGET = counter
SRC = counter.c

.PHONY: all run plot clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	chmod +x test.sh
	./test.sh

plot: results.csv
	python3 plot.py

clean:
	rm -f $(TARGET) results.csv *.png