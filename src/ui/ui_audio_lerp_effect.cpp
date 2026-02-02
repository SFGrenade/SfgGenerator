// Header assigned to this source
#include "ui/ui_audio_lerp_effect.hpp"

UiAudioLerpEffect::UiAudioLerpEffect( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger ) {
  this->setGeometry( 0, 0, 300, 200 );
}

UiAudioLerpEffect::~UiAudioLerpEffect() {}
