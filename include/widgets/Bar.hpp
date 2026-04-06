#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_gui.hpp"
#include "widgets/Widget.hpp"

// C++ std includes
#include <memory>
#include <vector>

class Bar : public Widget {
  using _base_ = Widget;
  using _base_::_base_;

  public:
  Bar( SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~Bar();

  public:
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  float GetValue();
  void SetValue( float value );
  SDL_Color GetColourActive();
  void SetColourActive( SDL_Color const& value );
  SDL_Color GetColourInactive();
  void SetColourInactive( SDL_Color const& value );

  private:
  float value_ = 0.0f;
  SDL_Color colourActive_ = { 0xff, 0xff, 0xff, 0xff };
  SDL_Color colourInactive_ = { 0xff, 0xff, 0xff, 0x80 };
};
