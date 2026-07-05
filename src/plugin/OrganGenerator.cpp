// Header assigned to this source
#include "plugin/OrganGenerator.hpp"

// Project includes
#include "libraryExtensions/math.hpp"

// C++ std includes
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <exception>
#include <functional>
#include <numbers>

namespace SfPb = SfgGenerator::Proto;

OrganGenerator::OrganGenerator()
    : _base_(), guiWindow_( nullptr, [this]( SDL_Window* ptr ) {
        WRAP_SDL_CALL_INST( SDL_HideWindow, ptr );
        SDL_DestroyWindow( ptr );
      } ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

OrganGenerator::~OrganGenerator() {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string OrganGenerator::get_name( void ) const {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "OrganGenerator";
}

#pragma region shit to override

bool OrganGenerator::init( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  logger_ = logger_->clone( "OrganGenerator" );

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );

  state_.Clear();
  state_.set_gui_width( 300 );
  state_.set_gui_height( 200 );
  noteMap_.clear();

  ret = ret && true;
  return ret;
}

bool OrganGenerator::activate( double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count ) {
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] enter( sample_rate={:f}, min_frames_count={:d}, max_frames_count={:d} )",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    sample_rate,
                    min_frames_count,
                    max_frames_count );
  bool ret = _base_::activate( sample_rate, min_frames_count, max_frames_count );
  noteMap_.setSampleRate( sample_rate );

  // todo: fixme: temporary single pipe here
  size_t i;
  decltype( fluePipes_ )::iterator iter;
  for( i = 0, iter = fluePipes_.begin(); iter != fluePipes_.end(); i++, iter++ ) {
    double freq = 440.0 * std::pow( 2.0, ( double( i ) - 69.0 ) / 12.0 );
    iter->length = SPEED_OF_SOUND / ( 2.0 * freq );
    iter->openess = FluePipe::Openess::Open;
    iter->Init( sample_rate, logger_->clone( fmt::format( "OrganGenerator::fluePipe_{:d}", i ) ) );
  }

  ret = ret && true;
  return ret;
}

void OrganGenerator::on_main_thread( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::on_main_thread();

  // synchronization of values needed:
  // 1. gui does `clap_host_->request_callback( clap_host_ );`
  // 2. this method does `clap_host_state_->mark_dirty( clap_host_ );`
  // 3. this method does `clap_host_params_->rescan( clap_host_, CLAP_PARAM_RESCAN_VALUES );`
  host_state_->mark_dirty( host_ );
  host_params_->rescan( host_, CLAP_PARAM_RESCAN_VALUES );
}

void OrganGenerator::reset( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );
  state_.Clear();
  state_.set_gui_width( 300 );
  state_.set_gui_height( 200 );
  noteMap_.clear();
}

