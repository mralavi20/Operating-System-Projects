CC = g++ -std=c++11
FLAGS = -lpthread
COMPILE_FLAGS = $(FLAGS) -c

ImageFilter.out: main.o bmp.o filter.o
	$(CC) main.o bmp.o filter.o $(FLAGS) -o ImageFilter.out

main.o: main.cpp bmp.h filter.h
	$(CC) $(COMPILE_FLAGS) main.cpp

bmp.o: bmp.h bmp.cpp
	$(CC) $(COMPILE_FLAGS) bmp.cpp

filter.o: filter.h filter.cpp bmp.h
	$(CC) $(COMPILE_FLAGS) filter.cpp

.PHONY: clean

clean:
	rm *.o