// Header assigned to this source
#include "ui/UiParamMultiplex.hpp"

// Other lib includes
#include <QWindow>

UiParamMultiplex::UiParamMultiplex( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger )  {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );
}

UiParamMultiplex::~UiParamMultiplex() {}
