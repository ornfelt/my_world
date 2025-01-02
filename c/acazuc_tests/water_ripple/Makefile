CC = gcc

CFLAGS = -Ofast -march=native -Wall -Wextra

LDFLAGS = -lm -lglfw -lGL

all: water_ripple

water_ripple: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm water_ripple

.PHONY: all clean
