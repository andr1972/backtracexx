#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <list>
#include <string>

namespace backtracexx
{
	typedef std::list< void const* > raw_backtrace_type;
	typedef std::list< std::string > symbolic_backtrace_type;

	raw_backtrace_type scan();
	symbolic_backtrace_type symbols( raw_backtrace_type const& );
}

#endif
