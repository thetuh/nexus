#pragma once

namespace raii
{
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

	private:
		HANDLE pid;
		PEPROCESS process;
		KAPC_STATE state;

		bool initialized = false;
	};

	class handle_guard
	{

	};
}