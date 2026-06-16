CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
TARGET = cat

all: $(TARGET)

$(TARGET): cat.c
	$(CC) $(CFLAGS) -o $(TARGET) cat.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
