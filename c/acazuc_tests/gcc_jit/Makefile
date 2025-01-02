CC = gcc

CFLAGS = -Ofast -march=native -g

LDFLAGS = -lgccjit

all: gcc_jit

gcc_jit: gcc_jit.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm gcc_jit

.PHONY: all clean
