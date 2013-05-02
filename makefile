# linux32 target
CXX32 := i486-gnu-linux-g++

# linux64 target
CXX64 := x86_64-gnu-linux-g++

CXXFLAGS := -Wall -Werror -pedantic -shared-libgcc -g2

all: example

example: example.cpp backtracexx.hpp backtracexx.cpp
	@mkdir -p bin-linux-{32,64}
	$(CXX32) $(CXXFLAGS) -fpic backtracexx.cpp -o bin-linux-32/libbacktracexx.so -shared -ldl
	$(CXX32) $(CXXFLAGS) example.cpp -o bin-linux-32/example -Lbin-linux-32 -lbacktracexx -Wl,--export-dynamic -Wl,-rpath,.
	$(CXX64) $(CXXFLAGS) -fpic backtracexx.cpp -o bin-linux-64/libbacktracexx.so -shared -ldl
	$(CXX64) $(CXXFLAGS) example.cpp -o bin-linux-64/example -Lbin-linux-64 -lbacktracexx -Wl,--export-dynamic -Wl,-rpath,.

clean:
	rm -rf bin-linux-{32,64}
