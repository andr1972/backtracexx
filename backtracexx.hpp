#ifndef backtracexx_hpp
#define backtracexx_hpp

#include <string>
#include <vector>

namespace backtracexx
{
	struct Frame
	{
		Frame();

		unsigned long address;
		std::string symbol;
		unsigned long displacement;
		std::string module;
		bool signalTrampoline;
	};

	typedef std::vector< Frame > Trace;

	Trace scan();
}

#endif