void OrganGenerator::process_event( clap_event_header_t const* hdr, clap_output_events_t const* /*out_events*/ ) {
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( hdr={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( hdr ) );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->size    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->size );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->time    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->time );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->space_id={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->space_id );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->type    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->type );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->flags   ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->flags );
  if( hdr->space_id != CLAP_CORE_EVENT_SPACE_ID ) {
    return;
  }
  if( hdr->type == CLAP_EVENT_NOTE_ON ) {
    clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
    noteMap_.handleEvent( ev );
  } else if( hdr->type == CLAP_EVENT_NOTE_OFF ) {
    clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
    noteMap_.handleEvent( ev );
  } else if( hdr->type == CLAP_EVENT_NOTE_CHOKE ) {
    clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
    noteMap_.handleEvent( ev );
  } else if( hdr->type == CLAP_EVENT_NOTE_END ) {
    clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
    noteMap_.handleEvent( ev );
  } else if( hdr->type == CLAP_EVENT_NOTE_EXPRESSION ) {
    clap_event_note_expression_t const* ev = reinterpret_cast< clap_event_note_expression_t const* >( hdr );
    PLUGIN_LOG_TRACE( host_,
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
  } else if( hdr->type == CLAP_EVENT_PARAM_VALUE ) {
    clap_event_param_value_t const* ev = reinterpret_cast< clap_event_param_value_t const* >( hdr );
    // PLUGIN_LOG_TRACE( host_,
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
  } else if( hdr->type == CLAP_EVENT_PARAM_MOD ) {
    clap_event_param_mod_t const* ev = reinterpret_cast< clap_event_param_mod_t const* >( hdr );
    PLUGIN_LOG_TRACE( host_,
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
    PLUGIN_LOG_TRACE( host_,
                      host_log_,
                      "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_BEGIN - param_id={:d}",
                      __FUNCTION__,
                      static_cast< void* >( this ),
                      ev->param_id );
  } else if( hdr->type == CLAP_EVENT_PARAM_GESTURE_END ) {
    clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
    PLUGIN_LOG_TRACE( host_,
                      host_log_,
                      "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_END - param_id={:d}",
                      __FUNCTION__,
                      static_cast< void* >( this ),
                      ev->param_id );
  } else if( hdr->type == CLAP_EVENT_TRANSPORT ) {
    clap_event_transport_t const* ev = reinterpret_cast< clap_event_transport_t const* >( hdr );
    PLUGIN_LOG_TRACE( host_,
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
    noteMap_.handleEvent( ev );
    // PLUGIN_LOG_TRACE( host_,
    //                host_log_,
    //                "[{:s}] [{:p}] CLAP_EVENT_MIDI - port_index={:d}, data={}",
    //                __FUNCTION__,
    //                static_cast< void* >( this ),
    //                ev->port_index,
    //                std::vector< uint8_t >( ev->data, ev->data + 3 ) );
    if( ev->data[0] == 0xB0 ) {
      // control change channel 0
      int param_id = ev->data[1] + 1;  // who knows if it's actually data[1]
      double value = double( ev->data[2] ) / double( 0x7F );
    }
  } else if( hdr->type == CLAP_EVENT_MIDI_SYSEX ) {
    clap_event_midi_sysex_t const* ev = reinterpret_cast< clap_event_midi_sysex_t const* >( hdr );
    PLUGIN_LOG_TRACE( host_,
                      host_log_,
                      "[{:s}] [{:p}] CLAP_EVENT_MIDI_SYSEX - port_index={:d}, buffer={:p}, size={}",
                      __FUNCTION__,
                      static_cast< void* >( this ),
                      ev->port_index,
                      static_cast< void const* >( ev->buffer ),
                      ev->size );
  } else if( hdr->type == CLAP_EVENT_MIDI2 ) {
    clap_event_midi2_t const* ev = reinterpret_cast< clap_event_midi2_t const* >( hdr );
    PLUGIN_LOG_TRACE( host_,
                      host_log_,
                      "[{:s}] [{:p}] CLAP_EVENT_MIDI2 - port_index={:d}, data={}",
                      __FUNCTION__,
                      static_cast< void* >( this ),
                      ev->port_index,
                      std::vector< uint8_t >( ev->data, ev->data + 4 ) );
  }
}

clap_process_status OrganGenerator::process( clap_process_t const* process ) {
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( process={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( process
  // )
  // );
  const uint32_t nframes = process->frames_count;
  const uint32_t nev = process->in_events->size( process->in_events );
  uint32_t ev_index = 0;
  uint32_t next_ev_frame = nev > 0 ? 0 : nframes;

  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] nframes      ={:d}", __FUNCTION__, static_cast< void* >( this ), nframes );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] nev          ={:d}", __FUNCTION__, static_cast< void* >( this ), nev );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] ev_index     ={:d}", __FUNCTION__, static_cast< void* >( this ), ev_index );
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] next_ev_frame={:d}", __FUNCTION__, static_cast< void* >( this ), next_ev_frame );

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

    /* process every samples until the next event */
    for( ; i < next_ev_frame; ++i ) {
      float out_total = 0.0f;
      for( uint32_t c = 0; c < process->audio_outputs[0].channel_count; c++ ) {
        float out = 0.0f;
        if( active_ && process_ ) {
          for( size_t fluePipe = 0; fluePipe < fluePipes_.size(); fluePipe++ ) {
            NoteMap::NoteDescription note;
            note.key = fluePipe;
            out += fluePipes_[fluePipe].Update( noteMap_.velocity( note ) );
          }
        }
        // store output
        if( process->audio_outputs[0].data32 )
          process->audio_outputs[0].data32[c][i] = out;
        else if( process->audio_outputs[0].data64 )
          process->audio_outputs[0].data64[c][i] = out;
      }
    }
  }

  return ( active_ && process_ ) ? CLAP_PROCESS_CONTINUE : CLAP_PROCESS_SLEEP;
}

