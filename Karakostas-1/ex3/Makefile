CC = gcc
CFLAGS = -Wall -O2 -pthread
TARGET = array_stats
SRC = array_stats.c

.PHONY: all clean

all: $(TARGET)

$(TARGET) : $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
clean:
	rm -f $(TARGET)