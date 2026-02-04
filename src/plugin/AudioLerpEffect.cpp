// Header assigned to this source
#include "plugin/AudioLerpEffect.hpp"

// C++ std includes
#include <algorithm>
#include <array>
#include <cstdio>
#include <exception>
#include <functional>
#include <vector>

namespace SfPb = SfgGenerator::Proto;

AudioLerpEffect::AudioLerpEffect() : _base_() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

AudioLerpEffect::~AudioLerpEffect() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string AudioLerpEffect::get_name( void ) const {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "AudioLerpEffect";
}

#pragma region shit to override

bool AudioLerpEffect::init( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  logger_ = logger_->clone( "AudioLerpEffect" );
  uiAleHolder_.set_logger( logger_->clone( "UiAleHolder" ) );
  uiAleHolder_.set_host( host_ );
  uiAleHolder_.set_state( &state_ );

  state_.Clear();

  ret = ret && true;
  return ret;
}

void AudioLerpEffect::on_main_thread( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::on_main_thread();

  // synchronization of values needed:
  // 1. gui does `clap_host_->request_callback( clap_host_ );`
  // 2. this method does `clap_host_params_->rescan( clap_host_, CLAP_PARAM_RESCAN_VALUES );`
  host_params_->rescan( host_, CLAP_PARAM_RESCAN_VALUES );
}

void AudioLerpEffect::reset( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  state_.Clear();
}

