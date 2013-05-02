#include "backtracexx.hpp"
#include <iomanip>
#include <iostream>

#if defined( __GNUC__ )
#include <cxxabi.h>
#include <dlfcn.h>
#include <unwind.h>
#elif defined( _MSC_VER )
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
		//
		//	extract caller's address from callee's return point.
		//
		unsigned long caller( unsigned long ret )
		{
			unsigned char const* ip = reinterpret_cast< unsigned char const* >( ret );
#if defined( __powerpc__ ) && !defined( __powerpc64__ )
			//	powerpc64 not tested.
			ip -= 4;
#elif defined( __sparc__ )
			//	the same for sparc v7/8/9.
			ip -= 8;
#elif defined( __alpha__ )
			ip -= 4;
#elif defined( __i386__ ) || defined( __x86_64__ ) || defined( WIN32 )
			//
			//	TODO:
			//		analysis of complex addressing forms (see intel/24319102.pdf).
			//		rework code to cover all cases.
			//
			//	call, near, relative
			if ( ip[ -5 ] == 0xe8 )
				return ( ret - 5 );
			//	call, near, absolute indirect
			if ( ip[ -2 ] == 0xff )
			{
				if ( ( ip[ -1 ] & 0xf8 ) == 0xd0 )	//	call *%reg
					return ( ret - 2 );
				if ( ( ip[ -1 ] & 0xf8 ) == 0x10 )	//	call *(%reg)
					return ( ret - 2 );
			}
#endif
			return ret;
		}

#if defined( __GNUC__ )

		void lookupSymbol( Frame& frame )
		{
			Dl_info info;
			if ( ::dladdr( reinterpret_cast< void* >( frame.address ), &info ) )
			{
				if ( info.dli_fname && strlen( info.dli_fname ) )
					frame.module = info.dli_fname;
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
							free( demangled );
						}
						else
							frame.symbol = info.dli_sname;
					}
				}
			}
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
				frame.address = caller( frame.address );
			lookupSymbol( frame );
			trace->push_back( frame );
			return _URC_NO_REASON;
		}

#elif defined( _MSC_VER ) && defined( WIN32 )

		void lookupSymbol( Frame& frame )
		{
			::MEMORY_BASIC_INFORMATION mbi;
			::VirtualQuery( reinterpret_cast< ::LPCVOID >( frame.address ), &mbi, sizeof( mbi ) );
			::CHAR moduleName[ MAX_PATH ];
			::GetModuleFileNameA( reinterpret_cast< ::HMODULE >( mbi.AllocationBase ), moduleName, sizeof( moduleName ) );
			frame.module = moduleName;
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
				}
				::SymUnloadModule64( ::GetCurrentProcess(), reinterpret_cast< ::DWORD64 >( mbi.AllocationBase ) );
			}
		}

#endif
	}

	Frame::Frame()
	:
		address(), displacement(), signalTrampoline()
	{
	}

	Trace scan( PEXCEPTION_POINTERS ex )
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
		::SymSetOptions( ::SymGetOptions() | SYMOPT_UNDNAME );
		::CONTEXT context = { 0 };
		::STACKFRAME64 stackFrame = { 0 };
		stackFrame.AddrPC.Mode = stackFrame.AddrFrame.Mode = stackFrame.AddrStack.Mode = AddrModeFlat;
		if ( ex )
		{
			context = *( ex->ContextRecord );
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
			Frame frame;
			unsigned long offset = static_cast< unsigned long >( stackFrame.AddrReturn.Offset );
			//
			//	the deepest frame pointer and return address of the process
			//	call chain are zeroed by kernel32.dll during process startup,
			//	so exclude such frame from trace and exit from loop.
			//
			if ( !offset )
				break;
			frame.address = caller( offset );
			lookupSymbol( frame );
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
			os	<< std::showbase << std::showpoint << std::hex << std::setw( 16 ) << f.address
				<< " : " << ( f.symbol.empty() ? "<unresolved symbol>" : f.symbol )
				<< "+" << f.displacement;
			if ( f.signalTrampoline )
				os << " [signal trampoline]";
			os << " [" << f.module << "]" << std::endl;
		}
		os << "==================" << std::endl;
		return os;
	}
}
