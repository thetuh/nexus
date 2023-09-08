#pragma once

/* a library made to automate resource disposal */
namespace raii
{
	/* call back a supplied procedure upon destruction */
	template <typename fn_callback, typename... fn_args>
	class scope_guard
	{
	public:
		scope_guard( const fn_callback& callback, const fn_args& ...args ) : callback{ callback }, args{ args... }, active( true ) { }
		~scope_guard( ) { invoke_callable( std::make_index_sequence < std::tuple_size_v<std::tuple<fn_args...>>>( ) ); }
		void cancel( ) { active = false; }

	private:
		template <size_t... Is>
		void invoke_callable( std::index_sequence<Is...> ) { if ( active ) std::invoke( callback, std::get<Is>( args )... ); }

	private:
		const fn_callback callback;
		const std::tuple< fn_args... > args;
		const bool active;
	};

	/* free memory upon destruction */
	class safe_memory
	{
	public:
		safe_memory( PVOID ptr, bool verbose = false ) : ptr( ptr ), verbose( verbose ) { }
		~safe_memory( ) { release( ); }

		safe_memory( const safe_memory& ) = delete;
		safe_memory& operator=( const safe_memory& ) = delete;

	public:
		PVOID get( ) { return ptr; }
		void release( )
		{
			if ( ptr )
			{
				ExFreePoolWithTag( ptr, 0 );

				if ( verbose )
					print( "freed resource\n" );

				ptr = nullptr;
			}
		}

	private:
		PVOID ptr;
		const bool verbose;
	};

	/* detach from process upon destruction */
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

		safe_process( const safe_process& ) = delete;
		safe_process& operator=( const safe_process& ) = delete;

	public:
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