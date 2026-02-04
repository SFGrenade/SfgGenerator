// Header assigned to this source
#include "plugin/NoiseGenerator.hpp"

// Project includes
#include <common/math.hpp>

// C++ std includes
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <exception>
#include <functional>
#include <numbers>

namespace SfPb = SfgGenerator::Proto;

NoiseGenerator::NoiseGenerator() : _base_() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

NoiseGenerator::~NoiseGenerator() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string NoiseGenerator::get_name( void ) const {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "NoiseGenerator";
}

#pragma region wave generators

namespace SFG_PRIVATE {
double sine_wave_StdSin( double phase, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return std::sin( phase * 2.0 * std::numbers::pi_v< double > ) * mix;
}
double sine_wave_CSin( double phase, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return sin( phase * 2.0 * std::numbers::pi_v< double > ) * mix;
}
double square_wave_PhaseWidth( double phase, double pwm, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return ( phase < pwm ) ? ( mix ) : ( -mix );
}
double square_wave_InversePhaseWidth( double phase, double pwm, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return ( phase < pwm ) ? ( -mix ) : ( mix );
}
double saw_wave_Phase( double phase, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return ( ( 2.0 * phase ) - 1.0 ) * mix;
}
double saw_wave_InversePhase( double phase, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return ( ( 2.0 * ( 1.0 - phase ) ) - 1.0 ) * mix;
}
double triangle_wave_ChunkLerp( double phase, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  if( phase < 0.25 ) {
    return std::lerp( 0.0, 1.0, ( phase - 0.0 ) * 4.0 ) * mix;
  }
  if( phase < 0.75 ) {
    return std::lerp( 1.0, -1.0, ( phase - 0.25 ) * 2.0 ) * mix;
  }
  return std::lerp( -1.0, 0.0, ( phase - 0.75 ) * 4.0 ) * mix;
}
double white_noise_StdRandom( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return dist( eng ) * mix;
}
double white_noise_RandMaxRand( double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  return ( ( 2.0 * static_cast< double >( rand() ) / static_cast< double >( RAND_MAX ) ) - 1.0 ) * mix;
}
double pink_noise_PaulKellettRefined( std::uniform_real_distribution< double >& dist,
                                      std::mt19937_64& eng,
                                      double& b0,
                                      double& b1,
                                      double& b2,
                                      double& b3,
                                      double& b4,
                                      double& b5,
                                      double& b6,
                                      double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  double white = dist( eng );
  b0 = 0.99886 * b0 + white * 0.0555179;
  b1 = 0.99332 * b1 + white * 0.0750759;
  b2 = 0.96900 * b2 + white * 0.1538520;
  b3 = 0.86650 * b3 + white * 0.3104856;
  b4 = 0.55000 * b4 + white * 0.5329522;
  b5 = -0.7616 * b5 - white * 0.0168980;
  double pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
  b6 = white * 0.115926;
  return std::clamp( pink * 0.11, -1.0, 1.0 ) * mix;  // empirically estimated
}
double pink_noise_PaulKellettEconomy( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double& b0, double& b1, double& b2, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  double white = dist( eng );
  b0 = 0.99765 * b0 + white * 0.0990460;
  b1 = 0.96300 * b1 + white * 0.2965164;
  b2 = 0.57000 * b2 + white * 1.0526913;
  double pink = b0 + b1 + b2 + white * 0.1848;
  return std::clamp( pink * 0.05, -1.0, 1.0 ) * mix;  // empirically estimated
}
double pink_noise_VossMcCartney( std::uniform_real_distribution< double >& dist,
                                 std::mt19937_64& eng,
                                 uint64_t& sampleIndex,
                                 std::vector< double >& streams,
                                 double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  double sum = 0.0;
  for( uint64_t i = 0; i < streams.size(); i++ ) {
    if( ( sampleIndex % sfg_upow( 2ull, i ) ) == 0 ) {
      streams[i] = dist( eng );
    }
    sum += streams[i];
  }
  sampleIndex++;
  return sum / static_cast< double >( streams.size() );
}
double pink_noise_IirFilterApproximation( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_BasicIntegration( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_LeakyIntegration( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_IntegerWalk( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_OnePoleIirFilter( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_CumulativeWithClamp( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_VoidAndCluster( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_PoissonDiskSampling( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_SimpleSpectralShaping( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_R2JitteredSampling( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_PermutedGradientNoise( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double violet_noise_FirstOrderDifference( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double violet_noise_FirstOrderIirFilter( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_PsychoacousticFilter( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_AweightingInversion( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_MultiBandpass( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_EqualLoudnessApproximation( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double velvet_noise_SporadicImpulse( std::uniform_real_distribution< double >& dist, std::mt19937_64& eng, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
}  // namespace SFG_PRIVATE

double NoiseGenerator::get_sample_sine_wave( double phase ) {
  switch( state_.synth_sine_wave_type() ) {
    case _pb_::SineWaveType::NoiseGenerator_SineWaveType_StdSin:
      return SFG_PRIVATE::sine_wave_StdSin( phase, state_.synth_sine_wave_mix() );
    case _pb_::SineWaveType::NoiseGenerator_SineWaveType_CSin:
      return SFG_PRIVATE::sine_wave_CSin( phase, state_.synth_sine_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_square_wave( double phase ) {
  switch( state_.synth_square_wave_type() ) {
    case _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_PhaseWidth:
      return SFG_PRIVATE::square_wave_PhaseWidth( phase, state_.synth_square_wave_pwm(), state_.synth_square_wave_mix() );
    case _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_InversePhaseWidth:
      return SFG_PRIVATE::square_wave_InversePhaseWidth( phase, state_.synth_square_wave_pwm(), state_.synth_square_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_saw_wave( double phase ) {
  switch( state_.synth_saw_wave_type() ) {
    case _pb_::SawWaveType::NoiseGenerator_SawWaveType_Phase:
      return SFG_PRIVATE::saw_wave_Phase( phase, state_.synth_saw_wave_mix() );
    case _pb_::SawWaveType::NoiseGenerator_SawWaveType_InversePhase:
      return SFG_PRIVATE::saw_wave_InversePhase( phase, state_.synth_saw_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_triangle_wave( double phase ) {
  switch( state_.synth_triangle_wave_type() ) {
    case _pb_::TriangleWaveType::NoiseGenerator_TriangleWaveType_ChunkLerp:
      return SFG_PRIVATE::triangle_wave_ChunkLerp( phase, state_.synth_triangle_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_white_noise( double phase ) {
  switch( state_.synth_white_noise_type() ) {
    case _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_StdRandom:
      return SFG_PRIVATE::white_noise_StdRandom( dist_, eng_, state_.synth_white_noise_mix() );
    case _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_RandMaxRand:
      return SFG_PRIVATE::white_noise_RandMaxRand( state_.synth_white_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_pink_noise( double phase ) {
  switch( state_.synth_pink_noise_type() ) {
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_PaulKellettRefined:
      return SFG_PRIVATE::pink_noise_PaulKellettRefined( dist_,
                                                         eng_,
                                                         pink_refined_b0_,
                                                         pink_refined_b1_,
                                                         pink_refined_b2_,
                                                         pink_refined_b3_,
                                                         pink_refined_b4_,
                                                         pink_refined_b5_,
                                                         pink_refined_b6_,
                                                         state_.synth_pink_noise_mix() );
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_PaulKellettEconomy:
      return SFG_PRIVATE::pink_noise_PaulKellettEconomy( dist_, eng_, pink_economy_b0_, pink_economy_b1_, pink_economy_b2_, state_.synth_pink_noise_mix() );
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_VossMcCartney:
      return SFG_PRIVATE::pink_noise_VossMcCartney( dist_, eng_, pink_VossMcCartney_sample_, pink_VossMcCartney_streams_, state_.synth_pink_noise_mix() );
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_IirFilterApproximation:
      return SFG_PRIVATE::pink_noise_IirFilterApproximation( dist_, eng_, state_.synth_pink_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_red_noise( double phase ) {
  switch( state_.synth_red_noise_type() ) {
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_BasicIntegration:
      return SFG_PRIVATE::red_noise_BasicIntegration( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_LeakyIntegration:
      return SFG_PRIVATE::red_noise_LeakyIntegration( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_IntegerWalk:
      return SFG_PRIVATE::red_noise_IntegerWalk( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_OnePoleIirFilter:
      return SFG_PRIVATE::red_noise_OnePoleIirFilter( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_CumulativeWithClamp:
      return SFG_PRIVATE::red_noise_CumulativeWithClamp( dist_, eng_, state_.synth_red_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_blue_noise( double phase ) {
  switch( state_.synth_blue_noise_type() ) {
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_VoidAndCluster:
      return SFG_PRIVATE::blue_noise_VoidAndCluster( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_PoissonDiskSampling:
      return SFG_PRIVATE::blue_noise_PoissonDiskSampling( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_SimpleSpectralShaping:
      return SFG_PRIVATE::blue_noise_SimpleSpectralShaping( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_R2JitteredSampling:
      return SFG_PRIVATE::blue_noise_R2JitteredSampling( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_PermutedGradientNoise:
      return SFG_PRIVATE::blue_noise_PermutedGradientNoise( dist_, eng_, state_.synth_blue_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_violet_noise( double phase ) {
  switch( state_.synth_violet_noise_type() ) {
    case _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderDifference:
      return SFG_PRIVATE::violet_noise_FirstOrderDifference( dist_, eng_, state_.synth_violet_noise_mix() );
    case _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderIirFilter:
      return SFG_PRIVATE::violet_noise_FirstOrderIirFilter( dist_, eng_, state_.synth_violet_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_grey_noise( double phase ) {
  switch( state_.synth_grey_noise_type() ) {
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_PsychoacousticFilter:
      return SFG_PRIVATE::grey_noise_PsychoacousticFilter( dist_, eng_, state_.synth_grey_noise_mix() );
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_AweightingInversion:
      return SFG_PRIVATE::grey_noise_AweightingInversion( dist_, eng_, state_.synth_grey_noise_mix() );
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_MultiBandpass:
      return SFG_PRIVATE::grey_noise_MultiBandpass( dist_, eng_, state_.synth_grey_noise_mix() );
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_EqualLoudnessApproximation:
      return SFG_PRIVATE::grey_noise_EqualLoudnessApproximation( dist_, eng_, state_.synth_grey_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_velvet_noise( double phase ) {
  switch( state_.synth_velvet_noise_type() ) {
    case _pb_::VelvetNoiseType::NoiseGenerator_VelvetNoiseType_SporadicImpulse:
      return SFG_PRIVATE::velvet_noise_SporadicImpulse( dist_, eng_, state_.synth_velvet_noise_mix() );
    default:
      return 0.0;
  }
}

#pragma endregion

#pragma region shit to override

bool NoiseGenerator::init( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  logger_ = logger_->clone( "NoiseGenerator" );
  uiNgHolder_.set_logger( logger_->clone( "UiNgHolder" ) );

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );

  state_.Clear();
  pink_refined_b0_ = 0.0;
  pink_refined_b1_ = 0.0;
  pink_refined_b2_ = 0.0;
  pink_refined_b3_ = 0.0;
  pink_refined_b4_ = 0.0;
  pink_refined_b5_ = 0.0;
  pink_refined_b6_ = 0.0;
  pink_economy_b0_ = 0.0;
  pink_economy_b1_ = 0.0;
  pink_economy_b2_ = 0.0;
  red_leaky_integrator_prev_ = 0.0;
  note_map_.clear();

  ret = ret && true;
  return ret;
}

void NoiseGenerator::on_main_thread( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::on_main_thread();

  // synchronization of values needed:
  // 1. gui does `clap_host_->request_callback( clap_host_ );`
  // 2. this method does `clap_host_params_->rescan( clap_host_, CLAP_PARAM_RESCAN_VALUES );`
  host_params_->rescan( host_, CLAP_PARAM_RESCAN_VALUES );
}

void NoiseGenerator::reset( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );
  state_.Clear();
  pink_refined_b0_ = 0.0;
  pink_refined_b1_ = 0.0;
  pink_refined_b2_ = 0.0;
  pink_refined_b3_ = 0.0;
  pink_refined_b4_ = 0.0;
  pink_refined_b5_ = 0.0;
  pink_refined_b6_ = 0.0;
  pink_economy_b0_ = 0.0;
  pink_economy_b1_ = 0.0;
  pink_economy_b2_ = 0.0;
  red_leaky_integrator_prev_ = 0.0;
  note_map_.clear();
}

void NoiseGenerator::process_event( clap_event_header_t const* hdr, clap_output_events_t const* out_events ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( hdr={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( hdr ) );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->size    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->size );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->time    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->time );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->space_id={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->space_id );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->type    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->type );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->flags   ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->flags );
  if( hdr->space_id == CLAP_CORE_EVENT_SPACE_ID ) {
    switch( hdr->type ) {
      case CLAP_EVENT_NOTE_ON: {
        clap_event_note_t const* ev = reinterpret_cast< clap_event_note_t const* >( hdr );
        // SFG_LOG_TRACE( host_, host_log_,
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
        // SFG_LOG_TRACE( host_, host_log_,
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
        // SFG_LOG_TRACE( host_, host_log_,
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
          state_.set_synth_sine_wave_type( static_cast< _pb_::SineWaveType >( ev->value ) );
        } else if( ev->param_id == 2 ) {
          state_.set_synth_sine_wave_mix( ev->value );
        } else if( ev->param_id == 3 ) {
          state_.set_synth_square_wave_type( static_cast< _pb_::SquareWaveType >( ev->value ) );
        } else if( ev->param_id == 4 ) {
          state_.set_synth_square_wave_pwm( ev->value );
        } else if( ev->param_id == 5 ) {
          state_.set_synth_square_wave_mix( ev->value );
        } else if( ev->param_id == 6 ) {
          state_.set_synth_saw_wave_type( static_cast< _pb_::SawWaveType >( ev->value ) );
        } else if( ev->param_id == 7 ) {
          state_.set_synth_saw_wave_mix( ev->value );
        } else if( ev->param_id == 8 ) {
          state_.set_synth_triangle_wave_type( static_cast< _pb_::TriangleWaveType >( ev->value ) );
        } else if( ev->param_id == 9 ) {
          state_.set_synth_triangle_wave_mix( ev->value );
        } else if( ev->param_id == 10 ) {
          state_.set_synth_white_noise_type( static_cast< _pb_::WhiteNoiseType >( ev->value ) );
        } else if( ev->param_id == 11 ) {
          state_.set_synth_white_noise_mix( ev->value );
        } else if( ev->param_id == 12 ) {
          state_.set_synth_pink_noise_type( static_cast< _pb_::PinkNoiseType >( ev->value ) );
        } else if( ev->param_id == 24 ) {
          state_.set_synth_pink_noise_vossmccartney_number( ev->value );
          // std::vector< double >( state_.synth_pink_noise_vossmccartney_number(), 0.0 ).swap( pink_VossMcCartney_streams_ );
        } else if( ev->param_id == 13 ) {
          state_.set_synth_pink_noise_mix( ev->value );
        } else if( ev->param_id == 14 ) {
          state_.set_synth_red_noise_type( static_cast< _pb_::RedNoiseType >( ev->value ) );
        } else if( ev->param_id == 15 ) {
          state_.set_synth_red_noise_mix( ev->value );
        } else if( ev->param_id == 16 ) {
          state_.set_synth_blue_noise_type( static_cast< _pb_::BlueNoiseType >( ev->value ) );
        } else if( ev->param_id == 17 ) {
          state_.set_synth_blue_noise_mix( ev->value );
        } else if( ev->param_id == 18 ) {
          state_.set_synth_violet_noise_type( static_cast< _pb_::VioletNoiseType >( ev->value ) );
        } else if( ev->param_id == 19 ) {
          state_.set_synth_violet_noise_mix( ev->value );
        } else if( ev->param_id == 20 ) {
          state_.set_synth_grey_noise_type( static_cast< _pb_::GreyNoiseType >( ev->value ) );
        } else if( ev->param_id == 21 ) {
          state_.set_synth_grey_noise_mix( ev->value );
        } else if( ev->param_id == 22 ) {
          state_.set_synth_velvet_noise_type( static_cast< _pb_::VelvetNoiseType >( ev->value ) );
        } else if( ev->param_id == 23 ) {
          state_.set_synth_velvet_noise_mix( ev->value );
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

clap_process_status NoiseGenerator::process( clap_process_t const* process ) {
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
      for( uint32_t c = 0; c < process->audio_outputs[0].channel_count; c++ ) {
        double out = 0.0;
        if( active_ && process_ ) {
          for( auto& item : note_map_ ) {
            // A4 (note id 69) is 440.0 Hz
            double freq = 440.0 * std::pow( 2.0, ( item.first - 69 ) / 12.0 );

            double sample = 0.0;
            // phase is 0.0 .. 1.0
            sample += get_sample_sine_wave( item.second.phase );
            sample += get_sample_square_wave( item.second.phase );
            sample += get_sample_saw_wave( item.second.phase );
            sample += get_sample_triangle_wave( item.second.phase );
            sample += get_sample_white_noise( item.second.phase );
            sample += get_sample_pink_noise( item.second.phase );
            sample += get_sample_red_noise( item.second.phase );
            sample += get_sample_blue_noise( item.second.phase );
            sample += get_sample_violet_noise( item.second.phase );
            sample += get_sample_grey_noise( item.second.phase );
            sample += get_sample_velvet_noise( item.second.phase );

            item.second.phase += freq / sample_rate_;
            if( item.second.phase >= 1.0 )
              item.second.phase -= 1.0;

            out += sample * item.second.velocity;
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
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 0;
  }
  return 1;
}

bool NoiseGenerator::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
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

bool NoiseGenerator::gui_is_api_supported( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  bool ret = _base_::gui_is_api_supported( api, is_floating );
  ret = ret || true;
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_api={:?}, out_is_floating={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 out_api,
                 static_cast< void* >( out_is_floating ) );
  bool ret = _base_::gui_get_preferred_api( out_api, out_is_floating );
  ret = ret && false;
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_create( std::string const& api, bool is_floating ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  bool ret = _base_::gui_create( api, is_floating );
  ret = ret || uiNgHolder_.clap_create( api, is_floating );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

void NoiseGenerator::gui_destroy( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  uiNgHolder_.clap_destroy();
  _base_::gui_destroy();
}

bool NoiseGenerator::gui_set_scale( double scale ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( scale={:f} )", __FUNCTION__, static_cast< void* >( this ), scale );
  bool ret = _base_::gui_set_scale( scale );
  ret = ret || uiNgHolder_.clap_set_scale( scale );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_get_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void* >( out_width ),
                 static_cast< void* >( out_height ) );
  bool ret = _base_::gui_get_size( out_width, out_height );
  ret = ret || uiNgHolder_.clap_get_size( out_width, out_height );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_can_resize( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::gui_can_resize();
  ret = ret || uiNgHolder_.clap_can_resize();
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_hints={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 __FUNCTION__,
                 static_cast< void* >( out_hints ) );
  bool ret = _base_::gui_get_resize_hints( out_hints );
  ret = ret || uiNgHolder_.clap_get_resize_hints( out_hints );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( out_width={:d}, out_height={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 *out_width,
                 *out_height );
  bool ret = _base_::gui_adjust_size( out_width, out_height );
  ret = ret || uiNgHolder_.clap_adjust_size( out_width, out_height );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_set_size( uint32_t width, uint32_t height ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( width={:d}, height={:d} )", __FUNCTION__, static_cast< void* >( this ), width, height );
  bool ret = _base_::gui_set_size( width, height );
  ret = ret || uiNgHolder_.clap_set_size( width, height );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_set_parent( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  bool ret = _base_::gui_set_parent( window );
  ret = ret || uiNgHolder_.clap_set_parent( window );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_set_transient( clap_window_t const* window ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  bool ret = _base_::gui_set_transient( window );
  ret = ret || uiNgHolder_.clap_set_transient( window );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

void NoiseGenerator::gui_suggest_title( std::string const& title ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( title={:?} )", __FUNCTION__, static_cast< void* >( this ), title );
  _base_::gui_suggest_title( title );
  uiNgHolder_.clap_suggest_title( title );
}

bool NoiseGenerator::gui_show( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::gui_show();
  ret = ret || uiNgHolder_.clap_show();
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::gui_hide( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::gui_hide();
  ret = ret || uiNgHolder_.clap_hide();
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

uint32_t NoiseGenerator::note_ports_count( bool is_input ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 1;
  }
  return 0;
}

bool NoiseGenerator::note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info ) {
  SFG_LOG_TRACE( host_,
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
    out_info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI_MPE | CLAP_NOTE_DIALECT_MIDI2;
    out_info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
    return true;
  }
  return false;
}

uint32_t NoiseGenerator::params_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
  // adjust according to NoiseGenerator.proto
  // while we could make it dynamic, without explicit gui i'd rather not
  return 24;
}

bool NoiseGenerator::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
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
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Sine Wave" );
      out_param_info->min_value = static_cast< double >( _pb_::SineWaveType::NoiseGenerator_SineWaveType_StdSin );
      out_param_info->max_value = static_cast< double >( _pb_::SineWaveType::NoiseGenerator_SineWaveType_CSin );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 1:
      out_param_info->id = 2;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Sine Wave" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 2:
      out_param_info->id = 3;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Square Wave" );
      out_param_info->min_value = static_cast< double >( _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_PhaseWidth );
      out_param_info->max_value = static_cast< double >( _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_InversePhaseWidth );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 3:
      out_param_info->id = 4;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "PWM" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Square Wave" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 4:
      out_param_info->id = 5;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Square Wave" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 5:
      out_param_info->id = 6;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Saw Wave" );
      out_param_info->min_value = static_cast< double >( _pb_::SawWaveType::NoiseGenerator_SawWaveType_Phase );
      out_param_info->max_value = static_cast< double >( _pb_::SawWaveType::NoiseGenerator_SawWaveType_InversePhase );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 6:
      out_param_info->id = 7;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Saw Wave" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 7:
      out_param_info->id = 8;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Triangle Wave" );
      out_param_info->min_value = static_cast< double >( _pb_::TriangleWaveType::NoiseGenerator_TriangleWaveType_ChunkLerp );
      out_param_info->max_value = static_cast< double >( _pb_::TriangleWaveType::NoiseGenerator_TriangleWaveType_ChunkLerp );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 8:
      out_param_info->id = 9;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Triangle Wave" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 9:
      out_param_info->id = 10;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "White Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_StdRandom );
      out_param_info->max_value = static_cast< double >( _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_RandMaxRand );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 10:
      out_param_info->id = 11;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "White Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 11:
      out_param_info->id = 12;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Pink Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_PaulKellettRefined );
      out_param_info->max_value = static_cast< double >( _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_IirFilterApproximation );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 23:
      out_param_info->id = 24;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "VossMcCartney Number" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Pink Noise" );
      out_param_info->min_value = 4;   // todo: fixme: no clue about these tbh
      out_param_info->max_value = 64;  // todo: fixme: no clue about these tbh
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 12:
      out_param_info->id = 13;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Pink Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 13:
      out_param_info->id = 14;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Red Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_BasicIntegration );
      out_param_info->max_value = static_cast< double >( _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_CumulativeWithClamp );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 14:
      out_param_info->id = 15;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Red Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 15:
      out_param_info->id = 16;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Blue Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_VoidAndCluster );
      out_param_info->max_value = static_cast< double >( _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_PermutedGradientNoise );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 16:
      out_param_info->id = 17;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Blue Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 17:
      out_param_info->id = 18;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Violet Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderDifference );
      out_param_info->max_value = static_cast< double >( _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderIirFilter );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 18:
      out_param_info->id = 19;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Violet Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 19:
      out_param_info->id = 20;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Grey Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_PsychoacousticFilter );
      out_param_info->max_value = static_cast< double >( _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_EqualLoudnessApproximation );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 20:
      out_param_info->id = 21;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Grey Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 21:
      out_param_info->id = 22;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Velvet Noise" );
      out_param_info->min_value = static_cast< double >( _pb_::VelvetNoiseType::NoiseGenerator_VelvetNoiseType_SporadicImpulse );
      out_param_info->max_value = static_cast< double >( _pb_::VelvetNoiseType::NoiseGenerator_VelvetNoiseType_SporadicImpulse );
      out_param_info->default_value = out_param_info->min_value;
      break;
    case 22:
      out_param_info->id = 23;
      out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Mix" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Velvet Noise" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
  }
  return true;
}

bool NoiseGenerator::params_get_value( clap_id param_id, double* out_value ) {
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
    ( *out_value ) = state_.synth_sine_wave_type();
    return true;
  } else if( param_id == 2 ) {
    ( *out_value ) = state_.synth_sine_wave_mix();
    return true;
  } else if( param_id == 3 ) {
    ( *out_value ) = state_.synth_square_wave_type();
    return true;
  } else if( param_id == 4 ) {
    ( *out_value ) = state_.synth_square_wave_pwm();
    return true;
  } else if( param_id == 5 ) {
    ( *out_value ) = state_.synth_square_wave_mix();
    return true;
  } else if( param_id == 6 ) {
    ( *out_value ) = state_.synth_saw_wave_type();
    return true;
  } else if( param_id == 7 ) {
    ( *out_value ) = state_.synth_saw_wave_mix();
    return true;
  } else if( param_id == 8 ) {
    ( *out_value ) = state_.synth_triangle_wave_type();
    return true;
  } else if( param_id == 9 ) {
    ( *out_value ) = state_.synth_triangle_wave_mix();
    return true;
  } else if( param_id == 10 ) {
    ( *out_value ) = state_.synth_white_noise_type();
    return true;
  } else if( param_id == 11 ) {
    ( *out_value ) = state_.synth_white_noise_mix();
    return true;
  } else if( param_id == 12 ) {
    ( *out_value ) = state_.synth_pink_noise_type();
    return true;
  } else if( param_id == 24 ) {
    ( *out_value ) = state_.synth_pink_noise_vossmccartney_number();
    return true;
  } else if( param_id == 13 ) {
    ( *out_value ) = state_.synth_pink_noise_mix();
    return true;
  } else if( param_id == 14 ) {
    ( *out_value ) = state_.synth_red_noise_type();
    return true;
  } else if( param_id == 15 ) {
    ( *out_value ) = state_.synth_red_noise_mix();
    return true;
  } else if( param_id == 16 ) {
    ( *out_value ) = state_.synth_blue_noise_type();
    return true;
  } else if( param_id == 17 ) {
    ( *out_value ) = state_.synth_blue_noise_mix();
    return true;
  } else if( param_id == 18 ) {
    ( *out_value ) = state_.synth_violet_noise_type();
    return true;
  } else if( param_id == 19 ) {
    ( *out_value ) = state_.synth_violet_noise_mix();
    return true;
  } else if( param_id == 20 ) {
    ( *out_value ) = state_.synth_grey_noise_type();
    return true;
  } else if( param_id == 21 ) {
    ( *out_value ) = state_.synth_grey_noise_mix();
    return true;
  } else if( param_id == 22 ) {
    ( *out_value ) = state_.synth_velvet_noise_type();
    return true;
  } else if( param_id == 23 ) {
    ( *out_value ) = state_.synth_velvet_noise_mix();
    return true;
  }
  return false;
}

bool NoiseGenerator::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
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
    std::string tmp_str = _pb_::SineWaveType_Name( static_cast< _pb_::SineWaveType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 2 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 3 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::SquareWaveType_Name( static_cast< _pb_::SquareWaveType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 4 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 5 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 6 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::SawWaveType_Name( static_cast< _pb_::SawWaveType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 7 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 8 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::TriangleWaveType_Name( static_cast< _pb_::TriangleWaveType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 9 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 10 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::WhiteNoiseType_Name( static_cast< _pb_::WhiteNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 11 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 12 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::PinkNoiseType_Name( static_cast< _pb_::PinkNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 24 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 13 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 14 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::RedNoiseType_Name( static_cast< _pb_::RedNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 15 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 16 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::BlueNoiseType_Name( static_cast< _pb_::BlueNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 17 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 18 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::VioletNoiseType_Name( static_cast< _pb_::VioletNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 19 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 20 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::GreyNoiseType_Name( static_cast< _pb_::GreyNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 21 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 22 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = _pb_::VelvetNoiseType_Name( static_cast< _pb_::VelvetNoiseType >( value ) );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  } else if( param_id == 23 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  }
  return false;
}

bool NoiseGenerator::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
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
    _pb_::SineWaveType out;
    bool ret = _pb_::SineWaveType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 2 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 3 ) {
    _pb_::SquareWaveType out;
    bool ret = _pb_::SquareWaveType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 4 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 5 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 6 ) {
    _pb_::SawWaveType out;
    bool ret = _pb_::SawWaveType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 7 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 8 ) {
    _pb_::TriangleWaveType out;
    bool ret = _pb_::TriangleWaveType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 9 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 10 ) {
    _pb_::WhiteNoiseType out;
    bool ret = _pb_::WhiteNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 11 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 12 ) {
    _pb_::PinkNoiseType out;
    bool ret = _pb_::PinkNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 24 ) {
    // todo: fixme: maybe some text to int??
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 13 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 14 ) {
    _pb_::RedNoiseType out;
    bool ret = _pb_::RedNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 15 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 16 ) {
    _pb_::BlueNoiseType out;
    bool ret = _pb_::BlueNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 17 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 18 ) {
    _pb_::VioletNoiseType out;
    bool ret = _pb_::VioletNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 19 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 20 ) {
    _pb_::GreyNoiseType out;
    bool ret = _pb_::GreyNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 21 ) {
    return text_to_double( param_value_text, out_value );
  } else if( param_id == 22 ) {
    _pb_::VelvetNoiseType out;
    bool ret = _pb_::VelvetNoiseType_Parse( param_value_text, &out );
    if( ret )
      ( *out_value ) = static_cast< double >( out );
    return ret;
  } else if( param_id == 23 ) {
    return text_to_double( param_value_text, out_value );
  }
  return false;
}

void NoiseGenerator::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
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

bool NoiseGenerator::state_save( clap_ostream_t const* stream ) {
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

bool NoiseGenerator::state_load( clap_istream_t const* stream ) {
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

bool NoiseGenerator::supports_audio_ports() const {
  return true;
}

bool NoiseGenerator::supports_gui() const {
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
  NoiseGenerator* noise_gen = new NoiseGenerator();
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
