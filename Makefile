CXX = g++
CXXFLAGS = -Wall

gol: main.o
	$(CXX) $(CXXFLAGS) -o gol main.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean :
	-rm *.o gol
