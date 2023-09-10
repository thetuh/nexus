#pragma once

namespace util
{
    /* wrapper for ZwQuerySystemInformation that handles the memory alloc size for you */
    /* remember to dispose after use */
    void* get_system_information( SYSTEM_INFORMATION_CLASS information_class );

    /* wrapper for MmCopyVirtualMemory */
    bool safe_copy( void* dst, void* src, SIZE_T size );

    HANDLE get_pid( const wchar_t* proc_name );

    /* crc16 strhash */
    template <typename str_type> __declspec( noinline ) constexpr unsigned short hash_str( str_type data, int len )
    {
        unsigned short crc = 0xFFFF; while ( len-- )
        {
            auto cur_char = *data++; if ( !cur_char ) break;
            crc ^= to_lower_ch( cur_char ) << 8; for ( int i = 0; i < 8; i++ )
                crc = crc & 0x8000 ? ( crc << 1 ) ^ 0x8408 : crc << 1;
        } return crc;
    }

    #define hash_str(str) [](){ constexpr unsigned short crc = util::hash_str(str, str_len(str)); return crc; }()

    /* encrypt/decrypt ptr */
    template <typename ptr_type>
    __forceinline ptr_type crypt_ptr( ptr_type Ptr )
    {
        typedef union { struct { USHORT key1; USHORT key2; USHORT key3; USHORT key4; }; ULONG64 key; } crypt_data;
        crypt_data key{ hash_str( __TIME__ ), hash_str( __DATE__ ), hash_str( __TIMESTAMP__ ), hash_str( __TIMESTAMP__ ) };
        return ( ptr_type ) ( ( DWORD64 ) Ptr ^ key.key );
    }

    /* spoof return address */
    inline PVOID ret_addr_stub_enc = 0;
    extern "C" void* _spoofer_stub( );
    template<typename Ret = void, typename First = void*, typename Second = void*, typename Third = void*, typename Fourth = void*, typename... Stack>
    __forceinline Ret spoof_call( void* Func, First a1 = First{}, Second a2 = Second{}, Third a3 = Third{}, Fourth a4 = Fourth{}, Stack... args )
    {
        struct shell_params { const void* a1; void* a2; void* a3; };
        shell_params call_ctx = { crypt_ptr( ret_addr_stub_enc ), Func, nullptr };
        typedef Ret( *ShellFn )( First, Second, Third, Fourth, PVOID, PVOID, Stack... );
        return ( ( ShellFn ) &util::_spoofer_stub )( a1, a2, a3, a4, &call_ctx, nullptr, args... );
    }
    __forceinline void set_spoof_stub( PVOID R15_Stub ) { ret_addr_stub_enc = crypt_ptr( R15_Stub ); }

    /* c runtime library */
    namespace crt
    {
        const char* to_lower( char* string );
        int isalpha( int c );
        unsigned long strtoul( const char* nptr, char** endptr, int base );
    }
}