CXX := g++
CXXFLAGS := -O1 -Wall -Werror -pedantic
LDXXFLAGS := -Wl,-export-dynamic -s -ldl -static-libgcc

all: example

example: example.cpp backtracexx.hpp backtracexx.cpp
	$(CXX) $(CXXFLAGS) backtracexx.cpp -c
	$(CXX) $(CXXFLAGS) example.cpp -c
	$(CXX) example.o backtracexx.o -o example $(LDXXFLAGS)

clean:
	rm -f *.o *.s *.ii example
