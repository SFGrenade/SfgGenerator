#pragma once

// Other lib includes
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// C++ std includes
#include <unordered_map>

class InputManager {
  private:
  struct KeyData {
    bool pressed = false;
    bool down = false;
    bool last_down = false;
  };
  struct MouseData {
    struct ButtonData {
      bool pressed = false;
      bool down = false;
      bool last_down = false;
      Uint8 clicks = 0;  // who knows when i might re-/set this
    };
    SDL_FPoint pos{ 0, 0 };
    SDL_FPoint relative_pos{ 0, 0 };
    SDL_FPoint wheel{ 0, 0 };          // x: negative (left) <=> positive (right); y: positive away (up) <=> negative towards (down)
    SDL_FPoint wheel_precise{ 0, 0 };  // x: negative (left) <=> positive (right); y: positive away (up) <=> negative towards (down)
    std::unordered_map< Uint8, InputManager::MouseData::ButtonData > buttons;
  };

  public:
  static void init();

  static void OnLogicUpdate_Early();
  static void OnLogicUpdate_Late();

  static void ProcessKeyEvent( SDL_KeyboardEvent const& event, bool down );
  static void ProcessMouseMoveEvent( SDL_MouseMotionEvent const& event );
  static void ProcessMouseButtonEvent( SDL_MouseButtonEvent const& event, bool down );
  static void ProcessMouseWheelEvent( SDL_MouseWheelEvent const& event );

  static bool IsKeyDown( SDL_Keycode key );
  static bool IsKeyPressed( SDL_Keycode key );

  static bool IsMouseButtonDown( Uint8 button );
  static bool IsMouseButtonPressed( Uint8 button );
  static Uint8 GetMouseButtonClicks( Uint8 button );
  static SDL_FPoint* GetMouse();
  static SDL_FPoint* GetMouseRelative();
  static SDL_FPoint* GetMouseWheel();
  static SDL_FPoint* GetMouseWheelPrecise();

  private:
  static std::unordered_map< SDL_Keycode, InputManager::KeyData > keys_;
  static InputManager::MouseData mouse_;
};
