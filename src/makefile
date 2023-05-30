CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
HEADERS = $(wildcard *.h)
TARGET = ccsds

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)