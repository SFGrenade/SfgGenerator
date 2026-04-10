#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_gui.hpp"
#include "widgets/Widget.hpp"

// C++ std includes
#include <memory>
#include <vector>

class Slider : public Widget {
  using _base_ = Widget;
  using _base_::_base_;

  public:
  enum class Orientation { Horizontal, Vertical };

  public:
  Slider( Slider::Orientation orientation, std::shared_ptr< spdlog::logger > logger, SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~Slider();

  public:
  void OnLogic() override;
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  Slider::Orientation GetOrientation();
  void SetOrientation( Slider::Orientation value );
  float GetMinValue();
  void SetMinValue( float value );
  float GetMaxValue();
  void SetMaxValue( float value );
  float GetDefaultValue();
  void SetDefaultValue( float value );
  float GetCurrentValue();
  void SetCurrentValue( float value );
  SDL_Color GetKnobColourActive();
  void SetKnobColourActive( SDL_Color const& value );
  SDL_Color GetKnobColourInactive();
  void SetKnobColourInactive( SDL_Color const& value );
  SDL_Color GetBarColourActive();
  void SetBarColourActive( SDL_Color const& value );
  SDL_Color GetBarColourInactive();
  void SetBarColourInactive( SDL_Color const& value );
  void OnValueChanged( std::function< void( float ) > const& callback );

  private:
  Slider::Orientation orientation_ = Slider::Orientation::Horizontal;
  float minValue_ = 0.0f;
  float maxValue_ = 0.0f;
  float defaultValue_ = 0.0f;
  float currentValue_ = 0.0f;
  SDL_Color knobColourActive_ = { 0xff, 0xff, 0xff, 0xff };
  SDL_Color knobColourInactive_ = { 0xff, 0xff, 0xff, 0x80 };
  SDL_Color barColourActive_ = { 0x80, 0x80, 0x80, 0xff };
  SDL_Color barColourInactive_ = { 0x80, 0x80, 0x80, 0x80 };
  SDL_FRect knobRect_;
  SDL_FRect barRect_;
  std::function< void( float ) > valueChangedCallback_ = nullptr;

  private:
  bool isDragging_ = false;
};
