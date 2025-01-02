CC = gcc

CFLAGS = -Ofast -march=native -g

LDFLAGS = -lGL -lglfw -lm

all: equalizer

equalizer: equalizer.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm equalizer

.PHONY: all clean
