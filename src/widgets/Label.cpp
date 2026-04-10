// Header assigned to this source
#include "widgets/Label.hpp"

Label::Label( std::string const& text, std::shared_ptr< spdlog::logger > logger, SDL_FRect position )
    : _base_( logger, position ), text_( text ), fontFile_( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" ) {
  logger_->trace( "[{:s}] [{:p}] enter( text={:?}, position=({:f}, {:f}, {:f}, {:f}) )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  text,
                  position.x,
                  position.y,
                  position.w,
                  position.h );
  ReopenFont();
}

Label::~Label() {
  if( font_ ) {
    TTF_CloseFont( font_ );
    font_ = nullptr;
  }
  if( textTexture_ ) {
    SDL_DestroyTexture( textTexture_ );
    textTexture_ = nullptr;
  }
}

void Label::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    _base_::OnRender( renderer );
    return;
  }

  // render new texture
  if( font_ && renderer ) {
    SDL_Surface* textSurface;
    if( IsActiveHierarchy() ) {
      textSurface = TTF_RenderText_Blended_Wrapped( font_, text_.c_str(), text_.size(), fontColourActive_, static_cast< uint32_t >( global_position_.w ) );
    } else {
      textSurface = TTF_RenderText_Blended_Wrapped( font_, text_.c_str(), text_.size(), fontColourInactive_, static_cast< uint32_t >( global_position_.w ) );
    }
    if( !textSurface )
      logger_->warn( "[{:s}] [{:p}] TTF_RenderText_Blended_Wrapped signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );

    textTextureSize_.w = static_cast< float >( textSurface->w );
    textTextureSize_.h = static_cast< float >( textSurface->h );

    // destroy old texture
    if( textTexture_ ) {
      SDL_DestroyTexture( textTexture_ );
      textTexture_ = nullptr;
    }
    textTexture_ = SDL_CreateTextureFromSurface( renderer.get(), textSurface );
    if( !textTexture_ )
      logger_->warn( "[{:s}] [{:p}] SDL_CreateTextureFromSurface signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
    SDL_DestroySurface( textSurface );
  }
  // align texture inside bounding box
  if( horizontalAlignment_ == Label::HorizontalAlignment::Left ) {
    textTextureSize_.x = global_position_.x;
  } else if( horizontalAlignment_ == Label::HorizontalAlignment::Centered ) {
    textTextureSize_.x = global_position_.x + ( ( global_position_.w - textTextureSize_.w ) / 2.0f );
  } else if( horizontalAlignment_ == Label::HorizontalAlignment::Right ) {
    textTextureSize_.x = ( global_position_.x + global_position_.w ) - textTextureSize_.w;
  }
  if( verticalAlignment_ == Label::VerticalAlignment::Top ) {
    textTextureSize_.y = global_position_.y;
  } else if( verticalAlignment_ == Label::VerticalAlignment::Centered ) {
    textTextureSize_.y = global_position_.y + ( ( global_position_.h - textTextureSize_.h ) / 2.0f );
  } else if( verticalAlignment_ == Label::VerticalAlignment::Bottom ) {
    textTextureSize_.y = ( global_position_.y + global_position_.h ) - textTextureSize_.h;
  }
  // safety, only have texture that's intersecting with the render size
  int w, h;
  if( !SDL_GetCurrentRenderOutputSize( renderer.get(), &w, &h ) )
    logger_->warn( "[{:s}] [{:p}] SDL_GetCurrentRenderOutputSize signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  SDL_FRect screen{ 0, 0, float( w ), float( h ) };
  SDL_FRect result;
  if( !SDL_GetRectIntersectionFloat( &textTextureSize_, &screen, &result ) )
    logger_->warn( "[{:s}] [{:p}] SDL_GetRectIntersectionFloat signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  // copy texture
  if( renderer && textTexture_ ) {
    if( !SDL_RenderTexture( renderer.get(), textTexture_, nullptr, &result ) )
      logger_->warn( "[{:s}] [{:p}] SDL_RenderTexture signalled error: {:s}", __FUNCTION__, static_cast< void* >( this ), SDL_GetError() );
  }

  _base_::OnRender( renderer );
}

std::string Label::GetText() {
  return text_;
}

void Label::SetText( std::string const& value ) {
  text_ = value;
}

Label::HorizontalAlignment Label::GetHorizontalAlignment() {
  return horizontalAlignment_;
}

void Label::SetHorizontalAlignment( Label::HorizontalAlignment value ) {
  horizontalAlignment_ = value;
}

Label::VerticalAlignment Label::GetVerticalAlignment() {
  return verticalAlignment_;
}

void Label::SetVerticalAlignment( Label::VerticalAlignment value ) {
  verticalAlignment_ = value;
}

std::filesystem::path Label::GetFontFile() {
  return fontFile_;
}

void Label::SetFontFile( std::filesystem::path const& value ) {
  fontFile_ = value;

  ReopenFont();
}

int32_t Label::GetFontSize() {
  return fontSize_;
}

void Label::SetFontSize( int32_t value ) {
  fontSize_ = value;

  ReopenFont();
}

SDL_Color Label::GetFontColourActive() {
  return fontColourActive_;
}

void Label::SetFontColourActive( SDL_Color const& value ) {
  fontColourActive_.r = value.r;
  fontColourActive_.g = value.g;
  fontColourActive_.b = value.b;
  fontColourActive_.a = value.a;
}

SDL_Color Label::GetFontColourInactive() {
  return fontColourInactive_;
}

void Label::SetFontColourInactive( SDL_Color const& value ) {
  fontColourInactive_.r = value.r;
  fontColourInactive_.g = value.g;
  fontColourInactive_.b = value.b;
  fontColourInactive_.a = value.a;
}

void Label::ReopenFont() {
  font_ = FontManager::getFont( fontFile_, fontSize_ );
}
