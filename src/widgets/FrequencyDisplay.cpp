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
  hann( fftWindow_.data(), fftInputSize_, false );
  // nuttallwin_octave( fftWindow_.data(), fftInputSize_, false );
  fftInput_.resize( fftInputSize_, 0.0 );
  fftOutput_ = std::make_shared< fftwf_complex[] >( fftOutputSize_ );
  curatedFftOutput_.resize( fftOutputSize_, FftOutputInformation{} );

  fftPlan_ = make_fftw_shared_ptr( fftwf_plan_dft_r2c_1d( fftInputSize_, fftInput_.data(), fftOutput_.get(), FFTW_MEASURE ) );
}

FrequencyDisplay::~FrequencyDisplay() {}

void FrequencyDisplay::InitUi( std::shared_ptr< Widget > parent ) {
  std::shared_ptr< Widget > self = shared_from_this();

  // set up more children in here
  std::vector< float > frequenciesToShow{ 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
  for( float frequency : frequenciesToShow ) {
    float centerXRelative = xForFrequencyRelative( frequency );

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
    tmp->SetFontColourActive( SDL_Color{ 0x00, 0xff, 0xff, 0xff } );
    tmp->SetFontColourInactive( SDL_Color{ 0x00, 0xff, 0xff, 0x80 } );
    tmp->SetPadding( 2.0f );
  }

  _base_::InitUi( parent );
}

void FrequencyDisplay::OnLogic() {
  _base_::OnLogic();
  if( !IsVisibleHierarchy() ) {
    return;
  }
  if( !IsActiveHierarchy() ) {
    return;
  }

  TryCalcFft();
}

void FrequencyDisplay::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    _base_::OnRender( renderer );
    return;
  }

  if( IsActiveHierarchy() ) {
    WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0xff, 0xff, 0xff );
  } else {
    WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0xff, 0xff, 0x80 );
  }
  WRAP_SDL_CALL_INST( SDL_RenderRect, renderer.get(), &global_position_ );

  {
    // have horizontal lines for [0, -10, -20, -30, -40, -50 & -60] dBFS
    if( IsActiveHierarchy() ) {
      WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0x40, 0x40, 0x40, 0xff );
    } else {
      WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0x40, 0x40, 0x40, 0x80 );
    }
    std::vector< float > magnitudeHelperLines{ -0.0f, -10.0f, -20.0f, -30.0f, -40.0f, -50.0f, -60.0f };
    for( float magnitude : magnitudeHelperLines ) {
      float y = global_position_.y
                + ( global_position_.h
                    * ( 1.0f - ( float( magnitude - FFT_DISPLAY_MIN_MAGNITUDE_DB ) / ( FFT_DISPLAY_MAX_MAGNITUDE_DB - FFT_DISPLAY_MIN_MAGNITUDE_DB ) ) ) );
      WRAP_SDL_CALL_INST( SDL_RenderLine, renderer.get(), global_position_.x, y, global_position_.x + global_position_.w, y );
    }
  }
  {
    // have vertical lines for [20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
    // 9000, 10000, 20000] Hz
    std::vector< float > frequencyHelperLines{ 20.0f,   30.0f,   40.0f,   50.0f,   60.0f,   70.0f,   80.0f,    90.0f,   100.0f,  200.0f,
                                               300.0f,  400.0f,  500.0f,  600.0f,  700.0f,  800.0f,  900.0f,   1000.0f, 2000.0f, 3000.0f,
                                               4000.0f, 5000.0f, 6000.0f, 7000.0f, 8000.0f, 9000.0f, 10000.0f, 20000.0f };
    std::vector< float > brighterHelperLines{ 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
    for( float frequency : frequencyHelperLines ) {
      if( std::find( brighterHelperLines.begin(), brighterHelperLines.end(), frequency ) != brighterHelperLines.end() ) {
        if( IsActiveHierarchy() ) {
          WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0x80, 0x80, 0x80, 0xff );
        } else {
          WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0x80, 0x80, 0x80, 0x80 );
        }
      } else {
        if( IsActiveHierarchy() ) {
          WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0x40, 0x40, 0x40, 0xff );
        } else {
          WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0x40, 0x40, 0x40, 0x80 );
        }
      }
      float freqLog = std::log10( frequency );
      float x = global_position_.x + ( global_position_.w * ( ( freqLog - MIN_FREQ_LOG ) / ( MAX_FREQ_LOG - MIN_FREQ_LOG ) ) );
      WRAP_SDL_CALL_INST( SDL_RenderLine, renderer.get(), x, global_position_.y, x, global_position_.y + global_position_.h );
    }
  }

  if( IsActiveHierarchy() ) {
    WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0xff, 0xff, 0xff );
  } else {
    WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0xff, 0xff, 0x80 );
  }
  /*size_t const AMOUNT_BINS = global_position_.w / 10;
  for( size_t i = 0; i < AMOUNT_BINS; i++ ) {
    float relativeI = float( i ) / float( AMOUNT_BINS - 1 );
    float relativeNextI = float( i + 1 ) / float( AMOUNT_BINS - 1 );
    float binMinimumFrequency = std::pow( 10.0f, MIN_FREQ_LOG + ( relativeI * ( MAX_FREQ_LOG - MIN_FREQ_LOG ) ) );
    float binMaximumFrequency = std::pow( 10.0f, MIN_FREQ_LOG + ( relativeNextI * ( MAX_FREQ_LOG - MIN_FREQ_LOG ) ) );*/
  for( size_t i = 0; i < global_position_.w; i++ ) {
    float binMinimumFrequency = std::pow( 10.0f, MIN_FREQ_LOG + ( float( i + 0 ) * ( MAX_FREQ_LOG - MIN_FREQ_LOG ) / float( global_position_.w - 1 ) ) );
    float binMaximumFrequency = std::pow( 10.0f, MIN_FREQ_LOG + ( float( i + 1 ) * ( MAX_FREQ_LOG - MIN_FREQ_LOG ) / float( global_position_.w - 1 ) ) );

    float binMagnitudeSum = 0.0f;
    for( size_t j = 0; j < curatedFftOutput_.size(); j++ ) {
      if( binMinimumFrequency <= curatedFftOutput_[j].frequency && curatedFftOutput_[j].frequency < binMaximumFrequency ) {
        binMagnitudeSum += curatedFftOutput_[j].magnitude;
      }
    }

    if( std::isnan( binMagnitudeSum ) || ( binMagnitudeSum <= 0.0f ) ) {
      // no direct frequencies for this bin band found, lets catmullRom this
      float fftFrequencyBin = float( fftInputSize_ ) * binMinimumFrequency / sampleRate_;

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
    float y = global_position_.y
              + ( global_position_.h
                  * ( 1.0f - ( float( magnitudeSumDb - FFT_DISPLAY_MIN_MAGNITUDE_DB ) / ( FFT_DISPLAY_MAX_MAGNITUDE_DB - FFT_DISPLAY_MIN_MAGNITUDE_DB ) ) ) );
    WRAP_SDL_CALL_INST( SDL_RenderLine, renderer.get(), x, global_position_.y + global_position_.h, x, y );

    // SDL_FRect tmp;
    // // set values
    // tmp.x = xForFrequency( binMinimumFrequency );
    // tmp.y = yForMagnitude( binMagnitudeSum );
    // tmp.w = xForFrequency( binMaximumFrequency ) - xForFrequency( binMinimumFrequency );
    // tmp.h = ( global_position_.y + global_position_.h ) - tmp.y;
    // // clamp values
    // tmp.x = std::clamp( tmp.x, global_position_.x, global_position_.x + global_position_.w );
    // tmp.y = std::clamp( tmp.y, global_position_.y, global_position_.y + global_position_.h );
    // tmp.w = std::clamp( tmp.w, 0.0f, ( global_position_.x + global_position_.w ) - tmp.x );
    // tmp.h = std::clamp( tmp.h, 0.0f, ( global_position_.y + global_position_.h ) - tmp.y );
    // WRAP_SDL_CALL_INST( SDL_RenderRect, renderer.get(), &tmp );
  }

  _base_::OnRender( renderer );
}

