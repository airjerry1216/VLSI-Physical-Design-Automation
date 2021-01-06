CC = g++
LIBS = -std=c++11 -O3
OBJS = main.o lib.o
hw2: $(OBJS)
	$(CC) $(OBJS) -o ../bin/hw3
main.o: main.cpp lib.h
	$(CC) -c main.cpp $(LIBS)
lib.o: lib.cpp lib.h
	$(CC) -c lib.cpp $(LIBS)
clean:
	rm *.o
	rm ../bin/*