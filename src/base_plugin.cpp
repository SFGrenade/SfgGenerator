#include "base_plugin.hpp"

#include <filesystem>
#include <iostream>
#include <numbers>

#include "_folders.hpp"

#pragma region Base Methods

BasePlugin::BasePlugin() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  clap_ambisonic_ = {
      .is_config_supported = BasePlugin::s_ambisonic_is_config_supported,
      .get_config = BasePlugin::s_ambisonic_get_config,
  };
  clap_audio_ports_activation_ = {
      .can_activate_while_processing = BasePlugin::s_audio_ports_activation_can_activate_while_processing,
  };
  clap_audio_ports_config_ = {
      .count = BasePlugin::s_audio_ports_config_count,
      .get = BasePlugin::s_audio_ports_config_get,
      .select = BasePlugin::s_audio_ports_config_select,
  };
  clap_audio_ports_config_info_ = {
      .current_config = BasePlugin::s_audio_ports_config_info_current_config,
      .get = BasePlugin::s_audio_ports_config_info_get,
  };
  clap_audio_ports_ = {
      .count = BasePlugin::s_audio_ports_count,
      .get = BasePlugin::s_audio_ports_get,
  };
  clap_configurable_audio_ports_ = {
      .can_apply_configuration = BasePlugin::s_configurable_audio_ports_can_apply_configuration,
      .apply_configuration = BasePlugin::s_configurable_audio_ports_apply_configuration,
  };
  clap_context_menu_ = {
      .populate = BasePlugin::s_context_menu_populate,
      .perform = BasePlugin::s_context_menu_perform,
  };
  clap_gui_ = {
      .is_api_supported = BasePlugin::s_gui_is_api_supported,
      .get_preferred_api = BasePlugin::s_gui_get_preferred_api,
      .create = BasePlugin::s_gui_create,
      .destroy = BasePlugin::s_gui_destroy,
      .set_scale = BasePlugin::s_gui_set_scale,
      .get_size = BasePlugin::s_gui_get_size,
      .can_resize = BasePlugin::s_gui_can_resize,
      .get_resize_hints = BasePlugin::s_gui_get_resize_hints,
      .adjust_size = BasePlugin::s_gui_adjust_size,
      .set_size = BasePlugin::s_gui_set_size,
      .set_parent = BasePlugin::s_gui_set_parent,
      .set_transient = BasePlugin::s_gui_set_transient,
      .suggest_title = BasePlugin::s_gui_suggest_title,
      .show = BasePlugin::s_gui_show,
      .hide = BasePlugin::s_gui_hide,
  };
  clap_latency_ = {
      .get = BasePlugin::s_latency_get,
  };
  clap_note_name_ = {
      .count = BasePlugin::s_note_name_count,
      .get = BasePlugin::s_note_name_get,
  };
  clap_note_ports_ = {
      .count = BasePlugin::s_note_ports_count,
      .get = BasePlugin::s_note_ports_get,
  };
  clap_param_indication_ = {
      .set_mapping = BasePlugin::s_param_indication_set_mapping,
      .set_automation = BasePlugin::s_param_indication_set_automation,
  };
  clap_params_ = {
      .count = BasePlugin::s_params_count,
      .get_info = BasePlugin::s_params_get_info,
      .get_value = BasePlugin::s_params_get_value,
      .value_to_text = BasePlugin::s_params_value_to_text,
      .text_to_value = BasePlugin::s_params_text_to_value,
      .flush = BasePlugin::s_params_flush,
  };
  clap_posix_fd_support_ = {
      .on_fd = BasePlugin::s_posix_fd_support_on_fd,
  };
  clap_preset_load_ = {
      .from_location = BasePlugin::s_preset_load_from_location,
  };
  clap_remote_controls_ = {
      .count = BasePlugin::s_remote_controls_count,
      .get = BasePlugin::s_remote_controls_get,
  };
  clap_render_ = {
      .has_hard_realtime_requirement = BasePlugin::s_render_has_hard_realtime_requirement,
      .set = BasePlugin::s_render_set,
  };
  clap_state_context_ = {
      .save = BasePlugin::s_state_context_save,
      .load = BasePlugin::s_state_context_load,
  };
  clap_state_ = {
      .save = BasePlugin::s_state_save,
      .load = BasePlugin::s_state_load,
  };
  clap_surround_ = {
      .is_channel_mask_supported = BasePlugin::s_surround_is_channel_mask_supported,
      .get_channel_map = BasePlugin::s_surround_get_channel_map,
  };
  clap_tail_ = {
      .get = BasePlugin::s_tail_get,
  };
  clap_thread_pool_ = {
      .exec = BasePlugin::s_thread_pool_exec,
  };
  clap_timer_support_ = {
      .on_timer = BasePlugin::s_timer_support_on_timer,
  };
  clap_track_info_ = {
      .changed = BasePlugin::s_track_info_changed,
  };
  clap_voice_info_ = {
      .get = BasePlugin::s_voice_info_get,
  };
}

