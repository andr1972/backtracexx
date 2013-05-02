#include "backtracexx.hpp"
#include <csetjmp>
#include <csignal>
#include <iostream>
#include <iterator>

jmp_buf context;

void signalHandler( int signalNumber )
{
	backtracexx::symbolic_backtrace_type s = backtracexx::symbols( backtracexx::scan() );
	std::copy(s.begin(), s.end(), std::ostream_iterator< std::string >( std::cout, "\n" ) );
	longjmp( context, 1 );
}

void zoo()
{
	volatile int* p = 0;
	*p = 0;
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
	if ( setjmp( context ) == 0 )
	{
		foo();
	}
	return 0;
}