#pragma endregion

#pragma region CLAP extensions

uint32_t OrganGenerator::audio_ports_count( bool is_input ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 0;
  }
  return 1;
}

bool OrganGenerator::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
  PLUGIN_LOG_TRACE( host_,
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
    return false;
  }
  out_info->id = 0;
  std::snprintf( out_info->name, sizeof( out_info->name ), "%s", "main" );
  out_info->channel_count = 1;
  out_info->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS | CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE;
  out_info->port_type = CLAP_PORT_MONO;
  out_info->in_place_pair = CLAP_INVALID_ID;
  return true;
}

bool OrganGenerator::gui_is_api_supported( std::string const& api, bool is_floating ) {
  bool ret = _base_::gui_is_api_supported( api, is_floating );
  return ret || true;
}

bool OrganGenerator::gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) {
  bool ret = _base_::gui_get_preferred_api( out_api, out_is_floating );
  return ret && false;
}

bool OrganGenerator::gui_create( std::string const& api, bool is_floating ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] init SDL", __FUNCTION__, static_cast< void* >( this ) );
  if( !SDL_InitSubSystem( SDL_INIT_VIDEO ) ) {
    PLUGIN_LOG_ERROR( host_, host_log_, "[{:s}] [{:p}] error initializing SDL: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }
  if( !TTF_Init() ) {
    PLUGIN_LOG_ERROR( host_, host_log_, "[{:s}] [{:p}] error initializing SDL_TTF: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  InputManager::init();

  guiRootWidget_ = std::make_shared< Widget >( logger_->clone( "OrganGenerator::guiRootWidget_" ) );
  guiRootWidget_->SetPadding( 5.0f );

  guiWidgetSomeLabel_
      = std::make_shared< Label >( "OrganGenerator", logger_->clone( "OrganGenerator::guiWidgetSomeLabel_" ), SDL_FRect{ 0.0f, 0.0f, 1.0f, 1.0f } );
  guiWidgetSomeLabel_->InitUi( guiRootWidget_ );
  guiWidgetSomeLabel_->SetHorizontalAlignment( Label::HorizontalAlignment::Centered );
  guiWidgetSomeLabel_->SetVerticalAlignment( Label::VerticalAlignment::Centered );
  guiWidgetSomeLabel_->SetFontFile( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" );
  guiWidgetSomeLabel_->SetFontSize( 18 );
  guiWidgetSomeLabel_->SetFontColourActive( SDL_Color{ 0xff, 0xff, 0xff, 0xff } );
  guiWidgetSomeLabel_->SetFontColourInactive( SDL_Color{ 0xff, 0xff, 0xff, 0x80 } );

  SDL_PropertiesID windowCreateProps = SDL_CreateProperties();
  WRAP_SDL_CALL_INST( SDL_SetBooleanProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true );
  WRAP_SDL_CALL_INST( SDL_SetBooleanProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true );
  WRAP_SDL_CALL_INST( SDL_SetNumberProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, state_.gui_width() );
  WRAP_SDL_CALL_INST( SDL_SetNumberProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, state_.gui_height() );
  WRAP_SDL_CALL_INST( SDL_SetNumberProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0 );
  WRAP_SDL_CALL_INST( SDL_SetNumberProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0 );
  if( !is_floating ) {
    WRAP_SDL_CALL_INST( SDL_SetBooleanProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true );
  } else {
    WRAP_SDL_CALL_INST( SDL_SetStringProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "com.SFGrenade.OrganGenerator" );
    WRAP_SDL_CALL_INST( SDL_SetBooleanProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false );
  }
  guiWindow_ = std::shared_ptr< SDL_Window >( SDL_CreateWindowWithProperties( windowCreateProps ), [this]( SDL_Window* ptr ) {
    if( ptr ) {
      WRAP_SDL_CALL_INST( SDL_HideWindow, ptr );
      SDL_DestroyWindow( ptr );
    }
  } );
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] window created at {:p}",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    static_cast< void* >( guiWindow_.get() ) );

  guiWindowRenderer_ = std::shared_ptr< SDL_Renderer >( SDL_CreateRenderer( guiWindow_.get(), nullptr ), []( SDL_Renderer* ptr ) {
    if( ptr ) {
      SDL_DestroyRenderer( ptr );
    }
  } );
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] window renderer at {:p}",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    static_cast< void* >( guiWindowRenderer_.get() ) );
  WRAP_SDL_CALL_INST( SDL_SetRenderDrawBlendMode, guiWindowRenderer_.get(), SDL_BLENDMODE_BLEND );

  guiTimer_ = Timer::createNative( 1, std::bind( &OrganGenerator::guiTimerCallback, this ) );
  guiTimer_->start();
  return true;
}

void OrganGenerator::gui_destroy( void ) {
  guiTimer_->stop();
  guiTimer_.reset();
  guiWindowRenderer_.reset();
  guiWindow_.reset();

  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] quit SDL", __FUNCTION__, static_cast< void* >( this ) );
  TTF_Quit();
  SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

bool OrganGenerator::gui_set_scale( double scale ) {
  return false;
}

bool OrganGenerator::gui_get_size( uint32_t* out_width, uint32_t* out_height ) {
  WRAP_SDL_CALL_INST( SDL_GetWindowSize, guiWindow_.get(), reinterpret_cast< int* >( out_width ), reinterpret_cast< int* >( out_height ) );
  return true;
}

bool OrganGenerator::gui_can_resize( void ) {
  return ( SDL_GetWindowFlags( guiWindow_.get() ) & SDL_WINDOW_RESIZABLE );
}

bool OrganGenerator::gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  return false;
}

bool OrganGenerator::gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  gui_set_size( *out_width, *out_height );
  gui_get_size( out_width, out_height );
  state_.set_gui_width( *out_width );
  state_.set_gui_height( *out_height );
  return true;
}

