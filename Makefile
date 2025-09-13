objects=csv_main.o csv.o
cc=g++ -Wall -Werror -std=c++14

all: main.out

main.out: $(objects)
	$(cc) -o main.out $(objects)

csv.o: csv.cpp csv.h
	$(cc) -c csv.cpp -g -o csv.o

csv_main.o: csv_main.cpp csv.h
	$(cc) -c csv_main.cpp -g -o csv_main.o

clean:
	rm -rf *.o *.out