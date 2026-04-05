// Header assigned to this source
#include "plugin/AudioAnalysis.hpp"

// Project includes
#include "widgets/HorizontalDbfsDisplay.hpp"

// C++ std includes
#include <algorithm>
#include <array>
#include <cstdio>
#include <exception>
#include <functional>
#include <vector>

namespace SfPb = SfgGenerator::Proto;

AudioAnalysis::AudioAnalysis()
    : _base_(),
      guiWindow_( nullptr,
                  []( SDL_Window* ptr ) {
                    SDL_HideWindow( ptr );
                    SDL_DestroyWindow( ptr );
                  } ),
      sampleQueue_( 4096 ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

AudioAnalysis::~AudioAnalysis() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string AudioAnalysis::get_name( void ) const {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "AudioAnalysis";
}

#pragma region shit to override

bool AudioAnalysis::init( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  logger_ = logger_->clone( "AudioAnalysis" );
  // uiAaHolder_.set_logger( logger_->clone( "UiAaHolder" ) );
  // uiAaHolder_.set_host( host_ );
  // uiAaHolder_.set_state( &state_ );

  state_.Clear();
  state_.set_gui_width( 300 );
  state_.set_gui_height( 200 );
  // state_.set_time_window( 0.1 );

  ret = ret && true;
  return ret;
}

bool AudioAnalysis::activate( double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( sample_rate={:f}, min_frames_count={:d}, max_frames_count={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 sample_rate,
                 min_frames_count,
                 max_frames_count );
  bool ret = _base_::activate( sample_rate, min_frames_count, max_frames_count );

  // uiAaHolder_.set_sample_rate( sample_rate );
  rmsMomentaryValueBuffer_.set_capacity( sample_rate * 0.4 );
  lufsMomentaryValueBuffer_.set_capacity( sample_rate * 0.4 );
  kWeightingFilterHighShelf_.setup( sample_rate, 2000.0, 4.0 );
  kWeightingFilterHighPass_.setup( sample_rate, 100.0 );
  rmsMomentaryValueBuffer_.resize( rmsMomentaryValueBuffer_.capacity(), 0.0 );
  lufsMomentaryValueBuffer_.resize( lufsMomentaryValueBuffer_.capacity(), 0.0 );

  rmsShortTermValueBuffer_.set_capacity( 30 );
  lufsShortTermValueBuffer_.set_capacity( 30 );
  rmsShortTermValueBuffer_.resize( rmsShortTermValueBuffer_.capacity(), 0.0 );
  lufsShortTermValueBuffer_.resize( lufsShortTermValueBuffer_.capacity(), 0.0 );

  std::fill( rmsMomentaryValueBuffer_.begin(), rmsMomentaryValueBuffer_.end(), 0.0f );
  std::fill( lufsMomentaryValueBuffer_.begin(), lufsMomentaryValueBuffer_.end(), 0.0f );
  std::fill( rmsShortTermValueBuffer_.begin(), rmsShortTermValueBuffer_.end(), 0.0f );
  std::fill( lufsShortTermValueBuffer_.begin(), lufsShortTermValueBuffer_.end(), 0.0f );

  rmsSamplesReceived_ = 0;
  lufsSamplesReceived_ = 0;

  ret = ret && true;
  return ret;
}

void AudioAnalysis::on_main_thread( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::on_main_thread();

  // synchronization of values needed:
  // 1. gui does `clap_host_->request_callback( clap_host_ );`
  // 2. this method does `clap_host_state_->mark_dirty( clap_host_ );`
  // 3. this method does `clap_host_params_->rescan( clap_host_, CLAP_PARAM_RESCAN_VALUES );`
  host_state_->mark_dirty( host_ );
  host_params_->rescan( host_, CLAP_PARAM_RESCAN_VALUES );
}

void AudioAnalysis::reset( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  state_.Clear();
  state_.set_gui_width( 300 );
  state_.set_gui_height( 200 );
  // state_.set_time_window( 0.1 );

  std::fill( rmsMomentaryValueBuffer_.begin(), rmsMomentaryValueBuffer_.end(), 0.0f );
  std::fill( lufsMomentaryValueBuffer_.begin(), lufsMomentaryValueBuffer_.end(), 0.0f );
  std::fill( rmsShortTermValueBuffer_.begin(), rmsShortTermValueBuffer_.end(), 0.0f );
  std::fill( lufsShortTermValueBuffer_.begin(), lufsShortTermValueBuffer_.end(), 0.0f );
}

void AudioAnalysis::process_event( clap_event_header_t const* hdr, clap_output_events_t const* /*out_events*/ ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( hdr={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( hdr ) );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->size    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->size );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->time    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->time );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->space_id={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->space_id );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->type    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->type );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->flags   ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->flags );
  if( hdr->space_id != CLAP_CORE_EVENT_SPACE_ID ) {
    return;
  }
  if( hdr->type == CLAP_EVENT_NOTE_ON ) {
    // ignored
  } else if( hdr->type == CLAP_EVENT_NOTE_OFF ) {
    // ignored
  } else if( hdr->type == CLAP_EVENT_NOTE_CHOKE ) {
    // ignored
  } else if( hdr->type == CLAP_EVENT_NOTE_END ) {
    // ignored
  } else if( hdr->type == CLAP_EVENT_NOTE_EXPRESSION ) {
    // ignored
  } else if( hdr->type == CLAP_EVENT_PARAM_VALUE ) {
    clap_event_param_value_t const* ev = reinterpret_cast< clap_event_param_value_t const* >( hdr );
    // SFG_LOG_TRACE( host_,
    //                host_log_,
    //                "[{:s}] [{:p}] CLAP_EVENT_PARAM_VALUE - param_id={:d}, cookie={:p}, note_id={:d}, port_index={:d}, channel={:d}, key={:d}, value={:f}",
    //                __FUNCTION__,
    //                static_cast< void* >( this ),
    //                ev->param_id,
    //                ev->cookie,
    //                ev->note_id,
    //                ev->port_index,
    //                ev->channel,
    //                ev->key,
    //                ev->value );
    // if( ev->param_id == 1 ) {
    //   state_.set_time_window( ev->value );
    //   last_time_window_ = ev->value;  // we only want to show things when UI changes state
    // }
  } else if( hdr->type == CLAP_EVENT_PARAM_MOD ) {
    clap_event_param_mod_t const* ev = reinterpret_cast< clap_event_param_mod_t const* >( hdr );
    SFG_LOG_TRACE( host_,
                   host_log_,
                   "[{:s}] [{:p}] CLAP_EVENT_PARAM_MOD - param_id={:d}, cookie={:p}, note_id={:d}, port_index={:d}, channel={:d}, key={:d}, amount={:f}",
                   __FUNCTION__,
                   static_cast< void* >( this ),
                   ev->param_id,
                   ev->cookie,
                   ev->note_id,
                   ev->port_index,
                   ev->channel,
                   ev->key,
                   ev->amount );
  } else if( hdr->type == CLAP_EVENT_PARAM_GESTURE_BEGIN ) {
    clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
    SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_BEGIN - param_id={:d}", __FUNCTION__, static_cast< void* >( this ), ev->param_id );
  } else if( hdr->type == CLAP_EVENT_PARAM_GESTURE_END ) {
    clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
    SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_END - param_id={:d}", __FUNCTION__, static_cast< void* >( this ), ev->param_id );
  } else if( hdr->type == CLAP_EVENT_TRANSPORT ) {
    clap_event_transport_t const* ev = reinterpret_cast< clap_event_transport_t const* >( hdr );
    SFG_LOG_TRACE( host_,
                   host_log_,
                   "[{:s}] [{:p}] CLAP_EVENT_TRANSPORT - flags=0x{:0>8X}, song_pos_beats={:d}, song_pos_seconds={:d}, tempo={:f}, tempo_inc={:f}, "
                   "loop_start_beats={:d}, loop_end_beats={:d}, loop_start_seconds={:d}, loop_end_seconds={:d}, bar_start={:d}, bar_number={:d}, "
                   "tsig_num={:d}, tsig_denom={:d}",
                   __FUNCTION__,
                   static_cast< void* >( this ),
                   ev->flags,
                   ev->song_pos_beats,
                   ev->song_pos_seconds,
                   ev->tempo,
                   ev->tempo_inc,
                   ev->loop_start_beats,
                   ev->loop_end_beats,
                   ev->loop_start_seconds,
                   ev->loop_end_seconds,
                   ev->bar_start,
                   ev->bar_number,
                   ev->tsig_num,
                   ev->tsig_denom );
  } else if( hdr->type == CLAP_EVENT_MIDI ) {
    clap_event_midi_t const* ev = reinterpret_cast< clap_event_midi_t const* >( hdr );
    // SFG_LOG_TRACE( host_,
    //                host_log_,
    //                "[{:s}] [{:p}] CLAP_EVENT_MIDI - port_index={:d}, data={}",
    //                __FUNCTION__,
    //                static_cast< void* >( this ),
    //                ev->port_index,
    //                std::vector< uint8_t >( ev->data, ev->data + 3 ) );
    // if( ev->data[0] == 0xB0 ) {
    //   // control change channel 0
    //   int param_id = ev->data[1] + 1;  // who knows if it's actually data[1]
    //   double value = double( ev->data[2] ) / double( 0x7F );
    //   if( param_id == 1 ) {
    //     state_.set_time_window( value );
    //     last_time_window_ = value;  // we only want to show things when UI changes state
    //   }
    // }
  } else if( hdr->type == CLAP_EVENT_MIDI_SYSEX ) {
    clap_event_midi_sysex_t const* ev = reinterpret_cast< clap_event_midi_sysex_t const* >( hdr );
    SFG_LOG_TRACE( host_,
                   host_log_,
                   "[{:s}] [{:p}] CLAP_EVENT_MIDI_SYSEX - port_index={:d}, buffer={:p}, size={}",
                   __FUNCTION__,
                   static_cast< void* >( this ),
                   ev->port_index,
                   static_cast< void const* >( ev->buffer ),
                   ev->size );
  } else if( hdr->type == CLAP_EVENT_MIDI2 ) {
    clap_event_midi2_t const* ev = reinterpret_cast< clap_event_midi2_t const* >( hdr );
    SFG_LOG_TRACE( host_,
                   host_log_,
                   "[{:s}] [{:p}] CLAP_EVENT_MIDI2 - port_index={:d}, data={}",
                   __FUNCTION__,
                   static_cast< void* >( this ),
                   ev->port_index,
                   std::vector< uint8_t >( ev->data, ev->data + 4 ) );
  }
}

