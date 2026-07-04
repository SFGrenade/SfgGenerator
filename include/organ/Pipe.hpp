#pragma once

// Project includes
#include "libraryExtensions/logging.hpp"

// C++ std includes
#include <random>

constexpr double const SPEED_OF_SOUND = 343.0;  // [m/s]

struct Pipe {
  virtual void Init( double sampleRate, std::shared_ptr< spdlog::logger > logger );
  virtual double Update( double inputVelocity = 0.0 );

  protected:
  std::shared_ptr< spdlog::logger > logger_;
  double sampleRate_;
};
