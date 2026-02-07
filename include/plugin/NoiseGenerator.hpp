#pragma once

// Project includes
#include "plugin/BasePlugin.hpp"
#include "plugin/NoiseGenerator.pb.h"

// Project includes
#include <common/_clap.hpp>
#include <common/_fmt.hpp>
#include <ui/UiNgHolder.hpp>

// C++ std includes
#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <vector>

class NoiseGenerator : BasePlugin {
  using _base_ = BasePlugin;
  using _base_::_base_;
  using _pb_ = SfgGenerator::Proto::NoiseGenerator;

  // see `clap_event_note_t`, this is a map of key to note
  struct NoteData {
    double phase = 0.0;
    double velocity = 0.0;  // aka amplitude
  };
  typedef std::map< int16_t, NoteData > NoteMap;

  public:
  NoiseGenerator();
  virtual ~NoiseGenerator();

  protected:
  std::string get_name( void ) const override;

  // wave generators
  private:
  double get_sample_sine_wave( double phase );
  double get_sample_square_wave( double phase );
  double get_sample_saw_wave( double phase );
  double get_sample_triangle_wave( double phase );
  double get_sample_white_noise( double phase );
  double get_sample_pink_noise( double phase );
  double get_sample_red_noise( double phase );
  double get_sample_blue_noise( double phase );
  double get_sample_violet_noise( double phase );
  double get_sample_grey_noise( double phase );
  double get_sample_velvet_noise( double phase );

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
  uint32_t note_ports_count( bool is_input ) override;
  bool note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info ) override;
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
  bool supports_note_ports() const override;
  bool supports_params() const override;
  bool supports_state() const override;

  protected:
  std::mt19937_64 eng_;
  std::uniform_real_distribution< double > dist_;
  UiNgHolder uiNgHolder_;

  // members to save and load
  protected:
  // params
  _pb_ state_;

  // internal values (no need to save these)
  double pink_refined_b0_ = 0.0;
  double pink_refined_b1_ = 0.0;
  double pink_refined_b2_ = 0.0;
  double pink_refined_b3_ = 0.0;
  double pink_refined_b4_ = 0.0;
  double pink_refined_b5_ = 0.0;
  double pink_refined_b6_ = 0.0;

  double pink_economy_b0_ = 0.0;
  double pink_economy_b1_ = 0.0;
  double pink_economy_b2_ = 0.0;

  uint64_t pink_VossMcCartney_sample_ = 0;
  std::vector< double > pink_VossMcCartney_streams_;

  double red_leaky_integrator_prev_ = 0.0;

  // temporary values
  NoteMap note_map_;

  // shit for the factory
  public:
  static clap_plugin_t* s_create( clap_host_t const* host );
  static clap_plugin_descriptor_t* descriptor_get( void );

  // for checking for changes
  private:
  double last_sineWaveType_ = -1.0;
  double last_sineWaveMix_ = -1.0;
  double last_squareWaveType_ = -1.0;
  double last_squareWavePwm_ = -1.0;
  double last_squareWaveMix_ = -1.0;
  double last_sawWaveType_ = -1.0;
  double last_sawWaveMix_ = -1.0;
  double last_triangleWaveType_ = -1.0;
  double last_triangleWaveMix_ = -1.0;
  double last_whiteNoiseType_ = -1.0;
  double last_whiteNoiseMix_ = -1.0;
  double last_pinkNoiseType_ = -1.0;
  double last_pinkNoiseMix_ = -1.0;
  double last_redNoiseType_ = -1.0;
  double last_redNoiseMix_ = -1.0;
  double last_blueNoiseType_ = -1.0;
  double last_blueNoiseMix_ = -1.0;
  double last_violetNoiseType_ = -1.0;
  double last_violetNoiseMix_ = -1.0;
  double last_greyNoiseType_ = -1.0;
  double last_greyNoiseMix_ = -1.0;
  double last_velvetNoiseType_ = -1.0;
  double last_velvetNoiseMix_ = -1.0;
};
