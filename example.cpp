#include "backtracexx.hpp"

#include <csignal>
#include <iostream>
#include <iterator>
#include <stdexcept>

void signalHandler( int signalNumber )
{
	backtracexx::symbolic_backtrace_type s = backtracexx::symbols( backtracexx::scan() );
	std::copy( s.begin(), s.end(), std::ostream_iterator< std::string >( std::cout, "\n" ) );
	throw std::runtime_error( "fatality." );
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
