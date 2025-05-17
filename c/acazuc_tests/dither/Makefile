all: dither

dither: dither.c
	$(CC) $(CFLAGS) -o $@ $< -lpng

clean:
	rm dither

.PHNOY: all clean
