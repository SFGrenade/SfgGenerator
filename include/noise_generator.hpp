#pragma once

#include <random>

#include "base_plugin.hpp"

#if __cplusplus
extern "C" {
#endif

class NoiseGenerator : BasePlugin {
  using _base_ = BasePlugin;
  using _base_::_base_;

  enum class SynthType : uint16_t {
    SineWave,
    SquareWave,
    SawWave,
    TriangleWave,
    WhiteNoise,
    PinkNoise,
    RedNoise,
    BlueNoise,
    VioletNoise,
    GreyNoise,
    VelvetNoise,
    MAX_VALUE,
  };
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
  void reset( void ) override;
  void process_event( clap_event_header_t const* hdr ) override;
  clap_process_status process( clap_process_t const* process ) override;

  // CLAP extensions
  public:
  uint32_t audio_ports_count( bool is_input ) override;
  bool audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) override;
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
  bool supports_note_ports() const override;
  bool supports_params() const override;
  bool supports_state() const override;

  protected:
  std::mt19937_64 eng_;
  std::uniform_real_distribution< double > dist_;

  // members to save and load
  protected:
  SynthType synth_type_ = SynthType::SineWave;
  double pink_refined_b0_ = 0.0;
  double pink_refined_b1_ = 0.0;
  double pink_refined_b2_ = 0.0;
  double pink_refined_b3_ = 0.0;
  double pink_refined_b4_ = 0.0;
  double pink_refined_b5_ = 0.0;
  double pink_refined_b6_ = 0.0;
  double red_leaky_integrator_prev_ = 0.0;
  NoteMap note_map_;

  // shit for the factory
  public:
  static clap_plugin_t* s_create( clap_host_t const* host );
  static clap_plugin_descriptor_t* descriptor_get( void );
};

#if __cplusplus
}
#endif
