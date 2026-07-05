#pragma once

// Project includes
#include "libraryExtensions/_clap.hpp"

// Other lib includes
#include <hedley/hedley.h>

// C++ std includes
#include <cstdint>

uint32_t plugin_factory_get_plugin_count( clap_plugin_factory const* factory );
clap_plugin_descriptor_t const* plugin_factory_get_plugin_descriptor( clap_plugin_factory const* factory, uint32_t index );
clap_plugin_t const* plugin_factory_create_plugin( clap_plugin_factory const* factory, clap_host_t const* host, char const* plugin_id );

bool entry_init( char const* plugin_path );
void entry_deinit( void );
// factory
void const* entry_get_factory( char const* factory_id );
