#include <Windows.h>
#include <iostream>
#include <communication.h>
#include <crypt.h>

static NTSTATUS( *NtUserCloseDesktop )( communication_t& a1 ) = nullptr;

int main( )
{
	LoadLibrary( L"user32.dll" );
	LoadLibrary( L"kernel32.dll" );

	const auto win32u = LoadLibrary( L"win32u.dll" );
	if ( !win32u )
	{
		std::cout << crypt("win32u not found\n");
		return 1;
	}

	communication_t communication;
	*( PVOID* ) &NtUserCloseDesktop = GetProcAddress( win32u, crypt( "NtUserCloseDesktop") );
	if ( !NtUserCloseDesktop )
	{
		std::cout << crypt( "NtUserCloseDesktop not found\n" );
		return 1;
	}

	int input;
	do
	{
		std::cout << crypt("---------------\n'1' ping the driver\n");
		std::cin >> input;
		switch ( input )
		{
			case 1:
			{
				NtUserCloseDesktop( communication );
				printf( crypt("driver: 0x%p\n"), communication.buffer );
				break;
			}
			default:
			{
				input = 0;
				break;
			}
		}
	} while ( input );

	return 0;
}