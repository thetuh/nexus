#pragma once

#include <functional>

/* a library made to automate resource disposal and safe cleanups following scope exit */
namespace raii
{
	/* call back a supplied procedure upon destruction */
	template <typename fn_callback, typename... fn_args>
	class scope_guard
	{
	public:
		scope_guard( const fn_callback& callback, const fn_args& ...args ) : callback{ callback }, args{ args... }, active( true ) { }
		~scope_guard( ) { execute( ); }

	public:
		void execute( ) { if ( active ) { invoke_callable( std::make_index_sequence < std::tuple_size_v<std::tuple<fn_args...>>>( ) ); active = false; } }
		void cancel( ) { active = false; }

	private:
		template <size_t... Is>
		void invoke_callable( std::index_sequence<Is...> ) { std::invoke( callback, std::get<Is>( args )... ); }

	private:
		const fn_callback callback;
		const std::tuple< fn_args... > args;
		bool active;
	};

	/* free memory upon object destruction */
	/* similar to std::unique_ptr with optional logging */
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
					print( "freed memory\n" );

				ptr = nullptr;
			}
		}

	private:
		PVOID ptr;
		const bool verbose;
	};

	/* free handle upon object destruction */
	class safe_handle
	{
	public:
		safe_handle( bool verbose = false ) : handle( nullptr ), verbose( verbose ) { }
		~safe_handle( ) { release( ); }

		safe_handle( const safe_handle& ) = delete;
		safe_handle& operator=( const safe_handle& ) = delete;

	public:
		HANDLE get( ) { return handle; }
		PHANDLE get_ref( ) { return &handle; }
		void release( ) { if ( handle ) { ZwClose( handle ); if ( verbose ) print( "freed handle\n" ); handle = nullptr; } }

	private:
		HANDLE handle;
		const bool verbose;
	};

	/* detach from process upon object destruction */
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
}