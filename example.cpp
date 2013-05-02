#include "backtracexx.hpp"

#include <csetjmp>
#include <csignal>
#include <iostream>
#include <iterator>
#include <stdexcept>

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
	throw std::runtime_error( "still alive?" );
}

void bar( void ( *f )() )
{
	f();
}

void foo()
{
	bar( &zoo );
}

int main()
{
	signal( SIGSEGV, signalHandler );
	try
	{
		foo();
	}
	catch ( std::exception const& e )
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