bool OrganGenerator::gui_set_size( uint32_t width, uint32_t height ) {
  WRAP_SDL_CALL_INST( SDL_SetWindowSize, guiWindow_.get(), width, height );
  state_.set_gui_width( width );
  state_.set_gui_height( height );
  return true;
}

bool OrganGenerator::gui_set_parent( clap_window_t const* window ) {
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
  WRAP_SDL_CALL_INST( SDL_SetWindowPosition, guiWindow_.get(), 0, 0 );
  return true;
}

bool OrganGenerator::gui_set_transient( clap_window_t const* window ) {
  WRAP_SDL_CALL_INST( SDL_RaiseWindow, guiWindow_.get() );
  return true;
}

void OrganGenerator::gui_suggest_title( std::string const& title ) {
  WRAP_SDL_CALL_INST( SDL_SetWindowTitle, guiWindow_.get(), title.c_str() );
}

bool OrganGenerator::gui_show( void ) {
  WRAP_SDL_CALL_INST( SDL_ShowWindow, guiWindow_.get() );
  return true;
}

bool OrganGenerator::gui_hide( void ) {
  WRAP_SDL_CALL_INST( SDL_HideWindow, guiWindow_.get() );
  return true;
}

uint32_t OrganGenerator::note_ports_count( bool is_input ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 1;
  }
  return 0;
}

bool OrganGenerator::note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info ) {
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] enter( index={:d}, is_input={}, out_info={:p} )",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    index,
                    is_input,
                    static_cast< void* >( out_info ) );
  if( index >= note_ports_count( is_input ) )
    return false;
  if( !out_info )
    return false;
  if( is_input ) {
    out_info->id = 0;
    std::snprintf( out_info->name, sizeof( out_info->name ), "%s", "main" );
    out_info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI /*| CLAP_NOTE_DIALECT_MIDI2*/;
    out_info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
    return true;
  }
  return false;
}

uint32_t OrganGenerator::params_count( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
  // adjust according to OrganGenerator.proto
  // while we could make it dynamic, without explicit gui i'd rather not
  return 0;
}

bool OrganGenerator::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
  PLUGIN_LOG_TRACE( host_,
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
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Todo" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "ToDo" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
  }
  return true;
}

