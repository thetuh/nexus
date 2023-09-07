#include <iostream>
#include <Windows.h>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string_view, HANDLE>module_handles;

std::uintptr_t sig_scan( const char* signature, std::string_view module_name ) noexcept
{
    auto pattern_to_byte = [ ]( const char* pattern ) noexcept -> std::vector<int>
    {
        auto bytes = std::vector<int>{ };
        auto start = const_cast< char* >( pattern );
        auto end = const_cast< char* >( pattern ) + std::strlen( pattern );

        for ( auto current = start; current < end; ++current )
        {
            if ( *current == '?' )
            {
                ++current;

                if ( *current == '?' )
                    ++current;

                bytes.push_back( -1 );
            }
            else
                bytes.push_back( std::strtoul( current, &current, 16 ) );
        }

        return bytes;
    };

    HANDLE handle{ NULL };
    if ( module_handles.find( module_name ) == module_handles.end( ) )
        handle = module_handles[ module_name ] = GetModuleHandleA( module_name.data( ) );
    else
        handle = module_handles[ module_name ];

    if ( !handle )
        return 0;

    auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( handle );
    auto nt_headers =
        reinterpret_cast< PIMAGE_NT_HEADERS >( reinterpret_cast< std::uint8_t* >( handle ) + dos_header->e_lfanew );

    auto size = nt_headers->OptionalHeader.SizeOfImage;
    auto bytes = pattern_to_byte( signature );
    auto scan_bytes = reinterpret_cast< std::uint8_t* >( handle );

    auto s = bytes.size( );
    auto d = bytes.data( );

    for ( auto i = 0ul; i < size - s; ++i )
    {
        bool found = true;

        for ( auto j = 0ul; j < s; ++j )
        {
            if ( scan_bytes[ i + j ] != d[ j ] && d[ j ] != -1 )
            {
                found = false;
                break;
            }
        }

        if ( found )
            return reinterpret_cast< std::uintptr_t >( &scan_bytes[ i ] );
    }

    return 0;
}

int main( )
{
    std::cout << sig_scan( "E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 75 15", "ntdll.dll" ) << "\n";
	getchar( );
}