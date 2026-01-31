#pragma once

// Project includes
#include "base_plugin.hpp"
#include "param_multiplex.pb.h"

// C++ std includes
#include <cstdint>
#include <string>

#if __cplusplus
extern "C" {
#endif

class ParamMultiplex : BasePlugin {
  using _base_ = BasePlugin;
  using _base_::_base_;
  using _pb_ = SfgGenerator::Proto::ParamMultiplex;

  public:
  ParamMultiplex();
  virtual ~ParamMultiplex();

  protected:
  std::string get_name( void ) const override;

  // shit to override
  public:
  bool init( void ) override;
  void deactivate( void ) override;
  void reset( void ) override;
  void process_event( clap_event_header_t const* hdr, clap_output_events_t const* out_events ) override;
  clap_process_status process( clap_process_t const* process ) override;

  // CLAP extensions
  public:
  uint32_t params_count( void ) override;
  bool params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) override;
  bool params_get_value( clap_id param_id, double* out_value ) override;
  bool params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) override;
  bool params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) override;
  void params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) override;
  uint32_t remote_controls_count( void ) override;
  bool remote_controls_get( uint32_t page_index, clap_remote_controls_page_t* out_page ) override;
  bool state_save( clap_ostream_t const* stream ) override;
  bool state_load( clap_istream_t const* stream ) override;

  // CLAP extensions, wether or not to pointer things to clap
  protected:
  bool supports_params() const override;
  bool supports_remote_controls( void ) const override;
  bool supports_state() const override;

  // members to save and load
  protected:
  // params
  _pb_ state_;
  bool doClearAndRescan_;

  // shit for the factory
  public:
  static clap_plugin_t* s_create( clap_host_t const* host );
  static clap_plugin_descriptor_t* descriptor_get( void );
};

#if __cplusplus
}
#endif