clap_process_status AudioAnalysis::process( clap_process_t const* process ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( process={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( process )
  // );
  const uint32_t nframes = process->frames_count;
  const uint32_t nev = process->in_events->size( process->in_events );
  uint32_t ev_index = 0;
  uint32_t next_ev_frame = nev > 0 ? 0 : nframes;

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
      // process->out_events->try_push( process->out_events, hdr );
      process_event( hdr, process->out_events );
      ++ev_index;
      if( ev_index == nev ) {
        // we reached the end of the event list
        next_ev_frame = nframes;
        break;
      }
    }

    // if( last_time_window_ != state_.time_window() ) {
    //   // time_window changed, safety event
    //   clap_event_param_value_t out_ev{};
    //   out_ev.header.size = sizeof( out_ev );
    //   out_ev.header.time = i;
    //   out_ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    //   out_ev.header.type = CLAP_EVENT_PARAM_VALUE;
    //   out_ev.header.flags = CLAP_EVENT_IS_LIVE;
    //   out_ev.param_id = 1;
    //   out_ev.value = state_.time_window();
    //   last_time_window_ = out_ev.value;
    //   process->out_events->try_push( process->out_events, &out_ev.header );
    // }

    /* process every samples until the next event */
    for( ; i < next_ev_frame; ++i ) {
      float monoOut = 0.0;
      for( uint32_t c = 0; c < process->audio_outputs[0].channel_count; c++ ) {
        float out = 0.0;
        if( active_ && process_ ) {
          if( process->audio_inputs[0].data32 )
            out = process->audio_inputs[0].data32[c][i];
          else if( process->audio_inputs[0].data64 )
            out = float( process->audio_inputs[0].data64[c][i] );
        }

        // uiAaHolder_.push_sample( out, c );
        monoOut += out;

        // store output
        if( process->audio_outputs[0].data32 )
          process->audio_outputs[0].data32[c][i] = out;
        else if( process->audio_outputs[0].data64 )
          process->audio_outputs[0].data64[c][i] = out;
      }
      sampleQueue_.push( monoOut / float( process->audio_outputs[0].channel_count ) );
    }
  }

  return ( active_ && process_ ) ? CLAP_PROCESS_CONTINUE : CLAP_PROCESS_SLEEP;
}

