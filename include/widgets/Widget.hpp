#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_sdl.hpp"

// C++ std includes
#include <memory>
#include <vector>

class Widget : public std::enable_shared_from_this< Widget > {
  public:
  Widget( SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~Widget();

  public:
  virtual void OnLogic();
  virtual void OnRender( std::shared_ptr< SDL_Renderer > renderer );

  void SetParent( std::shared_ptr< Widget > parent );

  bool IsDebug();
  void SetDebug( bool value );

  bool IsVisibleHierarchy();
  bool IsVisible();
  void SetVisible( bool value );

  bool IsActiveHierarchy();
  bool IsActive();
  void SetActive( bool value );

  float GetXHierarchy();
  float GetYHierarchy();
  float GetWHierarchy();
  float GetHHierarchy();

  float GetX();
  void SetX( float value );
  float GetY();
  void SetY( float value );
  float GetW();
  void SetW( float value );
  float GetH();
  void SetH( float value );
  float GetPadding();
  void SetPadding( float value );

  protected:
  void SetJustSwitchedVisible();
  void SetJustSwitchedActive();

  protected:
  bool cursorHovering_ = false;
  bool mbPressed_ = false;
  bool mbDown_ = false;

  protected:
  // required protected for rendering
  SDL_FRect global_position_;

  private:
  std::shared_ptr< Widget > parent_ = nullptr;
  std::vector< std::shared_ptr< Widget > > children_;

  bool debug_ = false;
  bool visible_ = true;
  bool justSwitchedVisible_ = false;
  bool active_ = true;
  bool justSwitchedActive_ = false;
  SDL_FRect position_;    // this is relative to parent (all vals 0.0-1.0), unless you're topmost (parent == nullptr)
  float padding_ = 0.0f;  // in pixels
};
