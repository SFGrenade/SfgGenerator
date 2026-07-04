#pragma once

// Project includes
#include "organ/Pipe.hpp"

// Other lib includes
#include <Iir.h>

// C++ std includes
#include <vector>

struct FluePipe : public Pipe {
  private:
  struct ExcitationData {
    double phase = 0.0;
    double phaseInc = 0.0;
  };

  public:
  static constexpr size_t const NUM_HARMONICS = 128;

  virtual void Init( double sampleRate, std::shared_ptr< spdlog::logger > logger ) override;
  virtual double Update( double inputVelocity = 0.0 ) override;

  enum class Shape { Rectanglular, Cylindrical };
  enum class Openess { Open, Closed };

  double length = 1.0;     // [m]
  double diameter = 0.15;  // [m]
  Shape shape = Shape::Cylindrical;
  Openess openess = Openess::Open;

  protected:
  double fundamentalFrequency_ = 0.0;
  // pulse train
  std::array<ExcitationData, NUM_HARMONICS> excitation_;
  size_t actualExcitations_ = 0;

  std::vector< double > delayLine1_;
  size_t delayLine1I_ = 0;
  std::vector< double > delayLine2_;
  size_t delayLine2I_ = 0;

  std::array< Iir::Butterworth::LowPass< 1 >, 4 > lossFilters_;

  Iir::Butterworth::LowPass< 1 > lowpass_;
};