#pragma endregion

#pragma region CLAP extensions

uint32_t AudioAnalysis::audio_ports_count( bool is_input ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  return 1;
}

bool AudioAnalysis::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
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
    out_info->id = 0;
    std::snprintf( out_info->name, sizeof( out_info->name ), "%s %d", "in", index );
    out_info->channel_count = 2;
    out_info->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS | CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE;
    out_info->port_type = CLAP_PORT_STEREO;
    out_info->in_place_pair = CLAP_INVALID_ID;
    return true;
  }
  out_info->id = 1;
  std::snprintf( out_info->name, sizeof( out_info->name ), "%s", "out" );
  out_info->channel_count = 2;
  out_info->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS | CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE;
  out_info->port_type = CLAP_PORT_STEREO;
  out_info->in_place_pair = CLAP_INVALID_ID;
  return true;
}

bool AudioAnalysis::gui_is_api_supported( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  bool ret = _base_::gui_is_api_supported( api, is_floating );
  return ret || true;
}

bool AudioAnalysis::gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) {
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

bool AudioAnalysis::gui_create( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );

  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] init SDL", __FUNCTION__, static_cast< void* >( this ) );
  if( !SDL_Init( SDL_INIT_VIDEO ) ) {
    SFG_LOG_ERROR( host_, host_log_, "[{:s}] [{:p}] error initializing SDL: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }
  if( !TTF_Init() ) {
    SFG_LOG_ERROR( host_, host_log_, "[{:s}] [{:p}] error initializing SDL_TTF: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  InputManager::init();

  guiRootWidget_ = std::make_shared< Widget >();
  {
    guiWidgetMomentaryRms_ = std::make_shared< HorizontalDbfsDisplay >( "Momentary RMS", SDL_FRect{ 0.0f, 0.0f, 1.0f, 0.125f } );
    guiWidgetMomentaryRms_->InitUi( guiRootWidget_ );
    guiWidgetMomentaryRms_->SetPadding( 2.0f );
    guiWidgetMomentaryRms_->SetUnit( "dBFS" );
  }
  {
    guiWidgetShortTermRms_ = std::make_shared< HorizontalDbfsDisplay >( "Short-Term RMS", SDL_FRect{ 0.0f, 0.125f, 1.0f, 0.125f } );
    guiWidgetShortTermRms_->InitUi( guiRootWidget_ );
    guiWidgetShortTermRms_->SetPadding( 2.0f );
    guiWidgetShortTermRms_->SetUnit( "dBFS" );
  }
  {
    guiWidgetMomentaryLufs_ = std::make_shared< HorizontalDbfsDisplay >( "Momentary LUFS", SDL_FRect{ 0.0f, 0.25f, 1.0f, 0.125f } );
    guiWidgetMomentaryLufs_->InitUi( guiRootWidget_ );
    guiWidgetMomentaryLufs_->SetPadding( 2.0f );
    guiWidgetMomentaryLufs_->SetUnit( "LUFS" );
  }
  {
    guiWidgetShortTermLufs_ = std::make_shared< HorizontalDbfsDisplay >( "Short-Term LUFS", SDL_FRect{ 0.0f, 0.375f, 1.0f, 0.125f } );
    guiWidgetShortTermLufs_->InitUi( guiRootWidget_ );
    guiWidgetShortTermLufs_->SetPadding( 2.0f );
    guiWidgetShortTermLufs_->SetUnit( "LUFS" );
  }

  SDL_PropertiesID windowCreateProps = SDL_CreateProperties();
  SDL_SetBooleanProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true );
  SDL_SetBooleanProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true );
  SDL_SetNumberProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, state_.gui_width() );
  SDL_SetNumberProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, state_.gui_height() );
  SDL_SetNumberProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0 );
  SDL_SetNumberProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0 );
  if( !is_floating ) {
    SDL_SetBooleanProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true );
  } else {
    SDL_SetStringProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "com.SFGrenade.AudioAnalysis" );
    SDL_SetBooleanProperty( windowCreateProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false );
  }
  guiWindow_ = std::shared_ptr< SDL_Window >( SDL_CreateWindowWithProperties( windowCreateProps ), []( SDL_Window* ptr ) {
    if( ptr ) {
      SDL_HideWindow( ptr );
      SDL_DestroyWindow( ptr );
    }
  } );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] window created at {:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void* >( guiWindow_.get() ) );

  guiWindowRenderer_ = std::shared_ptr< SDL_Renderer >( SDL_GetRenderer( guiWindow_.get() ) );
  if( !guiWindowRenderer_ ) {
    SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] window has no renderer, creating new one", __FUNCTION__, static_cast< void* >( this ) );
    guiWindowRenderer_ = std::shared_ptr< SDL_Renderer >( SDL_CreateRenderer( guiWindow_.get(), SDL_SOFTWARE_RENDERER ), []( SDL_Renderer* ptr ) {
      if( ptr ) {
        SDL_DestroyRenderer( ptr );
      }
    } );
  }
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] window renderer at {:p}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void* >( guiWindowRenderer_.get() ) );
  SDL_SetRenderDrawBlendMode( guiWindowRenderer_.get(), SDL_BLENDMODE_BLEND );

  guiTimer_ = Timer::createNative( 10, std::bind( &AudioAnalysis::guiTimerCallback, this ) );
  guiTimer_->start();
  return true;
}

