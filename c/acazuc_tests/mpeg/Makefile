CC = gcc

CFLAGS = -Ofast -march=native -g -Wall -Wextra

all: mpeg

jpeg: mpeg.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm mpeg

.PHONY: all clean
