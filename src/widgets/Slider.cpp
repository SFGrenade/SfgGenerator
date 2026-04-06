// Header assigned to this source
#include "widgets/Slider.hpp"

Slider::Slider( Slider::Orientation orientation, SDL_FRect position ) : _base_( position ), orientation_( orientation ) {}

Slider::~Slider() {}

void Slider::OnLogic() {
  _base_::OnLogic();
  if( !IsVisibleHierarchy() ) {
    return;
  }
  if( !IsActiveHierarchy() ) {
    return;
  }

  if( !isDragging_ && ( InputManager::GetMouseButtonClicks( 1 ) >= 2 ) ) {
    // user double-clicked (or more clicks, idc tbh)
    currentValue_ = defaultValue_;
    if( valueChangedCallback_ ) {
      valueChangedCallback_( currentValue_ );
    }
    return;
  }

  if( isDragging_ ) {
    float offset = 0.0f;
    SDL_FPoint* mousePos = InputManager::GetMouse();
    if( orientation_ == Slider::Orientation::Horizontal ) {
      offset = ( mousePos->x - barRect_.x ) / ( barRect_.w );
    } else if( orientation_ == Slider::Orientation::Vertical ) {
      offset = ( mousePos->y - barRect_.y ) / ( barRect_.h );
    }
    offset = std::clamp( offset, 0.0f, 1.0f );
    currentValue_ = minValue_ + ( ( maxValue_ - minValue_ ) * offset );
    if( valueChangedCallback_ ) {
      valueChangedCallback_( currentValue_ );
    }
  }

  if( !isDragging_ && mbPressed_ ) {
    // check if user is pressing on the knob
    isDragging_ = SDL_PointInRectFloat( InputManager::GetMouse(), &knobRect_ );
  } else if( isDragging_ && !InputManager::IsMouseButtonDown( 1 ) ) {
    // user let go of left mouse button, end dragging
    isDragging_ = false;
  }
}

void Slider::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  _base_::OnRender( renderer );
  if( !IsVisibleHierarchy() ) {
    return;
  }

  knobRect_.w = std::min( 20.0f, std::min( global_position_.w, global_position_.h ) );
  knobRect_.h = knobRect_.w;
  if( orientation_ == Slider::Orientation::Horizontal ) {
    knobRect_.x = global_position_.x + ( ( global_position_.w - knobRect_.w ) * ( ( currentValue_ - minValue_ ) / ( maxValue_ - minValue_ ) ) );
    knobRect_.y = global_position_.y + ( ( global_position_.h - knobRect_.h ) / 2.0f );
  } else if( orientation_ == Slider::Orientation::Vertical ) {
    knobRect_.x = global_position_.x + ( ( global_position_.w - knobRect_.w ) / 2.0f );
    knobRect_.y = global_position_.y + ( ( global_position_.h - knobRect_.h ) * ( ( currentValue_ - minValue_ ) / ( maxValue_ - minValue_ ) ) );
  }

  if( orientation_ == Slider::Orientation::Horizontal ) {
    barRect_.h = 2.0f;
    barRect_.w = global_position_.w - knobRect_.w + barRect_.h;
    barRect_.x = global_position_.x + ( knobRect_.w / 2.0f ) - ( barRect_.h / 2.0f );
    barRect_.y = global_position_.y + ( ( global_position_.h - barRect_.h ) / 2.0f );
  } else if( orientation_ == Slider::Orientation::Vertical ) {
    barRect_.w = 2.0f;
    barRect_.h = global_position_.h - knobRect_.h + barRect_.w;
    barRect_.y = global_position_.y + ( knobRect_.h / 2.0f ) - ( barRect_.w / 2.0f );
    barRect_.x = global_position_.x + ( ( global_position_.w - barRect_.w ) / 2.0f );
  }

  if( IsActiveHierarchy() ) {
    SDL_SetRenderDrawColor( renderer.get(), barColourActive_.r, barColourActive_.g, barColourActive_.b, barColourActive_.a );
  } else {
    SDL_SetRenderDrawColor( renderer.get(), barColourInactive_.r, barColourInactive_.g, barColourInactive_.b, barColourInactive_.a );
  }
  SDL_RenderFillRect( renderer.get(), &barRect_ );

  DrawHelper::drawFillCircle( renderer,
                              IsActiveHierarchy() ? knobColourActive_ : knobColourInactive_,
                              DrawHelper::centerOf( knobRect_ ),
                              knobRect_.w / 2.0f,
                              8 );
}

Slider::Orientation Slider::GetOrientation() {
  return orientation_;
}

void Slider::SetOrientation( Slider::Orientation value ) {
  orientation_ = value;
}

float Slider::GetMinValue() {
  return minValue_;
}

void Slider::SetMinValue( float value ) {
  minValue_ = value;
  defaultValue_ = std::clamp( defaultValue_, minValue_, maxValue_ );
  currentValue_ = std::clamp( currentValue_, minValue_, maxValue_ );
}

float Slider::GetMaxValue() {
  return maxValue_;
}

void Slider::SetMaxValue( float value ) {
  maxValue_ = value;
  defaultValue_ = std::clamp( defaultValue_, minValue_, maxValue_ );
  currentValue_ = std::clamp( currentValue_, minValue_, maxValue_ );
}

float Slider::GetDefaultValue() {
  return defaultValue_;
}

void Slider::SetDefaultValue( float value ) {
  defaultValue_ = std::clamp( value, minValue_, maxValue_ );
}

float Slider::GetCurrentValue() {
  return currentValue_;
}

void Slider::SetCurrentValue( float value ) {
  currentValue_ = std::clamp( value, minValue_, maxValue_ );
}

SDL_Color Slider::GetKnobColourActive() {
  return knobColourActive_;
}

void Slider::SetKnobColourActive( SDL_Color const& value ) {
  knobColourActive_.r = value.r;
  knobColourActive_.g = value.g;
  knobColourActive_.b = value.b;
  knobColourActive_.a = value.a;
}

SDL_Color Slider::GetKnobColourInactive() {
  return knobColourInactive_;
}

void Slider::SetKnobColourInactive( SDL_Color const& value ) {
  knobColourInactive_.r = value.r;
  knobColourInactive_.g = value.g;
  knobColourInactive_.b = value.b;
  knobColourInactive_.a = value.a;
}

SDL_Color Slider::GetBarColourActive() {
  return barColourActive_;
}

void Slider::SetBarColourActive( SDL_Color const& value ) {
  barColourActive_.r = value.r;
  barColourActive_.g = value.g;
  barColourActive_.b = value.b;
  barColourActive_.a = value.a;
}

SDL_Color Slider::GetBarColourInactive() {
  return barColourInactive_;
}

void Slider::SetBarColourInactive( SDL_Color const& value ) {
  barColourInactive_.r = value.r;
  barColourInactive_.g = value.g;
  barColourInactive_.b = value.b;
  barColourInactive_.a = value.a;
}

void Slider::OnValueChanged( std::function< void( float ) > const& callback ) {
  valueChangedCallback_ = callback;
}
