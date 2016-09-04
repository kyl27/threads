CC = g++-6
CFLAGS = -O2 -ggdb -Wall -fopenmp -pthread
LFLAGS =


all: main

main: main.cpp
	$(CC) $(CFLAGS) $(LFLAGS) -o main main.cpp

clean:
	rm -f *.o main

tar:
	tar -czvf threading.tar.gz *.h *.cpp
