CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
TARGET = echo

all: $(TARGET)

$(TARGET): echo.c
	$(CC) $(CFLAGS) -o $(TARGET) echo.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
