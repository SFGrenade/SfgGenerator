// Header assigned to this source
#include "ui/ui_param_multiplex.hpp"

UiParamMultiplex::UiParamMultiplex( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger )  {
  this->setGeometry( 0, 0, 300, 200 );
}

UiParamMultiplex::~UiParamMultiplex() {}
