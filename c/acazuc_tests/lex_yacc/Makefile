CFLAGS = -O2 -g -Wall -Wextra -Wshadow

all: glsl

glsl: lex.glsl.o y.glsl.o ast.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ -c

lex.glsl.c: glsl.l y.glsl.c y.glsl.h
	flex -o $@ --header-file=lex.glsl.h $<

y.glsl.c y.glsl.h: glsl.y ast.h
	yacc -d -o y.glsl.c --defines=y.glsl.h $<

clean:
	rm -f y.glsl.c y.glsl.o y.glsl.h glsl lex.glsl.c lex.glsl.o lex.glsl.h

.PHONY: all clean
