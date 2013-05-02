CXX := g++
CXXFLAGS += -Wall -Werror -pedantic

all: libbacktracexx.so example

libbacktracexx.so: backtracexx.hpp backtracexx.cpp
	$(CXX) backtracexx.cpp -o libbacktracexx.so -shared -ldl $(CXXFLAGS) \
	-O3 -fpic -funwind-tables -fno-exceptions -fno-rtti

example: example.cpp libbacktracexx.so
	$(CXX) example.cpp -o example ./libbacktracexx.so $(CXXFLAGS) \
	-O1 -funwind-tables -Wl,-export-dynamic

clean:
	rm -f libbacktracexx.so example
