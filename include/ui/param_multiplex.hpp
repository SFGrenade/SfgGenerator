#pragma once

// Project includes
#include <common/_clap.hpp>
#include <common/_fmt.hpp>

// C++ std includes
#include <cstdint>
#include <memory>
#include <string>

class UiPmHolder {
  // signals that the plugin will emit
  public:
  bool clap_create( std::string const& api, bool is_floating );
  void clap_destroy( void );
  bool clap_set_scale( double scale );
  bool clap_get_size( uint32_t* out_width, uint32_t* out_height );
  bool clap_can_resize( void );
  bool clap_get_resize_hints( clap_gui_resize_hints_t* out_hints );
  bool clap_adjust_size( uint32_t* out_width, uint32_t* out_height );
  bool clap_set_size( uint32_t width, uint32_t height );
  bool clap_set_parent( clap_window_t const* window );
  bool clap_set_transient( clap_window_t const* window );
  void clap_suggest_title( std::string const& title );
  bool clap_show( void );
  bool clap_hide( void );

  // pImpl programming technique
  public:
  // UiPmHolder() = delete;
  UiPmHolder( UiPmHolder& other ) = delete;
  UiPmHolder( UiPmHolder&& other ) = delete;
  explicit UiPmHolder();
  ~UiPmHolder();
  void set_logger( std::shared_ptr< spdlog::logger > logger );

  // pImpl programming technique
  private:
  struct Impl;
  mutable std::unique_ptr< UiPmHolder::Impl > impl_;
  std::shared_ptr< spdlog::logger > logger_;
};
