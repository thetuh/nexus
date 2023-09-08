#include "includes.h"

/*
* @todo:
*	add POC communication request handler
*/

void NTAPI function( )
{
	raii::scope_guard exit_msg( [ & ]( ) { print( "execution success!\n" ); } );
	const auto abort = [ & ]( const char* msg ) -> void
	{
		print( "%s\n", msg );
		return;
	};

	const auto win32kbase = memory::get_system_module_base( "win32kbase.sys" );
	if ( !win32kbase )
		abort( "win32kbase not found" );

	print( "win32kbase.sys: 0x%p\n", win32kbase );

	const auto pid = util::get_pid( L"explorer.exe" );
	if ( !pid )
		abort( "explorer not found" );

	print( "explorer process id: %d\n", pid );

	/* @https://www.unknowncheats.me/forum/general-programming-and-reversing/492970-reading-memory-win32kbase-sys.html */
	raii::safe_process process( pid, true );
	if ( !process.get( ) )
		abort( "could not attach to process" );

	print( "process: 0x%p\n", process.get( ) );

	const auto address = memory::sig_scan( "E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 75 15", win32kbase );
	if ( !address )
		abort( "signature not found" );

	print( "signature found at: 0x%p\n", address );
}

NTSTATUS DriverEntry( PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path )
{
	UNREFERENCED_PARAMETER( driver_object );
	UNREFERENCED_PARAMETER( registry_path );

	/* can supply useful info from kdmapper */
	HANDLE thread{ };
	if ( const auto status = PsCreateSystemThread( &thread, THREAD_ALL_ACCESS, 0, 0, 0, ( PKSTART_ROUTINE ) function, 0 ); !NT_SUCCESS( status ) )
	{
		print( "error: failed to create system thread (status: 0x%X)\n", status );
		ZwClose( thread );
		return STATUS_UNSUCCESSFUL;
	}

	ZwClose( thread );
	return STATUS_SUCCESS;
}