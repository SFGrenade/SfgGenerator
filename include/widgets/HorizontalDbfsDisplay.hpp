#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_gui.hpp"
#include "widgets/Bar.hpp"
#include "widgets/Label.hpp"
#include "widgets/Widget.hpp"

// C++ std includes
#include <memory>
#include <vector>

class HorizontalDbfsDisplay : public Widget {
  using _base_ = Widget;
  using _base_::_base_;

  public:
  HorizontalDbfsDisplay( std::string const& text, SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~HorizontalDbfsDisplay();

  public:
  void InitUi( std::shared_ptr< Widget > parent ) override;
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  float GetValue();
  void SetValue( float value );
  float GetDbfs();
  void SetDbfs( float dBFS );
  std::string GetUnit();
  void SetUnit( std::string const& value );

  private:
  std::string textDisplay_ = "";
  float dBFS_ = -70.0f;
  std::string unit_ = "dBFS";
  std::shared_ptr< Label > textLabel_ = nullptr;
  std::shared_ptr< Label > valueLabel_ = nullptr;
  std::shared_ptr< Bar > barDisplay_ = nullptr;
};