bool OrganGenerator::params_get_value( clap_id param_id, double* out_value ) {
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] enter( param_id={:d}, out_value={:p} )",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    param_id,
                    static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  if( param_id == 1 ) {
    ( *out_value ) = 0;
    return true;
  }
  return false;
}

bool OrganGenerator::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] enter( param_id={:d}, value={:f}, out_buffer={:p}, out_buffer_capacity={:d} )",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    param_id,
                    value,
                    static_cast< void* >( out_buffer ),
                    out_buffer_capacity );
  if( !out_buffer || ( out_buffer_capacity == 0 ) )
    return false;
  return false;
}

bool OrganGenerator::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
  PLUGIN_LOG_TRACE( host_,
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

void OrganGenerator::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] enter( in={:p}, out={:p} )",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    static_cast< void const* >( in ),
                    static_cast< void const* >( out ) );
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] in_size={:d}", __FUNCTION__, static_cast< void* >( this ), in->size( in ) );

  for( uint32_t i = 0; i < in->size( in ); i++ ) {
    process_event( in->get( in, i ), out );
  }
  return;
}

bool OrganGenerator::state_save( clap_ostream_t const* stream ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );

  ClapOStream tmp( stream );
  bool ret = state_.SerializeToOstream( &tmp );

  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool OrganGenerator::state_load( clap_istream_t const* stream ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );

  ClapIStream tmp( stream );
  bool ret = state_.ParseFromIstream( &tmp );

  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

#pragma endregion

#pragma region CLAP extensions, wether or not to pointer things to clap

bool OrganGenerator::supports_audio_ports() const {
  return true;
}

bool OrganGenerator::supports_gui() const {
  return true;
}

bool OrganGenerator::supports_note_ports() const {
  return true;
}

bool OrganGenerator::supports_params() const {
  return true;
}

bool OrganGenerator::supports_state() const {
  return true;
}

#pragma endregion

#pragma region GUI CALLBACK

void OrganGenerator::guiTimerCallback() {
  // PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );

#pragma region Inputs
  for( SDL_Event event; SDL_PollEvent( &event ); ) {
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
    WRAP_SDL_CALL_INST( SDL_GetWindowSize, guiWindow_.get(), &winW, &winH );
    guiRootWidget_->SetW( static_cast< float >( winW ) );
    guiRootWidget_->SetH( static_cast< float >( winH ) );
  }
  guiRootWidget_->OnLogic();

  InputManager::OnLogicUpdate_Late();
#pragma endregion Logic

#pragma region Rendering
  WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, guiWindowRenderer_.get(), 0x00, 0x00, 0x00, 0xff );
  WRAP_SDL_CALL_INST( SDL_RenderClear, guiWindowRenderer_.get() );

  // actually draw stuff here
  guiRootWidget_->OnRender( guiWindowRenderer_ );

  WRAP_SDL_CALL_INST( SDL_RenderPresent, guiWindowRenderer_.get() );
#pragma endregion Rendering
}

#pragma endregion

#pragma region shit for the factory

clap_plugin_t* OrganGenerator::s_create( clap_host_t const* host ) {
  clap_plugin_t* plugin = new clap_plugin_t();
  OrganGenerator* noise_gen = new OrganGenerator();
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

clap_plugin_descriptor_t* OrganGenerator::descriptor_get( void ) {
  static clap_plugin_descriptor_t clap_descriptor_;
  static std::vector< char const* > clap_descriptor_features_;

  static std::string const c_plugin_id_ = "com.SFGrenade.OrganGenerator";
  static std::string const c_plugin_name_ = "SFG-OrganGenerator";
  static std::string const c_plugin_vendor_ = "SFGrenade";
  static std::string const c_plugin_url_ = "https://sfgrena.de";
  static std::string const c_plugin_manual_url_ = "https://sfgrena.de";
  static std::string const c_plugin_support_url_ = "https://sfgrena.de";
  static std::string const c_plugin_version_ = "0.0.1";
  static std::string const c_plugin_description_ = "Awesome OrganGenerator. By SFGrenade.";
  static std::vector< std::string > const c_plugin_features_ = { CLAP_PLUGIN_FEATURE_INSTRUMENT, CLAP_PLUGIN_FEATURE_SYNTHESIZER, CLAP_PLUGIN_FEATURE_STEREO };

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
