// Header assigned to this source
#include "widgets/Bar.hpp"

Bar::Bar( SDL_FRect position ) : _base_( position ) {}

Bar::~Bar() {}

void Bar::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  _base_::OnRender( renderer );
  if( !IsVisibleHierarchy() ) {
    return;
  }

  if( IsActiveHierarchy() ) {
    SDL_SetRenderDrawColor( renderer.get(), colourActive_.r, colourActive_.g, colourActive_.b, colourActive_.a );
  } else {
    SDL_SetRenderDrawColor( renderer.get(), colourInactive_.r, colourInactive_.g, colourInactive_.b, colourInactive_.a );
  }
  SDL_FRect tmpRect = global_position_;
  tmpRect.w = global_position_.w * value_;
  SDL_RenderFillRect( renderer.get(), &tmpRect );
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