void AudioLerpEffect::process_event( clap_event_header_t const* hdr, clap_output_events_t const* out_events ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( hdr={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( hdr ) );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->size    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->size );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->time    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->time );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->space_id={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->space_id );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->type    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->type );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->flags   ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->flags );
  if( hdr->space_id == CLAP_CORE_EVENT_SPACE_ID ) {
    switch( hdr->type ) {
      case CLAP_EVENT_NOTE_ON: {
        break;
      }
      case CLAP_EVENT_NOTE_OFF: {
        break;
      }
      case CLAP_EVENT_NOTE_CHOKE: {
        break;
      }
      case CLAP_EVENT_NOTE_EXPRESSION: {
        clap_event_note_expression_t const* ev = reinterpret_cast< clap_event_note_expression_t const* >( hdr );
        SFG_LOG_TRACE( host_,
                       host_log_,
                       "[{:s}] [{:p}] CLAP_EVENT_NOTE_EXPRESSION - expression_id={:d}, note_id={:d}, port_index={:d}, channel={:d}, key={:d}, value={:f}",
                       __FUNCTION__,
                       static_cast< void* >( this ),
                       ev->expression_id,
                       ev->note_id,
                       ev->port_index,
                       ev->channel,
                       ev->key,
                       ev->value );
        // TODO: handle note expression
        break;
      }
      case CLAP_EVENT_PARAM_VALUE: {
        clap_event_param_value_t const* ev = reinterpret_cast< clap_event_param_value_t const* >( hdr );
        SFG_LOG_TRACE( host_,
                       host_log_,
                       "[{:s}] [{:p}] CLAP_EVENT_PARAM_VALUE - param_id={:d}, cookie={:p}, note_id={:d}, port_index={:d}, channel={:d}, key={:d}, value={:f}",
                       __FUNCTION__,
                       static_cast< void* >( this ),
                       ev->param_id,
                       ev->cookie,
                       ev->note_id,
                       ev->port_index,
                       ev->channel,
                       ev->key,
                       ev->value );
        if( ev->param_id == 1 ) {
          state_.set_a_b( ev->value );
        }
        break;
      }
      case CLAP_EVENT_PARAM_MOD: {
        clap_event_param_mod_t const* ev = reinterpret_cast< clap_event_param_mod_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_,
        //                "[{:s}] [{:p}] CLAP_EVENT_PARAM_MOD - param_id={:d}, cookie={:p}, note_id={:d}, port_index={:d}, channel={:d}, key={:d}, amount={:f}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->param_id,
        //                ev->cookie,
        //                ev->note_id,
        //                ev->port_index,
        //                ev->channel,
        //                ev->key,
        //                ev->amount );
        // TODO: handle parameter modulation
        break;
      }
      case CLAP_EVENT_PARAM_GESTURE_BEGIN: {
        clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_BEGIN - param_id={:d}", __FUNCTION__, static_cast< void* >( this ),
        // ev->param_id );
        // TODO: handle parameter modulation
        break;
      }
      case CLAP_EVENT_PARAM_GESTURE_END: {
        clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_END - param_id={:d}", __FUNCTION__, static_cast< void* >( this ),
        // ev->param_id );
        // TODO: handle parameter modulation
        break;
      }
      case CLAP_EVENT_TRANSPORT: {
        clap_event_transport_t const* ev = reinterpret_cast< clap_event_transport_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_,
        //                "[{:s}] [{:p}] CLAP_EVENT_TRANSPORT - flags=0x{:0>8X}, song_pos_beats={:d}, song_pos_seconds={:d}, tempo={:f}, tempo_inc={:f}, "
        //                "loop_start_beats={:d}, loop_end_beats={:d}, loop_start_seconds={:d}, loop_end_seconds={:d}, bar_start={:d}, bar_number={:d}, "
        //                "tsig_num={:d}, tsig_denom={:d}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->flags,
        //                ev->song_pos_beats,
        //                ev->song_pos_seconds,
        //                ev->tempo,
        //                ev->tempo_inc,
        //                ev->loop_start_beats,
        //                ev->loop_end_beats,
        //                ev->loop_start_seconds,
        //                ev->loop_end_seconds,
        //                ev->bar_start,
        //                ev->bar_number,
        //                ev->tsig_num,
        //                ev->tsig_denom );
        // TODO: handle transport event
        break;
      }
      case CLAP_EVENT_MIDI: {
        clap_event_midi_t const* ev = reinterpret_cast< clap_event_midi_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_,
        //                "[{:s}] [{:p}] CLAP_EVENT_MIDI - port_index={:d}, data={}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->port_index,
        //                std::vector< uint8_t >( ev->data, ev->data + 3 ) );
        // TODO: handle MIDI event
        break;
      }
      case CLAP_EVENT_MIDI_SYSEX: {
        clap_event_midi_sysex_t const* ev = reinterpret_cast< clap_event_midi_sysex_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_,
        //                "[{:s}] [{:p}] CLAP_EVENT_MIDI_SYSEX - port_index={:d}, buffer={:p}, size={}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->port_index,
        //                static_cast< void const* >( ev->buffer ),
        //                ev->size );
        // TODO: handle MIDI Sysex event
        break;
      }
      case CLAP_EVENT_MIDI2: {
        clap_event_midi2_t const* ev = reinterpret_cast< clap_event_midi2_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_,
        //                "[{:s}] [{:p}] CLAP_EVENT_MIDI2 - port_index={:d}, data={}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->port_index,
        //                std::vector< uint8_t >( ev->data, ev->data + 4 ) );
        // TODO: handle MIDI2 event
        break;
      }
    }
  }
}

