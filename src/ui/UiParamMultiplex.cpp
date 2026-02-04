// Header assigned to this source
#include "ui/UiParamMultiplex.hpp"

// Other lib includes
#include <QWindow>

UiParamMultiplex::UiParamMultiplex( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger )  {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );

  mainLabel_ = new QLabel( "Parameter Display", this );
  mainLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  // construct the main layout
  layout_ = new QGridLayout( this );
  layout_->addWidget( mainLabel_, 0, 0, 1, 1 );

  layout_->setRowStretch( 0, 1 );
  layout_->setColumnStretch( 0, 1 );

  setLayout( layout_ );
}

UiParamMultiplex::~UiParamMultiplex() {}
