CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O0 -pthread
TARGET = sleepingStylistSem

.PHONY: all clean

all: $(TARGET)

$(TARGET): sleepingStylistSem.c
	$(CC) $(CFLAGS) -o $(TARGET) sleepingStylistSem.c

clean:
	rm -f $(TARGET) *.o
