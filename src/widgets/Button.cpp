// Header assigned to this source
#include "widgets/Button.hpp"

Button::Button( std::function< void() > const& callback, std::string const& text, std::shared_ptr< spdlog::logger > logger, SDL_FRect position )
    : _base_( text, logger, position ), callback_( callback ) {
  logger_->trace( "[{:s}] [{:p}] enter( text={:?}, position=({:f}, {:f}, {:f}, {:f}) )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  text,
                  position.x,
                  position.y,
                  position.w,
                  position.h );
}

Button::~Button() {}

void Button::OnLogic() {
  _base_::OnLogic();
  if( !IsVisibleHierarchy() ) {
    return;
  }
  if( !IsActiveHierarchy() ) {
    return;
  }

  if( mbPressed_ && callback_ ) {
    callback_();
  }
}

void Button::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    _base_::OnRender( renderer );
    return;
  }

  // background
  bool drawBackground = IsActiveHierarchy() && ( mbDown_ || cursorHovering_ );
  if( drawBackground ) {
    if( mbDown_ ) {
      if( !SDL_SetRenderDrawColor( renderer.get(),
                                   backgroundColourPressed_.r,
                                   backgroundColourPressed_.g,
                                   backgroundColourPressed_.b,
                                   backgroundColourPressed_.a ) )
        logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
    } else if( cursorHovering_ ) {
      if( !SDL_SetRenderDrawColor( renderer.get(),
                                   backgroundColourHovering_.r,
                                   backgroundColourHovering_.g,
                                   backgroundColourHovering_.b,
                                   backgroundColourHovering_.a ) )
        logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
    }
    if( !SDL_RenderFillRect( renderer.get(), &global_position_ ) )
      logger_->warn( "[{:s}] [{:p}] SDL_RenderFillRect signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  bool drawBorder = true;
  if( drawBorder ) {
    if( IsActiveHierarchy() ) {
      if( !SDL_SetRenderDrawColor( renderer.get(), borderColourActive_.r, borderColourActive_.g, borderColourActive_.b, borderColourActive_.a ) )
        logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
    } else {
      if( !SDL_SetRenderDrawColor( renderer.get(), borderColourInactive_.r, borderColourInactive_.g, borderColourInactive_.b, borderColourInactive_.a ) )
        logger_->warn( "[{:s}] [{:p}] SDL_SetRenderDrawColor signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
    }
    if( !SDL_RenderRect( renderer.get(), &global_position_ ) )
      logger_->warn( "[{:s}] [{:p}] SDL_RenderRect signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  _base_::OnRender( renderer );
}

std::function< void() > Button::GetCallback() {
  return callback_;
}

void Button::SetCallback( std::function< void() > const& value ) {
  callback_ = value;
}

SDL_Color Button::GetBackgroundColourHovering() {
  return backgroundColourHovering_;
}

void Button::SetBackgroundColourHovering( SDL_Color const& value ) {
  backgroundColourHovering_.r = value.r;
  backgroundColourHovering_.g = value.g;
  backgroundColourHovering_.b = value.b;
  backgroundColourHovering_.a = value.a;
}

SDL_Color Button::GetBackgroundColourPressed() {
  return backgroundColourPressed_;
}

void Button::SetBackgroundColourPressed( SDL_Color const& value ) {
  backgroundColourPressed_.r = value.r;
  backgroundColourPressed_.g = value.g;
  backgroundColourPressed_.b = value.b;
  backgroundColourPressed_.a = value.a;
}

SDL_Color Button::GetBorderColourActive() {
  return borderColourActive_;
}

void Button::SetBorderColourActive( SDL_Color const& value ) {
  borderColourActive_.r = value.r;
  borderColourActive_.g = value.g;
  borderColourActive_.b = value.b;
  borderColourActive_.a = value.a;
}

SDL_Color Button::GetBorderColourInactive() {
  return borderColourInactive_;
}

void Button::SetBorderColourInactive( SDL_Color const& value ) {
  borderColourInactive_.r = value.r;
  borderColourInactive_.g = value.g;
  borderColourInactive_.b = value.b;
  borderColourInactive_.a = value.a;
}
