#pragma once

namespace raii
{
	class scope_guard
	{
	public:
		scope_guard( PVOID ptr, bool verbose = false ) : ptr( ptr ), verbose( verbose ) { }
		~scope_guard( ) { release( ); }

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

		PVOID get( ) { return ptr; }

	private:
		PVOID ptr = nullptr;

		bool verbose = false;
	};

	class safe_process
	{
	public:
		safe_process( HANDLE pid, bool verbose = false ) : pid( pid ), verbose( verbose )
		{
			if ( const auto status = PsLookupProcessByProcessId( pid, &process ); !NT_SUCCESS( status ) )
			{
				print( "error: process with pid %d not found (status: 0x%X)\n", pid, status );
				return;
			}

			KeStackAttachProcess( process, &state );
		}
		~safe_process( ) { detach( ); }

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

		safe_process( const safe_process& ) = delete;
		safe_process& operator=( const safe_process& ) = delete;

	private:
		PEPROCESS process = nullptr;
		HANDLE pid;
		KAPC_STATE state;

		bool verbose = false;
	};

	class safe_handle
	{
	public:
	};
}