#include "../includes.h"

void* util::get_system_information( SYSTEM_INFORMATION_CLASS information_class )
{
    unsigned long size = 0;
    ZwQuerySystemInformation( information_class, 0, 0, &size );

    /* obviously important not to use a resource guard here so the caller function can use it */
    void* info = ExAllocatePool( NonPagedPool, size );
    if ( !info )
        return nullptr;

    if ( !NT_SUCCESS( ZwQuerySystemInformation( information_class, info, size, &size ) ) )
    {
        ExFreePoolWithTag( info, 0 );
        return nullptr;
    }

    return info;
}

HANDLE util::get_pid( const wchar_t* proc_name )
{
    /* keep a reference to the actual region of memory that was allocated*/
    //raii::resource_guard buffer( get_system_information( SystemProcessInformation ) );
    std::unique_ptr<SYSTEM_PROCESS_INFORMATION> buffer( reinterpret_cast<PSYSTEM_PROCESS_INFORMATION>( get_system_information( SystemProcessInformation ) ) );
    if ( !buffer.get( ) )
        return 0;

    /* pointer that performs actual traversal of each process entry */
    PSYSTEM_PROCESS_INFORMATION info = ( PSYSTEM_PROCESS_INFORMATION ) buffer.get( );

    HANDLE pid{ };
    UNICODE_STRING name;
    RtlInitUnicodeString( &name, proc_name );

    do
    {
        if ( RtlEqualUnicodeString( &info->ImageName, &name, TRUE ) )
        {
            pid = ( HANDLE ) info->UniqueProcessId;
            break;
        }

        info = ( PSYSTEM_PROCESS_INFORMATION ) ( ( PUCHAR ) info + info->NextEntryOffset );
    } while ( info->NextEntryOffset );

    return pid;
}

const char* util::crt::to_lower( char* string )
{
    for ( char* pointer = string; *pointer != '\0'; ++pointer )
        *pointer = ( char ) ( short ) tolower( *pointer );

    return string;
}

int util::crt::isalpha( int c )
{
    char alphabet[ ] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    char* letter = alphabet;

    while ( *letter != '\0' && *letter != c )
        ++letter;

    if ( *letter )
        return 1;

    return 0;
}

/* @https://github.com/gcc-mirror/gcc/blob/master/libiberty/strtoul.c */
unsigned long util::crt::strtoul( const char* nptr, char** endptr, register int base )
{
    register const char* s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    do
    {
        c = *s++;
    } while ( isspace( c ) );
    if ( c == '-' )
    {
        neg = 1;
        c = *s++;
    }
    else if ( c == '+' )
        c = *s++;
    if ( ( base == 0 || base == 16 ) &&
        c == '0' && ( *s == 'x' || *s == 'X' ) )
    {
        c = s[ 1 ];
        s += 2;
        base = 16;
    }
    if ( base == 0 )
        base = c == '0' ? 8 : 10;
    cutoff = ( unsigned long ) ULONG_MAX / ( unsigned long ) base;
    cutlim = ( unsigned long ) ULONG_MAX % ( unsigned long ) base;
    for ( acc = 0, any = 0;; c = *s++ )
    {
        if ( isdigit( c ) )
            c -= '0';
        else if ( isalpha( c ) )
            c -= isupper( c ) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if ( c >= base )
            break;
        if ( any < 0 || acc > cutoff || ( acc == cutoff && c > cutlim ) )
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if ( any < 0 )
    {
        acc = ULONG_MAX;
    }
    else if ( neg )
        acc = -acc;
    if ( endptr != 0 )
        *endptr = ( char* ) ( any ? s - 1 : nptr );
    return ( acc );
}