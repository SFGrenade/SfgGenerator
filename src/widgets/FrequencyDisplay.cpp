// Header assigned to this source
#include "widgets/FrequencyDisplay.hpp"

// Project includes
#include "common/WindowFunctions.hpp"
#include "common/_fftw.hpp"
#include "widgets/Label.hpp"

// C++ std includes
#include <numbers>

FrequencyDisplay::FrequencyDisplay( double sampleRate, std::shared_ptr< spdlog::logger > logger, SDL_FRect position )
    : _base_( logger, position ), sampleRate_( sampleRate ), samples_( sampleRate * 0.4 ), FFT_DISPLAY_MAX_FREQ( sampleRate / 2.0f ) {
  logger_->trace( "[{:s}] [{:p}] enter( sampleRate={:f}, position=({:f}, {:f}, {:f}, {:f}) )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  sampleRate,
                  position.x,
                  position.y,
                  position.w,
                  position.h );
  MIN_FREQ_LOG = std::log10( FFT_DISPLAY_MIN_FREQ );
  MAX_FREQ_LOG = std::log10( FFT_DISPLAY_MAX_FREQ );
  fftInputSize_ = 1;
  while( fftInputSize_ < samples_.capacity() ) {
    fftInputSize_ = fftInputSize_ << 1;
  }
  fftOutputSize_ = fftInputSize_ / 2 + 1;
  fftWindow_.resize( fftInputSize_, 0.0 );
  // hann( fftWindow_.data(), fftInputSize_, false );
  nuttallwin_octave( fftWindow_.data(), fftInputSize_, false );
  fftInput_.resize( fftInputSize_, 0.0 );
  fftOutput_ = std::make_shared< fftwf_complex[] >( fftOutputSize_ );
  curatedFftOutput_.resize( fftOutputSize_, FftOutputInformation{} );

  fftPlan_ = make_fftw_shared_ptr( fftwf_plan_dft_r2c_1d( fftInputSize_, fftInput_.data(), fftOutput_.get(), FFTW_MEASURE ) );
}

FrequencyDisplay::~FrequencyDisplay() {}