BasePlugin::~BasePlugin() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string BasePlugin::get_name( void ) const {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "BasePlugin";
}

void BasePlugin::init_logger( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

#pragma endregion

#pragma region Overridable Init/Exit

bool BasePlugin::init( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  host_ambisonic_ = static_cast< clap_host_ambisonic_t const* >( host_->get_extension( host_, CLAP_EXT_AMBISONIC ) );
  host_audio_ports_config_ = static_cast< clap_host_audio_ports_config_t const* >( host_->get_extension( host_, CLAP_EXT_AUDIO_PORTS_CONFIG ) );
  host_audio_ports_ = static_cast< clap_host_audio_ports_t const* >( host_->get_extension( host_, CLAP_EXT_AUDIO_PORTS ) );
  host_context_menu_ = static_cast< clap_host_context_menu_t const* >( host_->get_extension( host_, CLAP_EXT_CONTEXT_MENU ) );
  host_event_registry_ = static_cast< clap_host_event_registry_t const* >( host_->get_extension( host_, CLAP_EXT_EVENT_REGISTRY ) );
  host_gui_ = static_cast< clap_host_gui_t const* >( host_->get_extension( host_, CLAP_EXT_GUI ) );
  host_latency_ = static_cast< clap_host_latency_t const* >( host_->get_extension( host_, CLAP_EXT_LATENCY ) );
  host_log_ = static_cast< clap_host_log_t const* >( host_->get_extension( host_, CLAP_EXT_LOG ) );
  host_note_name_ = static_cast< clap_host_note_name_t const* >( host_->get_extension( host_, CLAP_EXT_NOTE_NAME ) );
  host_note_ports_ = static_cast< clap_host_note_ports_t const* >( host_->get_extension( host_, CLAP_EXT_NOTE_PORTS ) );
  host_params_ = static_cast< clap_host_params_t const* >( host_->get_extension( host_, CLAP_EXT_PARAMS ) );
  host_posix_fd_support_ = static_cast< clap_host_posix_fd_support_t const* >( host_->get_extension( host_, CLAP_EXT_POSIX_FD_SUPPORT ) );
  host_preset_load_ = static_cast< clap_host_preset_load_t const* >( host_->get_extension( host_, CLAP_EXT_PRESET_LOAD ) );
  host_remote_controls_ = static_cast< clap_host_remote_controls_t const* >( host_->get_extension( host_, CLAP_EXT_REMOTE_CONTROLS ) );
  host_state_ = static_cast< clap_host_state_t const* >( host_->get_extension( host_, CLAP_EXT_STATE ) );
  host_surround_ = static_cast< clap_host_surround_t const* >( host_->get_extension( host_, CLAP_EXT_SURROUND ) );
  host_tail_ = static_cast< clap_host_tail_t const* >( host_->get_extension( host_, CLAP_EXT_TAIL ) );
  host_thread_check_ = static_cast< clap_host_thread_check_t const* >( host_->get_extension( host_, CLAP_EXT_THREAD_CHECK ) );
  host_thread_pool_ = static_cast< clap_host_thread_pool_t const* >( host_->get_extension( host_, CLAP_EXT_THREAD_POOL ) );
  host_timer_support_ = static_cast< clap_host_timer_support_t const* >( host_->get_extension( host_, CLAP_EXT_TIMER_SUPPORT ) );
  host_track_info_ = static_cast< clap_host_track_info_t const* >( host_->get_extension( host_, CLAP_EXT_TRACK_INFO ) );
  host_voice_info_ = static_cast< clap_host_voice_info_t const* >( host_->get_extension( host_, CLAP_EXT_VOICE_INFO ) );

  init_logger();

  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] g_plugin_path={:?}", __FUNCTION__, static_cast< void* >( this ), g_plugin_path );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_ambisonic={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_ambisonic_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_audio_ports_config={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_audio_ports_config_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_audio_ports={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_audio_ports_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_context_menu={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_context_menu_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_event_registry={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_event_registry_ ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host_gui={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_gui_ ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host_latency={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_latency_ ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host_log={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_log_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_note_name={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_note_name_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_note_ports={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_note_ports_ ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host_params={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_params_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_posix_fd_support={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_posix_fd_support_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_preset_load={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_preset_load_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_remote_controls={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_remote_controls_ ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host_state={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_state_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_surround={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_surround_ ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] host_tail={:p}", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( host_tail_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_thread_check={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_thread_check_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_thread_pool={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_thread_pool_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_timer_support={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_timer_support_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_track_info={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_track_info_ ) );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] host_voice_info={:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( host_voice_info_ ) );
  return true;
}

void BasePlugin::destroy( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  delete this;
}

bool BasePlugin::activate( double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] enter( sample_rate={:f}, min_frames_count={:d}, max_frames_count={:d} )",
                 __FUNCTION__,
                 sample_rate,
                 min_frames_count,
                 max_frames_count );
  sample_rate_ = sample_rate;
  active_ = true;
  return true;
}

