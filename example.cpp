#include "backtracexx.hpp"

#include <csetjmp>
#include <csignal>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <pthread.h>

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

void* foo( void* )
{
	bar( &zoo );
	pthread_exit( 0 );
	return 0;
}

int main( int argc, char const* const* argv )
{
	signal( SIGSEGV, signalHandler );
	if ( argc > 1 )
	{
		pthread_t t;
		pthread_create( &t, 0, &foo, 0 );
		pthread_join( t, 0 );
	}
	else
	{	
		foo( 0 );
	}
	return 0;
}
