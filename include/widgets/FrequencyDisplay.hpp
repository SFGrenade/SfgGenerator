#pragma once

// Project includes
#include "common/_fftw.hpp"
#include "common/_fmt.hpp"
#include "common/_gui.hpp"
#include "widgets/Widget.hpp"

// Other lib includes
#include <boost/circular_buffer.hpp>

// C++ std includes
#include <memory>
#include <vector>

class FrequencyDisplay : public Widget {
  using _base_ = Widget;
  using _base_::_base_;
  struct FftOutputInformation {
    float frequency = 0.0f;
    float magnitude = 0.0f;
    float magnitudeInDB = 0.0f;
    float phase = 0.0f;
  };

  public:
  FrequencyDisplay( double sampleRate, std::shared_ptr< spdlog::logger > logger, SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~FrequencyDisplay();

  public:
  void InitUi( std::shared_ptr< Widget > parent ) override;
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  void PushSample( float sample );

  private:
  double sampleRate_;
  boost::circular_buffer< float > samples_;
  size_t pushedSamples_ = 0;
  size_t fftInputSize_ = 0;
  size_t fftOutputSize_ = 0;
  std::vector< double > fftWindow_;
  std::vector< float > fftInput_;
  std::shared_ptr< fftwf_complex[] > fftOutput_;
  std::shared_ptr< fftwf_plan_s > fftPlan_;
  std::vector< FftOutputInformation > curatedFftOutput_;

  private:
  static constexpr float const FFT_DISPLAY_MIN_FREQ = 10.0f;
  float FFT_DISPLAY_MAX_FREQ = 44100.0f / 2.0f;  // set dynamically with the sampleRate_
  float MIN_FREQ_LOG = 0.0f;
  float MAX_FREQ_LOG = 0.0f;
};
