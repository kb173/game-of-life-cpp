CXX = g++
CXXFLAGS = -Wall -O3

gol: main.o Timing.o
	$(CXX) $(CXXFLAGS) -o gol main.o Timing.o

main.o: main.cpp Timing.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Timing.o: Timing.h

clean :
	-rm *.o gol
