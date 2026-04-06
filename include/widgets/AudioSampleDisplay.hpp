#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/_gui.hpp"
#include "widgets/Widget.hpp"

// Other lib includes
#include <boost/circular_buffer.hpp>

// C++ std includes
#include <memory>
#include <vector>

class AudioSampleDisplay : public Widget {
  using _base_ = Widget;
  using _base_::_base_;

  public:
  AudioSampleDisplay( double sampleRate, SDL_FRect position = { 0, 0, 0, 0 } );
  virtual ~AudioSampleDisplay();

  public:
  void OnRender( std::shared_ptr< SDL_Renderer > renderer ) override;

  void PushSample( float sample );

  private:
  boost::circular_buffer< float > samples_;
  std::vector< SDL_FPoint > points_;
};
