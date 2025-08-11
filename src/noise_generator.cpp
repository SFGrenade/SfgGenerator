#include "noise_generator.hpp"

#include <numbers>

#include "clap/process.h"

NoiseGenerator::NoiseGenerator() : _base_() {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

NoiseGenerator::~NoiseGenerator() {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string NoiseGenerator::get_name( void ) const {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "NoiseGenerator";
}

#pragma region wave generators

double NoiseGenerator::get_sample_sine_wave( double phase ) {
  return std::sin( phase * 2.0 * std::numbers::pi_v< double > );
}

double NoiseGenerator::get_sample_square_wave( double phase ) {
  return ( phase < 0.5 ) ? ( 1.0 ) : ( -1.0 );
}

double NoiseGenerator::get_sample_saw_wave( double phase ) {
  return ( 2.0 * phase ) - 1.0;
}

double NoiseGenerator::get_sample_triangle_wave( double phase ) {
  if( phase < 0.25 ) {
    return std::lerp( 0.0, 1.0, ( phase - 0.0 ) * 4.0 );
  }
  if( phase < 0.75 ) {
    return std::lerp( 1.0, -1.0, ( phase - 0.25 ) * 2.0 );
  }
  return std::lerp( -1.0, 0.0, ( phase - 0.75 ) * 4.0 );
}

double NoiseGenerator::get_sample_white_noise( double phase ) {
  return dist_( eng_ );
}

// Paul Kellett's refined method
double NoiseGenerator::get_sample_pink_noise( double phase ) {
  double white = get_sample_white_noise( phase );
  pink_refined_b0_ = 0.99886 * pink_refined_b0_ + white * 0.0555179;
  pink_refined_b1_ = 0.99332 * pink_refined_b1_ + white * 0.0750759;
  pink_refined_b2_ = 0.96900 * pink_refined_b2_ + white * 0.1538520;
  pink_refined_b3_ = 0.86650 * pink_refined_b3_ + white * 0.3104856;
  pink_refined_b4_ = 0.55000 * pink_refined_b4_ + white * 0.5329522;
  pink_refined_b5_ = -0.7616 * pink_refined_b5_ - white * 0.0168980;
  double pink
      = pink_refined_b0_ + pink_refined_b1_ + pink_refined_b2_ + pink_refined_b3_ + pink_refined_b4_ + pink_refined_b5_ + pink_refined_b6_ + white * 0.5362;
  pink_refined_b6_ = white * 0.115926;
  return std::clamp( pink * 0.11, -1.0, 1.0 );  // empirically estimated
}

// recursive 1-pole iir filter
double NoiseGenerator::get_sample_red_noise( double phase ) {
  static const double gain = 0.01;
  double white = get_sample_white_noise( phase );
  red_leaky_integrator_prev_ += gain * ( white - red_leaky_integrator_prev_ );
  double red = red_leaky_integrator_prev_;
  return std::clamp( red, -1.0, 1.0 );
}

double NoiseGenerator::get_sample_blue_noise( double phase ) {
  double blue = 0.0;
  return std::clamp( blue, -1.0, 1.0 );
}

double NoiseGenerator::get_sample_violet_noise( double phase ) {
  double violet = 0.0;
  return std::clamp( violet, -1.0, 1.0 );
}

double NoiseGenerator::get_sample_grey_noise( double phase ) {
  double grey = 0.0;
  return std::clamp( grey, -1.0, 1.0 );
}

double NoiseGenerator::get_sample_velvet_noise( double phase ) {
  double velvet = 0.0;
  return std::clamp( velvet, -1.0, 1.0 );
}

#pragma endregion

#pragma region shit to override

bool NoiseGenerator::init( void ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );
  synth_type_ = SynthType::SineWave;
  pink_refined_b0_ = 0.0;
  pink_refined_b1_ = 0.0;
  pink_refined_b2_ = 0.0;
  pink_refined_b3_ = 0.0;
  pink_refined_b4_ = 0.0;
  pink_refined_b5_ = 0.0;
  pink_refined_b6_ = 0.0;
  red_leaky_integrator_prev_ = 0.0;
  note_map_.clear();

  ret = ret && true;
  return ret;
}

void NoiseGenerator::reset( void ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );
  synth_type_ = SynthType::SineWave;
  pink_refined_b0_ = 0.0;
  pink_refined_b1_ = 0.0;
  pink_refined_b2_ = 0.0;
  pink_refined_b3_ = 0.0;
  pink_refined_b4_ = 0.0;
  pink_refined_b5_ = 0.0;
  pink_refined_b6_ = 0.0;
  red_leaky_integrator_prev_ = 0.0;
  note_map_.clear();
}

