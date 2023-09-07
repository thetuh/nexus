#include "includes.h"

/*
* @todo:
*	implement other raii classes
*	add POC um-km communication mechanism
*	dynamically retrieve explorer.exe pid rather than hardcode it
*/

void NTAPI function( )
{
    const auto win32kbase = memory::get_system_module_base( "win32kbase.sys" );
    if ( !win32kbase )
    {
		print( "win32kbase not found\n" );
        return;
    }
	print( "win32kbase.sys: 0x%p\n", win32kbase );

	/* @https://www.unknowncheats.me/forum/general-programming-and-reversing/492970-reading-memory-win32kbase-sys.html */
	raii::process_guard process( ( HANDLE ) 4068 );

	const auto address = memory::sig_scan( "E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 75 15", win32kbase );
	if ( !address )
	{
		print( "address not found\n" );
		return;
	}
	print( "address: 0x%p\n", address );

	print( "execution success\n" );
}

NTSTATUS DriverEntry( PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path )
{
	UNREFERENCED_PARAMETER( driver_object );
	UNREFERENCED_PARAMETER( registry_path );

	/* can supply useful info from kdmapper */
	HANDLE thread{ };
	if ( const auto status = PsCreateSystemThread( &thread, THREAD_ALL_ACCESS, 0, 0, 0, ( PKSTART_ROUTINE ) function, 0 ); !NT_SUCCESS( status ) )
	{
		print( "error: failed to create system thread (status: 0x%X\n", status );
		ZwClose( thread );
		return STATUS_UNSUCCESSFUL;
	}

	ZwClose( thread );
	return STATUS_SUCCESS;
}