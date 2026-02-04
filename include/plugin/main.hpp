#pragma once

// Project includes
#include <common/_clap.hpp>

// Other lib includes
#include <hedley/hedley.h>

// C++ std includes
#include <cstdint>

#if defined( SFG_GEN_EXPORT_CLAP_INIT )
#  define SFG_GEN_API HEDLEY_PUBLIC
#else
#  define SFG_GEN_API HEDLEY_IMPORT
#endif

uint32_t plugin_factory_get_plugin_count( clap_plugin_factory const* factory );
clap_plugin_descriptor_t const* plugin_factory_get_plugin_descriptor( clap_plugin_factory const* factory, uint32_t index );
clap_plugin_t const* plugin_factory_create_plugin( clap_plugin_factory const* factory, clap_host_t const* host, char const* plugin_id );

#if __cplusplus
extern "C" {
#endif

SFG_GEN_API bool entry_init( char const* plugin_path );
SFG_GEN_API void entry_deinit( void );

// factory
SFG_GEN_API void const* entry_get_factory( char const* factory_id );

#if __cplusplus
}
#endif

#if defined( SFG_GEN_API )
#undef SFG_GEN_API
#endif
