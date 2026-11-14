CC = gcc
CFLAGS = -Wall -O2 -pthread
TARGET = matrix_pow
SRC = matrix_pow

.PHONY all clean

all: $(TARGET)

$(TARGET) : $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
clean:
	rm -f $(TARGET)