#pragma once

#include "_clap.hpp"
#include "audio_lerp_effect.hpp"
#include "noise_generator.hpp"

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
};
static uint32_t plugin_factory_get_plugin_count( const struct clap_plugin_factory* factory );
static clap_plugin_descriptor_t const* plugin_factory_get_plugin_descriptor( const struct clap_plugin_factory* factory, uint32_t index );
static clap_plugin_t const* plugin_factory_create_plugin( const struct clap_plugin_factory* factory, clap_host_t const* host, char const* plugin_id );
static clap_plugin_factory_t const s_plugin_factory = {
    .get_plugin_count = plugin_factory_get_plugin_count,
    .get_plugin_descriptor = plugin_factory_get_plugin_descriptor,
    .create_plugin = plugin_factory_create_plugin,
};

static bool entry_init( char const* plugin_path );
static void entry_deinit( void );

// thread safe init counter
static bool entry_init_guard( char const* plugin_path );
// thread safe deinit counter
static void entry_deinit_guard( void );
// factory
static void const* entry_get_factory( char const* factory_id );

CLAP_EXPORT clap_plugin_entry_t const clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init_guard,
    .deinit = entry_deinit_guard,
    .get_factory = entry_get_factory,
};
