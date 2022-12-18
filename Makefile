CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

# Every .c except stream.c → its own binary (auto-discovered)
SRCS = $(filter-out stream.c,$(wildcard *.c))
APPS = $(SRCS:.c=)

.PHONY: all clean

all: $(APPS)

$(APPS): %: %.o stream.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c stream.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(APPS)