void AudioAnalysis::gui_destroy( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  guiTimer_->stop();
  guiTimer_.reset();
  guiWindowRenderer_.reset();
  guiWindow_.reset();

  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] quit SDL", __FUNCTION__, static_cast< void* >( this ) );
  TTF_Quit();
  SDL_Quit();
}

bool AudioAnalysis::gui_set_scale( double scale ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( scale={:f} )", __FUNCTION__, static_cast< void* >( this ), scale );
  return false;
}

bool AudioAnalysis::gui_get_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void* >( out_width ),
                 static_cast< void* >( out_height ) );
  SDL_GetWindowSize( guiWindow_.get(), reinterpret_cast< int* >( out_width ), reinterpret_cast< int* >( out_height ) );
  return true;
}

bool AudioAnalysis::gui_can_resize( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  return ( SDL_GetWindowFlags( guiWindow_.get() ) & SDL_WINDOW_RESIZABLE );
}

bool AudioAnalysis::gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_hints={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 __FUNCTION__,
                 static_cast< void* >( out_hints ) );
  return false;
}

bool AudioAnalysis::gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_width={:d}, out_height={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 *out_width,
                 *out_height );
  gui_set_size( *out_width, *out_height );
  gui_get_size( out_width, out_height );
  state_.set_gui_width( *out_width );
  state_.set_gui_height( *out_height );
  return true;
}

