CC = g++ -std=c++11

ImageFilter.out: main.o bmp.o filter.o
	$(CC) main.o bmp.o filter.o -o ImageFilter.out

main.o: main.cpp bmp.h filter.h
	$(CC) -c main.cpp

bmp.o: bmp.h bmp.cpp
	$(CC) -c bmp.cpp

filter.o: filter.h filter.cpp bmp.h
	$(CC) -c filter.cpp

.PHONY: clean

clean:
	rm *.o