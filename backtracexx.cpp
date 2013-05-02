#include "backtracexx.hpp"
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <cstring>

//
//	TODO:
//	- use libdwarf for printing line info for ELF objects.
//

#if defined( __GNUC__ )
#include <cxxabi.h>
#include <dlfcn.h>
#include <unwind.h>
#elif defined( _MSC_VER )
#pragma warning( disable : 4311 )	//	'reinterpret_cast' : pointer truncation from 'PVOID' to 'unsigned long'
#pragma warning( disable : 4312 )	//	'reinterpret_cast' : conversion from 'unsigned long' to 'void*' of greater size
#include <dbghelp.h>
//
//	please use a recent dbghelp.dll because older versions
//	have unexpected problems with symbols resolving, e.g.
//	::SymGetSymFromAddr() produces ERROR_INVALID_ADDRESS.
//
//	this code works fine with:
//	- dbghelp.dll v5.1.2600.2180 from WinXP/SP2.
//	- dbghelp.dll v6.5.0003.7 from Visual C++ 2005 Express Edition.
//
//	this code doesn't work with:
//	- dbghelp.dll v5.00.2195.6613 from Win2000/SP4.
//
#pragma comment( lib, "dbghelp" )
#endif

namespace backtracexx
{
	namespace
	{

#if defined( __GNUC__ )

		bool lookupSymbol( Frame& frame )
		{
			Dl_info info;
			if ( ::dladdr( reinterpret_cast< void* >( frame.address ), &info ) )
			{
				frame.moduleBaseAddress = reinterpret_cast< unsigned long >( info.dli_fbase );
				if ( info.dli_fname && std::strlen( info.dli_fname ) )
					frame.moduleName = info.dli_fname;
				if ( info.dli_saddr )
				{
					frame.displacement = frame.address - reinterpret_cast< unsigned long >( info.dli_saddr );
					int status;
					char* demangled = abi::__cxa_demangle( info.dli_sname, 0, 0, &status );
					if ( status != -1 )
					{
						if ( status == 0 )
						{
							frame.symbol = demangled;
							std::free( demangled );
						}
						else
							frame.symbol = info.dli_sname;
					}
				}
				return true;
			}
			return false;
		}

		_Unwind_Reason_Code helper( struct _Unwind_Context* ctx, Trace* trace )
		{
			int beforeInsn;
			_Unwind_Ptr ip = _Unwind_GetIPInfo( ctx, &beforeInsn );
			Frame frame;
			frame.address = ip;
			if ( beforeInsn )
				frame.signalTrampoline = true;
			else
				frame.address = frame.address;
			lookupSymbol( frame );
			trace->push_back( frame );
			return _URC_NO_REASON;
		}

#elif defined( _MSC_VER ) && defined( WIN32 )

		bool lookupSymbol( Frame& frame )
		{
			::MEMORY_BASIC_INFORMATION mbi;
			if ( !::VirtualQuery( reinterpret_cast< ::LPCVOID >( frame.address ), &mbi, sizeof( mbi ) ) )
				return false;
			::CHAR moduleName[ MAX_PATH ];
			::GetModuleFileNameA( reinterpret_cast< ::HMODULE >( mbi.AllocationBase ), moduleName, sizeof( moduleName ) );
			if ( mbi.Protect & PAGE_NOACCESS )
				return false;
			frame.moduleBaseAddress = reinterpret_cast< unsigned long >( mbi.AllocationBase );
			frame.moduleName = moduleName;
			int const MaxSymbolNameLength = 8192;
			::BYTE symbolBuffer[ sizeof( ::IMAGEHLP_SYMBOL64 ) + MaxSymbolNameLength ];
			::PIMAGEHLP_SYMBOL64 symbol = reinterpret_cast< ::PIMAGEHLP_SYMBOL64 >( symbolBuffer );
			symbol->SizeOfStruct = sizeof( symbolBuffer );
			symbol->MaxNameLength = MaxSymbolNameLength - 1;
			if ( ::SymLoadModule64( ::GetCurrentProcess(), 0, moduleName, 0,
				reinterpret_cast< ::DWORD64 >( mbi.AllocationBase ), 0 ) )
			{
				::DWORD64 displacement;
				if ( ::SymGetSymFromAddr64( ::GetCurrentProcess(), static_cast< ::DWORD64 >( frame.address ),
					&displacement, symbol ) )
				{
					frame.symbol = symbol->Name;
					frame.displacement = static_cast< unsigned long >( displacement );
					::IMAGEHLP_LINE64 line;
					line.SizeOfStruct = sizeof( ::IMAGEHLP_LINE64 );
					::DWORD lineDisplacement;
					if ( ::SymGetLineFromAddr64( ::GetCurrentProcess(), frame.address, &lineDisplacement, &line ) )
					{
						frame.fileName = line.FileName;
						frame.lineNumber = line.LineNumber;
					}
				}
				::SymUnloadModule64( ::GetCurrentProcess(), reinterpret_cast< ::DWORD64 >( mbi.AllocationBase ) );
			}
			return true;
		}

#endif
	}

