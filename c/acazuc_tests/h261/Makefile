CC = gcc

CFLAGS = -Ofast -march=native -g -Wall -Wextra -Wshadow

LDFLAGS = -lpng -lglfw -lGL

all: h261

h261: h261.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm h261

.PHONY: all clean
