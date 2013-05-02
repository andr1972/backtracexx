#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <iosfwd>
#include <string>
#include <list>

#if defined( WIN32 ) || defined( WIN64 )

#ifdef BACKTRACEXX_EXPORTS
#define BACKTRACEXX_EXPORT	__declspec( dllexport )
#else
#define BACKTRACEXX_EXPORT	__declspec( dllimport )
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
		explicit Frame( void const* address );

		void const* address;
		std::string symbol;
		long displacement;
		std::string moduleName;
		void const* moduleBaseAddress;
		std::string fileName;
		long lineNumber;
	};

	typedef std::list< Frame > Trace;

	//
	//	ctx == 0, scan() stack from current frame.
	//	ctx != 0, scan() stack from specified context (e.g. passed from SEH handler).
	//
	BACKTRACEXX_EXPORT Trace scan( ::PCONTEXT /* not used on linux */ ctx = 0 );
	BACKTRACEXX_EXPORT bool lookupSymbol( Frame& );
	BACKTRACEXX_EXPORT std::ostream& operator << ( std::ostream&, Trace const& );
}

#endif
