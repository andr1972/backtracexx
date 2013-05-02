#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <list>
#include <string>
#include <utility>

namespace backtracexx
{
	typedef std::pair< void const*,
		bool /* signal frame */ > unwind_point_type;

	typedef std::list< unwind_point_type > raw_backtrace_type;
	typedef std::list< std::string > symbolic_backtrace_type;

	raw_backtrace_type scan();
	symbolic_backtrace_type symbols( raw_backtrace_type const& );
}

#endif
