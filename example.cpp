#include "backtracexx.hpp"
#include <iostream>
#include <iterator>

void zoo()
{
	backtracexx::symbolic_backtrace_type s = backtracexx::symbols( backtracexx::scan() );
	std::copy(s.begin(), s.end(), std::ostream_iterator< std::string >( std::cout, "\n" ) );
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
	foo();
	return 0;
}
