CXX := g++
CXXFLAGS := -O1 -Wall -Werror -pedantic

all: example

example: example.cpp backtracexx.hpp backtracexx.cpp
	$(CXX) $(CXXFLAGS) -fpic backtracexx.cpp -o libbacktracexx.so -shared -s -ldl
	$(CXX) $(CXXFLAGS) example.cpp -o example -L. -lbacktracexx -s -Wl,--export-dynamic -Wl,-rpath,.

clean:
	rm -f *.o *.s *.ii example libbacktracexx.so