bool AudioAnalysis::gui_set_size( uint32_t width, uint32_t height ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( width={:d}, height={:d} )", __FUNCTION__, static_cast< void* >( this ), width, height );
  SDL_SetWindowSize( guiWindow_.get(), width, height );
  state_.set_gui_width( width );
  state_.set_gui_height( height );
  return true;
}

bool AudioAnalysis::gui_set_parent( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );

  if( window->api == CLAP_WINDOW_API_WIN32 ) {
    setParentWindow( guiWindow_, window );
  } else if( window->api == CLAP_WINDOW_API_COCOA ) {
    setParentWindow( guiWindow_, window );
  } else if( window->api == CLAP_WINDOW_API_X11 ) {
    setParentWindow( guiWindow_, window );
  } else if( window->api == CLAP_WINDOW_API_WAYLAND ) {
    setParentWindow( guiWindow_, window );
  } else {
    setParentWindow( guiWindow_, window );
  }
  SDL_SetWindowPosition( guiWindow_.get(), 0, 0 );
  return true;
}

bool AudioAnalysis::gui_set_transient( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  SDL_RaiseWindow( guiWindow_.get() );
  return true;
}

void AudioAnalysis::gui_suggest_title( std::string const& title ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( title={:?} )", __FUNCTION__, static_cast< void* >( this ), title );
  SDL_SetWindowTitle( guiWindow_.get(), title.c_str() );
}