void FrequencyDisplay::PushSample( float sample ) {
  samples_.push_back( std::clamp( sample, -1.0f, 1.0f ) );

  // todo: fixme: maybe add a toggle somewhere to choose between every 100ms or just always
  // pushedSamples_++;
}

void FrequencyDisplay::TryCalcFft() {
  // todo: fixme: maybe add a toggle somewhere to choose between every 100ms or just always
  // if( pushedSamples_ < size_t( samples_.capacity() / 4 ) ) {
  //   return;
  // }
  // pushedSamples_ -= size_t( samples_.capacity() / 4 );

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
    curatedFftOutput_[i].magnitude = std::sqrt( ( outReal * outReal ) + ( outImaginary * outImaginary ) ) / float( fftInputSize_ );
    curatedFftOutput_[i].magnitudeInDB = 20.0f * std::log10( curatedFftOutput_[i].magnitude + 1e-12f );
    curatedFftOutput_[i].phase = std::atan2( outImaginary, outReal ) * 180.0f / std::numbers::pi_v< float >;
  }
}

float FrequencyDisplay::xForFrequencyRelative( float frequency_Hz ) const {
  return ( ( std::log10( frequency_Hz ) - MIN_FREQ_LOG ) / ( MAX_FREQ_LOG - MIN_FREQ_LOG ) );
}

float FrequencyDisplay::xForFrequency( float frequency_Hz ) const {
  return global_position_.x + ( xForFrequencyRelative( frequency_Hz ) * global_position_.w );
}

float FrequencyDisplay::yForMagnitudeDbRelative( float magnitude_dB ) const {
  return 1.0f - ( ( magnitude_dB - FFT_DISPLAY_MIN_MAGNITUDE_DB ) / ( FFT_DISPLAY_MAX_MAGNITUDE_DB - FFT_DISPLAY_MIN_MAGNITUDE_DB ) );
}

float FrequencyDisplay::yForMagnitudeDb( float magnitude_dB ) const {
  return global_position_.y + ( yForMagnitudeDbRelative( magnitude_dB ) * global_position_.h );
}

float FrequencyDisplay::yForMagnitudeRelative( float magnitude ) const {
  if( magnitude <= 0.0f ) {
    magnitude = 1e-12f;
  }
  return yForMagnitudeDbRelative( 20.0f * std::log10( magnitude ) );
}

float FrequencyDisplay::yForMagnitude( float magnitude ) const {
  if( magnitude <= 0.0f ) {
    magnitude = 1e-12f;
  }
  return yForMagnitudeDb( 20.0f * std::log10( magnitude ) );
}
