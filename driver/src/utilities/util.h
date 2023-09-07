#pragma once

namespace util
{
    /* c runtime library */
    namespace crt
    {
        const char* to_lower( char* string );
        int isalpha( int c );
        unsigned long strtoul( const char* nptr, char** endptr, int base );
    }
}