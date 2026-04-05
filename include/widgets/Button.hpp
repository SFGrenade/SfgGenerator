#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_sdl.hpp"
#include "widgets/label.hpp"

// C++ std includes
#include <functional>
#include <memory>
#include <string>

class Button : public Label {
  using _base_ = Label;
  using _base_::_base_;

  public:
  Button( std::function< void() > const& callback, std::string const& text, SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~Button();

  public:
  void OnLogic() override;
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  std::function< void() > GetCallback();
  void SetCallback( std::function< void() > const& value );
  SDL_Color GetBackgroundColourHovering();
  void SetBackgroundColourHovering( SDL_Color const& value );
  SDL_Color GetBackgroundColourPressed();
  void SetBackgroundColourPressed( SDL_Color const& value );
  SDL_Color GetBorderColourActive();
  void SetBorderColourActive( SDL_Color const& value );
  SDL_Color GetBorderColourInactive();
  void SetBorderColourInactive( SDL_Color const& value );

  protected:
  SDL_Color backgroundColourHovering_ = { 0xff, 0xff, 0xff, 0x40 };
  SDL_Color backgroundColourPressed_ = { 0x00, 0x00, 0xff, 0x80 };
  SDL_Color borderColourActive_ = { 0xff, 0xff, 0xff, 0xff };
  SDL_Color borderColourInactive_ = { 0xff, 0xff, 0xff, 0x80 };

  private:
  std::function< void() > callback_;
};
