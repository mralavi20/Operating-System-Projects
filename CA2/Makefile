CC = g++

all: main.out building.out bill.out

main.out: main.cpp defines.h
	$(CC) main.cpp -o main.out

building.out: building.cpp defines.h
	$(CC) building.cpp -o building.out

bill.out: bill.cpp defines.h
	$(CC) bill.cpp -o bill.out


clean:
	rm -f main.out building.out bill.out

.PHONY: all clean