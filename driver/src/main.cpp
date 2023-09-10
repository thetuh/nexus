#include "includes.h"

__int64 ( __fastcall* o_NtUserCloseDesktop )( void* a1 );

void hook( communication_t* a1 )
{
	a1->buffer = 0x1337;
	print( crypt( "usermode communication operation detected\n") );
	print( crypt( "return address: 0x%p\n" ), _ReturnAddress( ) );
}

__int64 __fastcall hook_proxy( void* a1 )
{
	/* directly dereferencing is unstable since the original function expects any unsigned int, not necessarily a pointer */
	communication_t comm{ };
	if ( !util::spoof_call<bool>( util::safe_copy, &comm, a1, sizeof( communication_t ) ) || comm.key != COMMUNICATION_KEY )
		return o_NtUserCloseDesktop( a1 );

	util::spoof_call( hook, a1 );

	return 0;
}

void print_function( const char* msg )
{
	print( crypt( "print_function called | msg: '%s', return address: 0x%p\n" ), msg, _ReturnAddress( ) );
}

void NTAPI function( )
{
	raii::scope_guard exit_msg( [ & ]( ) { print( crypt( "execution success!\n" ) ); } );
	const auto abort = [ & ]( const char* msg ) -> void
	{
		exit_msg.cancel( );
		print( "error: %s\n", msg );
		return;
	};

	const auto pid = util::get_pid( L"explorer.exe" );
	if ( !pid )
		return abort( crypt( "explorer not found" ) );

	raii::safe_process explorer( pid, true );
	if ( !explorer.get( ) )
		return abort( crypt( "could not attach to explorer" ) );

	const auto win32kbase = memory::get_system_module_base( crypt( "win32kbase.sys" ) );
	if ( !win32kbase )
		return abort( crypt( "win32k not found" ) );

	print( crypt( "win32kbase.sys: 0x%p\n" ), win32kbase );

	auto signature_address = memory::sig_scan( crypt( "E8 ? ? ? ? 48 98 48 83 C4 28 C3 CC CC CC CC CC CC CC CC 48 8B C4 48 89 58 08 48 89 68" ), win32kbase );
	if ( !signature_address )
		return abort( crypt( "signature not found" ) );

	print( crypt( "signature found at: 0x%p\n" ), signature_address );

	const auto function_ptr = RVA( RVA( signature_address, 5 ) + 0x5E, 7);
	print( crypt( "resolved relative address: 0x%p\n" ), function_ptr );

	util::set_spoof_stub( ( PVOID ) memory::sig_scan( crypt( "FF 27" ), win32kbase ) );
	if ( !util::ret_addr_stub_enc )
		return abort( crypt( "spoof gadget not found" ) );

	print( crypt( "spoof gadget: 0x%p\n" ), util::crypt_ptr( util::ret_addr_stub_enc ) );

	print_function( crypt( "unspoofed call" ) );
	util::spoof_call( print_function, "spoofed call" );

	*( void** ) &o_NtUserCloseDesktop = InterlockedExchangePointer( ( void** ) function_ptr, hook_proxy );

	print( crypt( "swapped pointer to function\n" ) );
}

NTSTATUS DriverEntry( PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path )
{
	UNREFERENCED_PARAMETER( driver_object );
	UNREFERENCED_PARAMETER( registry_path );

	/* can supply useful info from kdmapper */
	raii::safe_handle thread_handle( true );
	if ( const auto status = PsCreateSystemThread( thread_handle.get_ref( ), THREAD_ALL_ACCESS, 0, 0, 0, ( PKSTART_ROUTINE ) function, 0 ); !NT_SUCCESS( status ) )
	{
		print( crypt( "error: failed to create system thread (status: 0x%X)\n" ), status );
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}