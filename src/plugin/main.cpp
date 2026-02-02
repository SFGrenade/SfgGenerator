// Header assigned to this source
#include "plugin/main.hpp"

// C++ std includes
#include <mutex>

#define DEBUG_LOG_ARGS_PLUGIN( fmt_string, ... ) \
  plug->host_log->log( plug->host,               \
                       CLAP_LOG_DEBUG,           \
                       fmt::format( "[{}] plugin={:p}, " fmt_string, __FUNCTION__, static_cast< void const* >( plugin ), ##__VA_ARGS__ ).c_str() )

#define DEBUG_LOG_ARGS_PLUG( fmt_string, ... ) \
  plug->host_log->log( plug->host,             \
                       CLAP_LOG_DEBUG,         \
                       fmt::format( "[{}] plug={:p}, " fmt_string, __FUNCTION__, static_cast< void* >( plug ), ##__VA_ARGS__ ).c_str() )

uint32_t plugin_factory_get_plugin_count( const struct clap_plugin_factory* factory ) {
  return sizeof( s_plugins ) / sizeof( s_plugins[0] );
}

clap_plugin_descriptor_t const* plugin_factory_get_plugin_descriptor( const struct clap_plugin_factory* factory, uint32_t index ) {
  return s_plugins[index].desc;
}

clap_plugin_t const* plugin_factory_create_plugin( const struct clap_plugin_factory* factory, clap_host_t const* host, char const* plugin_id ) {
  if( !clap_version_is_compatible( host->clap_version ) ) {
    return nullptr;
  }

  const int N = sizeof( s_plugins ) / sizeof( s_plugins[0] );
  for( int i = 0; i < N; ++i )
    if( !strcmp( plugin_id, s_plugins[i].desc->id ) )
      return s_plugins[i].create( host );

  return nullptr;
}

bool entry_init( char const* plugin_path ) {
  // perform the plugin initialization
  ClapGlobals::PLUGIN_PATH = plugin_path;
  return true;
}

void entry_deinit( void ) {
  // perform the plugin de-initialization
  ClapGlobals::PLUGIN_PATH.clear();
}

// thread safe init counter
bool entry_init_guard( char const* plugin_path ) {
  std::lock_guard< std::mutex > _( ClapGlobals::ENTRY_LOCK );
  int const cnt = ++ClapGlobals::ENTRY_INIT_COUNTER;
  if( cnt > 1 )
    return true;
  if( entry_init( plugin_path ) )
    return true;
  ClapGlobals::ENTRY_INIT_COUNTER = 0;
  return false;
}

// thread safe deinit counter
void entry_deinit_guard( void ) {
  std::lock_guard< std::mutex > _( ClapGlobals::ENTRY_LOCK );
  int const cnt = --ClapGlobals::ENTRY_INIT_COUNTER;
  if( cnt == 0 )
    entry_deinit();
}

void const* entry_get_factory( char const* factory_id ) {
  assert( ClapGlobals::ENTRY_INIT_COUNTER > 0 );
  if( ClapGlobals::ENTRY_INIT_COUNTER <= 0 )
    return nullptr;

  if( !strcmp( factory_id, CLAP_PLUGIN_FACTORY_ID ) )
    return &s_plugin_factory;
  return nullptr;
}
