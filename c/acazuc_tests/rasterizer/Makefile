CC = gcc

CFLAGS = -Ofast -march=native -g -Wall -Wextra -Wshadow

LDFLAGS = -lpng -lm

all: rasterizer

rasterizer: rasterizer.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm rasterizer

.PHONY: all clean