clap_process_status AudioLerpEffect::process( clap_process_t const* process ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( process={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( process )
  // );
  const uint32_t nframes = process->frames_count;
  const uint32_t nev = process->in_events->size( process->in_events );
  uint32_t ev_index = 0;
  uint32_t next_ev_frame = nev > 0 ? 0 : nframes;

  static double last_ab = -1.0;

  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] nframes      ={:d}", __FUNCTION__, static_cast< void* >( this ), nframes );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] nev          ={:d}", __FUNCTION__, static_cast< void* >( this ), nev );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] ev_index     ={:d}", __FUNCTION__, static_cast< void* >( this ), ev_index );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] next_ev_frame={:d}", __FUNCTION__, static_cast< void* >( this ), next_ev_frame );

  for( uint32_t i = 0; i < nframes; ) {
    /* handle every events that happrens at the frame "i" */
    while( ev_index < nev && next_ev_frame == i ) {
      const clap_event_header_t* hdr = process->in_events->get( process->in_events, ev_index );
      if( hdr->time != i ) {
        next_ev_frame = hdr->time;
        break;
      }
      process_event( hdr, process->out_events );
      ++ev_index;
      if( ev_index == nev ) {
        // we reached the end of the event list
        next_ev_frame = nframes;
        break;
      }
    }

    if( last_ab != state_.a_b() ) {
      // a_b changed, safety event
      clap_event_param_value_t out_ev{};
      out_ev.header.size = sizeof( out_ev );
      out_ev.header.time = i;
      out_ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
      out_ev.header.type = CLAP_EVENT_PARAM_VALUE;
      out_ev.header.flags = CLAP_EVENT_IS_LIVE;
      out_ev.param_id = 1;
      out_ev.value = state_.a_b();
      process->out_events->try_push( process->out_events, &out_ev.header );
    }

    /* process every samples until the next event */
    for( ; i < next_ev_frame; ++i ) {
      for( uint32_t c = 0; c < process->audio_outputs[0].channel_count; c++ ) {
        double out = 0.0;
        if( active_ && process_ ) {
          if( process->audio_inputs[0].data32 )
            out += ( 1.0 - state_.a_b() ) * process->audio_inputs[0].data32[c][i];
          else if( process->audio_inputs[0].data64 )
            out += ( 1.0 - state_.a_b() ) * process->audio_inputs[0].data64[c][i];

          if( process->audio_inputs[1].data32 )
            out += state_.a_b() * process->audio_inputs[1].data32[c][i];
          else if( process->audio_inputs[1].data64 )
            out += state_.a_b() * process->audio_inputs[1].data64[c][i];
        }
        // store output
        if( process->audio_outputs[0].data32 )
          process->audio_outputs[0].data32[c][i] = static_cast< float >( out );
        else if( process->audio_outputs[0].data64 )
          process->audio_outputs[0].data64[c][i] = out;
      }
    }
  }

  return ( active_ && process_ ) ? CLAP_PROCESS_CONTINUE : CLAP_PROCESS_SLEEP;
}

#pragma endregion

#pragma region CLAP extensions

uint32_t AudioLerpEffect::audio_ports_count( bool is_input ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 2;
  }
  return 1;
}

bool AudioLerpEffect::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( index={:d}, is_input={}, out_info={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 index,
                 is_input,
                 static_cast< void* >( out_info ) )
  if( index >= audio_ports_count( is_input ) )
    return false;
  if( !out_info )
    return false;
  if( is_input ) {
    if( ( index == 0 ) || ( index == 1 ) ) {
      out_info->id = index;
      std::snprintf( out_info->name, sizeof( out_info->name ), "%s %d", "input", index );
      out_info->channel_count = 2;
      out_info->flags = ( ( index == 0 ) ? CLAP_AUDIO_PORT_IS_MAIN : 0 ) | CLAP_AUDIO_PORT_SUPPORTS_64BITS | CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE;
      out_info->port_type = CLAP_PORT_STEREO;
      out_info->in_place_pair = CLAP_INVALID_ID;
      return true;
    }
    return false;
  }
  out_info->id = 2;
  std::snprintf( out_info->name, sizeof( out_info->name ), "%s", "main" );
  out_info->channel_count = 1;
  out_info->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS | CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE;
  out_info->port_type = CLAP_PORT_STEREO;
  out_info->in_place_pair = CLAP_INVALID_ID;
  return true;
}

bool AudioLerpEffect::gui_is_api_supported( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  bool ret = _base_::gui_is_api_supported( api, is_floating );
  return ret || true;
}

bool AudioLerpEffect::gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_api={:?}, out_is_floating={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 out_api,
                 static_cast< void* >( out_is_floating ) );
  bool ret = _base_::gui_get_preferred_api( out_api, out_is_floating );
  return ret && false;
}

bool AudioLerpEffect::gui_create( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  bool ret = _base_::gui_create( api, is_floating );
  return ret || uiAleHolder_.clap_create( api, is_floating );
}

void AudioLerpEffect::gui_destroy( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  uiAleHolder_.clap_destroy();
  _base_::gui_destroy();
}

bool AudioLerpEffect::gui_set_scale( double scale ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( scale={:f} )", __FUNCTION__, static_cast< void* >( this ), scale );
  bool ret = _base_::gui_set_scale( scale );
  return ret || uiAleHolder_.clap_set_scale( scale );
}

bool AudioLerpEffect::gui_get_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void* >( out_width ),
                 static_cast< void* >( out_height ) );
  bool ret = _base_::gui_get_size( out_width, out_height );
  return ret || uiAleHolder_.clap_get_size( out_width, out_height );
}