void NoiseGenerator::process_event( clap_event_header_t const* hdr ) {
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter( hdr={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( hdr ) );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] hdr->size    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->size );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] hdr->time    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->time );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] hdr->space_id={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->space_id );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] hdr->type    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->type );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] hdr->flags   ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->flags );
  if( hdr->space_id == CLAP_CORE_EVENT_SPACE_ID ) {
    switch( hdr->type ) {
      case CLAP_EVENT_NOTE_ON: {
        clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
        // SFG_LOG_TRACE( logger_,
        //                "[{:s}] [{:p}] CLAP_EVENT_NOTE_ON - note_id={:d}, port_index={:d}, channel={:d}, key={:d}, velocity={:f}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->note_id,
        //                ev->port_index,
        //                ev->channel,
        //                ev->key,
        //                ev->velocity );
        if( ev->key == -1 ) {
          for( auto& item : note_map_ ) {
            item.second.phase = 0.0;
            item.second.velocity = ev->velocity;
          }
        } else {
          if( !note_map_.contains( ev->key ) ) {
            note_map_[ev->key] = NoteData();
          }
          note_map_[ev->key].phase = 0.0;
          note_map_[ev->key].velocity = ev->velocity;
        }
        break;
      }
      case CLAP_EVENT_NOTE_OFF: {
        clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
        // SFG_LOG_TRACE( logger_,
        //                "[{:s}] [{:p}] CLAP_EVENT_NOTE_OFF - note_id={:d}, port_index={:d}, channel={:d}, key={:d}, velocity={:f}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->note_id,
        //                ev->port_index,
        //                ev->channel,
        //                ev->key,
        //                ev->velocity );
        if( ev->key == -1 ) {
          note_map_.clear();
        } else {
          if( note_map_.contains( ev->key ) ) {
            note_map_.erase( ev->key );
          }
        }
        break;
      }
      case CLAP_EVENT_NOTE_CHOKE: {
        clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
        // SFG_LOG_TRACE( logger_,
        //                "[{:s}] [{:p}] CLAP_EVENT_NOTE_CHOKE - note_id={:d}, port_index={:d}, channel={:d}, key={:d}, velocity={:f}",
        //                __FUNCTION__,
        //                static_cast< void* >( this ),
        //                ev->note_id,
        //                ev->port_index,
        //                ev->channel,
        //                ev->key,
        //                ev->velocity );
        if( ev->key == -1 ) {
          note_map_.clear();
        } else {
          if( note_map_.contains( ev->key ) ) {
            note_map_.erase( ev->key );
          }
        }
        break;
      }
      case CLAP_EVENT_NOTE_EXPRESSION: {
        clap_event_note_expression_t const* ev = reinterpret_cast< clap_event_note_expression_t const* >( hdr );
        SFG_LOG_TRACE( logger_,
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
        SFG_LOG_TRACE( logger_,
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
        if( ev->param_id == 0 ) {
          synth_type_ = static_cast< SynthType >( ev->value );
          if( synth_type_ >= SynthType::MAX_VALUE )
            synth_type_ = static_cast< SynthType >( static_cast< uint16_t >( SynthType::MAX_VALUE ) - 1 );
        }
        break;
      }
      case CLAP_EVENT_PARAM_MOD: {
        clap_event_param_mod_t const* ev = reinterpret_cast< clap_event_param_mod_t const* >( hdr );
        SFG_LOG_TRACE( logger_,
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
        // TODO: handle parameter modulation
        break;
      }
      case CLAP_EVENT_PARAM_GESTURE_BEGIN: {
        clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
        SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_BEGIN - param_id={:d}", __FUNCTION__, static_cast< void* >( this ), ev->param_id );
        // TODO: handle parameter modulation
        break;
      }
      case CLAP_EVENT_PARAM_GESTURE_END: {
        clap_event_param_gesture_t const* ev = reinterpret_cast< clap_event_param_gesture_t const* >( hdr );
        SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] CLAP_EVENT_PARAM_GESTURE_END - param_id={:d}", __FUNCTION__, static_cast< void* >( this ), ev->param_id );
        // TODO: handle parameter modulation
        break;
      }
      case CLAP_EVENT_TRANSPORT: {
        clap_event_transport_t const* ev = reinterpret_cast< clap_event_transport_t const* >( hdr );
        SFG_LOG_TRACE( logger_,
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
        // TODO: handle transport event
        break;
      }
      case CLAP_EVENT_MIDI: {
        clap_event_midi_t const* ev = reinterpret_cast< clap_event_midi_t const* >( hdr );
        SFG_LOG_TRACE( logger_,
                       "[{:s}] [{:p}] CLAP_EVENT_MIDI - port_index={:d}, data={}",
                       __FUNCTION__,
                       static_cast< void* >( this ),
                       ev->port_index,
                       std::vector< uint8_t >( ev->data, ev->data + 3 ) );
        // TODO: handle MIDI event
        break;
      }
      case CLAP_EVENT_MIDI_SYSEX: {
        clap_event_midi_sysex_t const* ev = reinterpret_cast< clap_event_midi_sysex_t const* >( hdr );
        SFG_LOG_TRACE( logger_,
                       "[{:s}] [{:p}] CLAP_EVENT_MIDI_SYSEX - port_index={:d}, buffer={:p}, size={}",
                       __FUNCTION__,
                       static_cast< void* >( this ),
                       ev->port_index,
                       static_cast< void const* >( ev->buffer ),
                       ev->size );
        // TODO: handle MIDI Sysex event
        break;
      }
      case CLAP_EVENT_MIDI2: {
        clap_event_midi2_t const* ev = reinterpret_cast< clap_event_midi2_t const* >( hdr );
        SFG_LOG_TRACE( logger_,
                       "[{:s}] [{:p}] CLAP_EVENT_MIDI2 - port_index={:d}, data={}",
                       __FUNCTION__,
                       static_cast< void* >( this ),
                       ev->port_index,
                       std::vector< uint8_t >( ev->data, ev->data + 4 ) );
        // TODO: handle MIDI2 event
        break;
      }
    }
  }
}

clap_process_status NoiseGenerator::process( clap_process_t const* process ) {
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter( process={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( process ) );
  const uint32_t nframes = process->frames_count;
  const uint32_t nev = process->in_events->size( process->in_events );
  uint32_t ev_index = 0;
  uint32_t next_ev_frame = nev > 0 ? 0 : nframes;

  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] nframes      ={:d}", __FUNCTION__, static_cast< void* >( this ), nframes );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] nev          ={:d}", __FUNCTION__, static_cast< void* >( this ), nev );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] ev_index     ={:d}", __FUNCTION__, static_cast< void* >( this ), ev_index );
  // SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] next_ev_frame={:d}", __FUNCTION__, static_cast< void* >( this ), next_ev_frame );

  for( uint32_t i = 0; i < nframes; ) {
    /* handle every events that happrens at the frame "i" */
    while( ev_index < nev && next_ev_frame == i ) {
      const clap_event_header_t* hdr = process->in_events->get( process->in_events, ev_index );
      if( hdr->time != i ) {
        next_ev_frame = hdr->time;
        break;
      }
      process_event( hdr );
      ++ev_index;
      if( ev_index == nev ) {
        // we reached the end of the event list
        next_ev_frame = nframes;
        break;
      }
    }

    /* process every samples until the next event */
    for( ; i < next_ev_frame; ++i ) {
      for( uint32_t c = 0; c < process->audio_outputs[0].channel_count; c++ ) {
        double out = 0.0;
        if( active_ && process_ ) {
          for( auto& item : note_map_ ) {
            // A4 (note id 69) is 440.0 Hz
            double freq = 440.0 * std::pow( 2.0, ( item.first - 69 ) / 12.0 );

            double sample = 0.0;
            // phase is 0.0 .. 1.0
            switch( synth_type_ ) {
              case SynthType::SineWave:
                sample = get_sample_sine_wave( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::SquareWave:
                sample = get_sample_square_wave( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::SawWave:
                sample = get_sample_saw_wave( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::TriangleWave:
                sample = get_sample_triangle_wave( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::WhiteNoise:
                sample = get_sample_white_noise( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::PinkNoise:
                sample = get_sample_pink_noise( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::RedNoise:
                sample = get_sample_red_noise( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::BlueNoise:
                sample = get_sample_blue_noise( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::VioletNoise:
                sample = get_sample_violet_noise( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::GreyNoise:
                sample = get_sample_grey_noise( item.second.phase ) * item.second.velocity;
                break;
              case SynthType::VelvetNoise:
                sample = get_sample_velvet_noise( item.second.phase ) * item.second.velocity;
                break;
              default:
                break;
            }

            item.second.phase += freq / sample_rate_;
            if( item.second.phase >= 1.0 )
              item.second.phase -= 1.0;

            out += sample;
          }
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

uint32_t NoiseGenerator::audio_ports_count( bool is_input ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 0;
  }
  return 1;
}

bool NoiseGenerator::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
  SFG_LOG_TRACE( logger_,
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

uint32_t NoiseGenerator::note_ports_count( bool is_input ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 1;
  }
  return 0;
}

bool NoiseGenerator::note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info ) {
  SFG_LOG_TRACE( logger_,
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
    out_info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI_MPE | CLAP_NOTE_DIALECT_MIDI2;
    out_info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
    return true;
  }
  return false;
}

uint32_t NoiseGenerator::params_count( void ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
  // we have 1 param, which is synth_type_
  return 1;
}

bool NoiseGenerator::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
  SFG_LOG_TRACE( logger_,
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
      out_param_info->id = 0;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = &synth_type_;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Synth" );
      out_param_info->min_value = static_cast< double >( SynthType::SineWave );
      out_param_info->max_value = static_cast< double >( static_cast< uint16_t >( SynthType::MAX_VALUE ) - 1 );
      out_param_info->default_value = static_cast< double >( SynthType::SineWave );
      break;
  }
  return true;
}

bool NoiseGenerator::params_get_value( clap_id param_id, double* out_value ) {
  SFG_LOG_TRACE( logger_,
                 "[{:s}] [{:p}] enter( param_id={:d}, out_value={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  if( param_id == 0 ) {
    *out_value = static_cast< double >( synth_type_ );
    return true;
  }
  return false;
}

bool NoiseGenerator::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
  SFG_LOG_TRACE( logger_,
                 "[{:s}] [{:p}] enter( param_id={:d}, value={:f}, out_buffer={:p}, out_info={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 value,
                 static_cast< void* >( out_buffer ),
                 out_buffer_capacity );
  if( !out_buffer || ( out_buffer_capacity == 0 ) )
    return false;
  if( param_id == 0 ) {
    SynthType val = static_cast< SynthType >( value );
    switch( val ) {
      case SynthType::SineWave:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Sine Wave" );
        break;
      case SynthType::SquareWave:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Square Wave" );
        break;
      case SynthType::SawWave:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Saw Wave" );
        break;
      case SynthType::TriangleWave:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Triangle Wave" );
        break;
      case SynthType::WhiteNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "White Noise" );
        break;
      case SynthType::PinkNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Pink Noise" );
        break;
      case SynthType::RedNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Red Noise" );
        break;
      case SynthType::BlueNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Blue Noise" );
        break;
      case SynthType::VioletNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Violet Noise" );
        break;
      case SynthType::GreyNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Grey Noise" );
        break;
      case SynthType::VelvetNoise:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "Velvet Noise" );
        break;
      default:
        std::snprintf( out_buffer, out_buffer_capacity, "%s", "__UNKNOWN__" );
        break;
    }
    return true;
  }
  return false;
}

bool NoiseGenerator::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
  SFG_LOG_TRACE( logger_,
                 "[{:s}] [{:p}] enter( param_id={:d}, param_value_text={:?}, out_value={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 param_value_text,
                 static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  if( param_id == 0 ) {
    if( param_value_text == "Sine Wave" ) {
      *out_value = static_cast< double >( SynthType::SineWave );
    } else if( param_value_text == "Square Wave" ) {
      *out_value = static_cast< double >( SynthType::SquareWave );
    } else if( param_value_text == "Saw Wave" ) {
      *out_value = static_cast< double >( SynthType::SawWave );
    } else if( param_value_text == "Triangle Wave" ) {
      *out_value = static_cast< double >( SynthType::TriangleWave );
    } else if( param_value_text == "White Noise" ) {
      *out_value = static_cast< double >( SynthType::WhiteNoise );
    } else if( param_value_text == "Pink Noise" ) {
      *out_value = static_cast< double >( SynthType::PinkNoise );
    } else if( param_value_text == "Red Noise" ) {
      *out_value = static_cast< double >( SynthType::RedNoise );
    } else if( param_value_text == "Blue Noise" ) {
      *out_value = static_cast< double >( SynthType::BlueNoise );
    } else if( param_value_text == "Violet Noise" ) {
      *out_value = static_cast< double >( SynthType::VioletNoise );
    } else if( param_value_text == "Grey Noise" ) {
      *out_value = static_cast< double >( SynthType::GreyNoise );
    } else if( param_value_text == "Velvet Noise" ) {
      *out_value = static_cast< double >( SynthType::VelvetNoise );
    } else {
      *out_value = static_cast< double >( SynthType::SineWave );
    }
    return true;
  }
  return false;
}

void NoiseGenerator::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
  SFG_LOG_TRACE( logger_,
                 "[{:s}] [{:p}] enter( in={:p}, out={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( in ),
                 static_cast< void const* >( out ) );
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] in_size={:d}", __FUNCTION__, static_cast< void* >( this ), in->size( in ) );

  for( uint32_t i = 0; i < in->size( in ); i++ ) {
    process_event( in->get( in, i ) );
  }
  return;
}

bool NoiseGenerator::state_save( clap_ostream_t const* stream ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );
  bool ret = true;
  ret = ( ( -1 ) != stream->write( stream, &synth_type_, sizeof( synth_type_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b0_, sizeof( pink_refined_b0_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b1_, sizeof( pink_refined_b1_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b2_, sizeof( pink_refined_b2_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b3_, sizeof( pink_refined_b3_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b4_, sizeof( pink_refined_b4_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b5_, sizeof( pink_refined_b5_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &pink_refined_b6_, sizeof( pink_refined_b6_ ) ) ) && ret;
  ret = ( ( -1 ) != stream->write( stream, &red_leaky_integrator_prev_, sizeof( red_leaky_integrator_prev_ ) ) ) && ret;
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::state_load( clap_istream_t const* stream ) {
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );
  bool ret = true;
  ret = ( 0 < stream->read( stream, &synth_type_, sizeof( synth_type_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b0_, sizeof( pink_refined_b0_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b1_, sizeof( pink_refined_b1_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b2_, sizeof( pink_refined_b2_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b3_, sizeof( pink_refined_b3_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b4_, sizeof( pink_refined_b4_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b5_, sizeof( pink_refined_b5_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &pink_refined_b6_, sizeof( pink_refined_b6_ ) ) ) && ret;
  ret = ( 0 < stream->read( stream, &red_leaky_integrator_prev_, sizeof( red_leaky_integrator_prev_ ) ) ) && ret;
  if( synth_type_ >= SynthType::MAX_VALUE )
    synth_type_ = static_cast< SynthType >( static_cast< uint16_t >( SynthType::MAX_VALUE ) - 1 );
  SFG_LOG_TRACE( logger_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

#pragma endregion

#pragma region CLAP extensions, wether or not to pointer things to clap

bool NoiseGenerator::supports_audio_ports() const {
  return true;
}

bool NoiseGenerator::supports_note_ports() const {
  return true;
}

bool NoiseGenerator::supports_params() const {
  return true;
}

bool NoiseGenerator::supports_state() const {
  return true;
}

#pragma endregion

#pragma region shit for the factory

clap_plugin_t* NoiseGenerator::s_create( clap_host_t const* host ) {
  clap_plugin_t* plugin = new clap_plugin_t();
  plugin->desc = descriptor_get();
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
  plugin->plugin_data = new NoiseGenerator();
  static_cast< NoiseGenerator* >( plugin->plugin_data )->host_ = host;
  return plugin;
};

clap_plugin_descriptor_t* NoiseGenerator::descriptor_get( void ) {
  static clap_plugin_descriptor_t clap_descriptor_;
  static std::vector< char const* > clap_descriptor_features_;

  static std::string const c_plugin_id_ = "com.SFGrenade.NoiseGenerator";
  static std::string const c_plugin_name_ = "SFG-NoiseGenerator";
  static std::string const c_plugin_vendor_ = "SFGrenade";
  static std::string const c_plugin_url_ = "https://sfgrena.de";
  static std::string const c_plugin_manual_url_ = "https://sfgrena.de";
  static std::string const c_plugin_support_url_ = "https://sfgrena.de";
  static std::string const c_plugin_version_ = "0.0.1";
  static std::string const c_plugin_description_ = "Awesome NoiseGenerator. By SFGrenade.";
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
