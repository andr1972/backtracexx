# backtracexx : cross-platform backtrace generation on Linux/MacOS/Windows

This small library uses unwind information and produces backtrace with optionally demangled symbols.

The [original sources](http://svn.pld-linux.org/cgi-bin/viewsvn/backtracexx/) are no longer available, but were imported to [GitHub](https://github.com/borneq/backtracexx) by Andrzej Borucki.

## Building

Note: currently, a GCC flavor of C++ compiler is required (that is, the code will not compile with e.g. Clang):

```
mkdir build
cmake -DCMAKE_CXX_COMPILER=g++-11 ..
make
```

## Usage

The following example program produces the output below:

```c++
#include "backtracexx.hpp"

#include <csetjmp>
#include <csignal>
#include <iostream>

jmp_buf context;

void signalHandler( int signalNumber )
{
	std::cerr << backtracexx::scan();
	longjmp( context, 1 );
}

void zoo()
{
	if ( setjmp( context ) == 0 )
	{
		volatile int* p = 0;
		*p = 0;
	}
}

void bar( void ( *f )() )
{
	f();
}

void foo()
{
	bar( &zoo );
}

int main( int argc, char const* const* argv )
{
	signal( SIGSEGV, signalHandler );
	foo();
	return 0;
}
```

```
./backtracexx_example 
=== backtrace ====
     0x10c81c55c : backtracexx::scan[abi:cxx11](void*)+0x33 [/Users/dmikushin/backtracexx/build/libbacktracexx.dylib @ 0x10c81a000 ]
     0x10c811588 : signalHandler(int)+0x1d [/Users/dmikushin/backtracexx/build/./backtracexx_example @ 0x10c80f000 ]
  0x7fff6383b42d : _sigtramp+0x1d [/usr/lib/system/libsystem_platform.dylib @ 0x7fff63837000 ]
     0x10c811605 : zoo()+0x2d [/Users/dmikushin/backtracexx/build/./backtracexx_example @ 0x10c80f000 ]
     0x10c81161f : bar(void (*)())+0x12 [/Users/dmikushin/backtracexx/build/./backtracexx_example @ 0x10c80f000 ]
     0x10c811635 : foo()+0x13 [/Users/dmikushin/backtracexx/build/./backtracexx_example @ 0x10c80f000 ]
     0x10c811660 : main+0x28 [/Users/dmikushin/backtracexx/build/./backtracexx_example @ 0x10c80f000 ]
==================
```

