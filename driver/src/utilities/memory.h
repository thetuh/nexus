#pragma once

namespace memory
{
    /* for internal usage only */
    namespace detail
    {
        inline std::vector<std::pair<const char*, uintptr_t>>module_cache;
        uintptr_t search_cache( const char* target );

        void* get_system_information( SYSTEM_INFORMATION_CLASS information_class );
    }

    std::uintptr_t get_system_module_base( char* module_name );
    std::uintptr_t sig_scan( const char* signature, const std::uintptr_t module_base ) noexcept;
}