#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <boost/logic/tribool.hpp>
#include <iosfwd>
#include <string>
#include <list>

#if defined( WIN32 ) || defined( WIN64 )

#ifdef _DLL	//	defined when /MD or /MDd (Multithread DLL) is specified.

#ifdef BACKTRACEXX_EXPORTS
#define BACKTRACEXX_EXPORT	__declspec( dllexport )
#else
#define BACKTRACEXX_EXPORT	__declspec( dllimport )
#endif

#else
#define BACKTRACEXX_EXPORT
#endif

struct _CONTEXT;
typedef struct _CONTEXT* PCONTEXT;

#else

#define BACKTRACEXX_EXPORT	__attribute__(( visibility( "default" ) ))
typedef void* PCONTEXT;

#endif

namespace backtracexx
{
	struct BACKTRACEXX_EXPORT Frame
	{
		Frame();

		unsigned long address;
		std::string symbol;
		unsigned long displacement;
		std::string moduleName;
		unsigned long moduleBaseAddress;
		std::string fileName;
		unsigned long lineNumber;
		boost::logic::tribool signalTrampoline;
	};

	typedef std::list< Frame > Trace;

	//
	//	ex == 0, scan() stack from current frame.
	//	ex != 0, scan() stack from specified context (e.g. passed from SEH handler).
	//
	BACKTRACEXX_EXPORT Trace scan( ::PCONTEXT /* not used on linux */ ctx = 0 );
	BACKTRACEXX_EXPORT std::ostream& operator << ( std::ostream&, Trace const& );
}

#endif