bool AudioLerpEffect::gui_can_resize( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::gui_can_resize();
  return ret || uiAleHolder_.clap_can_resize();
}

bool AudioLerpEffect::gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_hints={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 __FUNCTION__,
                 static_cast< void* >( out_hints ) );
  bool ret = _base_::gui_get_resize_hints( out_hints );
  return ret || uiAleHolder_.clap_get_resize_hints( out_hints );
}

bool AudioLerpEffect::gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_width={:d}, out_height={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 *out_width,
                 *out_height );
  bool ret = _base_::gui_adjust_size( out_width, out_height );
  return ret || uiAleHolder_.clap_adjust_size( out_width, out_height );
}

bool AudioLerpEffect::gui_set_size( uint32_t width, uint32_t height ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( width={:d}, height={:d} )", __FUNCTION__, static_cast< void* >( this ), width, height );
  bool ret = _base_::gui_set_size( width, height );
  return ret || uiAleHolder_.clap_set_size( width, height );
}

bool AudioLerpEffect::gui_set_parent( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  bool ret = _base_::gui_set_parent( window );
  return ret || uiAleHolder_.clap_set_parent( window );
}

bool AudioLerpEffect::gui_set_transient( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  bool ret = _base_::gui_set_transient( window );
  return ret || uiAleHolder_.clap_set_transient( window );
}

void AudioLerpEffect::gui_suggest_title( std::string const& title ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( title={:?} )", __FUNCTION__, static_cast< void* >( this ), title );
  _base_::gui_suggest_title( title );
  uiAleHolder_.clap_suggest_title( title );
}

bool AudioLerpEffect::gui_show( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::gui_show();
  return ret || uiAleHolder_.clap_show();
}

bool AudioLerpEffect::gui_hide( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::gui_hide();
  return ret || uiAleHolder_.clap_hide();
}

uint32_t AudioLerpEffect::params_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
  // adjust according to AudioLerpEffect.proto
  // while we could make it dynamic, without explicit gui i'd rather not
  return 1;
}

bool AudioLerpEffect::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_index={:d}, out_param_info={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_index,
                 static_cast< void* >( out_param_info ) );
  if( param_index >= params_count() )
    return false;
  if( !out_param_info )
    return false;
  switch( param_index ) {
    case 0:
      out_param_info->id = 1;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "A .. B" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Main" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
  }
  return true;
}

bool AudioLerpEffect::params_get_value( clap_id param_id, double* out_value ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_id={:d}, out_value={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  if( param_id == 1 ) {
    ( *out_value ) = state_.a_b();
    return true;
  }
  return false;
}

bool AudioLerpEffect::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_id={:d}, value={:f}, out_buffer={:p}, out_info={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 value,
                 static_cast< void* >( out_buffer ),
                 out_buffer_capacity );
  if( !out_buffer || ( out_buffer_capacity == 0 ) )
    return false;
  if( param_id == 1 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  }
  return false;
}

bool AudioLerpEffect::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_id={:d}, param_value_text={:?}, out_value={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 param_value_text,
                 static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  std::function< bool( std::string const&, double* ) > text_to_double = []( std::string const& param_value_text, double* out_value ) {
    try {
      ( *out_value ) = std::stod( param_value_text );
      if( ( *out_value ) > 1.0 ) {
        ( *out_value ) = ( *out_value ) / 100.0;
      }
      return true;
    } catch( std::exception const& ) {
      return false;
    }
  };
  if( param_id == 1 ) {
    return text_to_double( param_value_text, out_value );
  }
  return false;
}

