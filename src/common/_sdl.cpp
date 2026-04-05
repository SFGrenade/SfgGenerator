// Header assigned to this source
#include "common/_sdl.hpp"

std::unordered_map< SDL_Keycode, InputManager::KeyData > InputManager::keys_;
InputManager::MouseData InputManager::mouse_;

void InputManager::init() {}

void InputManager::OnLogicUpdate_Early() {
  for( auto& pair : InputManager::keys_ ) {
    pair.second.pressed = ( pair.second.down ) && ( !pair.second.last_down );
  }
  for( auto& pair : InputManager::mouse_.buttons ) {
    pair.second.pressed = ( pair.second.down ) && ( !pair.second.last_down );
  }
}

void InputManager::OnLogicUpdate_Late() {
  for( auto& pair : InputManager::keys_ ) {
    pair.second.last_down = pair.second.down;
  }
  for( auto& pair : InputManager::mouse_.buttons ) {
    pair.second.last_down = pair.second.down;
    pair.second.clicks = 0;
  }
  InputManager::mouse_.relative_pos.x = 0;
  InputManager::mouse_.relative_pos.y = 0;
  InputManager::mouse_.wheel.x = 0;
  InputManager::mouse_.wheel.y = 0;
  InputManager::mouse_.wheel_precise.x = 0;
  InputManager::mouse_.wheel_precise.y = 0;
}

void InputManager::ProcessKeyEvent( SDL_KeyboardEvent const& event, bool down ) {
  SDL_Keycode event_keysym_sym = static_cast< SDL_Keycode >( event.key );

  if( !InputManager::keys_.contains( event_keysym_sym ) ) {
    InputManager::keys_.emplace( event_keysym_sym, InputManager::KeyData{ false, false, false } );
  }

  auto& entry = InputManager::keys_[event_keysym_sym];
  entry.down = down;
}

void InputManager::ProcessMouseMoveEvent( SDL_MouseMotionEvent const& event ) {
  InputManager::mouse_.pos.x = static_cast< float >( event.x );
  InputManager::mouse_.pos.y = static_cast< float >( event.y );
  InputManager::mouse_.relative_pos.x += static_cast< float >( event.xrel );
  InputManager::mouse_.relative_pos.y += static_cast< float >( event.yrel );
}

void InputManager::ProcessMouseButtonEvent( SDL_MouseButtonEvent const& event, bool down ) {
  if( !InputManager::mouse_.buttons.contains( event.button ) ) {
    InputManager::mouse_.buttons.emplace( event.button, InputManager::MouseData::ButtonData{ false, false, false, 0 } );
  }

  auto& entry = InputManager::mouse_.buttons[event.button];
  entry.down = down;
  entry.clicks += event.clicks;
  // idk if x and y would be of interest here
}

void InputManager::ProcessMouseWheelEvent( SDL_MouseWheelEvent const& event ) {
  if( event.direction == SDL_MOUSEWHEEL_NORMAL ) {
    // direction is normal
    InputManager::mouse_.wheel.x += event.integer_x;
    InputManager::mouse_.wheel.y += event.integer_y;
    InputManager::mouse_.wheel_precise.x += event.x;
    InputManager::mouse_.wheel_precise.y += event.y;
  } else if( event.direction == SDL_MOUSEWHEEL_FLIPPED ) {
    // direction is flipped
    InputManager::mouse_.wheel.x += event.integer_x * ( -1 );
    InputManager::mouse_.wheel.y += event.integer_y * ( -1 );
    InputManager::mouse_.wheel_precise.x += event.x * ( -1 );
    InputManager::mouse_.wheel_precise.y += event.y * ( -1 );
  }
  // idk if mouseX and mouseY would be of interest here
}

bool InputManager::IsKeyDown( SDL_Keycode key ) {
  if( !InputManager::keys_.contains( key ) ) {
    return false;
  }

  auto& entry = InputManager::keys_[key];
  return entry.down;
}

bool InputManager::IsKeyPressed( SDL_Keycode key ) {
  if( !InputManager::keys_.contains( key ) ) {
    return false;
  }

  auto& entry = InputManager::keys_[key];

  return entry.pressed;
}

bool InputManager::IsMouseButtonDown( Uint8 button ) {
  if( !InputManager::mouse_.buttons.contains( button ) ) {
    return false;
  }

  auto& entry = InputManager::mouse_.buttons[button];

  return entry.down;
}

bool InputManager::IsMouseButtonPressed( Uint8 button ) {
  if( !InputManager::mouse_.buttons.contains( button ) ) {
    return false;
  }

  auto& entry = InputManager::mouse_.buttons[button];

  return entry.pressed;
}

Uint8 InputManager::GetMouseButtonClicks( Uint8 button ) {
  if( !InputManager::mouse_.buttons.contains( button ) ) {
    return 0;
  }

  auto& entry = InputManager::mouse_.buttons[button];

  return entry.clicks;
}

SDL_FPoint* InputManager::GetMouse() {
  return &InputManager::mouse_.pos;
}

SDL_FPoint* InputManager::GetMouseRelative() {
  return &InputManager::mouse_.relative_pos;
}

SDL_FPoint* InputManager::GetMouseWheel() {
  return &InputManager::mouse_.wheel;
}

SDL_FPoint* InputManager::GetMouseWheelPrecise() {
  return &InputManager::mouse_.wheel_precise;
}
