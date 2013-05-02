CXX := g++
CXXFLAGS := -O1 -Wall -Werror -pedantic

all: example

example: example.cpp backtracexx.hpp backtracexx.cpp
	$(CXX) $(CXXFLAGS) -fpic backtracexx.cpp -o libbacktracexx.so -shared -s -ldl -static-libgcc
	$(CXX) $(CXXFLAGS) example.cpp -o example ./libbacktracexx.so -s -Wl,--export-dynamic

clean:
	rm -f *.o *.s *.ii example libbacktracexx.so
