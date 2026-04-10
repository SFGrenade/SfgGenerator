#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_gui.hpp"
#include "widgets/Widget.hpp"

// C++ std includes
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class Label : public Widget {
  using _base_ = Widget;
  using _base_::_base_;

  public:
  enum class HorizontalAlignment { Left, Centered, Right };
  enum class VerticalAlignment { Top, Centered, Bottom };

  public:
  Label( std::string const& text, std::shared_ptr< spdlog::logger > logger, SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~Label();

  public:
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  std::string GetText();
  void SetText( std::string const& value );
  Label::HorizontalAlignment GetHorizontalAlignment();
  void SetHorizontalAlignment( Label::HorizontalAlignment value );
  Label::VerticalAlignment GetVerticalAlignment();
  void SetVerticalAlignment( Label::VerticalAlignment value );
  std::filesystem::path GetFontFile();
  void SetFontFile( std::filesystem::path const& value );
  int32_t GetFontSize();
  void SetFontSize( int32_t value );
  SDL_Color GetFontColourActive();
  void SetFontColourActive( SDL_Color const& value );
  SDL_Color GetFontColourInactive();
  void SetFontColourInactive( SDL_Color const& value );

  protected:
  void ReopenFont();

  protected:
  std::string text_ = "";
  Label::HorizontalAlignment horizontalAlignment_ = Label::HorizontalAlignment::Left;
  Label::VerticalAlignment verticalAlignment_ = Label::VerticalAlignment::Top;
  std::filesystem::path fontFile_ = "Resources/Fonts/NotoSansMono-Regular.ttf";
  int32_t fontSize_ = 18;
  SDL_Color fontColourActive_ = { 0xff, 0xff, 0xff, 0xff };
  SDL_Color fontColourInactive_ = { 0xff, 0xff, 0xff, 0x80 };

  private:
  TTF_Font* font_ = nullptr;
  SDL_Texture* textTexture_ = nullptr;
  SDL_FRect textTextureSize_;
};
