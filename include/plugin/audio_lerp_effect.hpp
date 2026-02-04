#pragma once

// Project includes
#include "plugin/base_plugin.hpp"
#include "plugin/audio_lerp_effect.pb.h"

// Project includes
#include <common/_clap.hpp>
#include <common/_fmt.hpp>
#include <ui/UiAleHolder.hpp>

// C++ std includes
#include <cstdint>
#include <string>

class AudioLerpEffect : BasePlugin {
  using _base_ = BasePlugin;
  using _base_::_base_;
  using _pb_ = SfgGenerator::Proto::AudioLerpEffect;

  public:
  AudioLerpEffect();
  virtual ~AudioLerpEffect();

  protected:
  std::string get_name( void ) const override;

  // shit to override
  public:
  bool init( void ) override;
  void on_main_thread( void ) override;
  void reset( void ) override;
  void process_event( clap_event_header_t const* hdr, clap_output_events_t const* out_events ) override;
  clap_process_status process( clap_process_t const* process ) override;

  // CLAP extensions
  public:
  uint32_t audio_ports_count( bool is_input ) override;
  bool audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) override;
  bool gui_is_api_supported( std::string const& api, bool is_floating ) override;
  bool gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) override;
  bool gui_create( std::string const& api, bool is_floating ) override;
  void gui_destroy( void ) override;
  bool gui_set_scale( double scale ) override;
  bool gui_get_size( uint32_t* out_width, uint32_t* out_height ) override;
  bool gui_can_resize( void ) override;
  bool gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) override;
  bool gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) override;
  bool gui_set_size( uint32_t width, uint32_t height ) override;
  bool gui_set_parent( clap_window_t const* window ) override;
  bool gui_set_transient( clap_window_t const* window ) override;
  void gui_suggest_title( std::string const& title ) override;
  bool gui_show( void ) override;
  bool gui_hide( void ) override;
  uint32_t params_count( void ) override;
  bool params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) override;
  bool params_get_value( clap_id param_id, double* out_value ) override;
  bool params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) override;
  bool params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) override;
  void params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) override;
  bool state_save( clap_ostream_t const* stream ) override;
  bool state_load( clap_istream_t const* stream ) override;

  // CLAP extensions, wether or not to pointer things to clap
  protected:
  bool supports_audio_ports() const override;
  bool supports_gui() const override;
  bool supports_params() const override;
  bool supports_state() const override;

  protected:
  UiAleHolder uiAleHolder_;

  // members to save and load
  protected:
  // params
  _pb_ state_;

  // shit for the factory
  public:
  static clap_plugin_t* s_create( clap_host_t const* host );
  static clap_plugin_descriptor_t* descriptor_get( void );
};
