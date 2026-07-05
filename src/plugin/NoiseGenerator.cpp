// Header assigned to this source
#include "plugin/NoiseGenerator.hpp"

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

// todo: fixme: adjust to NoiseGenerator::*Type
std::array< std::string, NoiseGenerator::_pb_::SineWaveType_ARRAYSIZE > const NoiseGenerator::SINEWAVE_OPTIONS{ "StdSin", "CSin" };
std::array< std::string, NoiseGenerator::_pb_::SquareWaveType_ARRAYSIZE > const NoiseGenerator::SQUAREWAVE_OPTIONS{ "PhaseWidth", "InversePhaseWidth" };
std::array< std::string, NoiseGenerator::_pb_::SawWaveType_ARRAYSIZE > const NoiseGenerator::SAWWAVE_OPTIONS{ "Phase", "InversePhase" };
std::array< std::string, NoiseGenerator::_pb_::TriangleWaveType_ARRAYSIZE > const NoiseGenerator::TRIANGLEWAVE_OPTIONS{ "ChunkLerp" };
std::array< std::string, NoiseGenerator::_pb_::WhiteNoiseType_ARRAYSIZE > const NoiseGenerator::WHITENOISE_OPTIONS{ "StdRandom", "RandMaxRand" };
std::array< std::string, NoiseGenerator::_pb_::PinkNoiseType_ARRAYSIZE > const NoiseGenerator::PINKNOISE_OPTIONS{ "PaulKellettRefined",
                                                                                                                  "PaulKellettEconomy",
                                                                                                                  "VossMcCartney",
                                                                                                                  "IirFilterApproximation" };
std::array< std::string, NoiseGenerator::_pb_::RedNoiseType_ARRAYSIZE > const NoiseGenerator::REDNOISE_OPTIONS{ "BasicIntegration",
                                                                                                                "LeakyIntegration",
                                                                                                                "IntegerWalk",
                                                                                                                "OnePoleIirFilter",
                                                                                                                "CumulativeWithClamp" };
std::array< std::string, NoiseGenerator::_pb_::BlueNoiseType_ARRAYSIZE > const NoiseGenerator::BLUENOISE_OPTIONS{ "VoidAndCluster",
                                                                                                                  "PoissonDiskSampling",
                                                                                                                  "SimpleSpectralShaping",
                                                                                                                  "R2JitteredSampling",
                                                                                                                  "PermutedGradientNoise" };
std::array< std::string, NoiseGenerator::_pb_::VioletNoiseType_ARRAYSIZE > const NoiseGenerator::VIOLETNOISE_OPTIONS{ "FirstOrderDifference",
                                                                                                                      "FirstOrderIirFilter" };
std::array< std::string, NoiseGenerator::_pb_::GreyNoiseType_ARRAYSIZE > const NoiseGenerator::GREYNOISE_OPTIONS{ "PsychoacousticFilter",
                                                                                                                  "AweightingInversion",
                                                                                                                  "MultiBandpass",
                                                                                                                  "EqualLoudnessApproximation" };
std::array< std::string, NoiseGenerator::_pb_::VelvetNoiseType_ARRAYSIZE > const NoiseGenerator::VELVETNOISE_OPTIONS{ "SporadicImpulse" };

