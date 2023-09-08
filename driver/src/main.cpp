#include "includes.h"

/*
* @todo:
*	finish implementing raii resource manager class
*	add POC communication request handler
*/

void NTAPI function( )
{
	raii::scope_guard exit_msg( [ & ]( ) { print( crypt("execution success!\n") ); } );
	const auto abort = [ & ]( const char* msg ) -> void
	{
		print( "%s\n", msg );
		return;
	};

	const auto win32kbase = memory::get_system_module_base( crypt("win32kbase.sys") );
	if ( !win32kbase )
		abort( crypt("win32kbase not found") );

	print( crypt("win32kbase.sys: 0x%p\n"), win32kbase );

	const auto pid = util::get_pid( L"explorer.exe" );
	if ( !pid )
		abort( crypt("explorer not found") );

	print( crypt("explorer process id: %d\n"), pid );

	/* @https://www.unknowncheats.me/forum/general-programming-and-reversing/492970-reading-memory-win32kbase-sys.html */
	raii::safe_process process( pid, true );
	if ( !process.get( ) )
		abort( crypt("could not attach to process") );

	print( crypt("process: 0x%p\n"), process.get( ) );

	const auto address = memory::sig_scan( crypt("E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 75 15"), win32kbase );
	if ( !address )
		abort( crypt("signature not found") );

	print( crypt("signature found at: 0x%p\n" ), address );
}

NTSTATUS DriverEntry( PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path )
{
	UNREFERENCED_PARAMETER( driver_object );
	UNREFERENCED_PARAMETER( registry_path );

	/* can supply useful info from kdmapper */
	raii::safe_handle thread_handle( true );
	if ( const auto status = PsCreateSystemThread( thread_handle.get_ref( ), THREAD_ALL_ACCESS, 0, 0, 0, ( PKSTART_ROUTINE ) function, 0 ); !NT_SUCCESS( status ) )
	{
		print( crypt("error: failed to create system thread (status: 0x%X)\n"), status );
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}