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
		// extract caller's address from callee's return point.
		unsigned char const* caller( unsigned char const* ip )
		{
#if defined( __powerpc__ ) && !defined( __powerpc64__ )
			// powerpc64 not tested.
			ip -= 4;
#elif defined( __sparc__ )
			// the same for sparc v7/8/9.
			ip -= 8;
#elif defined( __alpha__ )
			ip -= 4;
#elif defined( __i386__ ) || defined( __x86_64__ )
			//
			// TODO:
			//	analysis of complex addressing forms (see intel/24319102.pdf).
			//	rework code to cover all cases.
			//
			// call, near, relative
			if ( ip[ -5 ] == 0xe8 )
				return ( ip - 5 );
			// call, near, absolute indirect
			if ( ip[ -2 ] == 0xff )
			{
				if ( ( ip[ -1 ] & 0xf8 ) == 0xd0 ) // call *%reg
					return ( ip - 2 );
				if ( ( ip[ -1 ] & 0xf8 ) == 0x10 ) // call *(%reg)
					return ( ip - 2 );
			}
#endif
			return ip;
		}

		_Unwind_Reason_Code helper( struct _Unwind_Context* ctx, void* arg )
		{
			int beforeInsn = 0;
			_Unwind_Ptr ip = _Unwind_GetIPInfo( ctx, &beforeInsn );
			unwind_point_type up( reinterpret_cast< unsigned char const* >( ip ), beforeInsn );
			if ( !beforeInsn )
				up.first = caller( reinterpret_cast< unsigned char const* >( up.first ) );
			reinterpret_cast< raw_backtrace_type* >( arg )->push_back( up );
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
			unwind_point_type up = *i;
			os << std::setw( 18 ) << up.first << " : ";
			if ( dladdr( const_cast< void* >( up.first ), &info ) )
			{
				if ( !info.dli_saddr )
					// the image containing address is found, but no nearest symbol was found.
					os << "??";
				else
				{
					int status;
					char* demangled = abi::__cxa_demangle( info.dli_sname, 0, 0, &status );
					if ( status != -1 )
					{
						long offset = reinterpret_cast< long >( up.first ) - reinterpret_cast< long >( info.dli_saddr );
						os << ( ( status == 0 ) ? demangled : info.dli_sname ) << '+' << offset;
						if ( status == 0 )
							free( demangled );
					}
				}
				if ( info.dli_fname && strlen( info.dli_fname ) )
					os << " from " << info.dli_fname;
			}
			else
				os << "??";
			if ( up.second )
				os << " [signal frame]";
			sbt.push_back( os.str() );
		}
		return sbt;
	}
}
