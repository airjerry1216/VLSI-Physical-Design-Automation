CC = g++
LIBS = -std=c++11 -O3
OBJS = main.o fmlib.o
hw2: $(OBJS)
	$(CC) $(OBJS) -o ../bin/hw2
main.o: main.cpp fmlib.h
	$(CC) -c main.cpp $(LIBS)
fmlib.o: fmlib.cpp fmlib.h
	$(CC) -c fmlib.cpp $(LIBS)
clean:
	rm *.o
