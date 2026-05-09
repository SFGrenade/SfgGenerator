// Header assigned to this source
#include "widgets/Widget.hpp"

Widget::Widget( std::shared_ptr< spdlog::logger > logger, SDL_FRect position ) : _base_(), logger_( logger ) {
  logger_->trace( "[{:s}] [{:p}] enter( position=({:f}, {:f}, {:f}, {:f}) )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  position.x,
                  position.y,
                  position.w,
                  position.h );
  position_.x = position.x;
  position_.y = position.y;
  position_.w = position.w;
  position_.h = position.h;
}

Widget::~Widget() {
  for( auto child : children_ ) {
    if( child ) {
      child->active_ = false;
    }
  }
  children_.clear();
}

void Widget::InitUi( std::shared_ptr< Widget > parent ) {
  // remove from old parent
  if( parent_ ) {
    for( decltype( parent_->children_ )::iterator iter = parent_->children_.begin(); iter != parent_->children_.end(); ) {
      if( iter->get() == this ) {
        iter = parent_->children_.erase( iter );
      } else {
        iter++;
      }
    }
  }

  parent_ = parent;

  // add to new parent
  if( parent_ ) {
    parent_->children_.push_back( shared_from_this() );
  }
}

void Widget::OnLogic() {
  std::function< void() > resetValuesFunc = [this]() {
    // reset these values
    this->justSwitchedVisible_ = false;
    this->justSwitchedActive_ = false;
  };
  // do logic
  global_position_.x = GetXHierarchy();
  global_position_.y = GetYHierarchy();
  global_position_.w = GetWHierarchy();
  global_position_.h = GetHHierarchy();
  if( !IsVisibleHierarchy() ) {
    resetValuesFunc();
    return;
  }

  if( IsActiveHierarchy() && !( justSwitchedVisible_ || justSwitchedActive_ ) ) {
    cursorHovering_ = SDL_PointInRectFloat( InputManager::GetMouse(), &global_position_ );
    mbPressed_ = cursorHovering_ && InputManager::IsMouseButtonPressed( 1 );
    mbDown_ = cursorHovering_ && InputManager::IsMouseButtonDown( 1 );
  } else {
    cursorHovering_ = false;
    mbPressed_ = false;
    mbDown_ = false;
  }

  // then logic children
  for( auto& child : children_ ) {
    child->OnLogic();
  }
  resetValuesFunc();
}

void Widget::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  if( !IsVisibleHierarchy() ) {
    RenderChildren( renderer );
    return;
  }
  // do rendering
  if( debug_ ) {
    if( IsActiveHierarchy() ) {
      WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0x00, 0x00, 0xff );
    } else {
      WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0x00, 0x00, 0x80 );
    }
    WRAP_SDL_CALL_INST( SDL_RenderRect, renderer.get(), &global_position_ );
  }
  if( frame_ ) {
    if( IsActiveHierarchy() ) {
      WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0xff, 0xff, 0xff );
    } else {
      WRAP_SDL_CALL_INST( SDL_SetRenderDrawColor, renderer.get(), 0xff, 0xff, 0xff, 0x80 );
    }
    WRAP_SDL_CALL_INST( SDL_RenderRect, renderer.get(), &global_position_ );
  }

  // then render children
  RenderChildren( renderer );
}

bool Widget::IsDebug() {
  return debug_;
}

void Widget::SetDebug( bool value ) {
  debug_ = value;
}

bool Widget::IsFrame() {
  return frame_;
}

void Widget::SetFrame( bool value ) {
  frame_ = value;
}

bool Widget::IsVisibleHierarchy() {
  return ( parent_ ? parent_->IsVisibleHierarchy() : true ) && IsVisible();
}

bool Widget::IsVisible() {
  return visible_;
}

void Widget::SetVisible( bool value ) {
  if( value && !visible_ ) {
    SetJustSwitchedVisible();
  }
  visible_ = value;
}

bool Widget::IsActiveHierarchy() {
  return ( parent_ ? parent_->IsActiveHierarchy() : true ) && IsActive();
}

bool Widget::IsActive() {
  return active_;
}

void Widget::SetActive( bool value ) {
  if( value && !active_ ) {
    SetJustSwitchedActive();
  }
  active_ = value;
}

float Widget::GetXHierarchy() {
  // position is relative to parent (all vals 0.0-1.0), unless you're topmost (parent == nullptr)
  if( parent_ == nullptr ) {
    return GetX();
  }
  float parentXOffset = parent_->GetXHierarchy();
  float thisXOffset = GetX() * parent_->GetWHierarchy();
  return parentXOffset + thisXOffset + padding_;
}

float Widget::GetYHierarchy() {
  // position is relative to parent (all vals 0.0-1.0), unless you're topmost (parent == nullptr)
  if( parent_ == nullptr ) {
    return GetY();
  }
  float parentYOffset = parent_->GetYHierarchy();
  float thisYOffset = GetY() * parent_->GetHHierarchy();
  return parentYOffset + thisYOffset + padding_;
}

float Widget::GetWHierarchy() {
  // position is relative to parent (all vals 0.0-1.0), unless you're topmost (parent == nullptr)
  if( parent_ == nullptr ) {
    return GetW();
  }
  return ( parent_->GetWHierarchy() * GetW() ) - ( padding_ * 2.0f );
}

float Widget::GetHHierarchy() {
  // position is relative to parent (all vals 0.0-1.0), unless you're topmost (parent == nullptr)
  if( parent_ == nullptr ) {
    return GetH();
  }
  return ( parent_->GetHHierarchy() * GetH() ) - ( padding_ * 2.0f );
  ;
}

float Widget::GetX() {
  return position_.x;
}

void Widget::SetX( float value ) {
  position_.x = value;
}

float Widget::GetY() {
  return position_.y;
}

void Widget::SetY( float value ) {
  position_.y = value;
}

float Widget::GetW() {
  return position_.w;
}

void Widget::SetW( float value ) {
  position_.w = value;
}

float Widget::GetH() {
  return position_.h;
}

void Widget::SetH( float value ) {
  position_.h = value;
}

float Widget::GetPadding() {
  return padding_;
}

void Widget::SetPadding( float value ) {
  padding_ = value;
}

void Widget::RenderChildren( std::shared_ptr< SDL_Renderer > renderer ) {
  for( auto& child : children_ ) {
    child->OnRender( renderer );
  }
}

void Widget::SetJustSwitchedVisible() {
  justSwitchedVisible_ = true;
  for( auto& child : children_ ) {
    child->SetJustSwitchedVisible();
  }
}

void Widget::SetJustSwitchedActive() {
  justSwitchedActive_ = true;
  for( auto& child : children_ ) {
    child->SetJustSwitchedActive();
  }
}