void FrequencyDisplay::InitUi( std::shared_ptr< Widget > parent ) {
  std::shared_ptr< Widget > self = shared_from_this();

  // set up more children in here
  std::vector< float > frequenciesToShow{ 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 15000, 20000 };
  for( float frequency : frequenciesToShow ) {
    float centerXRelative = ( ( std::log10( frequency ) - MIN_FREQ_LOG ) / ( MAX_FREQ_LOG - MIN_FREQ_LOG ) );

    std::shared_ptr< Label > tmp = nullptr;
    if( frequency < 1000.0f ) {
      tmp = std::make_shared< Label >( fmt::format( "{:.0f} Hz", frequency ),
                                                  logger_->clone( fmt::format( "tmp {:.0f}", frequency ) ),
                                                  SDL_FRect{ centerXRelative - 0.25f, 0.0f, 0.5f, 1.0f } );
    } else if( 1000.0f <= frequency ) {
      tmp = std::make_shared< Label >( fmt::format( "{:.0f} kHz", frequency / 1000.0f ),
                                                  logger_->clone( fmt::format( "tmp {:.0f}", frequency ) ),
                                                  SDL_FRect{ centerXRelative - 0.25f, 0.0f, 0.5f, 1.0f } );
    }
    tmp->InitUi( self );
    tmp->SetHorizontalAlignment( Label::HorizontalAlignment::Centered );
    tmp->SetVerticalAlignment( Label::VerticalAlignment::Bottom );
    tmp->SetFontFile( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" );
    tmp->SetFontSize( 12 );
    tmp->SetFontColourActive( SDL_Color{ 0xff, 0x00, 0x00, 0xff } );
    tmp->SetFontColourInactive( SDL_Color{ 0xff, 0x00, 0x00, 0x80 } );
    tmp->SetPadding( 2.0f );
  }

  _base_::InitUi( parent );
}

void FrequencyDisplay::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    _base_::OnRender( renderer );
    return;
  }

  if( IsActiveHierarchy() ) {
    if( !SDL_SetRenderDrawColor( renderer.get(), 0xff, 0xff, 0xff, 0xff ) )
      logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  } else {
    if( !SDL_SetRenderDrawColor( renderer.get(), 0xff, 0xff, 0xff, 0x80 ) )
      logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }
  if( !SDL_RenderRect( renderer.get(), &global_position_ ) )
    logger_->warn( "[{:s}] [{:p}] SDL_RenderRect signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );

  if( IsActiveHierarchy() ) {
    if( !SDL_SetRenderDrawColor( renderer.get(), 0xff, 0xff, 0xff, 0xff ) )
      logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  } else {
    if( !SDL_SetRenderDrawColor( renderer.get(), 0xff, 0xff, 0xff, 0x80 ) )
      logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }
  for( size_t i = 0; i < global_position_.w; i++ ) {
    double binMinimumFrequency = std::pow( 10.0, MIN_FREQ_LOG + ( double( i + 0 ) * ( MAX_FREQ_LOG - MIN_FREQ_LOG ) / double( global_position_.w - 1 ) ) );
    double binMaximumFrequency = std::pow( 10.0, MIN_FREQ_LOG + ( double( i + 1 ) * ( MAX_FREQ_LOG - MIN_FREQ_LOG ) / double( global_position_.w - 1 ) ) );

    float binMagnitudeSum = 0.0f;
    for( size_t j = 0; j < curatedFftOutput_.size(); j++ ) {
      if( binMinimumFrequency <= curatedFftOutput_[j].frequency && curatedFftOutput_[j].frequency < binMaximumFrequency ) {
        binMagnitudeSum += curatedFftOutput_[j].magnitude;
      }
    }

    if( std::isnan( binMagnitudeSum ) || ( binMagnitudeSum <= 0.0f ) ) {
      // no direct frequencies for this bin band found, lets catmullRom this
      float fftFrequencyBin = ( float( fftInputSize_ ) * binMinimumFrequency / sampleRate_ );

      size_t aIndex = size_t( std::floor( fftFrequencyBin ) );
      size_t bIndex = size_t( std::ceil( fftFrequencyBin ) );
      float lerpFactor = fftFrequencyBin - float( aIndex );

      SplinePoint a{ curatedFftOutput_[aIndex - 1].frequency, curatedFftOutput_[aIndex - 1].magnitude };
      SplinePoint b{ curatedFftOutput_[aIndex].frequency, curatedFftOutput_[aIndex].magnitude };
      SplinePoint c{ curatedFftOutput_[bIndex].frequency, curatedFftOutput_[bIndex].magnitude };
      SplinePoint d{ curatedFftOutput_[bIndex + 1].frequency, curatedFftOutput_[bIndex + 1].magnitude };
      SplinePoint thisPoint = catmullRom( a, b, c, d, lerpFactor );

      binMagnitudeSum = thisPoint.second;
    }

    if( binMagnitudeSum < 0.0f ) {
      binMagnitudeSum = 0.0f;
    }
    float magnitudeSumDb = 20.0f * std::log10( binMagnitudeSum + 1e-12f );

    float x = global_position_.x + float( i );
    float y = global_position_.y + ( global_position_.h * ( 1.0f - float( magnitudeSumDb - ( -0.0f ) ) / ( 100.0f - ( -0.0f ) ) ) );
    if( !SDL_RenderLine( renderer.get(), x, global_position_.y + global_position_.h, x, y ) )
      logger_->warn( "[{:s}] [{:p}] SDL_RenderLine signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  _base_::OnRender( renderer );
}

void FrequencyDisplay::PushSample( float sample ) {
  samples_.push_back( std::clamp( sample, -1.0f, 1.0f ) );
  pushedSamples_++;

  if( pushedSamples_ < size_t( samples_.capacity() / 4 ) ) {
    return;
  }
  pushedSamples_ = 0;

  for( size_t i = 0; i < fftInputSize_; i++ ) {
    if( i < samples_.size() ) {
      fftInput_[i] = samples_[i] * fftWindow_[i];
    } else {
      fftInput_[i] = 0.0f;
    }
  }
  for( size_t i = 0; i < fftOutputSize_; i++ ) {
    fftOutput_[i][0] = 0.0f;
    fftOutput_[i][1] = 0.0f;
  }
  fftwf_execute( fftPlan_.get() );

  for( size_t i = 0; i < fftOutputSize_; i++ ) {
    float outReal = fftOutput_[i][0];
    float outImaginary = fftOutput_[i][1];

    curatedFftOutput_[i].frequency = float( i ) * float( sampleRate_ ) / float( fftInputSize_ );
    curatedFftOutput_[i].magnitude = std::sqrt( ( outReal * outReal ) + ( outImaginary * outImaginary ) );
    curatedFftOutput_[i].magnitudeInDB = 20.0f * std::log10( curatedFftOutput_[i].magnitude + 1e-12f );
    curatedFftOutput_[i].phase = std::atan2( outImaginary, outReal ) * 180.0f / std::numbers::pi_v< float >;
  }
}
