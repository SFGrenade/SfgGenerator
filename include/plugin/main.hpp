#pragma once

// Project includes
#include <common/_clap.hpp>

// C++ std includes
#include <cstdint>

#if defined( SFG_GEN_EXPORT_CLAP_INIT ) && ( defined( _WIN32 ) || defined( _WIN64 ) )
#define SFG_GEN_CLASS_API __declspec( dllexport )
#elif defined( _WIN32 ) || defined( _WIN64 )
#define SFG_GEN_CLASS_API __declspec( dllimport )
#endif

uint32_t plugin_factory_get_plugin_count( clap_plugin_factory const* factory );
clap_plugin_descriptor_t const* plugin_factory_get_plugin_descriptor( clap_plugin_factory const* factory, uint32_t index );
clap_plugin_t const* plugin_factory_create_plugin( clap_plugin_factory const* factory, clap_host_t const* host, char const* plugin_id );

#if __cplusplus
extern "C" {
#endif

SFG_GEN_CLASS_API bool entry_init( char const* plugin_path );
SFG_GEN_CLASS_API void entry_deinit( void );

// factory
SFG_GEN_CLASS_API void const* entry_get_factory( char const* factory_id );

#if __cplusplus
}
#endif

#if defined( SFG_GEN_CLASS_API )
#undef SFG_GEN_CLASS_API
#endif
