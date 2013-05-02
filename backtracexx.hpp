#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <iosfwd>
#include <string>
#include <vector>

#if defined( WIN32 ) || defined( WIN64 )
#ifdef BACKTRACEXX_EXPORTS
#define DEBUGTOOLS_EXPORT	__declspec( dllexport )
#else
#define DEBUGTOOLS_EXPORT	__declspec( dllimport )
#endif
#else
#define DEBUGTOOLS_EXPORT	__attribute__(( visibility( "default" ) ))
#endif

namespace backtracexx
{
	struct DEBUGTOOLS_EXPORT Frame
	{
		Frame();

		unsigned long address;
		std::string symbol;
		unsigned long displacement;
		std::string module;
		bool signalTrampoline;
	};

	typedef std::vector< Frame > Trace;

	DEBUGTOOLS_EXPORT Trace scan();
	DEBUGTOOLS_EXPORT std::ostream& operator << ( std::ostream&, Trace const& );
}

#endif