bool AudioAnalysis::gui_show( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  SDL_ShowWindow( guiWindow_.get() );
  return true;
}

bool AudioAnalysis::gui_hide( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  SDL_HideWindow( guiWindow_.get() );
  return true;
}

// uint32_t AudioAnalysis::params_count( void ) {
//   SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
//   // adjust according to AudioAnalysis.proto
//   // while we could make it dynamic, without explicit gui i'd rather not
//   return 1;
// }

// bool AudioAnalysis::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
//   SFG_LOG_TRACE( host_,
//                  host_log_,
//                  "[{:s}] [{:p}] enter( param_index={:d}, out_param_info={:p} )",
//                  __FUNCTION__,
//                  static_cast< void* >( this ),
//                  param_index,
//                  static_cast< void* >( out_param_info ) );
//   if( param_index >= params_count() )
//     return false;
//   if( !out_param_info )
//     return false;
//   switch( param_index ) {
//     case 0:
//       out_param_info->id = 1;
//       out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
//       out_param_info->cookie = nullptr;
//       std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Time Window" );
//       std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Main" );
//       out_param_info->min_value = 0.1;
//       out_param_info->max_value = 1.0;
//       out_param_info->default_value = 0.1;
//       break;
//   }
//   return true;
// }

// bool AudioAnalysis::params_get_value( clap_id param_id, double* out_value ) {
//   SFG_LOG_TRACE( host_,
//                  host_log_,
//                  "[{:s}] [{:p}] enter( param_id={:d}, out_value={:p} )",
//                  __FUNCTION__,
//                  static_cast< void* >( this ),
//                  param_id,
//                  static_cast< void* >( out_value ) );
//   if( !out_value )
//     return false;
//   if( param_id == 1 ) {
//     ( *out_value ) = state_.time_window();
//     return true;
//   }
//   return false;
// }

// bool AudioAnalysis::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
//   SFG_LOG_TRACE( host_,
//                  host_log_,
//                  "[{:s}] [{:p}] enter( param_id={:d}, value={:f}, out_buffer={:p}, out_info={:d} )",
//                  __FUNCTION__,
//                  static_cast< void* >( this ),
//                  param_id,
//                  value,
//                  static_cast< void* >( out_buffer ),
//                  out_buffer_capacity );
//   if( !out_buffer || ( out_buffer_capacity == 0 ) )
//     return false;
//   if( param_id == 1 ) {
//     std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
//     std::string tmp_str = std::to_string( value );
//     tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
//     return true;
//   }
//   return false;
// }