void BasePlugin::deactivate( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  active_ = false;
}

bool BasePlugin::start_processing( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  process_ = true;
  return true;
}

void BasePlugin::stop_processing( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  process_ = false;
}

void BasePlugin::on_main_thread( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

void BasePlugin::reset( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );

  active_ = true;
  process_ = false;
  sample_rate_ = 44100.0;
}

#pragma endregion

#pragma region CLAP exts instance methods

bool BasePlugin::ambisonic_is_config_supported( clap_ambisonic_config_t const* config ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::ambisonic_get_config( bool is_input, uint32_t port_index, clap_ambisonic_config_t* out_config ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::audio_ports_activation_can_activate_while_processing() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

uint32_t BasePlugin::audio_ports_config_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::audio_ports_config_get( uint32_t index, clap_audio_ports_config_t* out_config ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::audio_ports_config_select( clap_id config_id ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

clap_id BasePlugin::audio_ports_config_info_current_config( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::audio_ports_config_info_get( clap_id config_id, uint32_t port_index, bool is_input, clap_audio_port_info_t* out_info ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

uint32_t BasePlugin::audio_ports_count( bool is_input ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::configurable_audio_ports_can_apply_configuration( clap_audio_port_configuration_request const* requests, uint32_t request_count ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::configurable_audio_ports_apply_configuration( clap_audio_port_configuration_request const* requests, uint32_t request_count ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::context_menu_populate( clap_context_menu_target_t const* target, clap_context_menu_builder_t const* builder ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::context_menu_perform( clap_context_menu_target_t const* target, clap_id action_id ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_is_api_supported( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_create( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

void BasePlugin::gui_destroy( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

bool BasePlugin::gui_set_scale( double scale ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_get_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_can_resize( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_set_size( uint32_t width, uint32_t height ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_set_parent( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_set_transient( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

void BasePlugin::gui_suggest_title( std::string const& title ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

bool BasePlugin::gui_show( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::gui_hide( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

uint32_t BasePlugin::latency_get( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

uint32_t BasePlugin::note_name_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::note_name_get( uint32_t index, clap_note_name_t* out_note_name ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

uint32_t BasePlugin::note_ports_count( bool is_input ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

void BasePlugin::param_indication_set_mapping( clap_id param_id,
                                               bool has_mapping,
                                               clap_color_t const* color,
                                               std::string const& label,
                                               std::string const& description ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

void BasePlugin::param_indication_set_automation( clap_id param_id, uint32_t automation_state, clap_color_t const* color ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

uint32_t BasePlugin::params_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::params_get_value( clap_id param_id, double* out_value ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

void BasePlugin::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

void BasePlugin::posix_fd_support_on_fd( int fd, clap_posix_fd_flags_t flags ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

bool BasePlugin::preset_load_from_location( uint32_t location_kind, std::string const& location, std::string const& load_key ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

uint32_t BasePlugin::remote_controls_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

bool BasePlugin::remote_controls_get( uint32_t page_index, clap_remote_controls_page_t* out_page ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::render_has_hard_realtime_requirement( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::render_set( clap_plugin_render_mode mode ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::state_context_save( clap_ostream_t const* stream, uint32_t context_type ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::state_context_load( clap_istream_t const* stream, uint32_t context_type ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::state_save( clap_ostream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::state_load( clap_istream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

bool BasePlugin::surround_is_channel_mask_supported( uint64_t channel_mask ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

uint32_t BasePlugin::surround_get_channel_map( bool is_input, uint32_t port_index, uint8_t* out_channel_map, uint32_t channel_map_capacity ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

uint32_t BasePlugin::tail_get( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return 0;
}

void BasePlugin::thread_pool_exec( uint32_t task_index ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

void BasePlugin::timer_support_on_timer( clap_id timer_id ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

void BasePlugin::track_info_changed( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return;
}

bool BasePlugin::voice_info_get( clap_voice_info_t* out_info ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return false;
}

#pragma endregion

#pragma region CLAP exts support methods

bool BasePlugin::supports_ambisonic( void ) const {
  return false;
}

bool BasePlugin::supports_audio_ports_activation( void ) const {
  return false;
}

bool BasePlugin::supports_audio_ports_config( void ) const {
  return false;
}

bool BasePlugin::supports_audio_ports( void ) const {
  return false;
}

bool BasePlugin::supports_configurable_audio_ports( void ) const {
  return false;
}

bool BasePlugin::supports_context_menu( void ) const {
  return false;
}

bool BasePlugin::supports_gui( void ) const {
  return false;
}

bool BasePlugin::supports_latency( void ) const {
  return false;
}

bool BasePlugin::supports_note_name( void ) const {
  return false;
}

bool BasePlugin::supports_note_ports( void ) const {
  return false;
}

bool BasePlugin::supports_param_indication( void ) const {
  return false;
}

bool BasePlugin::supports_params( void ) const {
  return false;
}

bool BasePlugin::supports_posix_fd_support( void ) const {
  return false;
}

bool BasePlugin::supports_preset_load( void ) const {
  return false;
}

bool BasePlugin::supports_remote_controls( void ) const {
  return false;
}

bool BasePlugin::supports_render( void ) const {
  return false;
}

bool BasePlugin::supports_state_context( void ) const {
  return false;
}

bool BasePlugin::supports_state( void ) const {
  return false;
}

bool BasePlugin::supports_surround( void ) const {
  return false;
}

bool BasePlugin::supports_tail( void ) const {
  return false;
}

bool BasePlugin::supports_thread_pool( void ) const {
  return false;
}

bool BasePlugin::supports_timer_support( void ) const {
  return false;
}

bool BasePlugin::supports_track_info( void ) const {
  return false;
}

bool BasePlugin::supports_voice_info( void ) const {
  return false;
}

#pragma endregion

#pragma region Static Init/Exit

bool BasePlugin::s_init( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->init();
}

void BasePlugin::s_destroy( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->destroy();
}

bool BasePlugin::s_activate( clap_plugin const* plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->activate( sample_rate, min_frames_count, max_frames_count );
}

void BasePlugin::s_deactivate( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->deactivate();
}

bool BasePlugin::s_start_processing( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->start_processing();
}

void BasePlugin::s_stop_processing( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->stop_processing();
}

void BasePlugin::s_on_main_thread( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->on_main_thread();
}

void BasePlugin::s_reset( clap_plugin const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->reset();
}

clap_process_status BasePlugin::s_process( clap_plugin const* plugin, clap_process_t const* process ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return CLAP_PROCESS_ERROR;
  return plug->process( process );
}

void const* BasePlugin::s_get_extension( clap_plugin const* plugin, char const* id ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return nullptr;
  return plug->get_extension( std::string( id ) );
}

#pragma endregion

#pragma region Static CLAP exts

bool BasePlugin::s_ambisonic_is_config_supported( clap_plugin_t const* plugin, clap_ambisonic_config_t const* config ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->ambisonic_is_config_supported( config );
}

bool BasePlugin::s_ambisonic_get_config( clap_plugin_t const* plugin, bool is_input, uint32_t port_index, clap_ambisonic_config_t* out_config ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->ambisonic_get_config( is_input, port_index, out_config );
}

bool BasePlugin::s_audio_ports_activation_can_activate_while_processing( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->audio_ports_activation_can_activate_while_processing();
}

uint32_t BasePlugin::s_audio_ports_config_count( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->audio_ports_config_count();
}

bool BasePlugin::s_audio_ports_config_get( clap_plugin_t const* plugin, uint32_t index, clap_audio_ports_config_t* out_config ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->audio_ports_config_get( index, out_config );
}

bool BasePlugin::s_audio_ports_config_select( clap_plugin_t const* plugin, clap_id config_id ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->audio_ports_config_select( config_id );
}

clap_id BasePlugin::s_audio_ports_config_info_current_config( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->audio_ports_config_info_current_config();
}

bool BasePlugin::s_audio_ports_config_info_get( clap_plugin_t const* plugin,
                                                clap_id config_id,
                                                uint32_t port_index,
                                                bool is_input,
                                                clap_audio_port_info_t* out_info ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->audio_ports_config_info_get( config_id, port_index, is_input, out_info );
}

uint32_t BasePlugin::s_audio_ports_count( clap_plugin_t const* plugin, bool is_input ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->audio_ports_count( is_input );
}

bool BasePlugin::s_audio_ports_get( clap_plugin_t const* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->audio_ports_get( index, is_input, out_info );
}

bool BasePlugin::s_configurable_audio_ports_can_apply_configuration( clap_plugin_t const* plugin,
                                                                     clap_audio_port_configuration_request const* requests,
                                                                     uint32_t request_count ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->configurable_audio_ports_can_apply_configuration( requests, request_count );
}

bool BasePlugin::s_configurable_audio_ports_apply_configuration( clap_plugin_t const* plugin,
                                                                 clap_audio_port_configuration_request const* requests,
                                                                 uint32_t request_count ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->configurable_audio_ports_apply_configuration( requests, request_count );
}

bool BasePlugin::s_context_menu_populate( clap_plugin_t const* plugin, clap_context_menu_target_t const* target, clap_context_menu_builder_t const* builder ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->context_menu_populate( target, builder );
}

bool BasePlugin::s_context_menu_perform( clap_plugin_t const* plugin, clap_context_menu_target_t const* target, clap_id action_id ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->context_menu_perform( target, action_id );
}

bool BasePlugin::s_gui_is_api_supported( clap_plugin_t const* plugin, char const* api, bool is_floating ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_is_api_supported( std::string( api ), is_floating );
}

bool BasePlugin::s_gui_get_preferred_api( clap_plugin_t const* plugin, char const** out_api, bool* out_is_floating ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  std::string out_api_str;
  auto ret = plug->gui_get_preferred_api( out_api_str, out_is_floating );
  *out_api = out_api_str.c_str();
  return ret;
}

bool BasePlugin::s_gui_create( clap_plugin_t const* plugin, char const* api, bool is_floating ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_create( std::string( api ), is_floating );
}

void BasePlugin::s_gui_destroy( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->gui_destroy();
}

bool BasePlugin::s_gui_set_scale( clap_plugin_t const* plugin, double scale ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_set_scale( scale );
}

bool BasePlugin::s_gui_get_size( clap_plugin_t const* plugin, uint32_t* out_width, uint32_t* out_height ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_get_size( out_width, out_height );
}

bool BasePlugin::s_gui_can_resize( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_can_resize();
}

bool BasePlugin::s_gui_get_resize_hints( clap_plugin_t const* plugin, clap_gui_resize_hints_t* out_hints ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_get_resize_hints( out_hints );
}

bool BasePlugin::s_gui_adjust_size( clap_plugin_t const* plugin, uint32_t* out_width, uint32_t* out_height ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_adjust_size( out_width, out_height );
}

bool BasePlugin::s_gui_set_size( clap_plugin_t const* plugin, uint32_t width, uint32_t height ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_set_size( width, height );
}

bool BasePlugin::s_gui_set_parent( clap_plugin_t const* plugin, clap_window_t const* window ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_set_parent( window );
}

bool BasePlugin::s_gui_set_transient( clap_plugin_t const* plugin, clap_window_t const* window ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_set_transient( window );
}

void BasePlugin::s_gui_suggest_title( clap_plugin_t const* plugin, char const* title ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->gui_suggest_title( std::string( title ) );
}

bool BasePlugin::s_gui_show( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_show();
}

bool BasePlugin::s_gui_hide( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->gui_hide();
}

uint32_t BasePlugin::s_latency_get( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->latency_get();
}

uint32_t BasePlugin::s_note_name_count( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->note_name_count();
}

bool BasePlugin::s_note_name_get( clap_plugin_t const* plugin, uint32_t index, clap_note_name_t* out_note_name ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->note_name_get( index, out_note_name );
}

uint32_t BasePlugin::s_note_ports_count( clap_plugin_t const* plugin, bool is_input ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->note_ports_count( is_input );
}

bool BasePlugin::s_note_ports_get( clap_plugin_t const* plugin, uint32_t index, bool is_input, clap_note_port_info_t* out_info ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->note_ports_get( index, is_input, out_info );
}

void BasePlugin::s_param_indication_set_mapping( clap_plugin_t const* plugin,
                                                 clap_id param_id,
                                                 bool has_mapping,
                                                 clap_color_t const* color,
                                                 char const* label,
                                                 char const* description ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->param_indication_set_mapping( param_id, has_mapping, color, std::string( label ), std::string( description ) );
}

void BasePlugin::s_param_indication_set_automation( clap_plugin_t const* plugin, clap_id param_id, uint32_t automation_state, clap_color_t const* color ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->param_indication_set_automation( param_id, automation_state, color );
}

uint32_t BasePlugin::s_params_count( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->params_count();
}

bool BasePlugin::s_params_get_info( clap_plugin_t const* plugin, uint32_t param_index, clap_param_info_t* out_param_info ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->params_get_info( param_index, out_param_info );
}

bool BasePlugin::s_params_get_value( clap_plugin_t const* plugin, clap_id param_id, double* out_value ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->params_get_value( param_id, out_value );
}

bool BasePlugin::s_params_value_to_text( clap_plugin_t const* plugin, clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->params_value_to_text( param_id, value, out_buffer, out_buffer_capacity );
}

bool BasePlugin::s_params_text_to_value( clap_plugin_t const* plugin, clap_id param_id, char const* param_value_text, double* out_value ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->params_text_to_value( param_id, param_value_text, out_value );
}

void BasePlugin::s_params_flush( clap_plugin_t const* plugin, clap_input_events_t const* in, clap_output_events_t const* out ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->params_flush( in, out );
}

void BasePlugin::s_posix_fd_support_on_fd( clap_plugin_t const* plugin, int fd, clap_posix_fd_flags_t flags ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->posix_fd_support_on_fd( fd, flags );
}

bool BasePlugin::s_preset_load_from_location( clap_plugin_t const* plugin, uint32_t location_kind, char const* location, char const* load_key ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->preset_load_from_location( location_kind, std::string( location ), std::string( load_key ) );
}

uint32_t BasePlugin::s_remote_controls_count( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->remote_controls_count();
}

bool BasePlugin::s_remote_controls_get( clap_plugin_t const* plugin, uint32_t page_index, clap_remote_controls_page_t* out_page ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->remote_controls_get( page_index, out_page );
}

bool BasePlugin::s_render_has_hard_realtime_requirement( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->render_has_hard_realtime_requirement();
}

bool BasePlugin::s_render_set( clap_plugin_t const* plugin, clap_plugin_render_mode mode ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->render_set( mode );
}

bool BasePlugin::s_state_context_save( clap_plugin_t const* plugin, clap_ostream_t const* stream, uint32_t context_type ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->state_context_save( stream, context_type );
}

bool BasePlugin::s_state_context_load( clap_plugin_t const* plugin, clap_istream_t const* stream, uint32_t context_type ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->state_context_load( stream, context_type );
}

bool BasePlugin::s_state_save( clap_plugin_t const* plugin, clap_ostream_t const* stream ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->state_save( stream );
}

bool BasePlugin::s_state_load( clap_plugin_t const* plugin, clap_istream_t const* stream ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->state_load( stream );
}

bool BasePlugin::s_surround_is_channel_mask_supported( clap_plugin_t const* plugin, uint64_t channel_mask ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->surround_is_channel_mask_supported( channel_mask );
}

uint32_t BasePlugin::s_surround_get_channel_map( clap_plugin_t const* plugin,
                                                 bool is_input,
                                                 uint32_t port_index,
                                                 uint8_t* out_channel_map,
                                                 uint32_t channel_map_capacity ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->surround_get_channel_map( is_input, port_index, out_channel_map, channel_map_capacity );
}

uint32_t BasePlugin::s_tail_get( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return 0;
  return plug->tail_get();
}

void BasePlugin::s_thread_pool_exec( clap_plugin_t const* plugin, uint32_t task_index ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->thread_pool_exec( task_index );
}

void BasePlugin::s_timer_support_on_timer( clap_plugin_t const* plugin, clap_id timer_id ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->timer_support_on_timer( timer_id );
}

void BasePlugin::s_track_info_changed( clap_plugin_t const* plugin ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return;
  return plug->track_info_changed();
}

bool BasePlugin::s_voice_info_get( clap_plugin_t const* plugin, clap_voice_info_t* out_info ) {
  BasePlugin* plug = static_cast< BasePlugin* >( plugin->plugin_data );
  if( !plug )
    return false;
  return plug->voice_info_get( out_info );
}

#pragma endregion

#pragma region get_extensions

void const* BasePlugin::get_extension( std::string const& id ) const {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( id={:?} )", __FUNCTION__, static_cast< void const* >( this ), id );
  if( supports_ambisonic() && ( id == std::string( CLAP_EXT_AMBISONIC ) ) )
    return &clap_ambisonic_;
  if( supports_audio_ports_activation() && ( id == std::string( CLAP_EXT_AUDIO_PORTS_ACTIVATION ) ) )
    return &clap_audio_ports_activation_;
  if( supports_audio_ports_config() && ( id == std::string( CLAP_EXT_AUDIO_PORTS_CONFIG ) ) )
    return &clap_audio_ports_config_;
  if( supports_audio_ports_config() && ( id == std::string( CLAP_EXT_AUDIO_PORTS_CONFIG_INFO ) ) )
    return &clap_audio_ports_config_info_;
  if( supports_audio_ports() && ( id == std::string( CLAP_EXT_AUDIO_PORTS ) ) )
    return &clap_audio_ports_;
  if( supports_configurable_audio_ports() && ( id == std::string( CLAP_EXT_CONFIGURABLE_AUDIO_PORTS ) ) )
    return &clap_configurable_audio_ports_;
  if( supports_context_menu() && ( id == std::string( CLAP_EXT_CONTEXT_MENU ) ) )
    return &clap_context_menu_;
  if( supports_gui() && ( id == std::string( CLAP_EXT_GUI ) ) )
    return &clap_gui_;
  if( supports_latency() && ( id == std::string( CLAP_EXT_LATENCY ) ) )
    return &clap_latency_;
  if( supports_note_name() && ( id == std::string( CLAP_EXT_NOTE_NAME ) ) )
    return &clap_note_name_;
  if( supports_note_ports() && ( id == std::string( CLAP_EXT_NOTE_PORTS ) ) )
    return &clap_note_ports_;
  if( supports_param_indication() && ( id == std::string( CLAP_EXT_PARAM_INDICATION ) ) )
    return &clap_param_indication_;
  if( supports_params() && ( id == std::string( CLAP_EXT_PARAMS ) ) )
    return &clap_params_;
  if( supports_posix_fd_support() && ( id == std::string( CLAP_EXT_POSIX_FD_SUPPORT ) ) )
    return &clap_posix_fd_support_;
  if( supports_preset_load() && ( id == std::string( CLAP_EXT_PRESET_LOAD ) ) )
    return &clap_preset_load_;
  if( supports_remote_controls() && ( id == std::string( CLAP_EXT_REMOTE_CONTROLS ) ) )
    return &clap_remote_controls_;
  if( supports_render() && ( id == std::string( CLAP_EXT_RENDER ) ) )
    return &clap_render_;
  if( supports_state_context() && ( id == std::string( CLAP_EXT_STATE_CONTEXT ) ) )
    return &clap_state_context_;
  if( supports_state() && ( id == std::string( CLAP_EXT_STATE ) ) )
    return &clap_state_;
  if( supports_surround() && ( id == std::string( CLAP_EXT_SURROUND ) ) )
    return &clap_surround_;
  if( supports_tail() && ( id == std::string( CLAP_EXT_TAIL ) ) )
    return &clap_tail_;
  if( supports_thread_pool() && ( id == std::string( CLAP_EXT_THREAD_POOL ) ) )
    return &clap_thread_pool_;
  if( supports_timer_support() && ( id == std::string( CLAP_EXT_TIMER_SUPPORT ) ) )
    return &clap_timer_support_;
  if( supports_track_info() && ( id == std::string( CLAP_EXT_TRACK_INFO ) ) )
    return &clap_track_info_;
  if( supports_voice_info() && ( id == std::string( CLAP_EXT_VOICE_INFO ) ) )
    return &clap_voice_info_;
  return nullptr;
}

#pragma endregion
