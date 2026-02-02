// Header assigned to this source
#include "ui/ui_noise_generator.hpp"

UiNoiseGenerator::UiNoiseGenerator( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger ) {
  this->setGeometry( 0, 0, 300, 200 );
}

UiNoiseGenerator::~UiNoiseGenerator() {}
