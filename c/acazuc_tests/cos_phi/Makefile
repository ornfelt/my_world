CFLAGS = -Ofast -march=native -g

LDFLAGS = -lGL -lglfw -lm

all: cos_phi

cos_phi: cos_phi.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm cos_phi

.PHONY: all clean