// bool AudioAnalysis::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
//   SFG_LOG_TRACE( host_,
//                  host_log_,
//                  "[{:s}] [{:p}] enter( param_id={:d}, param_value_text={:?}, out_value={:p} )",
//                  __FUNCTION__,
//                  static_cast< void* >( this ),
//                  param_id,
//                  param_value_text,
//                  static_cast< void* >( out_value ) );
//   if( !out_value )
//     return false;
//   std::function< bool( std::string const&, double* ) > text_to_double = []( std::string const& param_value_text, double* out_value ) {
//     try {
//       ( *out_value ) = std::stod( param_value_text );
//       if( ( *out_value ) > 1.0 ) {
//         ( *out_value ) = ( *out_value ) / 100.0;
//       }
//       return true;
//     } catch( std::exception const& ) {
//       return false;
//     }
//   };
//   if( param_id == 1 ) {
//     return text_to_double( param_value_text, out_value );
//   }
//   return false;
// }

// void AudioAnalysis::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
//   SFG_LOG_TRACE( host_,
//                  host_log_,
//                  "[{:s}] [{:p}] enter( in={:p}, out={:p} )",
//                  __FUNCTION__,
//                  static_cast< void* >( this ),
//                  static_cast< void const* >( in ),
//                  static_cast< void const* >( out ) );
//   SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] in_size={:d}", __FUNCTION__, static_cast< void* >( this ), in->size( in ) );
//   for( uint32_t i = 0; i < in->size( in ); i++ ) {
//     process_event( in->get( in, i ), out );
//   }
//   return;
// }

bool AudioAnalysis::state_save( clap_ostream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );

  ClapOStream tmp( stream );
  bool ret = state_.SerializeToOstream( &tmp );

  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool AudioAnalysis::state_load( clap_istream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );

  ClapIStream tmp( stream );
  bool ret = state_.ParseFromIstream( &tmp );

  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

#pragma endregion

#pragma region CLAP extensions, wether or not to pointer things to clap

bool AudioAnalysis::supports_audio_ports() const {
  return true;
}

bool AudioAnalysis::supports_gui() const {
  return true;
}

// bool AudioAnalysis::supports_params() const {
//   return true;
// }

bool AudioAnalysis::supports_state() const {
  return true;
}

#pragma endregion

#pragma region GUI CALLBACK

void AudioAnalysis::guiTimerCallback() {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );

#pragma region Inputs
  for( SDL_Event event; SDL_PollEvent( &event ) != 0; ) {
    if( event.type == SDL_EVENT_QUIT ) {
      // shouldn't happen since we're inside a DAW
      break;
    } else if( event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED ) {
      // shouldn't happen since we're inside a DAW
    } else if( event.type == SDL_EVENT_WINDOW_HIDDEN ) {
      // shouldn't happen since we're inside a DAW
    } else if( event.type == SDL_EVENT_WINDOW_SHOWN ) {
      // shouldn't happen since we're inside a DAW
    } else if( ( event.type == SDL_EVENT_KEY_DOWN ) || ( event.type == SDL_EVENT_KEY_UP ) ) {
      InputManager::ProcessKeyEvent( event.key, event.type == SDL_EVENT_KEY_DOWN );
    } else if( event.type == SDL_EVENT_MOUSE_MOTION ) {
      InputManager::ProcessMouseMoveEvent( event.motion );
    } else if( ( event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ) || ( event.type == SDL_EVENT_MOUSE_BUTTON_UP ) ) {
      InputManager::ProcessMouseButtonEvent( event.button, event.type == SDL_EVENT_MOUSE_BUTTON_DOWN );
    } else if( event.type == SDL_EVENT_MOUSE_WHEEL ) {
      InputManager::ProcessMouseWheelEvent( event.wheel );
    }
  }
#pragma endregion Inputs