NoiseGenerator::NoiseGenerator()
    : _base_(),
      guiWindow_( nullptr,
                  [this]( SDL_Window* ptr ) {
                    WRAP_SDL_CALL_INST( SDL_HideWindow, ptr );
                    SDL_DestroyWindow( ptr );
                  } ),
      sampleQueueSineWave_( 4096 ),
      sampleQueueSquareWave_( 4096 ),
      sampleQueueSawWave_( 4096 ),
      sampleQueueTriangleWave_( 4096 ),
      sampleQueueWhiteNoise_( 4096 ),
      sampleQueuePinkNoise_( 4096 ),
      sampleQueueRedNoise_( 4096 ),
      sampleQueueBlueNoise_( 4096 ),
      sampleQueueVioletNoise_( 4096 ),
      sampleQueueGreyNoise_( 4096 ),
      sampleQueueVelvetNoise_( 4096 ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

NoiseGenerator::~NoiseGenerator() {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string NoiseGenerator::get_name( void ) const {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "NoiseGenerator";
}

#pragma region wave generators

namespace SFG_PRIVATE_NoiseGenerator {
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
    if( ( sampleIndex % sfg_upow( 2ull, static_cast< int64_t >( i ) ) ) == 0 ) {
      streams[i] = dist( eng );
    }
    sum += streams[i];
  }
  sampleIndex++;
  return sum / static_cast< double >( streams.size() );
}
double pink_noise_IirFilterApproximation( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_BasicIntegration( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_LeakyIntegration( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_IntegerWalk( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_OnePoleIirFilter( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double red_noise_CumulativeWithClamp( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_VoidAndCluster( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_PoissonDiskSampling( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_SimpleSpectralShaping( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_R2JitteredSampling( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double blue_noise_PermutedGradientNoise( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double violet_noise_FirstOrderDifference( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double violet_noise_FirstOrderIirFilter( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_PsychoacousticFilter( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_AweightingInversion( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_MultiBandpass( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double grey_noise_EqualLoudnessApproximation( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
double velvet_noise_SporadicImpulse( std::uniform_real_distribution< double >& /*dist*/, std::mt19937_64& /*eng*/, double mix ) {
  if( mix <= 0.0 )
    return 0.0;
  // TODO: FIXME: ADD IMPLEMENTATION
  return 0.0;
}
}  // namespace SFG_PRIVATE_NoiseGenerator

double NoiseGenerator::get_sample_sine_wave( double phase ) {
  switch( state_.synth_sine_wave_type() ) {
    case _pb_::SineWaveType::NoiseGenerator_SineWaveType_StdSin:
      return SFG_PRIVATE_NoiseGenerator::sine_wave_StdSin( phase, state_.synth_sine_wave_mix() );
    case _pb_::SineWaveType::NoiseGenerator_SineWaveType_CSin:
      return SFG_PRIVATE_NoiseGenerator::sine_wave_CSin( phase, state_.synth_sine_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_square_wave( double phase ) {
  switch( state_.synth_square_wave_type() ) {
    case _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_PhaseWidth:
      return SFG_PRIVATE_NoiseGenerator::square_wave_PhaseWidth( phase, state_.synth_square_wave_pwm(), state_.synth_square_wave_mix() );
    case _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_InversePhaseWidth:
      return SFG_PRIVATE_NoiseGenerator::square_wave_InversePhaseWidth( phase, state_.synth_square_wave_pwm(), state_.synth_square_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_saw_wave( double phase ) {
  switch( state_.synth_saw_wave_type() ) {
    case _pb_::SawWaveType::NoiseGenerator_SawWaveType_Phase:
      return SFG_PRIVATE_NoiseGenerator::saw_wave_Phase( phase, state_.synth_saw_wave_mix() );
    case _pb_::SawWaveType::NoiseGenerator_SawWaveType_InversePhase:
      return SFG_PRIVATE_NoiseGenerator::saw_wave_InversePhase( phase, state_.synth_saw_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_triangle_wave( double phase ) {
  switch( state_.synth_triangle_wave_type() ) {
    case _pb_::TriangleWaveType::NoiseGenerator_TriangleWaveType_ChunkLerp:
      return SFG_PRIVATE_NoiseGenerator::triangle_wave_ChunkLerp( phase, state_.synth_triangle_wave_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_white_noise( double /*phase*/ ) {
  switch( state_.synth_white_noise_type() ) {
    case _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_StdRandom:
      return SFG_PRIVATE_NoiseGenerator::white_noise_StdRandom( dist_, eng_, state_.synth_white_noise_mix() );
    case _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_RandMaxRand:
      return SFG_PRIVATE_NoiseGenerator::white_noise_RandMaxRand( state_.synth_white_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_pink_noise( double /*phase*/ ) {
  switch( state_.synth_pink_noise_type() ) {
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_PaulKellettRefined:
      return SFG_PRIVATE_NoiseGenerator::pink_noise_PaulKellettRefined( dist_,
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
      return SFG_PRIVATE_NoiseGenerator::pink_noise_PaulKellettEconomy( dist_,
                                                                        eng_,
                                                                        pink_economy_b0_,
                                                                        pink_economy_b1_,
                                                                        pink_economy_b2_,
                                                                        state_.synth_pink_noise_mix() );
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_VossMcCartney:
      return SFG_PRIVATE_NoiseGenerator::pink_noise_VossMcCartney( dist_,
                                                                   eng_,
                                                                   pink_VossMcCartney_sample_,
                                                                   pink_VossMcCartney_streams_,
                                                                   state_.synth_pink_noise_mix() );
    case _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_IirFilterApproximation:
      return SFG_PRIVATE_NoiseGenerator::pink_noise_IirFilterApproximation( dist_, eng_, state_.synth_pink_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_red_noise( double /*phase*/ ) {
  switch( state_.synth_red_noise_type() ) {
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_BasicIntegration:
      return SFG_PRIVATE_NoiseGenerator::red_noise_BasicIntegration( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_LeakyIntegration:
      return SFG_PRIVATE_NoiseGenerator::red_noise_LeakyIntegration( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_IntegerWalk:
      return SFG_PRIVATE_NoiseGenerator::red_noise_IntegerWalk( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_OnePoleIirFilter:
      return SFG_PRIVATE_NoiseGenerator::red_noise_OnePoleIirFilter( dist_, eng_, state_.synth_red_noise_mix() );
    case _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_CumulativeWithClamp:
      return SFG_PRIVATE_NoiseGenerator::red_noise_CumulativeWithClamp( dist_, eng_, state_.synth_red_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_blue_noise( double /*phase*/ ) {
  switch( state_.synth_blue_noise_type() ) {
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_VoidAndCluster:
      return SFG_PRIVATE_NoiseGenerator::blue_noise_VoidAndCluster( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_PoissonDiskSampling:
      return SFG_PRIVATE_NoiseGenerator::blue_noise_PoissonDiskSampling( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_SimpleSpectralShaping:
      return SFG_PRIVATE_NoiseGenerator::blue_noise_SimpleSpectralShaping( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_R2JitteredSampling:
      return SFG_PRIVATE_NoiseGenerator::blue_noise_R2JitteredSampling( dist_, eng_, state_.synth_blue_noise_mix() );
    case _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_PermutedGradientNoise:
      return SFG_PRIVATE_NoiseGenerator::blue_noise_PermutedGradientNoise( dist_, eng_, state_.synth_blue_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_violet_noise( double /*phase*/ ) {
  switch( state_.synth_violet_noise_type() ) {
    case _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderDifference:
      return SFG_PRIVATE_NoiseGenerator::violet_noise_FirstOrderDifference( dist_, eng_, state_.synth_violet_noise_mix() );
    case _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderIirFilter:
      return SFG_PRIVATE_NoiseGenerator::violet_noise_FirstOrderIirFilter( dist_, eng_, state_.synth_violet_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_grey_noise( double /*phase*/ ) {
  switch( state_.synth_grey_noise_type() ) {
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_PsychoacousticFilter:
      return SFG_PRIVATE_NoiseGenerator::grey_noise_PsychoacousticFilter( dist_, eng_, state_.synth_grey_noise_mix() );
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_AweightingInversion:
      return SFG_PRIVATE_NoiseGenerator::grey_noise_AweightingInversion( dist_, eng_, state_.synth_grey_noise_mix() );
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_MultiBandpass:
      return SFG_PRIVATE_NoiseGenerator::grey_noise_MultiBandpass( dist_, eng_, state_.synth_grey_noise_mix() );
    case _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_EqualLoudnessApproximation:
      return SFG_PRIVATE_NoiseGenerator::grey_noise_EqualLoudnessApproximation( dist_, eng_, state_.synth_grey_noise_mix() );
    default:
      return 0.0;
  }
}

double NoiseGenerator::get_sample_velvet_noise( double /*phase*/ ) {
  switch( state_.synth_velvet_noise_type() ) {
    case _pb_::VelvetNoiseType::NoiseGenerator_VelvetNoiseType_SporadicImpulse:
      return SFG_PRIVATE_NoiseGenerator::velvet_noise_SporadicImpulse( dist_, eng_, state_.synth_velvet_noise_mix() );
    default:
      return 0.0;
  }
}

#pragma endregion

#pragma region shit to override

bool NoiseGenerator::init( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  logger_ = logger_->clone( "NoiseGenerator" );

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );

  state_.Clear();
  state_.set_gui_width( 300 );
  state_.set_gui_height( 200 );
  state_.set_synth_square_wave_pwm( 0.5 );
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
  noteMap_.clear();

  ret = ret && true;
  return ret;
}

bool NoiseGenerator::activate( double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count ) {
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
  noteMap_.setAdsrParameters( 0.001, 1.0, 1.0, 0.01 );

  ret = ret && true;
  return ret;
}

void NoiseGenerator::on_main_thread( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::on_main_thread();

  // synchronization of values needed:
  // 1. gui does `clap_host_->request_callback( clap_host_ );`
  // 2. this method does `clap_host_state_->mark_dirty( clap_host_ );`
  // 3. this method does `clap_host_params_->rescan( clap_host_, CLAP_PARAM_RESCAN_VALUES );`
  host_state_->mark_dirty( host_ );
  host_params_->rescan( host_, CLAP_PARAM_RESCAN_VALUES );
}

void NoiseGenerator::reset( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  eng_ = std::mt19937_64( std::random_device{}() );
  dist_ = std::uniform_real_distribution< double >( -1.0, 1.0 );
  state_.Clear();
  state_.set_gui_width( 300 );
  state_.set_gui_height( 200 );
  state_.set_synth_square_wave_pwm( 0.5 );
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
  noteMap_.clear();
}

void NoiseGenerator::process_event( clap_event_header_t const* hdr, clap_output_events_t const* /*out_events*/ ) {
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

#define SFG_GEN_TMP_CHANGE_PARAM( op, id, stateVar, cast, lastVar ) \
  op( ev->param_id == id ) {                                        \
    state_.set_##stateVar( static_cast< cast >( ev->value ) );      \
    lastVar = ev->value;                                            \
  }

    SFG_GEN_TMP_CHANGE_PARAM( if, 1010, synth_sine_wave_type, _pb_::SineWaveType, last_sineWaveType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1015, synth_sine_wave_attack, double, last_sineWaveAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1016, synth_sine_wave_decay, double, last_sineWaveDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1017, synth_sine_wave_sustain, double, last_sineWaveSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1018, synth_sine_wave_release, double, last_sineWaveRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1019, synth_sine_wave_mix, double, last_sineWaveMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1020, synth_square_wave_type, _pb_::SquareWaveType, last_squareWaveType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1021, synth_square_wave_pwm, double, last_squareWavePwm_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1025, synth_square_wave_attack, double, last_squareWaveAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1026, synth_square_wave_decay, double, last_squareWaveDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1027, synth_square_wave_sustain, double, last_squareWaveSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1028, synth_square_wave_release, double, last_squareWaveRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1029, synth_square_wave_mix, double, last_squareWaveMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1030, synth_saw_wave_type, _pb_::SawWaveType, last_sawWaveType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1035, synth_saw_wave_attack, double, last_sawWaveAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1036, synth_saw_wave_decay, double, last_sawWaveDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1037, synth_saw_wave_sustain, double, last_sawWaveSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1038, synth_saw_wave_release, double, last_sawWaveRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1039, synth_saw_wave_mix, double, last_sawWaveMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1040, synth_triangle_wave_type, _pb_::TriangleWaveType, last_triangleWaveType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1045, synth_triangle_wave_attack, double, last_triangleWaveAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1046, synth_triangle_wave_decay, double, last_triangleWaveDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1047, synth_triangle_wave_sustain, double, last_triangleWaveSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1048, synth_triangle_wave_release, double, last_triangleWaveRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1049, synth_triangle_wave_mix, double, last_triangleWaveMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1050, synth_white_noise_type, _pb_::WhiteNoiseType, last_whiteNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1055, synth_white_noise_attack, double, last_whiteNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1056, synth_white_noise_decay, double, last_whiteNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1057, synth_white_noise_sustain, double, last_whiteNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1058, synth_white_noise_release, double, last_whiteNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1059, synth_white_noise_mix, double, last_whiteNoiseMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1060, synth_pink_noise_type, _pb_::PinkNoiseType, last_pinkNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1061, synth_pink_noise_vossmccartney_number, double, last_pinkNoiseVossMcCartneyNumber_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1065, synth_pink_noise_attack, double, last_pinkNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1066, synth_pink_noise_decay, double, last_pinkNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1067, synth_pink_noise_sustain, double, last_pinkNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1068, synth_pink_noise_release, double, last_pinkNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1069, synth_pink_noise_mix, double, last_pinkNoiseMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1070, synth_red_noise_type, _pb_::RedNoiseType, last_redNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1075, synth_red_noise_attack, double, last_redNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1076, synth_red_noise_decay, double, last_redNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1077, synth_red_noise_sustain, double, last_redNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1078, synth_red_noise_release, double, last_redNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1079, synth_red_noise_mix, double, last_redNoiseMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1080, synth_blue_noise_type, _pb_::BlueNoiseType, last_blueNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1085, synth_blue_noise_attack, double, last_blueNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1086, synth_blue_noise_decay, double, last_blueNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1087, synth_blue_noise_sustain, double, last_blueNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1088, synth_blue_noise_release, double, last_blueNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1089, synth_blue_noise_mix, double, last_blueNoiseMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1090, synth_violet_noise_type, _pb_::VioletNoiseType, last_violetNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1095, synth_violet_noise_attack, double, last_violetNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1096, synth_violet_noise_decay, double, last_violetNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1097, synth_violet_noise_sustain, double, last_violetNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1098, synth_violet_noise_release, double, last_violetNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1099, synth_violet_noise_mix, double, last_violetNoiseMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1100, synth_grey_noise_type, _pb_::GreyNoiseType, last_greyNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1105, synth_grey_noise_attack, double, last_greyNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1106, synth_grey_noise_decay, double, last_greyNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1107, synth_grey_noise_sustain, double, last_greyNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1108, synth_grey_noise_release, double, last_greyNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1109, synth_grey_noise_mix, double, last_greyNoiseMix_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1110, synth_velvet_noise_type, _pb_::VelvetNoiseType, last_velvetNoiseType_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1115, synth_velvet_noise_attack, double, last_velvetNoiseAttack_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1116, synth_velvet_noise_decay, double, last_velvetNoiseDecay_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1117, synth_velvet_noise_sustain, double, last_velvetNoiseSustain_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1118, synth_velvet_noise_release, double, last_velvetNoiseRelease_ )
    SFG_GEN_TMP_CHANGE_PARAM( else if, 1119, synth_velvet_noise_mix, double, last_velvetNoiseMix_ )

    // extra changes necessary
    if( ev->param_id == 1061 ) {
      // std::vector< double >( state_.synth_pink_noise_vossmccartney_number(), 0.0 ).swap( pink_VossMcCartney_streams_ );
    }

#undef SFG_GEN_TMP_CHANGE_PARAM

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

#define SFG_GEN_TMP_CHANGE_PARAM( op, id, stateVar, cast, lastVar ) \
  op( param_id == id ) {                                            \
    state_.set_##stateVar( static_cast< cast >( value ) );          \
    lastVar = value;                                                \
  }

      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( if, 1010, synth_sine_wave_type, _pb_::SineWaveType, last_sineWaveType_ )
      SFG_GEN_TMP_CHANGE_PARAM( if, 1015, synth_sine_wave_attack, double, last_sineWaveAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1016, synth_sine_wave_decay, double, last_sineWaveDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1017, synth_sine_wave_sustain, double, last_sineWaveSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1018, synth_sine_wave_release, double, last_sineWaveRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1019, synth_sine_wave_mix, double, last_sineWaveMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1020, synth_square_wave_type, _pb_::SquareWaveType, last_squareWaveType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1021, synth_square_wave_pwm, double, last_squareWavePwm_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1025, synth_square_wave_attack, double, last_squareWaveAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1026, synth_square_wave_decay, double, last_squareWaveDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1027, synth_square_wave_sustain, double, last_squareWaveSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1028, synth_square_wave_release, double, last_squareWaveRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1029, synth_square_wave_mix, double, last_squareWaveMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1030, synth_saw_wave_type, _pb_::SawWaveType, last_sawWaveType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1035, synth_saw_wave_attack, double, last_sawWaveAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1036, synth_saw_wave_decay, double, last_sawWaveDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1037, synth_saw_wave_sustain, double, last_sawWaveSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1038, synth_saw_wave_release, double, last_sawWaveRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1039, synth_saw_wave_mix, double, last_sawWaveMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1040, synth_triangle_wave_type, _pb_::TriangleWaveType, last_triangleWaveType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1045, synth_triangle_wave_attack, double, last_triangleWaveAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1046, synth_triangle_wave_decay, double, last_triangleWaveDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1047, synth_triangle_wave_sustain, double, last_triangleWaveSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1048, synth_triangle_wave_release, double, last_triangleWaveRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1049, synth_triangle_wave_mix, double, last_triangleWaveMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1050, synth_white_noise_type, _pb_::WhiteNoiseType, last_whiteNoiseType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1055, synth_white_noise_attack, double, last_whiteNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1056, synth_white_noise_decay, double, last_whiteNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1057, synth_white_noise_sustain, double, last_whiteNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1058, synth_white_noise_release, double, last_whiteNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1059, synth_white_noise_mix, double, last_whiteNoiseMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1060, synth_pink_noise_type, _pb_::PinkNoiseType, last_pinkNoiseType_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1061, synth_pink_noise_vossmccartney_number, double, last_pinkNoiseVossMcCartneyNumber_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1065, synth_pink_noise_attack, double, last_pinkNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1066, synth_pink_noise_decay, double, last_pinkNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1067, synth_pink_noise_sustain, double, last_pinkNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1068, synth_pink_noise_release, double, last_pinkNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1069, synth_pink_noise_mix, double, last_pinkNoiseMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1070, synth_red_noise_type, _pb_::RedNoiseType, last_redNoiseType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1075, synth_red_noise_attack, double, last_redNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1076, synth_red_noise_decay, double, last_redNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1077, synth_red_noise_sustain, double, last_redNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1078, synth_red_noise_release, double, last_redNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1079, synth_red_noise_mix, double, last_redNoiseMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1080, synth_blue_noise_type, _pb_::BlueNoiseType, last_blueNoiseType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1085, synth_blue_noise_attack, double, last_blueNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1086, synth_blue_noise_decay, double, last_blueNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1087, synth_blue_noise_sustain, double, last_blueNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1088, synth_blue_noise_release, double, last_blueNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1089, synth_blue_noise_mix, double, last_blueNoiseMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1090, synth_violet_noise_type, _pb_::VioletNoiseType, last_violetNoiseType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1095, synth_violet_noise_attack, double, last_violetNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1096, synth_violet_noise_decay, double, last_violetNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1097, synth_violet_noise_sustain, double, last_violetNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1098, synth_violet_noise_release, double, last_violetNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1099, synth_violet_noise_mix, double, last_violetNoiseMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1100, synth_grey_noise_type, _pb_::GreyNoiseType, last_greyNoiseType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1105, synth_grey_noise_attack, double, last_greyNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1106, synth_grey_noise_decay, double, last_greyNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1107, synth_grey_noise_sustain, double, last_greyNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1108, synth_grey_noise_release, double, last_greyNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1109, synth_grey_noise_mix, double, last_greyNoiseMix_ )
      // todo: fixme: check if midi does weird shit
      // SFG_GEN_TMP_CHANGE_PARAM( else if, 1110, synth_velvet_noise_type, _pb_::VelvetNoiseType, last_velvetNoiseType_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1115, synth_velvet_noise_attack, double, last_velvetNoiseAttack_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1116, synth_velvet_noise_decay, double, last_velvetNoiseDecay_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1117, synth_velvet_noise_sustain, double, last_velvetNoiseSustain_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1118, synth_velvet_noise_release, double, last_velvetNoiseRelease_ )
      SFG_GEN_TMP_CHANGE_PARAM( else if, 1119, synth_velvet_noise_mix, double, last_velvetNoiseMix_ )

      // extra changes necessary
      if( param_id == 1061 ) {
        // todo: fixme: check if midi does weird shit
        // clap_param_info_t tmp;
        // // todo, check which index
        // params_get_info( 23, &tmp );
        // double actualValue = tmp.min_value + ( ( tmp.max_value - tmp.min_value ) * value );
        // state_.set_synth_pink_noise_vossmccartney_number( static_cast< uint64_t >( value ) );
        // // std::vector< double >( state_.synth_pink_noise_vossmccartney_number(), 0.0 ).swap( pink_VossMcCartney_streams_ );
        // last_pinkNoiseVossMcCartneyNumber_ = value;  // we only want to show things when UI changes state
      }

#undef SFG_GEN_TMP_CHANGE_PARAM
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

clap_process_status NoiseGenerator::process( clap_process_t const* process ) {
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

#define SFG_GEN_TMP_CHECK_LAST_CHANGED( var, stateVar, id )               \
  if( var != double( state_.stateVar() ) ) {                              \
    clap_event_param_value_t out_ev{};                                    \
    out_ev.header.size = sizeof( out_ev );                                \
    out_ev.header.time = i;                                               \
    out_ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;                    \
    out_ev.header.type = CLAP_EVENT_PARAM_VALUE;                          \
    out_ev.header.flags = CLAP_EVENT_IS_LIVE;                             \
    out_ev.param_id = id;                                                 \
    out_ev.value = double( state_.stateVar() );                           \
    var = out_ev.value;                                                   \
    process->out_events->try_push( process->out_events, &out_ev.header ); \
  }

    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sineWaveType_, synth_sine_wave_type, 1010 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sineWaveAttack_, synth_sine_wave_attack, 1015 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sineWaveDecay_, synth_sine_wave_decay, 1016 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sineWaveSustain_, synth_sine_wave_sustain, 1017 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sineWaveRelease_, synth_sine_wave_release, 1018 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sineWaveMix_, synth_sine_wave_mix, 1019 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWaveType_, synth_square_wave_type, 1020 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWavePwm_, synth_square_wave_pwm, 1021 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWaveAttack_, synth_square_wave_attack, 1025 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWaveDecay_, synth_square_wave_decay, 1026 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWaveSustain_, synth_square_wave_sustain, 1027 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWaveRelease_, synth_square_wave_release, 1028 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_squareWaveMix_, synth_square_wave_mix, 1029 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sawWaveType_, synth_saw_wave_type, 1030 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sawWaveAttack_, synth_saw_wave_attack, 1035 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sawWaveDecay_, synth_saw_wave_decay, 1036 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sawWaveSustain_, synth_saw_wave_sustain, 1037 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sawWaveRelease_, synth_saw_wave_release, 1038 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_sawWaveMix_, synth_saw_wave_mix, 1039 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_triangleWaveType_, synth_triangle_wave_type, 1040 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_triangleWaveAttack_, synth_triangle_wave_attack, 1045 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_triangleWaveDecay_, synth_triangle_wave_decay, 1046 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_triangleWaveSustain_, synth_triangle_wave_sustain, 1047 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_triangleWaveRelease_, synth_triangle_wave_release, 1048 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_triangleWaveMix_, synth_triangle_wave_mix, 1049 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_whiteNoiseType_, synth_white_noise_type, 1050 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_whiteNoiseAttack_, synth_white_noise_attack, 1055 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_whiteNoiseDecay_, synth_white_noise_decay, 1056 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_whiteNoiseSustain_, synth_white_noise_sustain, 1057 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_whiteNoiseRelease_, synth_white_noise_release, 1058 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_whiteNoiseMix_, synth_white_noise_mix, 1059 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseType_, synth_pink_noise_type, 1060 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseVossMcCartneyNumber_, synth_pink_noise_vossmccartney_number, 1061 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseAttack_, synth_pink_noise_attack, 1065 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseDecay_, synth_pink_noise_decay, 1066 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseSustain_, synth_pink_noise_sustain, 1067 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseRelease_, synth_pink_noise_release, 1068 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_pinkNoiseMix_, synth_pink_noise_mix, 1069 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_redNoiseType_, synth_red_noise_type, 1070 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_redNoiseAttack_, synth_red_noise_attack, 1075 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_redNoiseDecay_, synth_red_noise_decay, 1076 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_redNoiseSustain_, synth_red_noise_sustain, 1077 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_redNoiseRelease_, synth_red_noise_release, 1078 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_redNoiseMix_, synth_red_noise_mix, 1079 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_blueNoiseType_, synth_blue_noise_type, 1080 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_blueNoiseAttack_, synth_blue_noise_attack, 1085 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_blueNoiseDecay_, synth_blue_noise_decay, 1086 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_blueNoiseSustain_, synth_blue_noise_sustain, 1087 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_blueNoiseRelease_, synth_blue_noise_release, 1088 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_blueNoiseMix_, synth_blue_noise_mix, 1089 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_violetNoiseType_, synth_violet_noise_type, 1090 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_violetNoiseAttack_, synth_violet_noise_attack, 1095 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_violetNoiseDecay_, synth_violet_noise_decay, 1096 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_violetNoiseSustain_, synth_violet_noise_sustain, 1097 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_violetNoiseRelease_, synth_violet_noise_release, 1098 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_violetNoiseMix_, synth_violet_noise_mix, 1099 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_greyNoiseType_, synth_grey_noise_type, 1100 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_greyNoiseAttack_, synth_grey_noise_attack, 1105 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_greyNoiseDecay_, synth_grey_noise_decay, 1106 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_greyNoiseSustain_, synth_grey_noise_sustain, 1107 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_greyNoiseRelease_, synth_grey_noise_release, 1108 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_greyNoiseMix_, synth_grey_noise_mix, 1109 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_velvetNoiseType_, synth_velvet_noise_type, 1110 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_velvetNoiseAttack_, synth_velvet_noise_attack, 1115 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_velvetNoiseDecay_, synth_velvet_noise_decay, 1116 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_velvetNoiseSustain_, synth_velvet_noise_sustain, 1117 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_velvetNoiseRelease_, synth_velvet_noise_release, 1118 );
    SFG_GEN_TMP_CHECK_LAST_CHANGED( last_velvetNoiseMix_, synth_velvet_noise_mix, 1119 );

#undef SFG_GEN_TMP_CHECK_LAST_CHANGED

    /* process every samples until the next event */
    for( ; i < next_ev_frame; ++i ) {
      float out_sine_wave = 0.0f;
      float out_square_wave = 0.0f;
      float out_saw_wave = 0.0f;
      float out_triangle_wave = 0.0f;
      float out_white_noise = 0.0f;
      float out_pink_noise = 0.0f;
      float out_red_noise = 0.0f;
      float out_blue_noise = 0.0f;
      float out_violet_noise = 0.0f;
      float out_grey_noise = 0.0f;
      float out_velvet_noise = 0.0f;
      for( uint32_t c = 0; c < process->audio_outputs[0].channel_count; c++ ) {
        float out = 0.0f;
        if( active_ && process_ ) {
          noteMap_.foreach( [this,
                             &out,
                             &out_sine_wave,
                             &out_square_wave,
                             &out_saw_wave,
                             &out_triangle_wave,
                             &out_white_noise,
                             &out_pink_noise,
                             &out_red_noise,
                             &out_blue_noise,
                             &out_violet_noise,
                             &out_grey_noise,
                             &out_velvet_noise]( std::pair< NoteMap::NoteDescription const, NoteMap::NoteData >& entry ) {
            // A4 (note id 69) is 440.0 Hz
            float freq = 440.0f * std::pow( 2.0f, ( float( entry.first.key ) - 69.0f ) / 12.0f );

            // phase is 0.0 .. 1.0
            float sample_sine_wave = get_sample_sine_wave( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_square_wave = get_sample_square_wave( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_saw_wave = get_sample_saw_wave( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_triangle_wave = get_sample_triangle_wave( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_white_noise = get_sample_white_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_pink_noise = get_sample_pink_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_red_noise = get_sample_red_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_blue_noise = get_sample_blue_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_violet_noise = get_sample_violet_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_grey_noise = get_sample_grey_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;
            float sample_velvet_noise = get_sample_velvet_noise( entry.second.phase ) * entry.second.velocity * entry.second.envelopeLevel;

            out_sine_wave += sample_sine_wave;
            out_square_wave += sample_square_wave;
            out_saw_wave += sample_saw_wave;
            out_triangle_wave += sample_triangle_wave;
            out_white_noise += sample_white_noise;
            out_pink_noise += sample_pink_noise;
            out_red_noise += sample_red_noise;
            out_blue_noise += sample_blue_noise;
            out_violet_noise += sample_violet_noise;
            out_grey_noise += sample_grey_noise;
            out_velvet_noise += sample_velvet_noise;

            float sample = sample_sine_wave + sample_square_wave + sample_saw_wave + sample_triangle_wave + sample_white_noise + sample_pink_noise
                           + sample_red_noise + sample_blue_noise + sample_violet_noise + sample_grey_noise + sample_velvet_noise;

            entry.second.phase += freq / sample_rate_;
            if( entry.second.phase >= 1.0f )
              entry.second.phase -= 1.0f;

            out += sample;
          } );
        }
        // store output
        if( process->audio_outputs[0].data32 )
          process->audio_outputs[0].data32[c][i] = out;
        else if( process->audio_outputs[0].data64 )
          process->audio_outputs[0].data64[c][i] = out;
      }
      sampleQueueSineWave_.push( out_sine_wave / float( process->audio_outputs[0].channel_count ) );
      sampleQueueSquareWave_.push( out_square_wave / float( process->audio_outputs[0].channel_count ) );
      sampleQueueSawWave_.push( out_saw_wave / float( process->audio_outputs[0].channel_count ) );
      sampleQueueTriangleWave_.push( out_triangle_wave / float( process->audio_outputs[0].channel_count ) );
      sampleQueueWhiteNoise_.push( out_white_noise / float( process->audio_outputs[0].channel_count ) );
      sampleQueuePinkNoise_.push( out_pink_noise / float( process->audio_outputs[0].channel_count ) );
      sampleQueueRedNoise_.push( out_red_noise / float( process->audio_outputs[0].channel_count ) );
      sampleQueueBlueNoise_.push( out_blue_noise / float( process->audio_outputs[0].channel_count ) );
      sampleQueueVioletNoise_.push( out_violet_noise / float( process->audio_outputs[0].channel_count ) );
      sampleQueueGreyNoise_.push( out_grey_noise / float( process->audio_outputs[0].channel_count ) );
      sampleQueueVelvetNoise_.push( out_velvet_noise / float( process->audio_outputs[0].channel_count ) );
    }
  }

  return ( active_ && process_ ) ? CLAP_PROCESS_CONTINUE : CLAP_PROCESS_SLEEP;
}

#pragma endregion

#pragma region CLAP extensions

uint32_t NoiseGenerator::audio_ports_count( bool is_input ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 0;
  }
  return 1;
}

bool NoiseGenerator::audio_ports_get( uint32_t index, bool is_input, clap_audio_port_info_t* out_info ) {
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
  if( is_input )
    return false;

  out_info->id = 0;
  std::snprintf( out_info->name, sizeof( out_info->name ), "%s", "main" );
  out_info->channel_count = 1;
  out_info->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS | CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE;
  out_info->port_type = CLAP_PORT_MONO;
  out_info->in_place_pair = CLAP_INVALID_ID;
  return true;
}

bool NoiseGenerator::gui_is_api_supported( std::string const& api, bool is_floating ) {
  bool ret = _base_::gui_is_api_supported( api, is_floating );
  return ret || true;
}

bool NoiseGenerator::gui_get_preferred_api( std::string& out_api, bool* out_is_floating ) {
  bool ret = _base_::gui_get_preferred_api( out_api, out_is_floating );
  return ret && false;
}

bool NoiseGenerator::gui_create( std::string const& api, bool is_floating ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] init SDL", __FUNCTION__, static_cast< void* >( this ) );
  if( !SDL_InitSubSystem( SDL_INIT_VIDEO ) ) {
    PLUGIN_LOG_ERROR( host_, host_log_, "[{:s}] [{:p}] error initializing SDL: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }
  if( !TTF_Init() ) {
    PLUGIN_LOG_ERROR( host_, host_log_, "[{:s}] [{:p}] error initializing SDL_TTF: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  std::function< int( int, int ) > customMod = []( int val, int max ) {
    while( val >= max )
      val -= max;
    while( val < 0 )
      val += max;
    return val;
  };

  InputManager::init();

  guiRootWidget_ = std::make_shared< Widget >( logger_->clone( "root" ) );
  guiRootWidget_->SetPadding( 5.0f );

  guiWidgetSineWave_ = std::make_shared< Widget >( logger_->clone( "SineWave" ), SDL_FRect{ 0.0f / 3.0f, 0.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetSquareWave_ = std::make_shared< Widget >( logger_->clone( "SquareWave" ), SDL_FRect{ 1.0f / 3.0f, 0.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetSawWave_ = std::make_shared< Widget >( logger_->clone( "SawWave" ), SDL_FRect{ 2.0f / 3.0f, 0.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetTriangleWave_ = std::make_shared< Widget >( logger_->clone( "TriangleWave" ), SDL_FRect{ 0.0f / 3.0f, 1.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetWhiteNoise_ = std::make_shared< Widget >( logger_->clone( "WhiteNoise" ), SDL_FRect{ 1.0f / 3.0f, 1.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetPinkNoise_ = std::make_shared< Widget >( logger_->clone( "PinkNoise" ), SDL_FRect{ 2.0f / 3.0f, 1.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetRedNoise_ = std::make_shared< Widget >( logger_->clone( "RedNoise" ), SDL_FRect{ 0.0f / 3.0f, 2.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetBlueNoise_ = std::make_shared< Widget >( logger_->clone( "BlueNoise" ), SDL_FRect{ 1.0f / 3.0f, 2.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetVioletNoise_ = std::make_shared< Widget >( logger_->clone( "VioletNoise" ), SDL_FRect{ 2.0f / 3.0f, 2.0f / 4.0f, 1.0f / 3.0f, 1.0f / 4.0f } );
  guiWidgetGreyNoise_ = std::make_shared< Widget >( logger_->clone( "GreyNoise" ), SDL_FRect{ 0.0f / 2.0f, 3.0f / 4.0f, 1.0f / 2.0f, 1.0f / 4.0f } );
  guiWidgetVelvetNoise_ = std::make_shared< Widget >( logger_->clone( "VelvetNoise" ), SDL_FRect{ 1.0f / 2.0f, 3.0f / 4.0f, 1.0f / 2.0f, 1.0f / 4.0f } );

  guiWidgetSineWavePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_sine_wave_type( _pb_::SineWaveType( customMod( int( state_.synth_sine_wave_type() ) - 1, _pb_::SineWaveType_ARRAYSIZE ) ) );
        guiWidgetSineWaveCurrentType_->SetText( fmt::format( "Sine Wave: {:s}", _pb_::SineWaveType_Name( state_.synth_sine_wave_type() ) ) );
      },
      "<",
      logger_->clone( "SineWavePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetSquareWavePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_square_wave_type( _pb_::SquareWaveType( customMod( int( state_.synth_square_wave_type() ) - 1, _pb_::SquareWaveType_ARRAYSIZE ) ) );
        guiWidgetSquareWaveCurrentType_->SetText( fmt::format( "Square Wave: {:s}", _pb_::SquareWaveType_Name( state_.synth_square_wave_type() ) ) );
      },
      "<",
      logger_->clone( "SquareWavePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetSawWavePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_saw_wave_type( _pb_::SawWaveType( customMod( int( state_.synth_saw_wave_type() ) - 1, _pb_::SawWaveType_ARRAYSIZE ) ) );
        guiWidgetSawWaveCurrentType_->SetText( fmt::format( "Saw Wave: {:s}", _pb_::SawWaveType_Name( state_.synth_saw_wave_type() ) ) );
      },
      "<",
      logger_->clone( "SawWavePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetTriangleWavePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_triangle_wave_type(
            _pb_::TriangleWaveType( customMod( int( state_.synth_triangle_wave_type() ) - 1, _pb_::TriangleWaveType_ARRAYSIZE ) ) );
        guiWidgetTriangleWaveCurrentType_->SetText( fmt::format( "Triangle Wave: {:s}", _pb_::TriangleWaveType_Name( state_.synth_triangle_wave_type() ) ) );
      },
      "<",
      logger_->clone( "TriangleWavePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetWhiteNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_white_noise_type( _pb_::WhiteNoiseType( customMod( int( state_.synth_white_noise_type() ) - 1, _pb_::WhiteNoiseType_ARRAYSIZE ) ) );
        guiWidgetWhiteNoiseCurrentType_->SetText( fmt::format( "White Noise: {:s}", _pb_::WhiteNoiseType_Name( state_.synth_white_noise_type() ) ) );
      },
      "<",
      logger_->clone( "WhiteNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetPinkNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_pink_noise_type( _pb_::PinkNoiseType( customMod( int( state_.synth_pink_noise_type() ) - 1, _pb_::PinkNoiseType_ARRAYSIZE ) ) );
        guiWidgetPinkNoiseCurrentType_->SetText( fmt::format( "Pink Noise: {:s}", _pb_::PinkNoiseType_Name( state_.synth_pink_noise_type() ) ) );
      },
      "<",
      logger_->clone( "PinkNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetRedNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_red_noise_type( _pb_::RedNoiseType( customMod( int( state_.synth_red_noise_type() ) - 1, _pb_::RedNoiseType_ARRAYSIZE ) ) );
        guiWidgetRedNoiseCurrentType_->SetText( fmt::format( "Red Noise: {:s}", _pb_::RedNoiseType_Name( state_.synth_red_noise_type() ) ) );
      },
      "<",
      logger_->clone( "RedNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetBlueNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_blue_noise_type( _pb_::BlueNoiseType( customMod( int( state_.synth_blue_noise_type() ) - 1, _pb_::BlueNoiseType_ARRAYSIZE ) ) );
        guiWidgetBlueNoiseCurrentType_->SetText( fmt::format( "Blue Noise: {:s}", _pb_::BlueNoiseType_Name( state_.synth_blue_noise_type() ) ) );
      },
      "<",
      logger_->clone( "BlueNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetVioletNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_violet_noise_type(
            _pb_::VioletNoiseType( customMod( int( state_.synth_violet_noise_type() ) - 1, _pb_::VioletNoiseType_ARRAYSIZE ) ) );
        guiWidgetVioletNoiseCurrentType_->SetText( fmt::format( "Violet Noise: {:s}", _pb_::VioletNoiseType_Name( state_.synth_violet_noise_type() ) ) );
      },
      "<",
      logger_->clone( "VioletNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetGreyNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_grey_noise_type( _pb_::GreyNoiseType( customMod( int( state_.synth_grey_noise_type() ) - 1, _pb_::GreyNoiseType_ARRAYSIZE ) ) );
        guiWidgetGreyNoiseCurrentType_->SetText( fmt::format( "Grey Noise: {:s}", _pb_::GreyNoiseType_Name( state_.synth_grey_noise_type() ) ) );
      },
      "<",
      logger_->clone( "GreyNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );
  guiWidgetVelvetNoisePreviousType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_velvet_noise_type(
            _pb_::VelvetNoiseType( customMod( int( state_.synth_velvet_noise_type() ) - 1, _pb_::VelvetNoiseType_ARRAYSIZE ) ) );
        guiWidgetVelvetNoiseCurrentType_->SetText( fmt::format( "Velvet Noise: {:s}", _pb_::VelvetNoiseType_Name( state_.synth_velvet_noise_type() ) ) );
      },
      "<",
      logger_->clone( "VelvetNoisePreviousType" ),
      SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.25f } );

  guiWidgetSineWaveCurrentType_ = std::make_shared< Label >( "Sine Wave", logger_->clone( "SineWaveCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetSquareWaveCurrentType_
      = std::make_shared< Label >( "Square Wave", logger_->clone( "SquareWaveCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetSawWaveCurrentType_ = std::make_shared< Label >( "Saw Wave", logger_->clone( "SawWaveCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetTriangleWaveCurrentType_
      = std::make_shared< Label >( "Triangle Wave", logger_->clone( "TriangleWaveCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetWhiteNoiseCurrentType_
      = std::make_shared< Label >( "White Noise", logger_->clone( "WhiteNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetPinkNoiseCurrentType_ = std::make_shared< Label >( "Pink Noise", logger_->clone( "PinkNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetRedNoiseCurrentType_ = std::make_shared< Label >( "Red Noise", logger_->clone( "RedNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetBlueNoiseCurrentType_ = std::make_shared< Label >( "Blue Noise", logger_->clone( "BlueNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetVioletNoiseCurrentType_
      = std::make_shared< Label >( "Violet Noise", logger_->clone( "VioletNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetGreyNoiseCurrentType_ = std::make_shared< Label >( "Grey Noise", logger_->clone( "GreyNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );
  guiWidgetVelvetNoiseCurrentType_
      = std::make_shared< Label >( "Velvet Noise", logger_->clone( "VelvetNoiseCurrentType" ), SDL_FRect{ 0.125f, 0.0f, 0.75f, 0.25f } );

  guiWidgetSineWaveNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_sine_wave_type( _pb_::SineWaveType( customMod( int( state_.synth_sine_wave_type() ) + 1, _pb_::SineWaveType_ARRAYSIZE ) ) );
        guiWidgetSineWaveCurrentType_->SetText( fmt::format( "Sine Wave: {:s}", _pb_::SineWaveType_Name( state_.synth_sine_wave_type() ) ) );
      },
      ">",
      logger_->clone( "SineWaveNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetSquareWaveNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_square_wave_type( _pb_::SquareWaveType( customMod( int( state_.synth_square_wave_type() ) + 1, _pb_::SquareWaveType_ARRAYSIZE ) ) );
        guiWidgetSquareWaveCurrentType_->SetText( fmt::format( "Square Wave: {:s}", _pb_::SquareWaveType_Name( state_.synth_square_wave_type() ) ) );
      },
      ">",
      logger_->clone( "SquareWaveNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetSawWaveNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_saw_wave_type( _pb_::SawWaveType( customMod( int( state_.synth_saw_wave_type() ) + 1, _pb_::SawWaveType_ARRAYSIZE ) ) );
        guiWidgetSawWaveCurrentType_->SetText( fmt::format( "Saw Wave: {:s}", _pb_::SawWaveType_Name( state_.synth_saw_wave_type() ) ) );
      },
      ">",
      logger_->clone( "SawWaveNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetTriangleWaveNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_triangle_wave_type(
            _pb_::TriangleWaveType( customMod( int( state_.synth_triangle_wave_type() ) + 1, _pb_::TriangleWaveType_ARRAYSIZE ) ) );
        guiWidgetTriangleWaveCurrentType_->SetText( fmt::format( "Triangle Wave: {:s}", _pb_::TriangleWaveType_Name( state_.synth_triangle_wave_type() ) ) );
      },
      ">",
      logger_->clone( "TriangleWaveNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetWhiteNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_white_noise_type( _pb_::WhiteNoiseType( customMod( int( state_.synth_white_noise_type() ) + 1, _pb_::WhiteNoiseType_ARRAYSIZE ) ) );
        guiWidgetWhiteNoiseCurrentType_->SetText( fmt::format( "White Noise: {:s}", _pb_::WhiteNoiseType_Name( state_.synth_white_noise_type() ) ) );
      },
      ">",
      logger_->clone( "WhiteNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetPinkNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_pink_noise_type( _pb_::PinkNoiseType( customMod( int( state_.synth_pink_noise_type() ) + 1, _pb_::PinkNoiseType_ARRAYSIZE ) ) );
        guiWidgetPinkNoiseCurrentType_->SetText( fmt::format( "Pink Noise: {:s}", _pb_::PinkNoiseType_Name( state_.synth_pink_noise_type() ) ) );
      },
      ">",
      logger_->clone( "PinkNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetRedNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_red_noise_type( _pb_::RedNoiseType( customMod( int( state_.synth_red_noise_type() ) + 1, _pb_::RedNoiseType_ARRAYSIZE ) ) );
        guiWidgetRedNoiseCurrentType_->SetText( fmt::format( "Red Noise: {:s}", _pb_::RedNoiseType_Name( state_.synth_red_noise_type() ) ) );
      },
      ">",
      logger_->clone( "RedNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetBlueNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_blue_noise_type( _pb_::BlueNoiseType( customMod( int( state_.synth_blue_noise_type() ) + 1, _pb_::BlueNoiseType_ARRAYSIZE ) ) );
        guiWidgetBlueNoiseCurrentType_->SetText( fmt::format( "Blue Noise: {:s}", _pb_::BlueNoiseType_Name( state_.synth_blue_noise_type() ) ) );
      },
      ">",
      logger_->clone( "BlueNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetVioletNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_violet_noise_type(
            _pb_::VioletNoiseType( customMod( int( state_.synth_violet_noise_type() ) + 1, _pb_::VioletNoiseType_ARRAYSIZE ) ) );
        guiWidgetVioletNoiseCurrentType_->SetText( fmt::format( "Violet Noise: {:s}", _pb_::VioletNoiseType_Name( state_.synth_violet_noise_type() ) ) );
      },
      ">",
      logger_->clone( "VioletNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetGreyNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_grey_noise_type( _pb_::GreyNoiseType( customMod( int( state_.synth_grey_noise_type() ) + 1, _pb_::GreyNoiseType_ARRAYSIZE ) ) );
        guiWidgetGreyNoiseCurrentType_->SetText( fmt::format( "Grey Noise: {:s}", _pb_::GreyNoiseType_Name( state_.synth_grey_noise_type() ) ) );
      },
      ">",
      logger_->clone( "GreyNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );
  guiWidgetVelvetNoiseNextType_ = std::make_shared< Button >(
      [this, customMod]() {
        state_.set_synth_velvet_noise_type(
            _pb_::VelvetNoiseType( customMod( int( state_.synth_velvet_noise_type() ) + 1, _pb_::VelvetNoiseType_ARRAYSIZE ) ) );
        guiWidgetVelvetNoiseCurrentType_->SetText( fmt::format( "Velvet Noise: {:s}", _pb_::VelvetNoiseType_Name( state_.synth_velvet_noise_type() ) ) );
      },
      ">",
      logger_->clone( "VelvetNoiseNextType" ),
      SDL_FRect{ 0.875f, 0.0f, 0.125f, 0.25f } );

  guiWidgetSineWaveMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "SineWaveMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetSquareWaveMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "SquareWaveMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetSawWaveMix_ = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "SawWaveMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetTriangleWaveMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "TriangleWaveMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetWhiteNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "WhiteNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetPinkNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "PinkNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetRedNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "RedNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetBlueNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "BlueNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetVioletNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "VioletNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetGreyNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "GreyNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );
  guiWidgetVelvetNoiseMix_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "VelvetNoiseMix" ), SDL_FRect{ 0.875f, 0.25f, 0.125f, 0.75f } );

  guiWidgetSquareWavePwm_
      = std::make_shared< Slider >( Slider::Orientation::Vertical, logger_->clone( "SquareWavePwm" ), SDL_FRect{ 0.00f, 0.25f, 0.125f, 0.75f } );

  guiWidgetSineWaveSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "SineWaveSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetSquareWaveSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "SquareWaveSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetSawWaveSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "SawWaveSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetTriangleWaveSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "TriangleWaveSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetWhiteNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "WhiteNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetPinkNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "PinkNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetRedNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "RedNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetBlueNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "BlueNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetVioletNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "VioletNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetGreyNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "GreyNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );
  guiWidgetVelvetNoiseSamples_
      = std::make_shared< AudioSampleDisplay >( sample_rate_, logger_->clone( "VelvetNoiseSamples" ), SDL_FRect{ 0.125f, 0.25f, 0.75f, 0.75f } );

  guiWidgetSineWaveMix_->OnValueChanged( [this]( float value ) { state_.set_synth_sine_wave_mix( value ); } );
  guiWidgetSquareWaveMix_->OnValueChanged( [this]( float value ) { state_.set_synth_square_wave_mix( value ); } );
  guiWidgetSawWaveMix_->OnValueChanged( [this]( float value ) { state_.set_synth_saw_wave_mix( value ); } );
  guiWidgetTriangleWaveMix_->OnValueChanged( [this]( float value ) { state_.set_synth_triangle_wave_mix( value ); } );
  guiWidgetWhiteNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_white_noise_mix( value ); } );
  guiWidgetPinkNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_pink_noise_mix( value ); } );
  guiWidgetRedNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_red_noise_mix( value ); } );
  guiWidgetBlueNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_blue_noise_mix( value ); } );
  guiWidgetVioletNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_violet_noise_mix( value ); } );
  guiWidgetGreyNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_grey_noise_mix( value ); } );
  guiWidgetVelvetNoiseMix_->OnValueChanged( [this]( float value ) { state_.set_synth_velvet_noise_mix( value ); } );

  guiWidgetSquareWavePwm_->OnValueChanged( [this]( float value ) { state_.set_synth_square_wave_pwm( value ); } );

  std::vector< std::shared_ptr< Widget > > frames{ guiWidgetSineWave_,
                                                   guiWidgetSquareWave_,
                                                   guiWidgetSawWave_,
                                                   guiWidgetTriangleWave_,
                                                   guiWidgetWhiteNoise_,
                                                   guiWidgetPinkNoise_,
                                                   guiWidgetRedNoise_,
                                                   guiWidgetBlueNoise_,
                                                   guiWidgetVioletNoise_,
                                                   guiWidgetGreyNoise_,
                                                   guiWidgetVelvetNoise_ };
  std::vector< std::shared_ptr< Button > > previousTypeButtons{ guiWidgetSineWavePreviousType_,
                                                                guiWidgetSquareWavePreviousType_,
                                                                guiWidgetSawWavePreviousType_,
                                                                guiWidgetTriangleWavePreviousType_,
                                                                guiWidgetWhiteNoisePreviousType_,
                                                                guiWidgetPinkNoisePreviousType_,
                                                                guiWidgetRedNoisePreviousType_,
                                                                guiWidgetBlueNoisePreviousType_,
                                                                guiWidgetVioletNoisePreviousType_,
                                                                guiWidgetGreyNoisePreviousType_,
                                                                guiWidgetVelvetNoisePreviousType_ };
  std::vector< std::shared_ptr< Label > > currentTypeLabels{ guiWidgetSineWaveCurrentType_,
                                                             guiWidgetSquareWaveCurrentType_,
                                                             guiWidgetSawWaveCurrentType_,
                                                             guiWidgetTriangleWaveCurrentType_,
                                                             guiWidgetWhiteNoiseCurrentType_,
                                                             guiWidgetPinkNoiseCurrentType_,
                                                             guiWidgetRedNoiseCurrentType_,
                                                             guiWidgetBlueNoiseCurrentType_,
                                                             guiWidgetVioletNoiseCurrentType_,
                                                             guiWidgetGreyNoiseCurrentType_,
                                                             guiWidgetVelvetNoiseCurrentType_ };
  std::vector< std::shared_ptr< Button > > nextTypeButtons{ guiWidgetSineWaveNextType_,
                                                            guiWidgetSquareWaveNextType_,
                                                            guiWidgetSawWaveNextType_,
                                                            guiWidgetTriangleWaveNextType_,
                                                            guiWidgetWhiteNoiseNextType_,
                                                            guiWidgetPinkNoiseNextType_,
                                                            guiWidgetRedNoiseNextType_,
                                                            guiWidgetBlueNoiseNextType_,
                                                            guiWidgetVioletNoiseNextType_,
                                                            guiWidgetGreyNoiseNextType_,
                                                            guiWidgetVelvetNoiseNextType_ };
  std::vector< std::shared_ptr< Slider > > mixSliders{ guiWidgetSineWaveMix_,
                                                       guiWidgetSquareWaveMix_,
                                                       guiWidgetSawWaveMix_,
                                                       guiWidgetTriangleWaveMix_,
                                                       guiWidgetWhiteNoiseMix_,
                                                       guiWidgetPinkNoiseMix_,
                                                       guiWidgetRedNoiseMix_,
                                                       guiWidgetBlueNoiseMix_,
                                                       guiWidgetVioletNoiseMix_,
                                                       guiWidgetGreyNoiseMix_,
                                                       guiWidgetVelvetNoiseMix_ };
  std::vector< std::shared_ptr< AudioSampleDisplay > > sampleDisplays{ guiWidgetSineWaveSamples_,
                                                                       guiWidgetSquareWaveSamples_,
                                                                       guiWidgetSawWaveSamples_,
                                                                       guiWidgetTriangleWaveSamples_,
                                                                       guiWidgetWhiteNoiseSamples_,
                                                                       guiWidgetPinkNoiseSamples_,
                                                                       guiWidgetRedNoiseSamples_,
                                                                       guiWidgetBlueNoiseSamples_,
                                                                       guiWidgetVioletNoiseSamples_,
                                                                       guiWidgetGreyNoiseSamples_,
                                                                       guiWidgetVelvetNoiseSamples_ };

  for( auto frames : frames ) {
    frames->InitUi( guiRootWidget_ );
    frames->SetFrame( true );
    frames->SetPadding( 5.0f );
  }

  for( size_t i = 0; i < previousTypeButtons.size(); i++ ) {
    auto& button = previousTypeButtons.at( i );
    button->InitUi( frames.at( i ) );
    button->SetHorizontalAlignment( Label::HorizontalAlignment::Centered );
    button->SetVerticalAlignment( Label::VerticalAlignment::Centered );
    button->SetFontFile( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" );
    button->SetFontSize( 18 );
    button->SetFontColourActive( SDL_Color{ 0xff, 0xff, 0xff, 0xff } );
    button->SetFontColourInactive( SDL_Color{ 0xff, 0xff, 0xff, 0x80 } );
    button->SetPadding( 5.0f );

    // THIS IS TO RESET THE SELECTION AND UPDATE THE LABELS!!!
    button->GetCallback()();
  }

  for( size_t i = 0; i < currentTypeLabels.size(); i++ ) {
    auto& label = currentTypeLabels.at( i );
    label->InitUi( frames.at( i ) );
    label->SetHorizontalAlignment( Label::HorizontalAlignment::Centered );
    label->SetVerticalAlignment( Label::VerticalAlignment::Centered );
    label->SetFontFile( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" );
    label->SetFontSize( 18 );
    label->SetFontColourActive( SDL_Color{ 0xff, 0xff, 0xff, 0xff } );
    label->SetFontColourInactive( SDL_Color{ 0xff, 0xff, 0xff, 0x80 } );
    label->SetPadding( 5.0f );
  }

  for( size_t i = 0; i < nextTypeButtons.size(); i++ ) {
    auto& button = nextTypeButtons.at( i );
    button->InitUi( frames.at( i ) );
    button->SetHorizontalAlignment( Label::HorizontalAlignment::Centered );
    button->SetVerticalAlignment( Label::VerticalAlignment::Centered );
    button->SetFontFile( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" );
    button->SetFontSize( 18 );
    button->SetFontColourActive( SDL_Color{ 0xff, 0xff, 0xff, 0xff } );
    button->SetFontColourInactive( SDL_Color{ 0xff, 0xff, 0xff, 0x80 } );
    button->SetPadding( 5.0f );

    // THIS IS TO RESET THE SELECTION AND UPDATE THE LABELS!!!
    button->GetCallback()();
  }

  for( size_t i = 0; i < mixSliders.size(); i++ ) {
    auto& slider = mixSliders.at( i );
    slider->InitUi( frames.at( i ) );
    slider->SetMinValue( 0.0f );
    slider->SetMaxValue( 1.0f );
    slider->SetDefaultValue( 0.0f );
    slider->SetPadding( 5.0f );
  }
  {
    guiWidgetSquareWavePwm_->InitUi( guiWidgetSquareWave_ );
    guiWidgetSquareWavePwm_->SetMinValue( 0.0f );
    guiWidgetSquareWavePwm_->SetMaxValue( 1.0f );
    guiWidgetSquareWavePwm_->SetDefaultValue( 0.5f );
    guiWidgetSquareWavePwm_->SetPadding( 5.0f );
  }

  guiWidgetSineWaveMix_->SetCurrentValue( state_.synth_sine_wave_mix() );
  guiWidgetSquareWaveMix_->SetCurrentValue( state_.synth_square_wave_mix() );
  guiWidgetSawWaveMix_->SetCurrentValue( state_.synth_saw_wave_mix() );
  guiWidgetTriangleWaveMix_->SetCurrentValue( state_.synth_triangle_wave_mix() );
  guiWidgetWhiteNoiseMix_->SetCurrentValue( state_.synth_white_noise_mix() );
  guiWidgetPinkNoiseMix_->SetCurrentValue( state_.synth_pink_noise_mix() );
  guiWidgetRedNoiseMix_->SetCurrentValue( state_.synth_red_noise_mix() );
  guiWidgetBlueNoiseMix_->SetCurrentValue( state_.synth_blue_noise_mix() );
  guiWidgetVioletNoiseMix_->SetCurrentValue( state_.synth_violet_noise_mix() );
  guiWidgetGreyNoiseMix_->SetCurrentValue( state_.synth_grey_noise_mix() );
  guiWidgetVelvetNoiseMix_->SetCurrentValue( state_.synth_velvet_noise_mix() );

  guiWidgetSquareWavePwm_->SetCurrentValue( state_.synth_square_wave_pwm() );

  for( size_t i = 0; i < sampleDisplays.size(); i++ ) {
    auto& display = sampleDisplays.at( i );
    display->InitUi( frames.at( i ) );
    display->SetPadding( 5.0f );
  }

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
    WRAP_SDL_CALL_INST( SDL_SetStringProperty, windowCreateProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "com.SFGrenade.NoiseGenerator" );
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

  guiTimer_ = Timer::createNative( 1, std::bind( &NoiseGenerator::guiTimerCallback, this ) );
  guiTimer_->start();
  return true;
}

void NoiseGenerator::gui_destroy( void ) {
  guiTimer_->stop();
  guiTimer_.reset();
  guiWindowRenderer_.reset();
  guiWindow_.reset();

  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] quit SDL", __FUNCTION__, static_cast< void* >( this ) );
  TTF_Quit();
  SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

bool NoiseGenerator::gui_set_scale( double scale ) {
  return false;
}

bool NoiseGenerator::gui_get_size( uint32_t* out_width, uint32_t* out_height ) {
  WRAP_SDL_CALL_INST( SDL_GetWindowSize, guiWindow_.get(), reinterpret_cast< int* >( out_width ), reinterpret_cast< int* >( out_height ) );
  return true;
}

bool NoiseGenerator::gui_can_resize( void ) {
  return ( SDL_GetWindowFlags( guiWindow_.get() ) & SDL_WINDOW_RESIZABLE );
}

bool NoiseGenerator::gui_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  return false;
}

bool NoiseGenerator::gui_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  gui_set_size( *out_width, *out_height );
  gui_get_size( out_width, out_height );
  state_.set_gui_width( *out_width );
  state_.set_gui_height( *out_height );
  return true;
}

bool NoiseGenerator::gui_set_size( uint32_t width, uint32_t height ) {
  WRAP_SDL_CALL_INST( SDL_SetWindowSize, guiWindow_.get(), width, height );
  state_.set_gui_width( width );
  state_.set_gui_height( height );
  return true;
}

bool NoiseGenerator::gui_set_parent( clap_window_t const* window ) {
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

bool NoiseGenerator::gui_set_transient( clap_window_t const* window ) {
  WRAP_SDL_CALL_INST( SDL_RaiseWindow, guiWindow_.get() );
  return true;
}

void NoiseGenerator::gui_suggest_title( std::string const& title ) {
  WRAP_SDL_CALL_INST( SDL_SetWindowTitle, guiWindow_.get(), title.c_str() );
}

bool NoiseGenerator::gui_show( void ) {
  WRAP_SDL_CALL_INST( SDL_ShowWindow, guiWindow_.get() );
  return true;
}

bool NoiseGenerator::gui_hide( void ) {
  WRAP_SDL_CALL_INST( SDL_HideWindow, guiWindow_.get() );
  return true;
}

uint32_t NoiseGenerator::note_ports_count( bool is_input ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( is_input={} )", __FUNCTION__, static_cast< void* >( this ), is_input );
  if( is_input ) {
    return 1;
  }
  return 0;
}

bool NoiseGenerator::note_ports_get( uint32_t index, bool is_input, clap_note_port_info_t* out_info ) {
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

uint32_t NoiseGenerator::params_count( void ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
  // adjust according to NoiseGenerator.proto
  // while we could make it dynamic, without explicit gui i'd rather not
  return 68;
}

bool NoiseGenerator::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
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

  /*
    out_param_info->id = 1;
    out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS | CLAP_PARAM_IS_ENUM;
    out_param_info->cookie = nullptr;
    std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Type" );
    std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Sine Wave" );
    out_param_info->min_value = static_cast< double >( _pb_::SineWaveType::NoiseGenerator_SineWaveType_StdSin );
    out_param_info->max_value = static_cast< double >( _pb_::SineWaveType::NoiseGenerator_SineWaveType_CSin );
    out_param_info->default_value = out_param_info->min_value;
  */

#define SFG_GEN_TMP_GET_INFO( op, paramIndex, paramId, paramFlags, paramName, paramModule, paramMin, paramMax, paramDefault ) \
  op( param_index == ( paramIndex ) ) {                                                                                       \
    out_param_info->id = paramId;                                                                                             \
    out_param_info->flags = ( paramFlags ) | CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;                         \
    out_param_info->cookie = nullptr;                                                                                         \
    std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", paramName );                                   \
    std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", paramModule );                             \
    out_param_info->min_value = paramMin;                                                                                     \
    out_param_info->max_value = paramMax;                                                                                     \
    out_param_info->default_value = paramDefault;                                                                             \
  }

  SFG_GEN_TMP_GET_INFO( if,
                        0,
                        1010,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Sine Wave",
                        static_cast< double >( _pb_::SineWaveType::NoiseGenerator_SineWaveType_StdSin ),
                        static_cast< double >( _pb_::SineWaveType::NoiseGenerator_SineWaveType_CSin ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 1, 1015, 0, "Attack", "Sine Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 2, 1016, 0, "Decay", "Sine Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 3, 1017, 0, "Sustain", "Sine Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 4, 1018, 0, "Release", "Sine Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 5, 1019, 0, "Mix", "Sine Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        6,
                        1020,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Square Wave",
                        static_cast< double >( _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_PhaseWidth ),
                        static_cast< double >( _pb_::SquareWaveType::NoiseGenerator_SquareWaveType_InversePhaseWidth ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 7, 1021, 0, "PWM", "Square Wave", 0.0, 1.0, 0.5 )
  SFG_GEN_TMP_GET_INFO( else if, 8, 1025, 0, "Attack", "Square Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 9, 1026, 0, "Decay", "Square Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 10, 1027, 0, "Sustain", "Square Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 11, 1028, 0, "Release", "Square Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 12, 1029, 0, "Mix", "Square Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        13,
                        1030,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Saw Wave",
                        static_cast< double >( _pb_::SawWaveType::NoiseGenerator_SawWaveType_Phase ),
                        static_cast< double >( _pb_::SawWaveType::NoiseGenerator_SawWaveType_InversePhase ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 14, 1035, 0, "Attack", "Saw Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 15, 1036, 0, "Decay", "Saw Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 16, 1037, 0, "Sustain", "Saw Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 17, 1038, 0, "Release", "Saw Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 18, 1039, 0, "Mix", "Saw Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        19,
                        1040,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Triangle Wave",
                        static_cast< double >( _pb_::TriangleWaveType::NoiseGenerator_TriangleWaveType_ChunkLerp ),
                        static_cast< double >( _pb_::TriangleWaveType::NoiseGenerator_TriangleWaveType_ChunkLerp ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 20, 1045, 0, "Attack", "Triangle Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 21, 1046, 0, "Decay", "Triangle Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 22, 1047, 0, "Sustain", "Triangle Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 23, 1048, 0, "Release", "Triangle Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 24, 1049, 0, "Mix", "Triangle Wave", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        25,
                        1050,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "White Noise",
                        static_cast< double >( _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_StdRandom ),
                        static_cast< double >( _pb_::WhiteNoiseType::NoiseGenerator_WhiteNoiseType_RandMaxRand ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 26, 1055, 0, "Attack", "White Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 27, 1056, 0, "Decay", "White Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 28, 1057, 0, "Sustain", "White Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 29, 1058, 0, "Release", "White Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 30, 1059, 0, "Mix", "White Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        31,
                        1060,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Pink Noise",
                        static_cast< double >( _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_PaulKellettRefined ),
                        static_cast< double >( _pb_::PinkNoiseType::NoiseGenerator_PinkNoiseType_IirFilterApproximation ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 32, 1061, 0, "VossMcCartney Number", "Pink Noise", 4, 64, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 33, 1065, 0, "Attack", "Pink Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 34, 1066, 0, "Decay", "Pink Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 35, 1067, 0, "Sustain", "Pink Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 36, 1068, 0, "Release", "Pink Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 37, 1069, 0, "Mix", "Pink Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        38,
                        1070,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Red Noise",
                        static_cast< double >( _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_BasicIntegration ),
                        static_cast< double >( _pb_::RedNoiseType::NoiseGenerator_RedNoiseType_CumulativeWithClamp ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 39, 1075, 0, "Attack", "Red Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 40, 1076, 0, "Decay", "Red Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 41, 1077, 0, "Sustain", "Red Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 42, 1078, 0, "Release", "Red Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 43, 1079, 0, "Mix", "Red Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        44,
                        1080,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Blue Noise",
                        static_cast< double >( _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_VoidAndCluster ),
                        static_cast< double >( _pb_::BlueNoiseType::NoiseGenerator_BlueNoiseType_PermutedGradientNoise ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 45, 1085, 0, "Attack", "Blue Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 46, 1086, 0, "Decay", "Blue Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 47, 1087, 0, "Sustain", "Blue Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 48, 1088, 0, "Release", "Blue Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 49, 1089, 0, "Mix", "Blue Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        50,
                        1090,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Violet Noise",
                        static_cast< double >( _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderDifference ),
                        static_cast< double >( _pb_::VioletNoiseType::NoiseGenerator_VioletNoiseType_FirstOrderIirFilter ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 51, 1095, 0, "Attack", "Violet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 52, 1096, 0, "Decay", "Violet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 53, 1097, 0, "Sustain", "Violet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 54, 1098, 0, "Release", "Violet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 55, 1099, 0, "Mix", "Violet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        56,
                        1100,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Grey Noise",
                        static_cast< double >( _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_PsychoacousticFilter ),
                        static_cast< double >( _pb_::GreyNoiseType::NoiseGenerator_GreyNoiseType_EqualLoudnessApproximation ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 57, 1105, 0, "Attack", "Grey Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 58, 1106, 0, "Decay", "Grey Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 59, 1107, 0, "Sustain", "Grey Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 60, 1108, 0, "Release", "Grey Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 61, 1109, 0, "Mix", "Grey Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if,
                        62,
                        1110,
                        CLAP_PARAM_IS_STEPPED,
                        "Type",
                        "Velvet Noise",
                        static_cast< double >( _pb_::VelvetNoiseType::NoiseGenerator_VelvetNoiseType_SporadicImpulse ),
                        static_cast< double >( _pb_::VelvetNoiseType::NoiseGenerator_VelvetNoiseType_SporadicImpulse ),
                        out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 63, 1115, 0, "Attack", "Velvet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 64, 1116, 0, "Decay", "Velvet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 65, 1117, 0, "Sustain", "Velvet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 66, 1118, 0, "Release", "Velvet Noise", 0.0, 1.0, out_param_info->min_value )
  SFG_GEN_TMP_GET_INFO( else if, 67, 1119, 0, "Mix", "Velvet Noise", 0.0, 1.0, out_param_info->min_value )

#undef SFG_GEN_TMP_GET_INFO

  return true;
}

bool NoiseGenerator::params_get_value( clap_id param_id, double* out_value ) {
  PLUGIN_LOG_TRACE( host_,
                    host_log_,
                    "[{:s}] [{:p}] enter( param_id={:d}, out_value={:p} )",
                    __FUNCTION__,
                    static_cast< void* >( this ),
                    param_id,
                    static_cast< void* >( out_value ) );
  if( !out_value )
    return false;

#define SFG_GEN_TMP_GET_VALUE( op, id, stateVar ) \
  op( param_id == id ) {                          \
    ( *out_value ) = state_.stateVar();           \
    return true;                                  \
  }

  SFG_GEN_TMP_GET_VALUE( if, 1010, synth_sine_wave_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1015, synth_sine_wave_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1016, synth_sine_wave_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1017, synth_sine_wave_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1018, synth_sine_wave_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1019, synth_sine_wave_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1020, synth_square_wave_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1021, synth_square_wave_pwm )
  SFG_GEN_TMP_GET_VALUE( else if, 1025, synth_square_wave_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1026, synth_square_wave_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1027, synth_square_wave_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1028, synth_square_wave_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1029, synth_square_wave_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1030, synth_saw_wave_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1035, synth_saw_wave_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1036, synth_saw_wave_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1037, synth_saw_wave_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1038, synth_saw_wave_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1039, synth_saw_wave_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1040, synth_triangle_wave_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1045, synth_triangle_wave_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1046, synth_triangle_wave_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1047, synth_triangle_wave_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1048, synth_triangle_wave_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1049, synth_triangle_wave_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1050, synth_white_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1055, synth_white_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1056, synth_white_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1057, synth_white_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1058, synth_white_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1059, synth_white_noise_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1060, synth_pink_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1061, synth_pink_noise_vossmccartney_number )
  SFG_GEN_TMP_GET_VALUE( else if, 1065, synth_pink_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1066, synth_pink_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1067, synth_pink_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1068, synth_pink_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1069, synth_pink_noise_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1070, synth_red_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1075, synth_red_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1076, synth_red_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1077, synth_red_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1078, synth_red_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1079, synth_red_noise_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1080, synth_blue_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1085, synth_blue_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1086, synth_blue_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1087, synth_blue_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1088, synth_blue_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1089, synth_blue_noise_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1090, synth_violet_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1095, synth_violet_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1096, synth_violet_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1097, synth_violet_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1098, synth_violet_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1099, synth_violet_noise_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1100, synth_grey_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1105, synth_grey_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1106, synth_grey_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1107, synth_grey_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1108, synth_grey_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1109, synth_grey_noise_mix )
  SFG_GEN_TMP_GET_VALUE( else if, 1110, synth_velvet_noise_type )
  SFG_GEN_TMP_GET_VALUE( else if, 1115, synth_velvet_noise_attack )
  SFG_GEN_TMP_GET_VALUE( else if, 1116, synth_velvet_noise_decay )
  SFG_GEN_TMP_GET_VALUE( else if, 1117, synth_velvet_noise_sustain )
  SFG_GEN_TMP_GET_VALUE( else if, 1118, synth_velvet_noise_release )
  SFG_GEN_TMP_GET_VALUE( else if, 1119, synth_velvet_noise_mix )

#undef SFG_GEN_TMP_GET_VALUE

  return false;
}

bool NoiseGenerator::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
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

#define SFG_GEN_TMP_GET_STRING( op, id, conversion )                                                        \
  op( param_id == id ) {                                                                                    \
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );                                           \
    std::string tmp_str = conversion;                                                                       \
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) ); \
    return true;                                                                                            \
  }

  SFG_GEN_TMP_GET_STRING( if, 1010, _pb_::SineWaveType_Name( static_cast< _pb_::SineWaveType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1015, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1016, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1017, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1018, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1019, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1020, _pb_::SquareWaveType_Name( static_cast< _pb_::SquareWaveType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1021, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1025, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1026, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1027, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1028, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1029, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1030, _pb_::SawWaveType_Name( static_cast< _pb_::SawWaveType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1035, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1036, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1037, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1038, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1039, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1040, _pb_::TriangleWaveType_Name( static_cast< _pb_::TriangleWaveType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1045, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1046, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1047, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1048, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1049, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1050, _pb_::WhiteNoiseType_Name( static_cast< _pb_::WhiteNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1055, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1056, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1057, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1058, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1059, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1060, _pb_::PinkNoiseType_Name( static_cast< _pb_::PinkNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1061, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1065, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1066, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1067, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1068, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1069, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1070, _pb_::RedNoiseType_Name( static_cast< _pb_::RedNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1075, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1076, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1077, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1078, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1079, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1080, _pb_::BlueNoiseType_Name( static_cast< _pb_::BlueNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1085, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1086, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1087, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1088, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1089, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1090, _pb_::VioletNoiseType_Name( static_cast< _pb_::VioletNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1095, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1096, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1097, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1098, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1099, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1100, _pb_::GreyNoiseType_Name( static_cast< _pb_::GreyNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1105, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1106, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1107, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1108, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1109, std::to_string( value ) )
  SFG_GEN_TMP_GET_STRING( else if, 1110, _pb_::VelvetNoiseType_Name( static_cast< _pb_::VelvetNoiseType >( value ) ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1115, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1116, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1117, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1118, std::to_string( value ) )
  // SFG_GEN_TMP_GET_STRING( else if, 1119, std::to_string( value ) )

#undef SFG_GEN_TMP_GET_STRING

  return false;
}

bool NoiseGenerator::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
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

#define SFG_GEN_TMP_TEXT_TO_ENUM( op, id, type )             \
  op( param_id == id ) {                                     \
    _pb_::type out;                                          \
    bool ret = _pb_::type##_Parse( param_value_text, &out ); \
    if( ret )                                                \
      ( *out_value ) = static_cast< double >( out );         \
    return ret;                                              \
  }
#define SFG_GEN_TMP_TEXT_TO_DOUBLE( op, id )              \
  op( param_id == id ) {                                  \
    return text_to_double( param_value_text, out_value ); \
  }

  SFG_GEN_TMP_TEXT_TO_ENUM( if, 1010, SineWaveType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1015 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1016 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1017 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1018 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1019 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1020, SquareWaveType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1021 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1025 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1026 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1027 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1028 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1029 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1030, SawWaveType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1035 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1036 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1037 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1038 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1039 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1040, TriangleWaveType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1045 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1046 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1047 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1048 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1049 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1050, WhiteNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1055 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1056 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1057 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1058 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1059 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1060, PinkNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1061 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1065 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1066 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1067 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1068 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1069 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1070, RedNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1075 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1076 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1077 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1078 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1079 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1080, BlueNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1085 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1086 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1087 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1088 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1089 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1090, VioletNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1095 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1096 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1097 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1098 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1099 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1100, GreyNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1105 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1106 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1107 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1108 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1109 )
  SFG_GEN_TMP_TEXT_TO_ENUM( else if, 1110, VelvetNoiseType )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1115 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1116 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1117 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1118 )
  SFG_GEN_TMP_TEXT_TO_DOUBLE( else if, 1119 )

#undef SFG_GEN_TMP_TEXT_TO_ENUM
#undef SFG_GEN_TMP_TEXT_TO_DOUBLE

  return false;
}

void NoiseGenerator::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
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

bool NoiseGenerator::state_save( clap_ostream_t const* stream ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );

  ClapOStream tmp( stream );
  bool ret = state_.SerializeToOstream( &tmp );

  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool NoiseGenerator::state_load( clap_istream_t const* stream ) {
  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );

  ClapIStream tmp( stream );
  bool ret = state_.ParseFromIstream( &tmp );

  PLUGIN_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
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

#pragma region GUI CALLBACK

void NoiseGenerator::guiTimerCallback() {
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
  sampleQueueSineWave_.consume_all( [this]( float sample ) { guiWidgetSineWaveSamples_->PushSample( sample ); } );
  sampleQueueSquareWave_.consume_all( [this]( float sample ) { guiWidgetSquareWaveSamples_->PushSample( sample ); } );
  sampleQueueSawWave_.consume_all( [this]( float sample ) { guiWidgetSawWaveSamples_->PushSample( sample ); } );
  sampleQueueTriangleWave_.consume_all( [this]( float sample ) { guiWidgetTriangleWaveSamples_->PushSample( sample ); } );
  sampleQueueWhiteNoise_.consume_all( [this]( float sample ) { guiWidgetWhiteNoiseSamples_->PushSample( sample ); } );
  sampleQueuePinkNoise_.consume_all( [this]( float sample ) { guiWidgetPinkNoiseSamples_->PushSample( sample ); } );
  sampleQueueRedNoise_.consume_all( [this]( float sample ) { guiWidgetRedNoiseSamples_->PushSample( sample ); } );
  sampleQueueBlueNoise_.consume_all( [this]( float sample ) { guiWidgetBlueNoiseSamples_->PushSample( sample ); } );
  sampleQueueVioletNoise_.consume_all( [this]( float sample ) { guiWidgetVioletNoiseSamples_->PushSample( sample ); } );
  sampleQueueGreyNoise_.consume_all( [this]( float sample ) { guiWidgetGreyNoiseSamples_->PushSample( sample ); } );
  sampleQueueVelvetNoise_.consume_all( [this]( float sample ) { guiWidgetVelvetNoiseSamples_->PushSample( sample ); } );
  {
    if( state_.synth_sine_wave_mix() != guiWidgetSineWaveMix_->GetCurrentValue() ) {
      guiWidgetSineWaveMix_->SetCurrentValue( state_.synth_sine_wave_mix() );
    }
    if( state_.synth_square_wave_mix() != guiWidgetSquareWaveMix_->GetCurrentValue() ) {
      guiWidgetSquareWaveMix_->SetCurrentValue( state_.synth_square_wave_mix() );
    }
    if( state_.synth_saw_wave_mix() != guiWidgetSawWaveMix_->GetCurrentValue() ) {
      guiWidgetSawWaveMix_->SetCurrentValue( state_.synth_saw_wave_mix() );
    }
    if( state_.synth_triangle_wave_mix() != guiWidgetTriangleWaveMix_->GetCurrentValue() ) {
      guiWidgetTriangleWaveMix_->SetCurrentValue( state_.synth_triangle_wave_mix() );
    }
    if( state_.synth_white_noise_mix() != guiWidgetWhiteNoiseMix_->GetCurrentValue() ) {
      guiWidgetWhiteNoiseMix_->SetCurrentValue( state_.synth_white_noise_mix() );
    }
    if( state_.synth_pink_noise_mix() != guiWidgetPinkNoiseMix_->GetCurrentValue() ) {
      guiWidgetPinkNoiseMix_->SetCurrentValue( state_.synth_pink_noise_mix() );
    }
    if( state_.synth_red_noise_mix() != guiWidgetRedNoiseMix_->GetCurrentValue() ) {
      guiWidgetRedNoiseMix_->SetCurrentValue( state_.synth_red_noise_mix() );
    }
    if( state_.synth_blue_noise_mix() != guiWidgetBlueNoiseMix_->GetCurrentValue() ) {
      guiWidgetBlueNoiseMix_->SetCurrentValue( state_.synth_blue_noise_mix() );
    }
    if( state_.synth_violet_noise_mix() != guiWidgetVioletNoiseMix_->GetCurrentValue() ) {
      guiWidgetVioletNoiseMix_->SetCurrentValue( state_.synth_violet_noise_mix() );
    }
    if( state_.synth_grey_noise_mix() != guiWidgetGreyNoiseMix_->GetCurrentValue() ) {
      guiWidgetGreyNoiseMix_->SetCurrentValue( state_.synth_grey_noise_mix() );
    }
    if( state_.synth_velvet_noise_mix() != guiWidgetVelvetNoiseMix_->GetCurrentValue() ) {
      guiWidgetVelvetNoiseMix_->SetCurrentValue( state_.synth_velvet_noise_mix() );
    }
    if( state_.synth_square_wave_pwm() != guiWidgetSquareWavePwm_->GetCurrentValue() ) {
      guiWidgetSquareWavePwm_->SetCurrentValue( state_.synth_square_wave_pwm() );
    }
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
