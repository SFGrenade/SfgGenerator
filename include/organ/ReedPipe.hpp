#pragma once

// Project includes
#include "organ/Pipe.hpp"

// C++ std includes
#include <cstdint>

struct ReedPipe : public Pipe {
  virtual void Init( double sampleRate, std::shared_ptr< spdlog::logger > logger ) override;
  virtual double Update( double inputVelocity = 0.0 ) override;

  protected:
  std::vector< double > delayLine_;
};
