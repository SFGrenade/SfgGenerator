// Header assigned to this source
#include "common/_gui.hpp"

// C++ std includes
#include <numbers>

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

std::map< int, std::vector< int > > DrawHelper::indicesCache_;

SDL_Point DrawHelper::centerOf( SDL_Rect const& rect ) {
  return { rect.x + rect.w / 2, rect.y + rect.h / 2 };
}

SDL_FPoint DrawHelper::centerOf( SDL_FRect const& rect ) {
  return { rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f };
}

void DrawHelper::drawCircle( std::shared_ptr< SDL_Renderer > renderer, SDL_Color const& colour, SDL_FPoint const& center, float radius, int numSegments ) {
  std::vector< SDL_FPoint > points( numSegments + 1 );
  for( int i = 0; i < points.size(); i++ ) {
    float angle = 2.0f * std::numbers::pi_v< float > * float( i ) / float( numSegments );
    points[i].x = center.x + radius * std::cos( angle );
    points[i].y = center.y + radius * std::sin( angle );
  }
  SDL_SetRenderDrawColor( renderer.get(), colour.r, colour.g, colour.b, colour.a );
  SDL_RenderLines( renderer.get(), points.data(), points.size() );
}

void DrawHelper::drawFillCircle( std::shared_ptr< SDL_Renderer > renderer, SDL_Color const& colour, SDL_FPoint const& center, float radius, int numSegments ) {
  std::vector< SDL_Vertex > points( numSegments );
  for( int i = 0; i < points.size(); i++ ) {
    float angle = 2.0f * std::numbers::pi_v< float > * float( i ) / float( points.size() );
    points[i].position.x = center.x + radius * std::cos( angle );
    points[i].position.y = center.y + radius * std::sin( angle );
    points[i].color.r = float( colour.r ) / float( 0xff );
    points[i].color.g = float( colour.g ) / float( 0xff );
    points[i].color.b = float( colour.b ) / float( 0xff );
    points[i].color.a = float( colour.a ) / float( 0xff );
  }
  decltype( indicesCache_ )::iterator iter = indicesCache_.find( numSegments );
  if( iter == indicesCache_.end() ) {
    std::vector< int > indices;
    for( int i = 1; i <= points.size(); i++ ) {
      indices.push_back( 0 );
      indices.push_back( ( i ) % points.size() );
      indices.push_back( ( i + 1 ) % points.size() );
      if( int( i + 1 ) == int( points.size() - 1 ) ) {
        break;
      }
    }
    auto tmp = indicesCache_.insert( { numSegments, indices } );
    if( tmp.second ) {
      iter = tmp.first;
    }
  }
  SDL_RenderGeometry( renderer.get(), nullptr, points.data(), points.size(), iter->second.data(), iter->second.size() );
}

#if defined( SFG_GEN_IS_LINUX )
// System includes
#include <signal.h>
#include <time.h>

void setParentWindow( std::shared_ptr< SDL_Window > window, clap_window_t const* parent ) {
  // todo: fixme: missing implementation
}
#endif

#if defined( SFG_GEN_IS_MACOS )
// System includes
#include <CoreFoundation/CoreFoundation.h>

void setParentWindow( std::shared_ptr< SDL_Window > window, clap_window_t const* parent ) {
  // todo: fixme: missing implementation
}
#endif

#if defined( SFG_GEN_IS_WINDOWS )
// System includes
#if !defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined( NOMINMAX )
#define NOMINMAX
#endif
#include <Windows.h>

void setParentWindow( std::shared_ptr< SDL_Window > window, clap_window_t const* parent ) {
  SDL_PropertiesID windowProps = SDL_GetWindowProperties( window.get() );
  HWND hwnd = reinterpret_cast< HWND >( SDL_GetPointerProperty( windowProps, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr ) );
  if( hwnd && parent ) {
    SetParent( hwnd, reinterpret_cast< HWND >( parent->win32 ) );
    SetWindowLongPtr( hwnd, GWL_STYLE, WS_CHILD );
  }
}
#endif
