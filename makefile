CXX := g++
CXXFLAGS := -O1 -Wall -Werror -pedantic -shared-libgcc

all: example

example: example.cpp backtracexx.hpp backtracexx.cpp
	@mkdir bin-linux
	$(CXX) $(CXXFLAGS) -fpic backtracexx.cpp -o bin-linux/libbacktracexx.so -shared -s -ldl
	$(CXX) $(CXXFLAGS) example.cpp -o bin-linux/example -Lbin-linux/ -lbacktracexx -s -Wl,--export-dynamic -Wl,-rpath,.

clean:
	rm -rf bin-linux