	Frame::Frame()
	:
		address(), displacement(), lineNumber(), signalTrampoline()
	{
	}

	Trace scan( ::PCONTEXT ctx )
	{
		Trace trace;

#if defined( __GNUC__ )

		//
		//	libgcc takes care about proper stack walking.
		//
		_Unwind_Backtrace( reinterpret_cast< _Unwind_Trace_Fn >( helper ), &trace );

#elif defined( _MSC_VER ) && defined( WIN32 )

		::HANDLE process = ::GetCurrentProcess();
		::SymInitialize( process, 0, FALSE );
		::SymSetOptions( ::SymGetOptions() | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES );
		::CONTEXT context = { 0 };
		::STACKFRAME64 stackFrame = { 0 };
		stackFrame.AddrPC.Mode = stackFrame.AddrFrame.Mode = stackFrame.AddrStack.Mode = AddrModeFlat;
		if ( ctx )
		{
			context = *ctx;
			Frame frame;
			frame.address = context.Eip;
			lookupSymbol( frame );
			trace.push_back( frame );
		}
		else
		{
			__asm
			{
				call $ + 5;
				pop eax;
				mov context.Eip, eax;
				mov context.Esp, esp;
				mov context.Ebp, ebp;
			}
		}
		stackFrame.AddrPC.Offset = context.Eip;
		stackFrame.AddrStack.Offset = context.Esp;
		stackFrame.AddrFrame.Offset = context.Ebp;
		while ( ::StackWalk64( IMAGE_FILE_MACHINE_I386, process, ::GetCurrentThread(),
			&stackFrame, &context, 0, ::SymFunctionTableAccess64, ::SymGetModuleBase64, 0 ) )
		{
			unsigned long offset = static_cast< unsigned long >( stackFrame.AddrReturn.Offset );
			//
			//	the deepest frame pointer and return address of the process
			//	call chain are zeroed by kernel32.dll during process startup,
			//	so exclude such frame from trace and exit from loop.
			//
			if ( !offset )
				break;
			Frame frame;
			frame.address = offset;
			if ( lookupSymbol( frame ) )
				trace.push_back( frame );
		}
		::SymCleanup( process );

#endif

		return trace;
	}

	std::ostream& operator << ( std::ostream& os, Trace const& t )
	{
		os << "=== backtrace ====" << std::endl;
		for ( backtracexx::Trace::const_iterator i = t.begin(); i != t.end(); ++i )
		{
			backtracexx::Frame const& f = *i;
			os << std::showbase << std::hex << std::setw( 16 ) << f.address << " : ";
			if ( f.symbol.empty() )
				os << '?';
			else
				os << f.symbol << '+' << f.displacement;
			if ( f.signalTrampoline )
				os << " [signal trampoline]";
			os << " [" << f.moduleName << " @ " << std::showbase << std::hex << f.moduleBaseAddress << " ]" << std::endl;
			if ( !f.fileName.empty() )
			{
				static std::string filler( 14, ' ' );
				os << filler << "at : " << f.fileName << ':' << std::dec << f.lineNumber << std::endl;
			}
		}
		os << "==================" << std::endl;
		return os;
	}
}
