#pragma once

namespace raii
{
	class scope_guard
	{
	public:
		scope_guard( PVOID ptr, bool verbose = false ) : ptr( ptr ), verbose( verbose ) { }
		~scope_guard( ) { release( ); }

		PVOID get( ) { return ptr; }

		void release( )
		{
			if ( ptr )
			{
				ExFreePoolWithTag( ptr, 0 );

				if ( verbose )
					print( "freed resource" );

				ptr = nullptr;
			}
		}

		scope_guard( const scope_guard& ) = delete;
		scope_guard& operator=( const scope_guard& ) = delete;

	private:
		PVOID ptr;
		const bool verbose;
	};

	/* automatically detaches upon destruction */
	class safe_process_attach
	{
	public:
		safe_process_attach( HANDLE pid, bool verbose = false ) : pid( pid ), verbose( verbose )
		{
			if ( const auto status = PsLookupProcessByProcessId( pid, &process ); !NT_SUCCESS( status ) )
			{
				print( "error: process with pid %d not found (status: 0x%X)\n", pid, status );
				return;
			}

			KeStackAttachProcess( process, &state );
		}

		~safe_process_attach( ) { detach( ); }

		PEPROCESS get( ) { return process; }
		void detach( )
		{
			if ( process )
			{
				KeUnstackDetachProcess( &state );

				if ( verbose )
					print( "detached from process id: %d\n", pid );

				process = nullptr;
			}
		}

		safe_process_attach( const safe_process_attach& ) = delete;
		safe_process_attach& operator=( const safe_process_attach& ) = delete;

	private:
		PEPROCESS process;
		KAPC_STATE state;
		const HANDLE pid;
		const bool verbose;
	};

	class safe_handle
	{
	public:
	};
}