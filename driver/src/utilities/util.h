#pragma once

namespace util
{
    /* wrapper for ZwQuerySystemInformation that handles the memory allocation */
    /* remember to free after use ! */
    void* get_system_information( SYSTEM_INFORMATION_CLASS information_class );

    HANDLE get_pid( const wchar_t* proc_name );

    /* c runtime library */
    namespace crt
    {
        const char* to_lower( char* string );
        int isalpha( int c );
        unsigned long strtoul( const char* nptr, char** endptr, int base );
    }
}