CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O0 -pthread
TARGET = sleepingStylistMon
OBJS = sleepingStylistMon.o monitor.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

sleepingStylistMon.o: sleepingStylistMon.c monitor.h
	$(CC) $(CFLAGS) -c sleepingStylistMon.c

monitor.o: monitor.c monitor.h
	$(CC) $(CFLAGS) -c monitor.c

clean:
	rm -f $(TARGET) $(OBJS)
