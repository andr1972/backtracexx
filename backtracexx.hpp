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
#else
#define BACKTRACEXX_EXPORT	__attribute__(( visibility( "default" ) ))
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

	BACKTRACEXX_EXPORT Trace scan();
	BACKTRACEXX_EXPORT std::ostream& operator << ( std::ostream&, Trace const& );
}

#endif
