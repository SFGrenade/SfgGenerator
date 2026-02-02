// Header assigned to this source
#include "ui/ui_noise_generator.hpp"

// Other lib includes
#include <QWindow>

UiNoiseGenerator::UiNoiseGenerator( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger ) {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );
}

UiNoiseGenerator::~UiNoiseGenerator() {}
