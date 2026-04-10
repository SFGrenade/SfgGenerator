// Header assigned to this source
#include "widgets/Bar.hpp"

Bar::Bar( std::shared_ptr< spdlog::logger > logger, SDL_FRect position ) : _base_( logger, position ) {
  logger_->trace( "[{:s}] [{:p}] enter( position=({:f}, {:f}, {:f}, {:f}) )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  position.x,
                  position.y,
                  position.w,
                  position.h );
}

Bar::~Bar() {}

void Bar::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    return;
  }

  if( IsActiveHierarchy() ) {
    if( !SDL_SetRenderDrawColor( renderer.get(), colourActive_.r, colourActive_.g, colourActive_.b, colourActive_.a ) )
      logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  } else {
    if( !SDL_SetRenderDrawColor( renderer.get(), colourInactive_.r, colourInactive_.g, colourInactive_.b, colourInactive_.a ) )
      logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }
  SDL_FRect tmpRect = global_position_;
  tmpRect.w = global_position_.w * value_;
  if( !SDL_RenderFillRect( renderer.get(), &tmpRect ) )
    logger_->warn( "[{:s}] [{:p}] SDL_RenderFillRect signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );

  _base_::OnRender( renderer );
}

float Bar::GetValue() {
  return value_;
}

void Bar::SetValue( float value ) {
  value_ = std::clamp( value, 0.0f, 1.0f );
}

SDL_Color Bar::GetColourActive() {
  return colourActive_;
}

void Bar::SetColourActive( SDL_Color const& value ) {
  colourActive_.r = value.r;
  colourActive_.g = value.g;
  colourActive_.b = value.b;
  colourActive_.a = value.a;
}

SDL_Color Bar::GetColourInactive() {
  return colourInactive_;
}

void Bar::SetColourInactive( SDL_Color const& value ) {
  colourInactive_.r = value.r;
  colourInactive_.g = value.g;
  colourInactive_.b = value.b;
  colourInactive_.a = value.a;
}
