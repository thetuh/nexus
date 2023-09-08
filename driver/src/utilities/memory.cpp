#include "../includes.h"

std::uintptr_t memory::sig_scan( const char* signature, const std::uintptr_t module_base ) noexcept
{
    if ( !module_base )
        return 0;

    /* move this to util? */
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
                bytes.push_back( util::crt::strtoul( current, &current, 16 ) );
        }

        return bytes;
    };

    auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( module_base );
    auto nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS >( module_base + dos_header->e_lfanew );

    auto size = nt_headers->OptionalHeader.SizeOfImage;
    auto bytes = pattern_to_byte( signature );
    auto s = bytes.size( );
    auto d = bytes.data( );

    auto scan_bytes = reinterpret_cast< std::uint8_t* >( module_base );

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

uintptr_t memory::get_system_module_base( char* module_name )
{
    util::crt::to_lower( module_name );

    if ( const auto address = detail::search_cache( module_name ); address )
        return address;

    raii::resource_guard buffer( util::get_system_information( SystemModuleInformation ) );
    if ( !buffer.get( ) )
        return NULL;

    const PRTL_PROCESS_MODULES info = ( PRTL_PROCESS_MODULES ) buffer.get( );
    for ( size_t i = 0; i < info->NumberOfModules; ++i )
    {
        const auto& mod = info->Modules[ i ];
        const auto lowered_name = util::crt::to_lower( ( char* ) mod.FullPathName + mod.OffsetToFileName );

        if ( !detail::search_cache( lowered_name ) )
            detail::module_cache.emplace_back( std::make_pair( lowered_name, ( uintptr_t ) mod.ImageBase ) );

        if ( !strcmp( lowered_name, module_name ) )
            return ( uintptr_t ) mod.ImageBase;
    }

    return NULL;
}

uintptr_t memory::detail::search_cache( const char* target )
{
    /* binary search */

    int left = 0;
    int right = module_cache.size( ) - 1;

    while ( left <= right )
    {
        int mid = left + ( right - left ) / 2;

        const int result = strcmp( module_cache[ mid ].first, target );
        if ( result == 0 )
            return module_cache[ mid ].second;

        else if ( result < 0 )
            left = mid + 1;  // target is in the right half
        else
            right = mid - 1;  // target is in the left half
    }

    return 0;
}