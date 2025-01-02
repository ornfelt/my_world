CC = gcc

CFLAGS = -Ofast -march=native -g -Wall -Wextra

LDFLAGS = -lpng16

all: jpeg

jpeg: jpeg.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm jpeg

.PHONY: all clean
