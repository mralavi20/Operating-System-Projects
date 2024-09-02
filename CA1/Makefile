CC = gcc
FLAG = -l cjson

all: r.out c.out

r.out: restaurant.c defines.h
	$(CC) restaurant.c -l cjson -o r.out

c.out: customer.c defines.h
	$(CC) customer.c -l cjson -o c.out

clean:
	rm -f r.out c.out

.PHONY: all clean