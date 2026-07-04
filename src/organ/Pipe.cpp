// Header assigned to this source
#include "organ/Pipe.hpp"

void Pipe::Init( double sampleRate, std::shared_ptr< spdlog::logger > logger ) {
  logger_ = logger;
  logger_->trace( "[{:s}] [{:p}] enter( sampleRate={:f} )", __FUNCTION__, static_cast< void* >( this ), sampleRate );
  sampleRate_ = sampleRate;
}

double Pipe::Update( double inputVelocity ) {
  return 0.0;
}
