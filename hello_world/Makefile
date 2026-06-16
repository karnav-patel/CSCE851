CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
TARGET = hello_world

all: $(TARGET)

$(TARGET): hello_world.c
	$(CC) $(CFLAGS) -o $(TARGET) hello_world.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