void AudioLerpEffect::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( in={:p}, out={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( in ),
                 static_cast< void const* >( out ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] in_size={:d}", __FUNCTION__, static_cast< void* >( this ), in->size( in ) );

  for( uint32_t i = 0; i < in->size( in ); i++ ) {
    process_event( in->get( in, i ), out );
  }
  return;
}

bool AudioLerpEffect::state_save( clap_ostream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );
  bool ret = true;
  std::string buffer{};
  if( !state_.SerializeToString( &buffer ) ) {
    ret = false;
  } else {
    int64_t written = 0;
    while( written < static_cast< int64_t >( buffer.size() ) ) {
      int64_t tmp = stream->write( stream, buffer.data() + written, buffer.size() - written );
      if( tmp < 0 ) {
        return false;
      }
      written += tmp;
    }
    ret = written == static_cast< int64_t >( buffer.size() );
  }
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool AudioLerpEffect::state_load( clap_istream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );
  bool ret = true;
  std::string buffer{};
  std::array< char, 4096 > temp;
  while( true ) {
    int64_t n = stream->read( stream, temp.data(), temp.size() );
    if( n <= 0 ) {
      break;
    }
    buffer.append( temp.data(), static_cast< size_t >( n ) );
  }
  ret = ret && ( buffer.size() > 0 );
  ret = ret && state_.ParseFromString( buffer );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

#pragma endregion

#pragma region CLAP extensions, wether or not to pointer things to clap

bool AudioLerpEffect::supports_audio_ports() const {
  return true;
}

bool AudioLerpEffect::supports_gui() const {
  return true;
}

bool AudioLerpEffect::supports_params() const {
  return true;
}

bool AudioLerpEffect::supports_state() const {
  return true;
}

#pragma endregion

#pragma region shit for the factory

clap_plugin_t* AudioLerpEffect::s_create( clap_host_t const* host ) {
  clap_plugin_t* plugin = new clap_plugin_t();
  AudioLerpEffect* noise_gen = new AudioLerpEffect();
  noise_gen->host_ = host;
  plugin->desc = descriptor_get();
  plugin->plugin_data = noise_gen;
  plugin->init = s_init;
  plugin->destroy = s_destroy;
  plugin->activate = s_activate;
  plugin->deactivate = s_deactivate;
  plugin->start_processing = s_start_processing;
  plugin->stop_processing = s_stop_processing;
  plugin->reset = s_reset;
  plugin->process = s_process;
  plugin->get_extension = s_get_extension;
  plugin->on_main_thread = s_on_main_thread;
  return plugin;
};

clap_plugin_descriptor_t* AudioLerpEffect::descriptor_get( void ) {
  static clap_plugin_descriptor_t clap_descriptor_;
  static std::vector< char const* > clap_descriptor_features_;

  static std::string const c_plugin_id_ = "com.SFGrenade.AudioLerpEffect";
  static std::string const c_plugin_name_ = "SFG-AudioLerpEffect";
  static std::string const c_plugin_vendor_ = "SFGrenade";
  static std::string const c_plugin_url_ = "https://sfgrena.de";
  static std::string const c_plugin_manual_url_ = "https://sfgrena.de";
  static std::string const c_plugin_support_url_ = "https://sfgrena.de";
  static std::string const c_plugin_version_ = "0.0.1";
  static std::string const c_plugin_description_ = "Awesome AudioLerpEffect. By SFGrenade.";
  static std::vector< std::string > const c_plugin_features_
      = { CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, CLAP_PLUGIN_FEATURE_UTILITY, CLAP_PLUGIN_FEATURE_MIXING, CLAP_PLUGIN_FEATURE_STEREO };

  clap_descriptor_.clap_version = CLAP_VERSION_INIT;
  clap_descriptor_.id = c_plugin_id_.c_str();
  clap_descriptor_.name = c_plugin_name_.c_str();
  clap_descriptor_.vendor = c_plugin_vendor_.c_str();
  clap_descriptor_.url = c_plugin_url_.c_str();
  clap_descriptor_.manual_url = c_plugin_manual_url_.c_str();
  clap_descriptor_.support_url = c_plugin_support_url_.c_str();
  clap_descriptor_.version = c_plugin_version_.c_str();
  clap_descriptor_.description = c_plugin_description_.c_str();
  // + 1 so we leave last entry as nullptr
  clap_descriptor_features_.resize( c_plugin_features_.size() + 1, nullptr );
  for( size_t i = 0; i < c_plugin_features_.size(); i++ ) {
    clap_descriptor_features_[i] = c_plugin_features_[i].c_str();
  }
  clap_descriptor_.features = clap_descriptor_features_.data();

  return &clap_descriptor_;
}

#pragma endregion
