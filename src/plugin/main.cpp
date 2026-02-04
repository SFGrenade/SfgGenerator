// Header assigned to this source
#include "plugin/main.hpp"

// Project includes
#include "plugin/audio_lerp_effect.hpp"
#include "plugin/noise_generator.hpp"
#include "plugin/param_multiplex.hpp"

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
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( factory={:p}, host={:p}, plugin_id={:?} )", __FUNCTION__, static_cast< void const* >( factory ), static_cast< void const* >( host ), plugin_id );
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

  auto fileSink = std::make_shared< spdlog::sinks::basic_file_sink_mt >( "C:\\Users\\SFG\\Downloads\\log.log", true );
  fileSink->set_level( spdlog::level::level_enum::trace );
  ClapGlobals::PLUGIN_LOGGER = std::make_shared< spdlog::logger >( "main", fileSink );
  ClapGlobals::PLUGIN_LOGGER->set_level( spdlog::level::level_enum::trace );
  ClapGlobals::PLUGIN_LOGGER->flush_on( spdlog::level::level_enum::trace );
  ClapGlobals::PLUGIN_LOGGER->set_pattern( "[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%n] [%l] %v" );
  // spdlog::register_logger( ClapGlobals::PLUGIN_LOGGER );

  ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter( plugin_path={:?} )", __FUNCTION__, plugin_path );
  return true;
}

void entry_deinit( void ) {
  if( ClapGlobals::PLUGIN_LOGGER )
    ClapGlobals::PLUGIN_LOGGER->trace( "[{:s}] enter()", __FUNCTION__ );
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
