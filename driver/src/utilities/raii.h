#pragma once

/* an raii memory manager made specifically for kernel development */
namespace raii
{
	/* 
	* @note:
	*	all copy constructors and copy assignment operators are deleted to enforce single ownership
	*/

	class process_guard
	{
	public:
		process_guard( HANDLE pid ) : pid( pid )
		{
			if ( const auto status = PsLookupProcessByProcessId( pid, &process ); !NT_SUCCESS( status ) )
			{
				print( "error: process with pid %d not found (status: 0x%X\n", pid, status );
				return;
			}

			KeStackAttachProcess( process, &state );
			initialized = true;
		}

		~process_guard( )
		{
			detach( );
		}

		void detach( )
		{
			if ( initialized )
			{
				KeUnstackDetachProcess( &state );
				initialized = false;
			}
		}

		process_guard( const process_guard& ) = delete;
		process_guard& operator=( const process_guard& ) = delete;

	private:
		HANDLE pid;
		PEPROCESS process;
		KAPC_STATE state;

		bool initialized = false;
	};

	/* for generic memory alloc/dealloc */
	class resource_guard
	{
	public:
		resource_guard( PVOID ptr ) : ptr( ptr ) { }
		~resource_guard( ) { print( "freeing resource!\n" ); ExFreePoolWithTag( ptr, 0 ); }

		resource_guard( const resource_guard& ) = delete;
		resource_guard& operator=( const resource_guard& ) = delete;

		PVOID get( ) { return ptr; }

	private:
		PVOID ptr;
	};

	class handle_guard
	{

	};
}