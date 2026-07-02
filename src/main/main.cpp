// Header assigned to this source
#include "main/main.hpp"

// Project includes
#include "plugin/AudioAnalysis.hpp"
#include "plugin/AudioLerpEffect.hpp"
#include "plugin/NoiseGenerator.hpp"
#include "plugin/ParamMultiplex.hpp"
#include "vendor/luigi2.h"

#define DEBUG_LOG_ARGS_PLUGIN( fmt_string, ... ) \
  plug->host_log->log( plug->host,               \
                       CLAP_LOG_DEBUG,           \
                       fmt::format( "[{}] plugin={:p}, " fmt_string, __FUNCTION__, static_cast< void const* >( plugin ), ##__VA_ARGS__ ).c_str() )

#define DEBUG_LOG_ARGS_PLUG( fmt_string, ... ) \
  plug->host_log->log( plug->host,             \
                       CLAP_LOG_DEBUG,         \
                       fmt::format( "[{}] plug={:p}, " fmt_string, __FUNCTION__, static_cast< void* >( plug ), ##__VA_ARGS__ ).c_str() )

static struct {
  clap_plugin_descriptor_t const* desc;
  clap_plugin_t*( CLAP_ABI* create )( clap_host_t const* host );
} s_plugins[] = {
    {
        .desc = NoiseGenerator::descriptor_get(),
        .create = NoiseGenerator::s_create,
    },
    {
        .desc = AudioLerpEffect::descriptor_get(),
        .create = AudioLerpEffect::s_create,
    },
    {
        .desc = ParamMultiplex::descriptor_get(),
        .create = ParamMultiplex::s_create,
    },
    {
        .desc = AudioAnalysis::descriptor_get(),
        .create = AudioAnalysis::s_create,
    },
};

static clap_plugin_factory_t const s_plugin_factory = {
    .get_plugin_count = plugin_factory_get_plugin_count,
    .get_plugin_descriptor = plugin_factory_get_plugin_descriptor,
    .create_plugin = plugin_factory_create_plugin,
};

uint32_t plugin_factory_get_plugin_count( clap_plugin_factory const* factory ) {
  if( ClapGlobals::PLUGIN_LOGGER )
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( factory={:p} )", __FUNCTION__, static_cast< void const* >( factory ) );
  return sizeof( s_plugins ) / sizeof( s_plugins[0] );
}

clap_plugin_descriptor_t const* plugin_factory_get_plugin_descriptor( clap_plugin_factory const* factory, uint32_t index ) {
  if( ClapGlobals::PLUGIN_LOGGER )
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( factory={:p}, index={:d} )", __FUNCTION__, static_cast< void const* >( factory ), index );
  return s_plugins[index].desc;
}

clap_plugin_t const* plugin_factory_create_plugin( clap_plugin_factory const* factory, clap_host_t const* host, char const* plugin_id ) {
  if( ClapGlobals::PLUGIN_LOGGER )
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( factory={:p}, host={:p}, plugin_id={:?} )",
                                       __FUNCTION__,
                                       static_cast< void const* >( factory ),
                                       static_cast< void const* >( host ),
                                       plugin_id );
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

  std::filesystem::path tempFolder = std::getenv( "TEMP" );
  auto fileSink = std::make_shared< spdlog::sinks::basic_file_sink_mt >( ( tempFolder / "SfgGenerator.log" ).string(), false );
  fileSink->set_level( spdlog::level::level_enum::trace );
  ClapGlobals::PLUGIN_LOGGER = std::make_shared< spdlog::logger >( "main", fileSink );
  ClapGlobals::PLUGIN_LOGGER->set_level( spdlog::level::level_enum::trace );
  ClapGlobals::PLUGIN_LOGGER->flush_on( spdlog::level::level_enum::trace );
  ClapGlobals::PLUGIN_LOGGER->set_pattern( "[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%n] [%l] %v" );
  // spdlog::register_logger( ClapGlobals::PLUGIN_LOGGER );

  ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( plugin_path={:?} )", __FUNCTION__, plugin_path );

  ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] Initializing luigi2", __FUNCTION__ );
  UIInitialise();
  ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] Initialized luigi2", __FUNCTION__ );

  return true;
}

void entry_deinit( void ) {
  if( ClapGlobals::PLUGIN_LOGGER ) {
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter()", __FUNCTION__ );
  }
  // perform the plugin de-initialization
  spdlog::shutdown();
  ClapGlobals::PLUGIN_PATH.clear();
}

void const* entry_get_factory( char const* factory_id ) {
  if( ClapGlobals::PLUGIN_LOGGER )
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( factory_id={:?} )", __FUNCTION__, factory_id );
  if( !strcmp( factory_id, CLAP_PLUGIN_FACTORY_ID ) )
    return &s_plugin_factory;
  return nullptr;
}

static std::mutex G_ENTRY_LOCK;
static std::atomic_int32_t G_ENTRY_INIT_COUNTER = 0;

// thread safe init counter
bool entry_init_guard( char const* plugin_path ) {
  std::lock_guard< std::mutex > _( G_ENTRY_LOCK );
  int const cnt = ++G_ENTRY_INIT_COUNTER;
  if( cnt > 1 )
    return true;
  if( entry_init( plugin_path ) )
    return true;
  G_ENTRY_INIT_COUNTER = 0;
  return false;
}

// thread safe deinit counter
void entry_deinit_guard( void ) {
  std::lock_guard< std::mutex > _( G_ENTRY_LOCK );
  int const cnt = --G_ENTRY_INIT_COUNTER;
  if( cnt == 0 ) {
    entry_deinit();
  }
}

void const* entry_get_factory_guard( char const* factory_id ) {
  if( ( G_ENTRY_INIT_COUNTER <= 0 ) )
    return nullptr;
  return entry_get_factory( factory_id );
}

#if __cplusplus
extern "C" {
#endif

CLAP_EXPORT clap_plugin_entry_t const clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init_guard,
    .deinit = entry_deinit_guard,
    .get_factory = entry_get_factory_guard,
};

#if __cplusplus
}
#endif
