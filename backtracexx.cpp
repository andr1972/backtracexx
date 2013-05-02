#include "backtracexx.hpp"
#include <cxxabi.h>
#include <dlfcn.h>
#include <unwind.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace backtracexx
{
	namespace
	{
		unsigned char const* callpoint( unsigned char const* ip )
		{
			// not implemented yet.
			return ip;
		}

		_Unwind_Reason_Code helper( struct _Unwind_Context* ctx, void* arg )
		{
			_Unwind_Ptr ip = _Unwind_GetIP( ctx );
			reinterpret_cast< raw_backtrace_type* >( arg )->push_back(
				callpoint( reinterpret_cast< unsigned char const* >( ip ) ) );
			return _URC_NO_REASON;
		}
	}

	raw_backtrace_type scan()
	{
		raw_backtrace_type trace;
		_Unwind_Backtrace( helper, &trace );
		return trace;
	}

	symbolic_backtrace_type symbols( raw_backtrace_type const& bt )
	{
		std::ostringstream os;
		os.setf( std::ios_base::hex, std::ios_base::basefield );
		os.setf( std::ios_base::showbase );
		symbolic_backtrace_type sbt;
		for ( raw_backtrace_type::const_iterator i = bt.begin(), e = bt.end(); i != e; ++i )
		{
			os.str( std::string() );
			Dl_info info;
			if ( dladdr( *i, &info ) )
			{
				long offset = reinterpret_cast< long >( *i ) - reinterpret_cast< long >( info.dli_saddr );
				int status;
				char* demangled = abi::__cxa_demangle( info.dli_sname, 0, 0, &status );
				if ( status != -1 )
				{
					os	<< std::setw( 18 ) << *i << " : "
						<< ( ( status == 0 ) ? demangled : info.dli_sname )
						<< '+' << offset << " from " << info.dli_fname;
					if ( status == 0 )
						free( demangled );
				}
			}
			else
				os << std::setw( 18 ) << *i << " ??";
			sbt.push_back( os.str() );
		}
		return sbt;
	}
}
