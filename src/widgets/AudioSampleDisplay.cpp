// Header assigned to this source
#include "widgets/AudioSampleDisplay.hpp"

AudioSampleDisplay::AudioSampleDisplay( double sampleRate, SDL_FRect position )
    : _base_( position ), samples_( sampleRate * 0.1 ), points_( sampleRate * 0.1 ) {}

AudioSampleDisplay::~AudioSampleDisplay() {}

void AudioSampleDisplay::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  _base_::OnRender( renderer );
  if( !IsVisibleHierarchy() ) {
    return;
  }

  for( size_t i = 0; i < points_.size(); i++ ) {
    auto& point = points_.at( i );
    auto const& sample = samples_.at( i );

    point.x = global_position_.x + ( global_position_.w * ( float( i ) / float( points_.size() - 1 ) ) );
    point.y = std::lerp( global_position_.y + global_position_.h, global_position_.y, ( sample + 1.0f ) / 2.0f );
  }


  if( IsActiveHierarchy() ) {
    SDL_SetRenderDrawColor( renderer.get(), 0xff, 0xff, 0xff, 0xff );
  } else {
    SDL_SetRenderDrawColor( renderer.get(), 0xff, 0xff, 0xff, 0x80 );
  }
  SDL_RenderRect( renderer.get(), &global_position_ );
  SDL_RenderLines( renderer.get(), points_.data(), points_.size() );
}

void AudioSampleDisplay::PushSample( float sample ) {
  samples_.push_back( std::clamp( sample, -1.0f, 1.0f ) );
}
