// Header assigned to this source
#include "widgets/AudioSampleDisplay.hpp"

AudioSampleDisplay::AudioSampleDisplay( double sampleRate, std::shared_ptr< spdlog::logger > logger, SDL_FRect position )
    : _base_( logger, position ), samples_( sampleRate * 0.1 ), points_( sampleRate * 0.1 ) {
  logger_->trace( "[{:s}] [{:p}] enter( sampleRate={:f}, position=({:f}, {:f}, {:f}, {:f}) )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  sampleRate,
                  position.x,
                  position.y,
                  position.w,
                  position.h );
}

AudioSampleDisplay::~AudioSampleDisplay() {}

void AudioSampleDisplay::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    _base_::OnRender( renderer );
    return;
  }

  for( size_t i = 0; i < points_.size(); i++ ) {
    auto& point = points_.at( i );
    auto const& sample = samples_.at( i );

    point.x = global_position_.x + ( global_position_.w * ( float( i ) / float( points_.size() - 1 ) ) );
    point.y = std::lerp( global_position_.y + global_position_.h, global_position_.y, ( sample + 1.0f ) / 2.0f );
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
  if( !SDL_RenderLines( renderer.get(), points_.data(), points_.size() ) )
    logger_->warn( "[{:s}] [{:p}] SDL_RenderLines signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );

  _base_::OnRender( renderer );
}

void AudioSampleDisplay::PushSample( float sample ) {
  samples_.push_back( std::clamp( sample, -1.0f, 1.0f ) );
}
