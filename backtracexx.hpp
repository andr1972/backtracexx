#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <iosfwd>
#include <string>
#include <vector>

#if defined( WIN32 ) || defined( WIN64 )

#ifdef BACKTRACEXX_EXPORTS
#define BACKTRACEXX_EXPORT	__declspec( dllexport )
#else
#define BACKTRACEXX_EXPORT	__declspec( dllimport )
#endif

#include <windows.h>
#include <winnt.h>

#else

#define BACKTRACEXX_EXPORT	__attribute__(( visibility( "default" ) ))
typedef void* PEXCEPTION_POINTERS;

#endif

namespace backtracexx
{
	struct BACKTRACEXX_EXPORT Frame
	{
		Frame();

		unsigned long address;
		std::string symbol;
		unsigned long displacement;
		std::string module;
		bool signalTrampoline;
	};

	typedef std::vector< Frame > Trace;

	//
	//	ex == 0, scan() stack from current frame.
	//	ex != 0, scan() stack from specified context (e.g. passed from SEH handler).
	//
	BACKTRACEXX_EXPORT Trace scan( ::PEXCEPTION_POINTERS /* not used on linux */ ex = 0 );
	BACKTRACEXX_EXPORT std::ostream& operator << ( std::ostream&, Trace const& );
}

#endif
