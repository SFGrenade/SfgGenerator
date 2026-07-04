// Header assigned to this source
#include "organ/ReedPipe.hpp"

void ReedPipe::Init( double sampleRate, std::shared_ptr< spdlog::logger > logger ) {
  Pipe::Init( sampleRate, logger );
  logger_->trace( "[{:s}] [{:p}] enter( sampleRate={:f} )", __FUNCTION__, static_cast< void* >( this ), sampleRate );
}

double ReedPipe::Update( double inputVelocity ) {
  return 0.0;
}
