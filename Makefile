CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

TARGET = myprogram

all: $(TARGET)

$(TARGET): main.o stream.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o stream.o

main.o: main.c stream.h
	$(CC) $(CFLAGS) -c main.c

stream.o: stream.c stream.h
	$(CC) $(CFLAGS) -c stream.c

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
