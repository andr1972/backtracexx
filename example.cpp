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
