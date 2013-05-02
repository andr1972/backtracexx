#include "backtracexx.hpp"

#include <csetjmp>
#include <csignal>
#include <iostream>
#include <iterator>
#include <stdexcept>

jmp_buf context;

void signalHandler( int signalNumber )
{
	backtracexx::Trace t = backtracexx::scan();
	for ( backtracexx::Trace::const_iterator i = t.begin(); i != t.end(); ++i )
	{
		backtracexx::Frame const& f = *i;
		std::printf( "0x%016lx : %s+0x%lx [%s]\n", f.address,
			( f.symbol.empty() ? "<unresolved symbol>" : f.symbol.c_str() ),
			f.displacement, f.module.c_str() );
	}
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