#pragma region Logic
  InputManager::OnLogicUpdate_Early();

  {
    int winW;
    int winH;
    SDL_GetWindowSize( guiWindow_.get(), &winW, &winH );
    guiRootWidget_->SetW( static_cast< float >( winW ) );
    guiRootWidget_->SetH( static_cast< float >( winH ) );
  }
  sampleQueue_.consume_all( [this]( float monoOut ) {
    rmsMomentaryValueBuffer_.push_back( monoOut );
    rmsSamplesReceived_++;
    lufsMomentaryValueBuffer_.push_back( kWeightingFilterHighPass_.filter( kWeightingFilterHighShelf_.filter( monoOut ) ) );
    lufsSamplesReceived_++;

    if( rmsSamplesReceived_ >= ( rmsMomentaryValueBuffer_.capacity() / 4 ) ) {
      rmsSamplesReceived_ = 0;
      rmsShortTermValueBuffer_.push_back( averageOf( rmsMomentaryValueBuffer_ ) );
      if( guiWidgetMomentaryRms_ )
        guiWidgetMomentaryRms_->SetValue( rmsShortTermValueBuffer_.back() );
      if( guiWidgetShortTermRms_ )
        guiWidgetShortTermRms_->SetValue( averageOf( rmsShortTermValueBuffer_ ) );
    }
    if( lufsSamplesReceived_ >= ( lufsMomentaryValueBuffer_.capacity() / 4 ) ) {
      lufsSamplesReceived_ = 0;
      lufsShortTermValueBuffer_.push_back( averageOf( lufsMomentaryValueBuffer_ ) );
      if( guiWidgetMomentaryLufs_ )
        guiWidgetMomentaryLufs_->SetValue( lufsShortTermValueBuffer_.back() );
      if( guiWidgetShortTermLufs_ )
        guiWidgetShortTermLufs_->SetValue( averageOf( lufsShortTermValueBuffer_ ) );
    }
  } );
  guiRootWidget_->OnLogic();

  InputManager::OnLogicUpdate_Late();
#pragma endregion Logic

#pragma region Rendering
  SDL_SetRenderDrawColor( guiWindowRenderer_.get(), 0x00, 0x00, 0x00, 0xff );
  SDL_RenderClear( guiWindowRenderer_.get() );

  // actually draw stuff here
  guiRootWidget_->OnRender( guiWindowRenderer_ );

  SDL_RenderPresent( guiWindowRenderer_.get() );
#pragma endregion Rendering
}

float AudioAnalysis::averageOf( boost::circular_buffer< float > const& buffer ) const {
  return std::accumulate( buffer.begin(), buffer.end(), 0.0f, []( float a, float b ) { return std::abs( a ) + std::abs( b ); } ) / float( buffer.size() );
}

#pragma endregion

#pragma region shit for the factory

clap_plugin_t* AudioAnalysis::s_create( clap_host_t const* host ) {
  clap_plugin_t* plugin = new clap_plugin_t();
  AudioAnalysis* noise_gen = new AudioAnalysis();
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

clap_plugin_descriptor_t* AudioAnalysis::descriptor_get( void ) {
  static clap_plugin_descriptor_t clap_descriptor_;
  static std::vector< char const* > clap_descriptor_features_;

  static std::string const c_plugin_id_ = "com.SFGrenade.AudioAnalysis";
  static std::string const c_plugin_name_ = "SFG-AudioAnalysis";
  static std::string const c_plugin_vendor_ = "SFGrenade";
  static std::string const c_plugin_url_ = "https://sfgrena.de";
  static std::string const c_plugin_manual_url_ = "https://sfgrena.de";
  static std::string const c_plugin_support_url_ = "https://sfgrena.de";
  static std::string const c_plugin_version_ = "0.0.1";
  static std::string const c_plugin_description_ = "Awesome AudioAnalysis. By SFGrenade.";
  static std::vector< std::string > const c_plugin_features_ = { CLAP_PLUGIN_FEATURE_AUDIO_EFFECT,
                                                                 CLAP_PLUGIN_FEATURE_ANALYZER,
                                                                 CLAP_PLUGIN_FEATURE_UTILITY,
                                                                 CLAP_PLUGIN_FEATURE_MIXING,
                                                                 CLAP_PLUGIN_FEATURE_MASTERING,
                                                                 CLAP_PLUGIN_FEATURE_STEREO };

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
