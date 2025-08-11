#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "_clap.hpp"
#include "_spdlog.hpp"

#define SFG_LOG_TRACE( logger, fmt_string, ... ) \
  if( logger )                                   \
    logger->trace( fmt_string, ##__VA_ARGS__ );  \
  std::cout << fmt::format( fmt_string, ##__VA_ARGS__ ) << std::endl;

#define SFG_LOG_DEBUG( logger, fmt_string, ... ) \
  if( logger )                                   \
    logger->debug( fmt_string, ##__VA_ARGS__ );  \
  std::cout << fmt::format( fmt_string, ##__VA_ARGS__ ) << std::endl;

#define SFG_LOG_INFO( logger, fmt_string, ... ) \
  if( logger )                                  \
    logger->info( fmt_string, ##__VA_ARGS__ );  \
  std::cout << fmt::format( fmt_string, ##__VA_ARGS__ ) << std::endl;

#define SFG_LOG_WARN( logger, fmt_string, ... ) \
  if( logger )                                  \
    logger->warn( fmt_string, ##__VA_ARGS__ );  \
  std::cout << fmt::format( fmt_string, ##__VA_ARGS__ ) << std::endl;

#define SFG_LOG_ERROR( logger, fmt_string, ... ) \
  if( logger )                                   \
    logger->error( fmt_string, ##__VA_ARGS__ );  \
  std::cout << fmt::format( fmt_string, ##__VA_ARGS__ ) << std::endl;

#define SFG_LOG_CRITICAL( logger, fmt_string, ... ) \
  if( logger )                                      \
    logger->critical( fmt_string, ##__VA_ARGS__ );  \
  std::cout << fmt::format( fmt_string, ##__VA_ARGS__ ) << std::endl;

// #undef SFG_LOG_TRACE
// #undef SFG_LOG_DEBUG
// #undef SFG_LOG_INFO
// #undef SFG_LOG_WARN
// #undef SFG_LOG_ERROR
// #undef SFG_LOG_CRITICAL

// #define SFG_LOG_TRACE( logger, fmt_string, ... )
// #define SFG_LOG_DEBUG( logger, fmt_string, ... )
// #define SFG_LOG_INFO( logger, fmt_string, ... )
// #define SFG_LOG_WARN( logger, fmt_string, ... )
// #define SFG_LOG_ERROR( logger, fmt_string, ... )
// #define SFG_LOG_CRITICAL( logger, fmt_string, ... )

#if __cplusplus
extern "C" {
#endif

class BasePlugin {
  public:
  BasePlugin();
  virtual ~BasePlugin();

  protected:
  virtual std::string get_name( void ) const;
  virtual void init_logger( void );

  // shit to override
  public:
  virtual bool init( void );
  virtual void destroy( void );
  virtual bool activate( double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count );
  virtual void deactivate( void );
  virtual bool start_processing( void );
  virtual void stop_processing( void );
  virtual void on_main_thread( void );
  virtual void reset( void );
  virtual void process_event( clap_event_header_t const* hdr ) = 0;
  virtual clap_process_status process( clap_process_t const* process ) = 0;

  // CLAP extensions, these will have basic implementations
  public:
  virtual bool ambisonic_is_config_supported( clap_ambisonic_config_t const* config );
  virtual bool ambisonic_get_config( bool is_input, uint32_t port_index, clap_ambisonic_config_t* out_config );
  virtual bool audio_ports_activation_can_activate_while_processing( void );
  virtual uint32_t audio_ports_config_count( void );
  virtual bool audio_ports_config_get( uint32_t index, clap_audio_ports_config_t* out_config );
  virtual bool audio_ports_config_select( clap_id config_id );
  virtual clap_id audio_ports_config_info_current_config( void );
  virtual bool audio_ports_config_info_get( clap_id config_id, uint32_t port_index, bool is_input, clap_audio_port_info_t* out_info );
  virtual uint32_t audio_ports_count( bool is_input );
  virtual bool audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info );
  virtual bool configurable_audio_ports_can_apply_configuration( clap_audio_port_configuration_request const* requests, uint32_t request_count );
  virtual bool configurable_audio_ports_apply_configuration( clap_audio_port_configuration_request const* requests, uint32_t request_count );
  virtual bool context_menu_populate( clap_context_menu_target_t const* target, clap_context_menu_builder_t const* builder );
  virtual bool context_menu_perform( clap_context_menu_target_t const* target, clap_id action_id );
  virtual bool gui_is_api_supported( std::string const& api, bool is_floating );
  virtual bool gui_get_preferred_api( std::string& out_api, bool* out_is_floating );
  virtual bool gui_create( std::string const& api, bool is_floating );
  virtual void gui_destroy( void );
  virtual bool gui_set_scale( double scale );
  virtual bool gui_get_size( uint32_t* out_width, uint32_t* out_height );
  virtual bool gui_can_resize( void );
  virtual bool gui_get_resize_hints( clap_gui_resize_hints_t* out_hints );
  virtual bool gui_adjust_size( uint32_t* out_width, uint32_t* out_height );
  virtual bool gui_set_size( uint32_t width, uint32_t height );
  virtual bool gui_set_parent( clap_window_t const* window );
  virtual bool gui_set_transient( clap_window_t const* window );
  virtual void gui_suggest_title( std::string const& title );
  virtual bool gui_show( void );
  virtual bool gui_hide( void );
  virtual uint32_t latency_get( void );
  virtual uint32_t note_name_count( void );
  virtual bool note_name_get( uint32_t index, clap_note_name_t* out_note_name );
  virtual uint32_t note_ports_count( bool is_input );
  virtual bool note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info );
  virtual void param_indication_set_mapping( clap_id param_id,
                                             bool has_mapping,
                                             clap_color_t const* color,
                                             std::string const& label,
                                             std::string const& description );
  virtual void param_indication_set_automation( clap_id param_id, uint32_t automation_state, clap_color_t const* color );
  virtual uint32_t params_count( void );
  virtual bool params_get_info( uint32_t param_index, clap_param_info_t* out_param_info );
  virtual bool params_get_value( clap_id param_id, double* out_value );
  virtual bool params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity );
  virtual bool params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value );
  virtual void params_flush( clap_input_events_t const* in, clap_output_events_t const* out );
  virtual void posix_fd_support_on_fd( int fd, clap_posix_fd_flags_t flags );
  virtual bool preset_load_from_location( uint32_t location_kind, std::string const& location, std::string const& load_key );
  virtual uint32_t remote_controls_count( void );
  virtual bool remote_controls_get( uint32_t page_index, clap_remote_controls_page_t* out_page );
  virtual bool render_has_hard_realtime_requirement( void );
  virtual bool render_set( clap_plugin_render_mode mode );
  virtual bool state_context_save( clap_ostream_t const* stream, uint32_t context_type );
  virtual bool state_context_load( clap_istream_t const* stream, uint32_t context_type );
  virtual bool state_save( clap_ostream_t const* stream );
  virtual bool state_load( clap_istream_t const* stream );
  virtual bool surround_is_channel_mask_supported( uint64_t channel_mask );
  virtual uint32_t surround_get_channel_map( bool is_input, uint32_t port_index, uint8_t* out_channel_map, uint32_t channel_map_capacity );
  virtual uint32_t tail_get( void );
  virtual void thread_pool_exec( uint32_t task_index );
  virtual void timer_support_on_timer( clap_id timer_id );
  virtual void track_info_changed( void );
  virtual bool voice_info_get( clap_voice_info_t* out_info );

  // CLAP extensions, wether or not to pointer things to clap
  protected:
  virtual bool supports_ambisonic( void ) const;
  virtual bool supports_audio_ports_activation( void ) const;
  virtual bool supports_audio_ports_config( void ) const;
  virtual bool supports_audio_ports( void ) const;
  virtual bool supports_configurable_audio_ports( void ) const;
  virtual bool supports_context_menu( void ) const;
  virtual bool supports_gui( void ) const;
  virtual bool supports_latency( void ) const;
  virtual bool supports_note_name( void ) const;
  virtual bool supports_note_ports( void ) const;
  virtual bool supports_param_indication( void ) const;
  virtual bool supports_params( void ) const;
  virtual bool supports_posix_fd_support( void ) const;
  virtual bool supports_preset_load( void ) const;
  virtual bool supports_remote_controls( void ) const;
  virtual bool supports_render( void ) const;
  virtual bool supports_state_context( void ) const;
  virtual bool supports_state( void ) const;
  virtual bool supports_surround( void ) const;
  virtual bool supports_tail( void ) const;
  virtual bool supports_thread_pool( void ) const;
  virtual bool supports_timer_support( void ) const;
  virtual bool supports_track_info( void ) const;
  virtual bool supports_voice_info( void ) const;

  // shit for CLAP, these are gonna be pointer'd to clap
  public:
  static bool s_init( clap_plugin const* plugin );
  static void s_destroy( clap_plugin const* plugin );
  static bool s_activate( clap_plugin const* plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count );
  static void s_deactivate( clap_plugin const* plugin );
  static bool s_start_processing( clap_plugin const* plugin );
  static void s_stop_processing( clap_plugin const* plugin );
  static void s_on_main_thread( clap_plugin const* plugin );
  static void s_reset( clap_plugin const* plugin );
  static clap_process_status s_process( clap_plugin const* plugin, clap_process_t const* process );
  static void const* s_get_extension( clap_plugin const* plugin, char const* id );

  // CLAP extensions
  private:
  static bool s_ambisonic_is_config_supported( clap_plugin_t const* plugin, clap_ambisonic_config_t const* config );
  static bool s_ambisonic_get_config( clap_plugin_t const* plugin, bool is_input, uint32_t port_index, clap_ambisonic_config_t* out_config );
  static bool s_audio_ports_activation_can_activate_while_processing( clap_plugin_t const* plugin );
  static uint32_t s_audio_ports_config_count( clap_plugin_t const* plugin );
  static bool s_audio_ports_config_get( clap_plugin_t const* plugin, uint32_t index, clap_audio_ports_config_t* out_config );
  static bool s_audio_ports_config_select( clap_plugin_t const* plugin, clap_id config_id );
  static clap_id s_audio_ports_config_info_current_config( clap_plugin_t const* plugin );
  static bool s_audio_ports_config_info_get( clap_plugin_t const* plugin,
                                             clap_id config_id,
                                             uint32_t port_index,
                                             bool is_input,
                                             clap_audio_port_info_t* out_info );
  static uint32_t s_audio_ports_count( clap_plugin_t const* plugin, bool is_input );
  static bool s_audio_ports_get( clap_plugin_t const* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* out_info );
  static bool s_configurable_audio_ports_can_apply_configuration( clap_plugin_t const* plugin,
                                                                  clap_audio_port_configuration_request const* requests,
                                                                  uint32_t request_count );
  static bool s_configurable_audio_ports_apply_configuration( clap_plugin_t const* plugin,
                                                              clap_audio_port_configuration_request const* requests,
                                                              uint32_t request_count );
  static bool s_context_menu_populate( clap_plugin_t const* plugin, clap_context_menu_target_t const* target, clap_context_menu_builder_t const* builder );
  static bool s_context_menu_perform( clap_plugin_t const* plugin, clap_context_menu_target_t const* target, clap_id action_id );
  static bool s_gui_is_api_supported( clap_plugin_t const* plugin, char const* api, bool is_floating );
  static bool s_gui_get_preferred_api( clap_plugin_t const* plugin, char const** out_api, bool* out_is_floating );
  static bool s_gui_create( clap_plugin_t const* plugin, char const* api, bool is_floating );
  static void s_gui_destroy( clap_plugin_t const* plugin );
  static bool s_gui_set_scale( clap_plugin_t const* plugin, double scale );
  static bool s_gui_get_size( clap_plugin_t const* plugin, uint32_t* out_width, uint32_t* out_height );
  static bool s_gui_can_resize( clap_plugin_t const* plugin );
  static bool s_gui_get_resize_hints( clap_plugin_t const* plugin, clap_gui_resize_hints_t* out_hints );
  static bool s_gui_adjust_size( clap_plugin_t const* plugin, uint32_t* out_width, uint32_t* out_height );
  static bool s_gui_set_size( clap_plugin_t const* plugin, uint32_t width, uint32_t height );
  static bool s_gui_set_parent( clap_plugin_t const* plugin, clap_window_t const* window );
  static bool s_gui_set_transient( clap_plugin_t const* plugin, clap_window_t const* window );
  static void s_gui_suggest_title( clap_plugin_t const* plugin, char const* title );
  static bool s_gui_show( clap_plugin_t const* plugin );
  static bool s_gui_hide( clap_plugin_t const* plugin );
  static uint32_t s_latency_get( clap_plugin_t const* plugin );
  static uint32_t s_note_name_count( clap_plugin_t const* plugin );
  static bool s_note_name_get( clap_plugin_t const* plugin, uint32_t index, clap_note_name_t* out_note_name );
  static uint32_t s_note_ports_count( clap_plugin_t const* plugin, bool is_input );
  static bool s_note_ports_get( clap_plugin_t const* plugin, uint32_t index, bool is_input, clap_note_port_info_t* out_info );
  static void s_param_indication_set_mapping( clap_plugin_t const* plugin,
                                              clap_id param_id,
                                              bool has_mapping,
                                              clap_color_t const* color,
                                              char const* label,
                                              char const* description );
  static void s_param_indication_set_automation( clap_plugin_t const* plugin, clap_id param_id, uint32_t automation_state, clap_color_t const* color );
  static uint32_t s_params_count( clap_plugin_t const* plugin );
  static bool s_params_get_info( clap_plugin_t const* plugin, uint32_t param_index, clap_param_info_t* out_param_info );
  static bool s_params_get_value( clap_plugin_t const* plugin, clap_id param_id, double* out_value );
  static bool s_params_value_to_text( clap_plugin_t const* plugin, clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity );
  static bool s_params_text_to_value( clap_plugin_t const* plugin, clap_id param_id, char const* param_value_text, double* out_value );
  static void s_params_flush( clap_plugin_t const* plugin, clap_input_events_t const* in, clap_output_events_t const* out );
  static void s_posix_fd_support_on_fd( clap_plugin_t const* plugin, int fd, clap_posix_fd_flags_t flags );
  static bool s_preset_load_from_location( clap_plugin_t const* plugin, uint32_t location_kind, char const* location, char const* load_key );
  static uint32_t s_remote_controls_count( clap_plugin_t const* plugin );
  static bool s_remote_controls_get( clap_plugin_t const* plugin, uint32_t page_index, clap_remote_controls_page_t* out_page );
  static bool s_render_has_hard_realtime_requirement( clap_plugin_t const* plugin );
  static bool s_render_set( clap_plugin_t const* plugin, clap_plugin_render_mode mode );
  static bool s_state_context_save( clap_plugin_t const* plugin, clap_ostream_t const* stream, uint32_t context_type );
  static bool s_state_context_load( clap_plugin_t const* plugin, clap_istream_t const* stream, uint32_t context_type );
  static bool s_state_save( clap_plugin_t const* plugin, clap_ostream_t const* stream );
  static bool s_state_load( clap_plugin_t const* plugin, clap_istream_t const* stream );
  static bool s_surround_is_channel_mask_supported( clap_plugin_t const* plugin, uint64_t channel_mask );
  static uint32_t s_surround_get_channel_map( clap_plugin_t const* plugin,
                                              bool is_input,
                                              uint32_t port_index,
                                              uint8_t* out_channel_map,
                                              uint32_t channel_map_capacity );
  static uint32_t s_tail_get( clap_plugin_t const* plugin );
  static void s_thread_pool_exec( clap_plugin_t const* plugin, uint32_t task_index );
  static void s_timer_support_on_timer( clap_plugin_t const* plugin, clap_id timer_id );
  static void s_track_info_changed( clap_plugin_t const* plugin );
  static bool s_voice_info_get( clap_plugin_t const* plugin, clap_voice_info_t* out_info );

  private:
  void const* get_extension( std::string const& id ) const;

  protected:
  std::shared_ptr< spdlog::logger > logger_ = nullptr;
  std::atomic_bool active_ = true;
  std::atomic_bool process_ = false;
  double sample_rate_ = 44100.0;

  // CLAP extensions, these are gonna be pointer'd to clap
  protected:
  clap_plugin_ambisonic_t clap_ambisonic_;
  clap_plugin_audio_ports_activation_t clap_audio_ports_activation_;
  clap_plugin_audio_ports_config_t clap_audio_ports_config_;
  clap_plugin_audio_ports_config_info_t clap_audio_ports_config_info_;
  clap_plugin_audio_ports_t clap_audio_ports_;  // important in general!
  clap_plugin_configurable_audio_ports_t clap_configurable_audio_ports_;
  clap_plugin_context_menu_t clap_context_menu_;
  clap_plugin_gui_t clap_gui_;
  clap_plugin_latency_t clap_latency_;
  clap_plugin_note_name_t clap_note_name_;
  clap_plugin_note_ports_t clap_note_ports_;  // important in general!
  clap_plugin_param_indication_t clap_param_indication_;
  clap_plugin_params_t clap_params_;  // important for params!
  clap_plugin_posix_fd_support_t clap_posix_fd_support_;
  clap_plugin_preset_load_t clap_preset_load_;
  clap_plugin_remote_controls_t clap_remote_controls_;
  clap_plugin_render_t clap_render_;
  clap_plugin_state_context_t clap_state_context_;
  clap_plugin_state_t clap_state_;
  clap_plugin_surround_t clap_surround_;
  clap_plugin_tail_t clap_tail_;
  clap_plugin_thread_pool_t clap_thread_pool_;
  clap_plugin_timer_support_t clap_timer_support_;
  clap_plugin_track_info_t clap_track_info_;
  clap_plugin_voice_info_t clap_voice_info_;

  protected:
  clap_host_t const* host_ = nullptr;

  clap_host_ambisonic_t const* host_ambisonic_ = nullptr;
  clap_host_audio_ports_config_t const* host_audio_ports_config_ = nullptr;
  clap_host_audio_ports_t const* host_audio_ports_ = nullptr;  // important in general!
  clap_host_context_menu_t const* host_context_menu_ = nullptr;
  clap_host_event_registry_t const* host_event_registry_ = nullptr;
  clap_host_gui_t const* host_gui_ = nullptr;
  clap_host_latency_t const* host_latency_ = nullptr;
  clap_host_log_t const* host_log_ = nullptr;
  clap_host_note_name_t const* host_note_name_ = nullptr;
  clap_host_note_ports_t const* host_note_ports_ = nullptr;  // important in general!
  clap_host_params_t const* host_params_ = nullptr;          // important for params!
  clap_host_posix_fd_support_t const* host_posix_fd_support_ = nullptr;
  clap_host_preset_load_t const* host_preset_load_ = nullptr;
  clap_host_remote_controls_t const* host_remote_controls_ = nullptr;
  clap_host_state_t const* host_state_ = nullptr;
  clap_host_surround_t const* host_surround_ = nullptr;
  clap_host_tail_t const* host_tail_ = nullptr;
  clap_host_thread_check_t const* host_thread_check_ = nullptr;
  clap_host_thread_pool_t const* host_thread_pool_ = nullptr;
  clap_host_timer_support_t const* host_timer_support_ = nullptr;
  clap_host_track_info_t const* host_track_info_ = nullptr;
  clap_host_voice_info_t const* host_voice_info_ = nullptr;
};

#if __cplusplus
}
#endif

template < typename T >
bool read_multi_byte( clap_istream_t const* stream, T* value, size_t size ) {
  bool ret = true;
  uintmax_t* val_ptr = reinterpret_cast< uintmax_t* >( value );
  for( size_t i = 0; i < size; i++ ) {
    uint8_t val{};
    ret = ret && stream->read( stream, &val, 1 );
    ( *val_ptr ) = ( *val_ptr ) | ( static_cast< uintmax_t >( val ) << ( i * 8 ) );
  }
  return ret;
}

template < typename T >
bool write_multi_byte( clap_ostream_t const* stream, T value, size_t size ) {
  bool ret = true;
  uintmax_t val_converted;
  uintmax_t* val_ptr = &val_converted;
  ( *val_ptr ) = ( *reinterpret_cast< uintmax_t* >( &value ) );
  for( size_t i = 0; i < size; i++ ) {
    uint8_t val = static_cast< uint8_t >( ( val_converted >> ( i * 8 ) ) & 0xFF );
    ret = ret && stream->write( stream, &val, 1 );
  }
  return ret;
}
