CFLAGS = -g

all: parse

parse: lex.yy.o y.tab.o ast.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ -c

lex.yy.c: parse.l y.tab.c
	flex $<

y.tab.c: parse.y
	yacc -d -v $<

.PHONY: all
