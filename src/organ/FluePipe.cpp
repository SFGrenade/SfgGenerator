// Header assigned to this source
#include "organ/FluePipe.hpp"

// C++ std includes
#include <numbers>
#include <numeric>

void FluePipe::Init( double sampleRate, std::shared_ptr< spdlog::logger > logger ) {
  Pipe::Init( sampleRate, logger );
  logger_->trace( "[{:s}] [{:p}] enter( sampleRate={:f} )", __FUNCTION__, static_cast< void* >( this ), sampleRate );

  if( openess == Openess::Open ) {
    fundamentalFrequency_ = SPEED_OF_SOUND / ( 2.0 * length );
  } else if( openess == Openess::Closed ) {
    fundamentalFrequency_ = SPEED_OF_SOUND / ( 4.0 * length );
  }

  excitation_[0].phase = 0.0;
  excitation_[0].phaseInc = fundamentalFrequency_ / sampleRate_;
  actualExcitations_ = 1;
  for( size_t i = 1; i < excitation_.size(); i++ ) {
    excitation_[i].phase = 0.0;
    excitation_[i].phaseInc = 0.0;
    double harmonicFrequency = fundamentalFrequency_ * double( i );
    if( harmonicFrequency >= ( sampleRate / 2.0 ) ) {
      continue;
    }
    if( openess == Openess::Open ) {
      if( ( i % 2 ) == 0 ) {
        excitation_[i].phaseInc = ( fundamentalFrequency_ * double( i ) ) / sampleRate_;
        actualExcitations_++;
      }
    } else if( openess == Openess::Closed ) {
      excitation_[i].phaseInc = ( fundamentalFrequency_ * double( i ) ) / sampleRate_;
      actualExcitations_++;
    }
  }

  size_t delaySamples = sampleRate_ * ( 2.0 * length ) / SPEED_OF_SOUND;
  std::vector< double >( delaySamples, 0.0 ).swap( delayLine1_ );
  delayLine1I_ = 0;
  std::vector< double >( delaySamples, 0.0 ).swap( delayLine2_ );
  delayLine2I_ = 0;

  double baseCutoff = fundamentalFrequency_ * 4.0;

  for( size_t i = 0; i < lossFilters_.size(); i++ ) {
    double lossFilterFreq = baseCutoff * std::pow( 2.0, double( i + 1 ) );
    lossFilterFreq = std::min( sampleRate_ / 2.0 - 1.0, lossFilterFreq );
    lossFilters_[i].setup( sampleRate_, lossFilterFreq );
  }

  double finalLowpassFreq = baseCutoff * 4.0;
  finalLowpassFreq = std::min( sampleRate_ / 2.0 - 1.0, finalLowpassFreq );
  lowpass_.setup( sampleRate_, finalLowpassFreq );
}

double FluePipe::Update( double inputVelocity ) {
  if( inputVelocity > 0.0 ) {
    for( auto& exc : excitation_ ) {
      if( exc.phaseInc <= 0.0 ) {
        continue;
      }
      exc.phase += exc.phaseInc;
      while( exc.phase >= 1.0 ) {
        exc.phase -= 1.0;
      }
    }
  }

  // pulse train
  double sineExcitation = 0.0;
  // double sawExcitation = 0.0;
  if( inputVelocity > 0.0 ) {
    for( auto const& exc : excitation_ ) {
      if( exc.phaseInc <= 0.0 ) {
        continue;
      }
      sineExcitation += inputVelocity * std::sin( 2.0 * std::numbers::pi_v< double > * exc.phase );
      // sawExcitation += inputVelocity * std::lerp( -1.0, 1.0, exc.phase );
    }
    sineExcitation /= double( actualExcitations_ / 16 );
    // sawExcitation /= double( actualExcitations_ / 16 );
  }

  // jet/flue nonlinearity
  double excitation = std::tanh( sineExcitation );

  double out1 = delayLine1_[delayLine1I_];
  double out2 = delayLine2_[delayLine2I_];

  double in1, in2;
  if( openess == Openess::Open ) {
    in1 = out2 * 0.99;
    in2 = out1 * 0.99;
  } else {
    in1 = -out2 * 0.99;
    in2 = out1 * 0.99;
  }
  in1 += excitation;

  for( auto& filter : lossFilters_ ) {
    in1 = filter.filter( in1 );
  }

  double output = lowpass_.filter( ( in1 + in2 ) / 2.0 );

  delayLine1_[delayLine1I_] = in1;
  delayLine2_[delayLine2I_] = in2;

  delayLine1I_ = ( delayLine1I_ + 1 ) % delayLine1_.size();
  delayLine2I_ = ( delayLine2I_ + 1 ) % delayLine2_.size();

  return output;
}
