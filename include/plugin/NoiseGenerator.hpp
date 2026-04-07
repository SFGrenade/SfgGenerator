#pragma once

// Project includes
#include "common/NoteMap.hpp"
#include "common/Timer.hpp"
#include "common/_clap.hpp"
#include "common/_fmt.hpp"
#include "plugin/BasePlugin.hpp"
#include "plugin/NoiseGenerator.pb.h"
#include "widgets/AudioSampleDisplay.hpp"
#include "widgets/Button.hpp"
#include "widgets/Label.hpp"
#include "widgets/Slider.hpp"
#include "widgets/Widget.hpp"


// Other lib includes
#include <boost/lockfree/spsc_queue.hpp>

// C++ std includes
#include <cstdint>
#include <random>
#include <string>
#include <vector>

class NoiseGenerator : BasePlugin {
  using _base_ = BasePlugin;
  using _base_::_base_;
  using _pb_ = SfgGenerator::Proto::NoiseGenerator;

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
  std::shared_ptr< Widget > guiRootWidget_ = nullptr;
  // sine wave
  std::shared_ptr< Widget > guiWidgetSineWave_ = nullptr;
  std::shared_ptr< Button > guiWidgetSineWavePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetSineWaveCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetSineWaveNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetSineWaveMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetSineWaveSamples_ = nullptr;
  // square wave
  std::shared_ptr< Widget > guiWidgetSquareWave_ = nullptr;
  std::shared_ptr< Button > guiWidgetSquareWavePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetSquareWaveCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetSquareWaveNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetSquareWavePwm_ = nullptr;
  std::shared_ptr< Slider > guiWidgetSquareWaveMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetSquareWaveSamples_ = nullptr;
  // saw wave
  std::shared_ptr< Widget > guiWidgetSawWave_ = nullptr;
  std::shared_ptr< Button > guiWidgetSawWavePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetSawWaveCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetSawWaveNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetSawWaveMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetSawWaveSamples_ = nullptr;
  // triangle wave
  std::shared_ptr< Widget > guiWidgetTriangleWave_ = nullptr;
  std::shared_ptr< Button > guiWidgetTriangleWavePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetTriangleWaveCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetTriangleWaveNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetTriangleWaveMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetTriangleWaveSamples_ = nullptr;
  // white noise
  std::shared_ptr< Widget > guiWidgetWhiteNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetWhiteNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetWhiteNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetWhiteNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetWhiteNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetWhiteNoiseSamples_ = nullptr;
  // pink noise
  std::shared_ptr< Widget > guiWidgetPinkNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetPinkNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetPinkNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetPinkNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetPinkNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetPinkNoiseSamples_ = nullptr;
  // red noise
  std::shared_ptr< Widget > guiWidgetRedNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetRedNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetRedNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetRedNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetRedNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetRedNoiseSamples_ = nullptr;
  // blue noise
  std::shared_ptr< Widget > guiWidgetBlueNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetBlueNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetBlueNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetBlueNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetBlueNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetBlueNoiseSamples_ = nullptr;
  // violet noise
  std::shared_ptr< Widget > guiWidgetVioletNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetVioletNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetVioletNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetVioletNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetVioletNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetVioletNoiseSamples_ = nullptr;
  // grey noise
  std::shared_ptr< Widget > guiWidgetGreyNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetGreyNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetGreyNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetGreyNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetGreyNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetGreyNoiseSamples_ = nullptr;
  // velvet noise
  std::shared_ptr< Widget > guiWidgetVelvetNoise_ = nullptr;
  std::shared_ptr< Button > guiWidgetVelvetNoisePreviousType_ = nullptr;
  std::shared_ptr< Label > guiWidgetVelvetNoiseCurrentType_ = nullptr;
  std::shared_ptr< Button > guiWidgetVelvetNoiseNextType_ = nullptr;
  std::shared_ptr< Slider > guiWidgetVelvetNoiseMix_ = nullptr;
  std::shared_ptr< AudioSampleDisplay > guiWidgetVelvetNoiseSamples_ = nullptr;
  // gui
  std::shared_ptr< SDL_Window > guiWindow_ = nullptr;
  std::shared_ptr< SDL_Renderer > guiWindowRenderer_ = nullptr;
  std::unique_ptr< Timer > guiTimer_ = nullptr;
  // Display sample queues
  boost::lockfree::spsc_queue< float > sampleQueueSineWave_;
  boost::lockfree::spsc_queue< float > sampleQueueSquareWave_;
  boost::lockfree::spsc_queue< float > sampleQueueSawWave_;
  boost::lockfree::spsc_queue< float > sampleQueueTriangleWave_;
  boost::lockfree::spsc_queue< float > sampleQueueWhiteNoise_;
  boost::lockfree::spsc_queue< float > sampleQueuePinkNoise_;
  boost::lockfree::spsc_queue< float > sampleQueueRedNoise_;
  boost::lockfree::spsc_queue< float > sampleQueueBlueNoise_;
  boost::lockfree::spsc_queue< float > sampleQueueVioletNoise_;
  boost::lockfree::spsc_queue< float > sampleQueueGreyNoise_;
  boost::lockfree::spsc_queue< float > sampleQueueVelvetNoise_;

  void guiTimerCallback();

  protected:
  // todo: fixme: adjust to NoiseGenerator::*Type
  static std::array< std::string, _pb_::SineWaveType_ARRAYSIZE > const SINEWAVE_OPTIONS;
  static std::array< std::string, _pb_::SquareWaveType_ARRAYSIZE > const SQUAREWAVE_OPTIONS;
  static std::array< std::string, _pb_::SawWaveType_ARRAYSIZE > const SAWWAVE_OPTIONS;
  static std::array< std::string, _pb_::TriangleWaveType_ARRAYSIZE > const TRIANGLEWAVE_OPTIONS;
  static std::array< std::string, _pb_::WhiteNoiseType_ARRAYSIZE > const WHITENOISE_OPTIONS;
  static std::array< std::string, _pb_::PinkNoiseType_ARRAYSIZE > const PINKNOISE_OPTIONS;
  static std::array< std::string, _pb_::RedNoiseType_ARRAYSIZE > const REDNOISE_OPTIONS;
  static std::array< std::string, _pb_::BlueNoiseType_ARRAYSIZE > const BLUENOISE_OPTIONS;
  static std::array< std::string, _pb_::VioletNoiseType_ARRAYSIZE > const VIOLETNOISE_OPTIONS;
  static std::array< std::string, _pb_::GreyNoiseType_ARRAYSIZE > const GREYNOISE_OPTIONS;
  static std::array< std::string, _pb_::VelvetNoiseType_ARRAYSIZE > const VELVETNOISE_OPTIONS;

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
  NoteMap noteMap_;

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
